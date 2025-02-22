/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "FileItem.h"
#include "AdvancedSettings.h"
#include "Picture.h"
#include "VideoInfoTag.h"
#include "Util.h"
#include "FileSystem/StackDirectory.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "SpecialProtocol.h"

#include "DVDClock.h"
#include "DVDFileInfo.h"
#include "DVDStreamInfo.h"
#include "DVDInputStreams/DVDInputStream.h"
#include "DVDInputStreams/DVDFactoryInputStream.h"
#include "DVDDemuxers/DVDDemux.h"
#include "DVDDemuxers/DVDDemuxUtils.h"
#include "DVDDemuxers/DVDFactoryDemuxer.h"
#include "DVDDemuxers/DVDDemuxFFmpeg.h"
#include "DVDCodecs/DVDCodecs.h"
#include "DVDCodecs/DVDFactoryCodec.h"
#include "DVDCodecs/Video/DVDVideoCodec.h"
#include "DVDCodecs/Video/DVDVideoCodecFFmpeg.h"

#include "FileSystem/File.h"

#include "../ffmpeg/DllAvFormat.h"
#include "../ffmpeg/DllAvCodec.h"
#include "../ffmpeg/DllSwScale.h"


bool CDVDFileInfo::GetFileDuration(const CStdString &path, int& duration)
{
  std::auto_ptr<CDVDInputStream> input;
  std::auto_ptr<CDVDDemux> demux;

  input.reset(CDVDFactoryInputStream::CreateInputStream(NULL, path, ""));
  if (!input.get())
    return false;

  if (!input->Open(path, ""))
    return false;

  demux.reset(CDVDFactoryDemuxer::CreateDemuxer(input.get()));
  if (!demux.get())
    return false;

  duration = demux->GetStreamLength();
  if (duration > 0)
    return true;
  else
    return false;
}

bool CDVDFileInfo::ExtractThumb(const CStdString &strPath, const CStdString &strTarget, CStreamDetails *pStreamDetails)
{
  int nFrameNum = 50;

  CStdString strExt = CUtil::GetExtension(strPath);
  strExt.ToLower();
  if ( strExt == ".vob") // some vobs crash us especially now when not decrypting dvds
    return false;
  
  if( strExt == ".m2ts")
    nFrameNum = 200;

  CStdString strFile;
  if (CUtil::IsStack(strPath))
    strFile = DIRECTORY::CStackDirectory::GetFirstStackedFile(strPath);
  else
    strFile = strPath;

  int nTime = CTimeUtils::GetTimeMS();
  CDVDInputStream *pInputStream = CDVDFactoryInputStream::CreateInputStream(NULL, strFile, "");
  if (!pInputStream)
  {
    CLog::Log(LOGERROR, "InputStream: Error creating stream for %s", strFile.c_str());
    return false;
  }

  if (pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD))
  {
    CLog::Log(LOGERROR, "InputStream: dvd streams not supported for thumb extraction, file: %s", strFile.c_str());
    delete pInputStream;
    return false;
  }

  if (!pInputStream->Open(strFile.c_str(), ""))
  {
    CLog::Log(LOGERROR, "InputStream: Error opening, %s", strFile.c_str());
    if (pInputStream)
      delete pInputStream;
    return false;
  }

  CDVDDemuxFFmpeg *pDemuxer = new CDVDDemuxFFmpeg();

  try
  {
    if(!pDemuxer->Open(pInputStream))
    {
      delete pInputStream;
      CLog::Log(LOGERROR, "%s - Error creating demuxer", __FUNCTION__);
      return false;
    }
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - Exception thrown when opening demuxer", __FUNCTION__);
    if (pDemuxer)
    delete pDemuxer;
    delete pInputStream;
    return false;
  }

  if (pStreamDetails)
    DemuxerToStreamDetails(pDemuxer, *pStreamDetails);

  CDemuxStream* pStream = NULL;
  int nVideoStream = -1;
  for (int i = 0; i < pDemuxer->GetNrOfStreams(); i++)
  {
    pStream = pDemuxer->GetStream(i);
    if (pStream)
    {
      if(pStream->type == STREAM_VIDEO)
      nVideoStream = i;
      else
        pStream->SetDiscard(AVDISCARD_ALL);
    }
  }

  bool bOk = false;
  if (nVideoStream != -1)
  {
    CDVDVideoCodec *pVideoCodec;
    
    CDVDStreamInfo hint(*pDemuxer->GetStream(nVideoStream), true);
    hint.software = true;
    
    if (hint.codec == CODEC_ID_MPEG2VIDEO || hint.codec == CODEC_ID_MPEG1VIDEO)
    {
      // libmpeg2 is not thread safe so use ffmepg for mpeg2/mpeg1 thumb extraction 
      CDVDCodecOptions dvdOptions;
      pVideoCodec = CDVDFactoryCodec::OpenCodec(new CDVDVideoCodecFFmpeg(), hint, dvdOptions);
    }
    // disable thumbs for rm files
#ifdef HAS_EMBEDDED
    else if(hint.codec == CODEC_ID_RV10 || hint.codec == CODEC_ID_RV20 || hint.codec == CODEC_ID_RV30 || hint.codec == CODEC_ID_RV40)
    {
      pVideoCodec = NULL;
    }
#endif
    else
    {
      pVideoCodec = CDVDFactoryCodec::CreateVideoCodec( hint );
    }

    if (pVideoCodec)
    {
      int nTotalLen = pDemuxer->GetStreamLength();
      int nSeekTo = nTotalLen / 3;

      CLog::Log(LOGDEBUG,"%s - seeking to pos %dms (total: %dms) in %s", __FUNCTION__, nSeekTo, nTotalLen, strFile.c_str());
      if (pDemuxer->SeekTime(nSeekTo, true))
      {
        DemuxPacket* pPacket = NULL;
        int iDecoderState = VC_ERROR;
        DVDVideoPicture picture;
  
        int abort_index = pDemuxer->GetNrOfStreams() * nFrameNum;
        bool bHasFrame = false;
          do
          {
            pPacket = pDemuxer->Read();
          if (!pPacket)
            break;
          
          if (pPacket->iStreamId != nVideoStream)
            {
            CDVDDemuxUtils::FreeDemuxPacket(pPacket);
            continue;
          }
          
              iDecoderState = pVideoCodec->Decode(pPacket->pData, pPacket->iSize, pPacket->pts, pPacket->dts);
              CDVDDemuxUtils::FreeDemuxPacket(pPacket);
          
              if (iDecoderState & VC_PICTURE)
          {
            memset(&picture, 0, sizeof(DVDVideoPicture));
            if (pVideoCodec->GetPicture(&picture))
            {
              bHasFrame = true;              
              if(!(picture.iFlags & DVP_FLAG_DROPPED) && picture.iFrameType == FRAME_TYPE_I)
                break;
            }
            }
          
        } while (abort_index--);

        if (bHasFrame)
            {
                int nWidth = g_advancedSettings.m_thumbSize;
                double aspect = (double)picture.iWidth / (double)picture.iHeight;
                int nHeight = (int)((double)g_advancedSettings.m_thumbSize / aspect);

                DllSwScale dllSwScale;
                dllSwScale.Load();

            BYTE *pOutBuf = new BYTE[nWidth * nHeight * 4];
                struct SwsContext *context = dllSwScale.sws_getContext(picture.iWidth, picture.iHeight, 
                      PIX_FMT_YUV420P, nWidth, nHeight, PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
            uint8_t *src[] = { picture.data[0], picture.data[1], picture.data[2], 0 };
            int     srcStride[] = { picture.iLineSize[0], picture.iLineSize[1], picture.iLineSize[2], 0 };
            uint8_t *dst[] = { pOutBuf, 0, 0, 0 };
            int     dstStride[] = { nWidth*4, 0, 0, 0 };

                if (context)
                {
                  dllSwScale.sws_scale(context, src, srcStride, 0, picture.iHeight, dst, dstStride);  
                  dllSwScale.sws_freeContext(context);

                  CPicture::CreateThumbnailFromSurface(pOutBuf, nWidth, nHeight, nWidth * 4, strTarget);
                  bOk = true; 
                }

                dllSwScale.Unload();                
                delete [] pOutBuf;
              }
              else 
              {
          CLog::Log(LOGDEBUG,"%s - decode failed in %s", __FUNCTION__, strFile.c_str());
          }
        }
      delete pVideoCodec;
    }
  }

  if (pDemuxer)
    delete pDemuxer;

  delete pInputStream;

  if(!bOk)
    CPicture::CreateThumbnail("special://xbmc/skin/boxee/media/defaultvideothumb.png", strTarget, true);
  
  int nTotalTime = CTimeUtils::GetTimeMS() - nTime;
  CLog::Log(LOGDEBUG,"%s - measured %d ms to extract thumb from file <%s> ", __FUNCTION__, nTotalTime, strFile.c_str());
  return bOk;
}

void CDVDFileInfo::GetFileMetaData(const CStdString &strPath, CFileItem *pItem)
{
  if (!pItem)
    return;

  CDVDInputStream *pInputStream = CDVDFactoryInputStream::CreateInputStream(NULL, strPath, "");
  if (!pInputStream)
  {
    CLog::Log(LOGERROR, "%s - Error creating stream for %s", __FUNCTION__, strPath.c_str());
    return ;
  }

  if (pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) || !pInputStream->Open(strPath.c_str(), ""))
  {
    CLog::Log(LOGERROR, "%s - invalid stream in %s", __FUNCTION__, strPath.c_str());
    delete pInputStream;
    return ;
  }

  CDVDDemuxFFmpeg *pDemuxer = new CDVDDemuxFFmpeg;

  try
  {
    if (!pDemuxer->Open(pInputStream))
    {
      CLog::Log(LOGERROR, "%s - Error opening demuxer", __FUNCTION__);
      delete pDemuxer;
      delete pInputStream;
      return ;
    }
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - Exception thrown when opening demuxer", __FUNCTION__);
    if (pDemuxer)
      delete pDemuxer;
    delete pInputStream;
    return ;
  }

  AVFormatContext *pContext = pDemuxer->m_pFormatContext; 
  if (pContext)
  {
    int nLenMsec = pDemuxer->GetStreamLength();
    CStdString strDuration;
    int nHours = nLenMsec / 1000 / 60 / 60;
    int nMinutes = ((nLenMsec / 1000) - nHours * 3600) / 60;
    int nSec = (nLenMsec / 1000)  - nHours * 3600 - nMinutes * 60;
    strDuration.Format("%d", nLenMsec);
    pItem->SetProperty("duration-msec", strDuration);
    strDuration.Format("%02d:%02d:%02d", nHours, nMinutes, nSec);
    pItem->SetProperty("duration-str", strDuration);
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52,83,0)
    const char * tags[] = {"title", "author", "copyright", "comment",
                           "album", "year", "track", "genre" };

    for (unsigned int i=0; i<sizeof(tags)/sizeof(tags[0]); i++)
    {
      AVMetadataTag *tag = pDemuxer->AvGetMetaData(pContext->metadata, tags[i], NULL, 0);

      if (tag)
        pItem->SetProperty(tags[i], tag->value);
    }
#else
    pItem->SetProperty("title", pContext->title);
    pItem->SetProperty("author", pContext->author);
    pItem->SetProperty("copyright", pContext->copyright);
    pItem->SetProperty("comment", pContext->comment);
    pItem->SetProperty("album", pContext->album);
    strDuration.Format("%d", pContext->year);
    pItem->SetProperty("year", strDuration);
    strDuration.Format("%d", pContext->track);
    pItem->SetProperty("track", strDuration);
    pItem->SetProperty("genre", pContext->genre);
#endif
        }

  delete pDemuxer;
  pInputStream->Close();
  delete pInputStream;

}

/**
 * \brief Open the item pointed to by pItem and extact streamdetails
 * \return true if the stream details have changed
 */
bool CDVDFileInfo::GetFileStreamDetails(CFileItem *pItem)
{
  if (!pItem)
    return false;

  CStdString strFileNameAndPath;
  if (pItem->HasVideoInfoTag())
  {
    strFileNameAndPath = pItem->GetVideoInfoTag()->m_strFileNameAndPath;
    if (CUtil::IsStack(strFileNameAndPath))
      strFileNameAndPath = DIRECTORY::CStackDirectory::GetFirstStackedFile(strFileNameAndPath);
  }
  else
    return false;

  CDVDInputStream *pInputStream = CDVDFactoryInputStream::CreateInputStream(NULL, strFileNameAndPath, "");
  if (!pInputStream)
    return false;

  if (pInputStream->IsStreamType(DVDSTREAM_TYPE_DVD) || !pInputStream->Open(strFileNameAndPath.c_str(), ""))
  {
    delete pInputStream;
    return false;
  }

  CDVDDemux *pDemuxer = CDVDFactoryDemuxer::CreateDemuxer(pInputStream);
  if (pDemuxer)
  {
    bool retVal = DemuxerToStreamDetails(pDemuxer, pItem->GetVideoInfoTag()->m_streamDetails);
    delete pDemuxer;
    delete pInputStream;
    return retVal;
  }
  else
  {
    delete pInputStream;
    return false;
  }
}

/* returns true if details have been added */
bool CDVDFileInfo::DemuxerToStreamDetails(CDVDDemux *pDemux, CStreamDetails &details)
{
  bool retVal = false;
  details.Reset();

  for (int iStream=0; iStream<pDemux->GetNrOfStreams(); iStream++)
  {
    CDemuxStream *stream = pDemux->GetStream(iStream);
    if (stream->type == STREAM_VIDEO)
    {
      CStreamDetailVideo *p = new CStreamDetailVideo();
      p->m_iWidth = ((CDemuxStreamVideo *)stream)->iWidth;
      p->m_iHeight = ((CDemuxStreamVideo *)stream)->iHeight;
      p->m_fAspect = ((CDemuxStreamVideo *)stream)->fAspect;
      if (p->m_fAspect == 0.0f)
        p->m_fAspect = (float)p->m_iWidth / p->m_iHeight;
      pDemux->GetStreamCodecName(iStream, p->m_strCodec);
      details.AddStream(p);
      retVal = true;
    }

    else if (stream->type == STREAM_AUDIO)
    {
      CStreamDetailAudio *p = new CStreamDetailAudio();
      p->m_iChannels = ((CDemuxStreamAudio *)stream)->iChannels;
      if (stream->language)
        p->m_strLanguage = stream->language;
      pDemux->GetStreamCodecName(iStream, p->m_strCodec);
      details.AddStream(p);
      retVal = true;
    }

    else if (stream->type == STREAM_SUBTITLE)
    {
      if (stream->language)
      {
        CStreamDetailSubtitle *p = new CStreamDetailSubtitle();
        p->m_strLanguage = stream->language;
        details.AddStream(p);
        retVal = true;
      }
    }
  }  /* for iStream */

  details.DetermineBestStreams();
  return retVal;
}


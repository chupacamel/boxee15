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

#include "CacheStrategy.h"
#ifdef _LINUX
#include "PlatformInclude.h"
#endif
#include "Util.h"
#include "utils/log.h"
#include "utils/SingleLock.h"
#include "SpecialProtocol.h"
#include "CharsetConverter.h"
#include "utils/TimeUtils.h"
#ifdef _WIN32
#include "PlatformDefs.h" //for PRIdS, PRId64
#endif

namespace XFILE {

CCacheStrategy::CCacheStrategy() : m_bEndOfInput(false)
{
}


CCacheStrategy::~CCacheStrategy()
{
}

void CCacheStrategy::EndOfInput() {
	m_bEndOfInput = true;
}

bool CCacheStrategy::IsEndOfInput()
{
  return m_bEndOfInput;
}

void CCacheStrategy::ClearEndOfInput()
{
  m_bEndOfInput = false;
}

CSimpleFileCache::CSimpleFileCache() 
  : m_hCacheFileRead(NULL)
  , m_hCacheFileWrite(NULL)
  , m_hDataAvailEvent(NULL)
  , m_nStartPosition(0)
  , m_nWritePosition(0)
  , m_nReadPosition(0) {
}

CSimpleFileCache::~CSimpleFileCache()
{
	Close();
}

int CSimpleFileCache::Open()
{
	Close();

	m_bEndOfInput = false;

	m_hDataAvailEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  CStdString fileName = CUtil::GetNextFilename(_P("special://temp/filecache%03d.cache"), 999);
	if(fileName.empty())
	{
		CLog::Log(LOGERROR, "%s - Unable to generate a new filename", __FUNCTION__);
		Close();
		return CACHE_RC_ERROR;
	}

#ifdef WIN32
	CStdStringW strWFile;
    g_charsetConverter.utf8ToW(fileName, strWFile, false);
	m_hCacheFileWrite = CreateFileW(strWFile.c_str()
						, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE
						, NULL
						, CREATE_ALWAYS
						, FILE_ATTRIBUTE_NORMAL
						, NULL);
#else

	m_hCacheFileWrite = CreateFile(fileName.c_str()
						, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE
						, NULL
						, CREATE_ALWAYS
						, FILE_ATTRIBUTE_NORMAL
						, NULL);
#endif

	if(m_hCacheFileWrite == INVALID_HANDLE_VALUE)
	{
		CLog::Log(LOGERROR, "%s - failed to create file %s with error code %d", __FUNCTION__, fileName.c_str(), GetLastError());
		Close();
		return CACHE_RC_ERROR;
	}

#ifdef WIN32
    g_charsetConverter.utf8ToW(fileName, strWFile, false);
	m_hCacheFileRead = CreateFileW(strWFile.c_str()
						, GENERIC_READ, FILE_SHARE_WRITE
						, NULL
						, OPEN_EXISTING
						, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE 
						, NULL);
#else
	m_hCacheFileRead = CreateFile(fileName.c_str()
						, GENERIC_READ, FILE_SHARE_WRITE
						, NULL
						, OPEN_EXISTING
						, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE 
						, NULL);
#endif

	if(m_hCacheFileRead == INVALID_HANDLE_VALUE)
	{
		CLog::Log(LOGERROR, "%s - failed to open file %s with error code %d", __FUNCTION__, fileName.c_str(), GetLastError());
		Close();
		return CACHE_RC_ERROR;
	}

	return CACHE_RC_OK;
}

int CSimpleFileCache::Close() 
{
	if (m_hDataAvailEvent)
		CloseHandle(m_hDataAvailEvent);

	m_hDataAvailEvent = NULL;

	if (m_hCacheFileWrite)
		CloseHandle(m_hCacheFileWrite);

	m_hCacheFileWrite = NULL;

	if (m_hCacheFileRead)
		CloseHandle(m_hCacheFileRead);

	m_hCacheFileRead = NULL;

	return CACHE_RC_OK;
}

int CSimpleFileCache::WriteToCache(const char *pBuffer, size_t iSize)
{
	DWORD iWritten=0;
  if (!WriteFile(m_hCacheFileWrite, pBuffer, iSize, &iWritten, NULL))
  {
		CLog::Log(LOGERROR, "%s - failed to write to file. err: %u",
                          __FUNCTION__, GetLastError());
		return CACHE_RC_ERROR;
	}
	
	// when reader waits for data it will wait on the event.
	SetEvent(m_hDataAvailEvent);

  m_nWritePosition += iWritten;
	return iWritten;
}

int64_t CSimpleFileCache::GetAvailableRead()
{
	return m_nWritePosition - m_nReadPosition;
}

int CSimpleFileCache::ReadFromCache(char *pBuffer, size_t iMaxSize)
{
  int64_t iAvailable = GetAvailableRead();
	if ( iAvailable <= 0 ) {
		return m_bEndOfInput?CACHE_RC_EOF : CACHE_RC_WOULD_BLOCK;
	}

	if ((int64_t) iMaxSize > iAvailable)
		iMaxSize = (size_t)iAvailable;

	DWORD iRead = 0;
	if (!ReadFile(m_hCacheFileRead, pBuffer, iMaxSize, &iRead, NULL)) {
    CLog::Log(LOGERROR,"CSimpleFileCache::ReadFromCache - failed to read %"PRIdS" bytes.", iMaxSize);
		return CACHE_RC_ERROR;
	}
  m_nReadPosition += iRead;	
	return iRead;
}

int64_t CSimpleFileCache::WaitForData(unsigned int iMinAvail, unsigned int iMillis)
{
  if( iMillis == 0 || IsEndOfInput() )
    return GetAvailableRead();

  unsigned int timeout = CTimeUtils::GetTimeMS() + iMillis;
  unsigned int time;
  while ( !IsEndOfInput() && (time = CTimeUtils::GetTimeMS()) < timeout )
  {
    int64_t iAvail = GetAvailableRead();
    if (iAvail >= iMinAvail)
      return iAvail;

    // busy look (sleep max 1 sec each round)
    DWORD dwRc = WaitForSingleObject(m_hDataAvailEvent, (timeout - time)>1000?(timeout - time):1000 );
    if (dwRc == WAIT_FAILED || dwRc == WAIT_ABANDONED)
      return CACHE_RC_ERROR;
  }

  if( IsEndOfInput() )
    return GetAvailableRead();

  return CACHE_RC_TIMEOUT;
}

int64_t CSimpleFileCache::Seek(int64_t iFilePosition, int iWhence)
{
  
  CLog::Log(LOGDEBUG,"CSimpleFileCache::Seek, seeking to %"PRId64, iFilePosition);
  
  int64_t iTarget = iFilePosition - m_nStartPosition;
  if (SEEK_END == iWhence) 
  {
    CLog::Log(LOGERROR,"%s, cant seek relative to end", __FUNCTION__);
    return CACHE_RC_ERROR;
  }
  else if (SEEK_CUR == iWhence)
    iTarget = iFilePosition + m_nReadPosition;
  
  if (iTarget < 0)
  {
    CLog::Log(LOGDEBUG,"CSimpleFileCache::Seek, request seek before start of cache.");
    return CACHE_RC_ERROR;
  }
  
  int64_t nDiff = iTarget - m_nWritePosition;
  if ( nDiff > (3 * 1024 * 1024) || (nDiff > 0 && WaitForData((unsigned int)nDiff, 3000) == CACHE_RC_TIMEOUT)  ) {		
    CLog::Log(LOGWARNING,"%s - attempt to seek pass read data (seek to %"PRId64". max: %"PRId64". reset read pointer. (%"PRId64":%d)", __FUNCTION__, iTarget, m_nWritePosition, iFilePosition, iWhence);
    return  CACHE_RC_ERROR;
  }
  
  LARGE_INTEGER pos;
  pos.QuadPart = iTarget;
  
  if(!SetFilePointerEx(m_hCacheFileRead, pos, &pos, FILE_BEGIN))
    return CACHE_RC_ERROR;
  
  m_nReadPosition = iTarget;
  
  return pos.QuadPart;
}

void CSimpleFileCache::Reset(int64_t iSourcePosition)
{
  LARGE_INTEGER pos;
  pos.QuadPart = 0;
  
  SetFilePointerEx(m_hCacheFileWrite, pos, NULL, FILE_BEGIN);
  SetFilePointerEx(m_hCacheFileRead, pos, NULL, FILE_BEGIN);
  m_nStartPosition = iSourcePosition;
  m_nReadPosition = 0;
  m_nWritePosition = 0;
}

void CSimpleFileCache::EndOfInput()
{
  CCacheStrategy::EndOfInput();
  SetEvent(m_hDataAvailEvent); 
}

}

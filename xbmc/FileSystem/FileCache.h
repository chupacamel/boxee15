#pragma once
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

#include "IFile.h"
#include "CacheStrategy.h"
#include "utils/CriticalSection.h"
#include "FileSystem/File.h"
#include "utils/Thread.h"

namespace XFILE
{  

  class CFileCache : public IFile, public CThread
  {
  public:
    CFileCache();
    CFileCache(CCacheStrategy *pCache, bool bDeleteCache=true);
    virtual ~CFileCache();
    
    void SetCacheStrategy(CCacheStrategy *pCache, bool bDeleteCache=true);

    // CThread methods
    virtual void Process();
    virtual void OnExit();
    virtual void StopThread(bool bWait = true);
    
    // IFIle methods
    virtual bool          Open(const CURI& url);
    virtual bool          Attach(IFile *pFile);
    virtual void          Close();
    virtual bool          Exists(const CURI& url);
    virtual int           Stat(const CURI& url, struct __stat64* buffer);
    
    virtual unsigned int  Read(void* lpBuf, int64_t uiBufSize);
    
    virtual int64_t       Seek(int64_t iFilePosition, int iWhence);
    virtual int64_t       GetPosition();
    virtual int64_t       GetLength();
    
    virtual ICacheInterface* GetCache();
    IFile *GetFileImp();

    virtual CStdString GetContent();

    virtual __int64	GetAvailableRead();

  private:
    CCacheStrategy *m_pCache;
    bool      m_bDeleteCache;
    int64_t   m_seekPossible;
    bool      m_bCanSeekBack;
    CFile      m_source;
    CStdString    m_sourcePath;
    CEvent      m_seekEvent;
    CEvent      m_seekEnded;
    int        m_nBytesToBuffer;
    time_t      m_tmLastBuffering;
    int64_t      m_nSeekResult;
    int64_t      m_seekPos;
    int64_t      m_readPos;
    CCriticalSection m_sync;
  };

}

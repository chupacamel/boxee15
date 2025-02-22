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

// FileSmb.cpp: implementation of the CFileSMB class.
//
//////////////////////////////////////////////////////////////////////

#include "system.h"

#if defined(HAS_FILESYSTEM_SMB)
#include "FileSmb.h"
#include "GUIPassword.h"
#include "SMBDirectory.h"
#include "Util.h"
#ifndef _WIN32PC
#include <libsmbclient.h>
#endif
#include "../utils/Network.h"
#include "AdvancedSettings.h"
#include "GUISettings.h"
#include "utils/SingleLock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

#include "config.h"
#include "Application.h"

using namespace XFILE;
using namespace DIRECTORY;

// For embedded systems, use vanilla smbclient
#ifdef HAS_EMBEDDED
#define DEPRECATED_SMBC_INTERFACE
#endif

void xb_smbc_log(const char* msg)
{
  CLog::Log(LOGINFO, "%s%s", "smb: ", msg);
}

void xb_smbc_auth(const char *srv, const char *shr, char *wg, int wglen,
                  char *un, int unlen, char *pw, int pwlen)
{
  return ;
}

smbc_get_cached_srv_fn orig_cache;

SMBCSRV* xb_smbc_cache(SMBCCTX* c, const char* server, const char* share, const char* workgroup, const char* username)
{
  return orig_cache(c, server, share, workgroup, username);
}

CSMB::CSMB()
{
  m_context = NULL;
}

CSMB::~CSMB()
{
  Deinit();
}

void CSMB::Deinit(bool idle /* = false */)
{
  CSingleLock lock(*this);

  /* samba goes loco if deinited while it has some files opened */
  if (m_context)
  {
    try
    {
      smbc_set_context(NULL);

      if( 0 == smbc_free_context(m_context, idle ? 1 : 0) )
      {
        m_context = NULL;
      }
    }
#ifndef _LINUX
    catch(win32_exception e)
    {
      e.writelog(__FUNCTION__);
    }
#else
    catch(...)
    {
      CLog::Log(LOGERROR,"exception on CSMB::Deinit. errno: %d", errno);
    }
#endif
  }
}

void CSMB::Init()
{
  CSingleLock lock(*this);
  if (!m_context)
  {
#ifdef _WIN32
    // set the log function
    set_log_callback(xb_smbc_log);
#endif

#ifdef _LINUX
    // Create ~/.smb/smb.conf
    char smb_conf[MAX_PATH];
    sprintf(smb_conf, "%s/.smb", getenv("HOME"));
    mkdir(smb_conf, 0755);
    sprintf(smb_conf, "%s/.smb/smb.conf", getenv("HOME"));
    FILE* f = fopen(smb_conf, "w");
    if (f != NULL)
    {
      fprintf(f, "[global]\n");
//      fprintf(f, "socket options = SO_RCVBUF=8192 TCP_NODELAY IPTOS_LOWDELAY IPTOS_THROUGHPUT\n");
      fprintf(f, "socket options = TCP_NODELAY IPTOS_LOWDELAY SO_RCVBUF=65536 SO_SNDBUF=65536\n");
      fprintf(f, "local master = no\n");
      fprintf(f, "debug = 0\n");
//      fprintf(f, "log level = 9\n");
      fprintf(f, "lock directory = %s/.smb/\n", getenv("HOME"));
      fprintf(f, "client lanman auth = yes\n");
      fprintf(f, "lanman auth = yes\n");
      // if a wins-server is set, we have to change name resolve order to
      if ( g_guiSettings.GetString("smb.winsserver").length() > 0 && !g_guiSettings.GetString("smb.winsserver").Equals("0.0.0.0") )
      {
        fprintf(f, "  wins server = %s\n", g_guiSettings.GetString("smb.winsserver").c_str());
        fprintf(f, "  name resolve order = bcast wins host\n");
      }
      else
        fprintf(f, "name resolve order = bcast host\n");

      if (g_advancedSettings.m_sambadoscodepage.length() > 0)
        fprintf(f, "dos charset = %s\n", g_advancedSettings.m_sambadoscodepage.c_str());
      else
        fprintf(f, "dos charset = CP850\n");

      fprintf(f, "workgroup = %s\n", g_guiSettings.GetString("smb.workgroup").c_str());
      fclose(f);
    }
#endif

    smbc_init(xb_smbc_auth, 0);

    // setup our context
    m_context = smbc_new_context();
#ifndef DEPRECATED_SMBC_INTERFACE
    smbc_setDebug(m_context, 0);
    smbc_setFunctionAuthData(m_context, xb_smbc_auth);
    orig_cache = smbc_getFunctionGetCachedServer(m_context);
    smbc_setFunctionGetCachedServer(m_context, xb_smbc_cache);
    smbc_setOptionOneSharePerServer(m_context, false);
    smbc_setOptionBrowseMaxLmbCount(m_context, 0);
    smbc_setTimeout(m_context, g_advancedSettings.m_sambaclienttimeout * 1000);
#else
    m_context->user = strdup("Guest");
    m_context->debug = 0;
    m_context->callbacks.auth_fn = xb_smbc_auth;
    orig_cache = m_context->callbacks.get_cached_srv_fn;
    m_context->callbacks.get_cached_srv_fn = xb_smbc_cache;
    m_context->options.one_share_per_server = false;
    m_context->options.browse_max_lmb_count = 3;
    m_context->timeout = g_advancedSettings.m_sambaclienttimeout * 1000;
#endif

    // initialize samba and do some hacking into the settings
    if (smbc_init_context(m_context))
    {
      /* setup old interface to use this context */
      smbc_set_context(m_context);

#ifndef _LINUX
      // if a wins-server is set, we have to change name resolve order to
      if ( g_guiSettings.GetString("smb.winsserver").length() > 0 && !g_guiSettings.GetString("smb.winsserver").Equals("0.0.0.0") )
      {
        lp_do_parameter( -1, "wins server", g_guiSettings.GetString("smb.winsserver").c_str());
        lp_do_parameter( -1, "name resolve order", "bcast wins host");
      }
      else
        lp_do_parameter( -1, "name resolve order", "bcast host");

      if (g_advancedSettings.m_sambadoscodepage.length() > 0)
        lp_do_parameter( -1, "dos charset", g_advancedSettings.m_sambadoscodepage.c_str());
      else
        lp_do_parameter( -1, "dos charset", "CP850");
#endif
    }
    else
    {
      smbc_free_context(m_context, 1);
      m_context = NULL;
    }
  }
#ifdef _LINUX
  m_LastActive = CTimeUtils::GetTimeMS();
#endif
}

void CSMB::Purge()
{
#ifndef _LINUX
  CSingleLock lock(*this);
  smbc_purge();
#endif
}

/*
 * For each new connection samba creates a new session
 * But this is not what we want, we just want to have one session at the time
 * This means that we have to call smbc_purge() if samba created a new session
 * Samba will create a new session when:
 * - connecting to another server
 * - connecting to another share on the same server (share, not a different folder!)
 *
 * We try to avoid lot's of purge commands because it slow samba down.
 */
void CSMB::PurgeEx(const CURI& url)
{
  CSingleLock lock(*this);
  CStdString strShare = url.GetFileName().substr(0, url.GetFileName().Find('/'));

#ifndef _LINUX
  if (m_strLastShare.length() > 0 && (m_strLastShare != strShare || m_strLastHost != url.GetHostName()))
    smbc_purge();
#endif

  m_strLastShare = strShare;
  m_strLastHost = url.GetHostName();
}

CStdString CSMB::URLEncode(const CURI &url)
{
  /* due to smb wanting encoded urls we have to build it manually */

  CStdString flat = "smb://";

  if(url.GetDomain().length() > 0)
  {
    flat += URLEncode(url.GetDomain());
    flat += ";";
  }

  /* samba messes up of password is set but no username is set. don't know why yet */
  /* probably the url parser that goes crazy */
  if(url.GetUserName().length() > 0 /* || url.GetPassWord().length() > 0 */)
  {
    flat += URLEncode(url.GetUserName());
    flat += ":";
    flat += URLEncode(url.GetPassWord());
    flat += "@";
  }
  else if( !url.GetHostName().IsEmpty() && !g_guiSettings.GetString("smb.username").IsEmpty() )
  {
    /* okey this is abit uggly to do this here, as we don't really only url encode */
    /* but it's the simplest place to do so */
    flat += URLEncode(g_guiSettings.GetString("smb.username"));
    flat += ":";
    flat += URLEncode(g_guiSettings.GetString("smb.password"));
    flat += "@";
  }

  flat += URLEncode(url.GetHostName());

  /* okey sadly since a slash is an invalid name we have to tokenize */
  std::vector<CStdString> parts;
  std::vector<CStdString>::iterator it;
  CUtil::Tokenize(url.GetFileName(), parts, "/");
  for( it = parts.begin(); it != parts.end(); it++ )
  {
    flat += "/";
    flat += URLEncode((*it));
  }

  /* okey options should go here, thou current samba doesn't support any */

  return flat;
}

CStdString CSMB::URLEncode(const CStdString &value)
{
  CStdString encoded(value);
  CUtil::URLEncode(encoded);
  return encoded;
}

#ifndef _LINUX
DWORD CSMB::ConvertUnixToNT(int error)
{
  DWORD nt_error;
  if (error == ENODEV || error == ENETUNREACH || error == WSAETIMEDOUT) nt_error = NT_STATUS_INVALID_COMPUTER_NAME;
  else if(error == WSAECONNREFUSED || error == WSAECONNABORTED) nt_error = NT_STATUS_CONNECTION_REFUSED;
  else nt_error = map_nt_error_from_unix(error);

  return nt_error;
}
#endif

#ifdef _LINUX
/* This is called from CApplication::ProcessSlow() and is used to tell if smbclient have been idle for too long */
void CSMB::CheckIfIdle()
{
/* We check if there are open connections. This is done without a lock to not halt the mainthread. It should be thread safe as
   worst case scenario is that m_OpenConnections could read 0 and then changed to 1 if this happens it will enter the if wich will lead to another check, wich is locked.  */
  if (m_OpenConnections == 0)
  { /* I've set the the maxiumum IDLE time to be 1 min and 30 sec. */
    CSingleLock lock(m_connectionCountLock);
    if (m_OpenConnections == 0 /* check again - when locked */ && m_context != NULL && (CTimeUtils::GetTimeMS() - m_LastActive) > 90000)
    {
      CLog::Log(LOGNOTICE, "Samba is idle. Closing the remaining connections");
      smb.Deinit(true);
    }
  }
}

void CSMB::SetActivityTime()
{
  CSingleLock lock(m_connectionCountLock);
  m_LastActive = CTimeUtils::GetTimeMS();
}

/* The following two function is used to keep track on how many Opened files/directories there are.
   This makes the idle timer not count if a movie is paused for example */
void CSMB::AddActiveConnection()
{
  CSingleLock lock(m_connectionCountLock);
  m_OpenConnections++;
}
void CSMB::AddIdleConnection()
{
  CSingleLock lock(m_connectionCountLock);
  m_OpenConnections--;
  /* If we close a file we reset the idle timer so that we don't have any wierd behaviours if a user
     leaves the movie paused for a long while and then press stop */
  m_LastActive = CTimeUtils::GetTimeMS();
}
#endif

CSMB smb;

CFileSMB::CFileSMB()
{
  smb.Init();
  m_fd = -1;
  m_filePos  = 0;
  m_fileSize = 0;
  m_has_error = false;
#ifdef _LINUX
  smb.AddActiveConnection();
#endif
}

CFileSMB::~CFileSMB()
{
  Close();
#ifdef _LINUX
  smb.AddIdleConnection();
#endif
}

int64_t CFileSMB::GetPosition()
{
  if (m_fd == -1) return 0;
  CSingleLock lock(m_connectionLock);
  return m_filePos;
}

int64_t CFileSMB::GetLength()
{
  if (m_fd == -1) return 0;
  return m_fileSize;
}

bool CFileSMB::Open(const CURI& url)
{
  Close();

  // we can't open files like smb://file.f or smb://server/file.f
  // if a file matches the if below return false, it can't exist on a samba share.
  if (!IsValidFile(url.GetFileName()))
  {
      CLog::Log(LOGNOTICE,"FileSmb->Open: Bad URL : '%s'",url.GetFileName().c_str());
      return false;
  }
  m_url = url;

  // opening a file to another computer share will create a new session
  // when opening smb://server xbms will try to find folder.jpg in all shares
  // listed, which will create lot's of open sessions.

  CStdString strFileName;
  m_fd = OpenFile(url, strFileName);

  CLog::Log(LOGDEBUG,"CFileSMB::Open - opened %s, fd=%d",url.GetFileName().c_str(), m_fd);
  if (m_fd == -1)
  {
    // write error to logfile
#ifndef _LINUX
    int nt_error = smb.ConvertUnixToNT(errno);
    CLog::Log(LOGINFO, "FileSmb->Open: Unable to open file : '%s'\nunix_err:'%x' nt_err : '%x' error : '%s'", strFileName.c_str(), errno, nt_error, get_friendly_nt_error_msg(nt_error));
#else
    CLog::Log(LOGINFO, "FileSmb->Open: Unable to open file : '%s'\nunix_err:'%x' error : '%s'", strFileName.c_str(), errno, strerror(errno));
#endif
    return false;
  }

  CSingleLock lock(smb);
#ifndef _LINUX
  struct __stat64 tmpBuffer = {0};
#else
  struct stat tmpBuffer;
#endif
  if (smbc_stat(strFileName, &tmpBuffer) < 0)
  {
    smbc_close(m_fd);
    m_fd = -1;
    return false;
  }

  m_fileSize = tmpBuffer.st_size;

  int64_t ret = smbc_lseek(m_fd, 0, SEEK_SET);
  if ( ret < 0 )
  {
    smbc_close(m_fd);
    m_fd = -1;
    return false;
  }

  m_filePos = 0;

  // We've successfully opened the file!
  return true;
}


/// \brief Checks authentication against SAMBA share. Reads password cache created in CSMBDirectory::OpenDir().
/// \param strAuth The SMB style path
/// \return SMB file descriptor
/*
int CFileSMB::OpenFile(CStdString& strAuth)
{
  int fd = -1;

  CStdString strPath = g_passwordManager.GetSMBAuthFilename(strAuth);

  fd = smbc_open(strPath.c_str(), O_RDONLY, 0);
  // TODO: Run a loop here that prompts for our username/password as appropriate?
  // We have the ability to run a file (eg from a button action) without browsing to
  // the directory first.  In the case of a password protected share that we do
  // not have the authentication information for, the above smbc_open() will have
  // returned negative, and the file will not be opened.  While this is not a particular
  // likely scenario, we might want to implement prompting for the password in this case.
  // The code from SMBDirectory can be used for this.
  if(fd >= 0)
    strAuth = strPath;

  return fd;
}
*/

int CFileSMB::OpenFile(const CURI &url, CStdString& strAuth)
{
  int fd = -1;
  smb.Init();
  /* original auth name */
  strAuth = smb.URLEncode(url);

  CStdString strPath = g_passwordManager.GetSMBAuthFilename(strAuth);

  { CSingleLock lock(smb);
  fd = smbc_open(strPath.c_str(), O_RDONLY, 0);
  }

  // file open failed, try to open the directory to force authentication
#ifndef _LINUX
  if (fd < 0 && smb.ConvertUnixToNT(errno) == NT_STATUS_ACCESS_DENIED)
#else
  if (fd < 0 && errno == EACCES)
#endif
  {
    CURI urlshare(url);

    /* just replace the filename with the sharename */
    urlshare.SetFileName(url.GetShareName());

    CSMBDirectory smbDir;
    smbDir.SetAllowPrompting(false);
    fd = smbDir.Open(urlshare);

    // directory open worked, try opening the file again
    if (fd >= 0)
    {
      CSingleLock lock(smb);
      // close current directory filehandle
      // dont need to purge since its the same server and share
      smbc_closedir(fd);

      // set up new filehandle (as CFileSMB::Open does)
      strPath = g_passwordManager.GetSMBAuthFilename(strPath);
      fd = smbc_open(strPath.c_str(), O_RDONLY, 0);
    }
  }

  if (fd >= 0)
    strAuth = strPath;

  return fd;
}

bool CFileSMB::Exists(const CURI& url)
{
  // if a file matches the if below return false, it can't exist on a samba share.
  if (url.GetFileName().Find('/') < 0 ||
      url.GetFileName().at(0) == '.' ||
      url.GetFileName().Find("/.") >= 0) return false;
  smb.Init();
  CStdString strFileName = smb.URLEncode(url);
  strFileName = g_passwordManager.GetSMBAuthFilename(strFileName);

#ifndef _LINUX
  struct __stat64 info;
#else
  struct stat info;
#endif

  CSingleLock lock(smb);
  int iResult = smbc_stat(strFileName, &info);

  if (iResult < 0) return false;
  return true;
}

int CFileSMB::Stat(struct __stat64* buffer)
{
  if (m_fd == -1)
    return -1;

#ifndef _LINUX
  struct __stat64 tmpBuffer;
#else
  struct stat tmpBuffer;
#endif
  bzero(&tmpBuffer, sizeof(tmpBuffer));

  CSingleLock lock(smb);
  int iResult = smbc_fstat(m_fd, &tmpBuffer);

  buffer->st_dev = tmpBuffer.st_dev;
  buffer->st_ino = tmpBuffer.st_ino;
  buffer->st_mode = tmpBuffer.st_mode;
  buffer->st_nlink = tmpBuffer.st_nlink;
  buffer->st_uid = tmpBuffer.st_uid;
  buffer->st_gid = tmpBuffer.st_gid;
  buffer->st_rdev = tmpBuffer.st_rdev;
  buffer->st_size = tmpBuffer.st_size;
  buffer->st_atime = tmpBuffer.st_atime;
  buffer->st_mtime = tmpBuffer.st_mtime;
  buffer->st_ctime = tmpBuffer.st_ctime;

  return iResult;
}

int CFileSMB::Stat(const CURI& url, struct __stat64* buffer)
{
  smb.Init();
  CStdString strFileName = smb.URLEncode(url);
  strFileName = g_passwordManager.GetSMBAuthFilename(strFileName);

  CSingleLock lock(smb);

#ifndef _LINUX
  struct __stat64 tmpBuffer;
#else
  struct stat tmpBuffer;
#endif
  bzero(&tmpBuffer, sizeof(tmpBuffer));

  int iResult = smbc_stat(strFileName, &tmpBuffer);
  buffer->st_dev = tmpBuffer.st_dev;
  buffer->st_ino = tmpBuffer.st_ino;
  buffer->st_mode = tmpBuffer.st_mode;
  buffer->st_nlink = tmpBuffer.st_nlink;
  buffer->st_uid = tmpBuffer.st_uid;
  buffer->st_gid = tmpBuffer.st_gid;
  buffer->st_rdev = tmpBuffer.st_rdev;
  buffer->st_size = tmpBuffer.st_size;
  buffer->st_atime = tmpBuffer.st_atime;
  buffer->st_mtime = tmpBuffer.st_mtime;
  buffer->st_ctime = tmpBuffer.st_ctime;

  return iResult;
}

unsigned int CFileSMB::Read(void *lpBuf, int64_t uiBufSize)
{
  if (g_application.m_bStop)
    return 0;

  if (m_fd == -1) return 0;
  CSingleLock lock(smb); // Init not called since it has to be "inited" by now
#ifdef _LINUX
  smb.SetActivityTime();
#endif
  /* work around stupid bug in samba */
  /* some samba servers has a bug in it where the */
  /* 17th bit will be ignored in a request of data */
  /* this can lead to a very small return of data */
  /* also worse, a request of exactly 64k will return */
  /* as if eof, client has a workaround for windows */
  /* thou it seems other servers are affected too */
  if( uiBufSize >= 64*1024-2 )
    uiBufSize = 64*1024-2;

  int bytesRead = smbc_read(m_fd, lpBuf, (int)uiBufSize);

  if (bytesRead < 0)
  {
#ifndef _LINUX
    CLog::Log(LOGERROR, "CFileSMB::Read - Error( %s )", get_friendly_nt_error_msg(smb.ConvertUnixToNT(errno)));
#else
    CLog::Log(LOGERROR, "CFileSMB::Read - Error( %s )", strerror(errno));
#endif
    m_has_error = true;
    m_error = "SMB: Server stopped responding.";
    return 0;
  }

  {
    CSingleLock lock(m_connectionLock); // Init not called since it has to be "inited" by now
    m_filePos += bytesRead;
  }

  return (unsigned int)bytesRead;
}


bool CFileSMB::HasFileError(CStdString* err) {
  if (m_has_error) {
    *err = m_error;
  }
  return m_has_error;
}



int64_t CFileSMB::Seek(int64_t iFilePosition, int iWhence)
{
  if (m_fd == -1) return -1;
  if(iWhence == SEEK_POSSIBLE)
    return 1;

  // Short circuit calls to smb since the locking here is painful
  if( iWhence == SEEK_SET && iFilePosition == m_filePos ) // seek to current
    return iFilePosition;
  else if( iWhence == SEEK_CUR && iFilePosition == 0 )    // poor man's ftell
    return iFilePosition;

  CSingleLock lock(smb); // Init not called since it has to be "inited" by now
#ifdef _LINUX
  smb.SetActivityTime();
#endif
  INT64 pos = smbc_lseek(m_fd, iFilePosition, iWhence);

  if ( pos < 0 )
  {
#ifndef _LINUX
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, get_friendly_nt_error_msg(smb.ConvertUnixToNT(errno)));
#else
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, strerror(errno));
#endif
    return -1;
  }

  {
    CSingleLock lock(m_connectionLock); // Init not called since it has to be "inited" by now
    m_filePos = (__int64)pos;
  }

  return (int64_t)pos;
}

void CFileSMB::Close()
{
  if (m_fd != -1)
  {
    CLog::Log(LOGDEBUG,"CFileSMB::Close closing fd %d", m_fd);
    CSingleLock lock(smb);
    smbc_close(m_fd);
  }
  m_fd = -1;
  m_filePos = 0;
  m_fileSize = 0;
}

int CFileSMB::Write(const void* lpBuf, int64_t uiBufSize)
{
  if (m_fd == -1) return -1;
  DWORD dwNumberOfBytesWritten = 0;

  // lpBuf can be safely casted to void* since xmbc_write will only read from it.
  smb.Init();
  CSingleLock lock(smb);
  dwNumberOfBytesWritten = smbc_write(m_fd, (void*)lpBuf, (DWORD)uiBufSize);

  if (dwNumberOfBytesWritten > 0)
  {
    CSingleLock lock(m_connectionLock); // Init not called since it has to be "inited" by now
    m_filePos += dwNumberOfBytesWritten;
  }

  return (int)dwNumberOfBytesWritten;
}

bool CFileSMB::Delete(const CURI& url)
{
  smb.Init();
  CStdString strFile = g_passwordManager.GetSMBAuthFilename(smb.URLEncode(url));

  CSingleLock lock(smb);

  int result = smbc_unlink(strFile.c_str());

  if(result != 0)
#ifndef _LINUX
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, get_friendly_nt_error_msg(smb.ConvertUnixToNT(errno)));
#else
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, strerror(errno));
#endif

  return (result == 0);
}

bool CFileSMB::Rename(const CURI& url, const CURI& urlnew)
{
  smb.Init();
  CStdString strFile = g_passwordManager.GetSMBAuthFilename(smb.URLEncode(url));
  CStdString strFileNew = g_passwordManager.GetSMBAuthFilename(smb.URLEncode(urlnew));

  CSingleLock lock(smb);

  int result = smbc_rename(strFile.c_str(), strFileNew.c_str());

  if(result != 0)
#ifndef _LINUX
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, get_friendly_nt_error_msg(smb.ConvertUnixToNT(errno)));
#else
    CLog::Log(LOGERROR, "%s - Error( %s )", __FUNCTION__, strerror(errno));
#endif

  return (result == 0);
}

bool CFileSMB::OpenForWrite(const CURI& url, bool bOverWrite)
{
  Close();
  smb.Init();
  // we can't open files like smb://file.f or smb://server/file.f
  // if a file matches the if below return false, it can't exist on a samba share.
  if (!IsValidFile(url.GetFileName())) return false;

  CStdString strFileName = smb.URLEncode(url);
  strFileName = g_passwordManager.GetSMBAuthFilename(strFileName);

  CSingleLock lock(smb);

  if (bOverWrite)
  {
    CLog::Log(LOGWARNING, "FileSmb::OpenForWrite() called with overwriting enabled! - %s", strFileName.c_str());
    m_fd = smbc_creat(strFileName.c_str(), 0);
  }
  else
  {
    m_fd = smbc_open(strFileName.c_str(), O_RDWR, 0);
  }

  if (m_fd == -1)
  {
    // write error to logfile
#ifndef _LINUX
    int nt_error = map_nt_error_from_unix(errno);
    CLog::Log(LOGERROR, "FileSmb->Open: Unable to open file : '%s'\nunix_err:'%x' nt_err : '%x' error : '%s'", strFileName.c_str(), errno, nt_error, get_friendly_nt_error_msg(nt_error));
#else
    CLog::Log(LOGERROR, "FileSmb->Open: Unable to open file : '%s'\nunix_err:'%x' error : '%s'", strFileName.c_str(), errno, strerror(errno));
#endif
    return false;
  }

  // We've successfully opened the file!
  return true;
}

bool CFileSMB::IsValidFile(const CStdString& strFileName)
{
  if (strFileName.Find('/') == -1 || /* doesn't have sharename */
      strFileName.Right(2) == "/." || /* not current folder */
      strFileName.Right(3) == "/..")  /* not parent folder */
      return false;
  return true;
}

#endif

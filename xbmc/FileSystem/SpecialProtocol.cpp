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

#include "SpecialProtocol.h"
#include "URL.h"
#include "Util.h"
#include "GUISettings.h"
#include "Settings.h"
#include "utils/log.h"
#include "utils/SingleLock.h"

#ifdef _LINUX
#include <dirent.h>
#endif

using namespace std;

CCriticalSection CSpecialProtocol::m_mapPathsGuard;
std::map<CStdString, CStdString> CSpecialProtocol::m_cachePaths;

map<CStdString, CStdString> CSpecialProtocol::m_pathMap;

void CSpecialProtocol::SetProfilePath(const CStdString &dir)
{
  SetPath("profile", dir);
  CLog::Log(LOGNOTICE, "special://profile/ is mapped to: %s", GetPath("profile").c_str());
}

void CSpecialProtocol::SetXBMCPath(const CStdString &dir)
{
  SetPath("xbmc", dir);
}

void CSpecialProtocol::SetHomePath(const CStdString &dir)
{
  SetPath("home", dir);
}

void CSpecialProtocol::SetUserHomePath(const CStdString &dir)
{
  SetPath("userhome", dir);
}

void CSpecialProtocol::SetMasterProfilePath(const CStdString &dir)
{
  SetPath("masterprofile", dir);
}

void CSpecialProtocol::SetTempPath(const CStdString &dir)
{
  SetPath("temp", dir);
}

#ifdef WIN32
void CSpecialProtocol::SetMyVideosPath(const CStdString &dir)
{
  SetPath("myvideos", dir);
}

void CSpecialProtocol::SetMyMusicPath(const CStdString &dir)
{
  SetPath("mymusic", dir);
}

void CSpecialProtocol::SetMyPicturesPath(const CStdString &dir)
{
  SetPath("mypictures", dir);
}
#endif

bool CSpecialProtocol::XBMCIsHome()
{
  return TranslatePath("special://xbmc") == TranslatePath("special://home");
}

bool CSpecialProtocol::ComparePath(const CStdString &path1, const CStdString &path2)
{
  return TranslatePath(path1) == TranslatePath(path2);
}

CStdString CSpecialProtocol::TranslatePath(const CStdString &path)
{
  CURI url(path);
  
  // check for special-protocol, if not, return
  if (!url.GetProtocol().Equals("special"))
  {
    // So this is not good - this code is doing windows <-> non windows path fixups
    // for many urls in the system, which probably isn't what the function was originally
    // designed for. See the _P() macro which points to this func.

#if defined(_LINUX) && defined(_DEBUG)
    if (path.length() >= 2 && path[1] == ':')
    {
      CLog::Log(LOGWARNING, "Trying to access old style dir: %s\n", path.c_str());
     // printf("Trying to access old style dir: %s\n", path.c_str());
    }
#endif
    
    CStdString fixPath = path;
    
#if defined(_LINUX)
    fixPath.Replace('\\', '/');
//#else
//    // upnp rar and zip slashes should never be reversed
//	if( !url.GetProtocol().Equals("upnp") && !url.GetProtocol().Equals("rar") && !url.GetProtocol().Equals("zip") )
//    {
//    fixPath.Replace('/', '\\');
//    }
#endif
    
    return fixPath;
  }
  
  CStdString FullFileName = url.GetFileName();
 
  CStdString translatedPath;
  CStdString FileName;
  CStdString RootDir;
  
  // Split up into the special://root and the rest of the filename
  int pos = FullFileName.Find('/');
  if (pos != -1 && pos > 1)
  {
    RootDir = FullFileName.Left(pos);
    
    if (pos < FullFileName.GetLength())
      FileName = FullFileName.Mid(pos + 1);
  }
  else
    RootDir = FullFileName;
  
  if (RootDir.Equals("subtitles"))
    CUtil::AddFileToFolder(g_guiSettings.GetString("subtitles.custompath"), FileName, translatedPath);
  else if (RootDir.Equals("userdata"))
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), FileName, translatedPath);
  else if (RootDir.Equals("database"))
    CUtil::AddFileToFolder(g_settings.GetDatabaseFolder(), FileName, translatedPath);
  else if (RootDir.Equals("thumbnails"))
    CUtil::AddFileToFolder(g_settings.GetThumbnailsFolder(), FileName, translatedPath);
  else if (RootDir.Equals("recordings"))
    CUtil::AddFileToFolder(g_guiSettings.GetString("mymusic.recordingpath", false), FileName, translatedPath);
  else if (RootDir.Equals("screenshots"))
    CUtil::AddFileToFolder(g_guiSettings.GetString("pictures.screenshotpath", false), FileName, translatedPath);
  else if (RootDir.Equals("musicplaylists"))
    CUtil::AddFileToFolder(CUtil::MusicPlaylistsLocation(), FileName, translatedPath);
  else if (RootDir.Equals("videoplaylists"))
    CUtil::AddFileToFolder(CUtil::VideoPlaylistsLocation(), FileName, translatedPath);
  else if (RootDir.Equals("cdrips"))
    CUtil::AddFileToFolder(g_guiSettings.GetString("cddaripper.path"), FileName, translatedPath);
  else if (RootDir.Equals("skin"))
    CUtil::AddFileToFolder(g_graphicsContext.GetMediaDir(), FileName, translatedPath);

  // from here on, we have our "real" special paths
  else if (RootDir.Equals("xbmc"))
    CUtil::AddFileToFolder(GetPath("xbmc"), FileName, translatedPath);
  else if (RootDir.Equals("home"))
    CUtil::AddFileToFolder(GetPath("home"), FileName, translatedPath);
  else if (RootDir.Equals("userhome"))
    CUtil::AddFileToFolder(GetPath("userhome"), FileName, translatedPath);  
  else if (RootDir.Equals("temp"))
    CUtil::AddFileToFolder(GetPath("temp"), FileName, translatedPath);
  else if (RootDir.Equals("profile"))
    CUtil::AddFileToFolder(GetPath("profile"), FileName, translatedPath);
  else if (RootDir.Equals("masterprofile"))
    CUtil::AddFileToFolder(GetPath("masterprofile"), FileName, translatedPath);
#ifdef WIN32
  else if (RootDir.Equals("myvideos"))
    CUtil::AddFileToFolder(GetPath("myvideos"), FileName, translatedPath);
  else if (RootDir.Equals("mymusic"))
    CUtil::AddFileToFolder(GetPath("mymusic"), FileName, translatedPath);
  else if (RootDir.Equals("mypictures"))
    CUtil::AddFileToFolder(GetPath("mypictures"), FileName, translatedPath);
#endif

  // check if we need to recurse in
  if (CUtil::IsSpecial(translatedPath))
  { // we need to recurse in, as there may be multiple translations required
    return TranslatePath(translatedPath);
  }

  // Validate the final path, just in case
  return CURI::ValidatePath(translatedPath);
}

CStdString CSpecialProtocol::TranslatePathConvertCase(const CStdString& path)
{
  {
    CSingleLock lock(m_mapPathsGuard);
    std::map<CStdString,CStdString>::iterator iter = m_cachePaths.find(path);
    
    if (iter != m_cachePaths.end())
    {
      CStdString tmp = iter->second;
      return tmp;
      //return iter->second;
    }
  }
  
  CStdString translatedPath = TranslatePath(path);

#ifdef _LINUX
  if ((!CUtil::IsSpecial(translatedPath)) && (translatedPath.Find("://") > 0))
    return translatedPath;

  // If the file exists with the requested name, simply return it
  struct stat stat_buf;
  if (stat(translatedPath.c_str(), &stat_buf) == 0)
  {
    CSingleLock lock(m_mapPathsGuard);
    m_cachePaths[path] = translatedPath;    
    return translatedPath;
  }

  bool needToAddSlashAtEnd = CUtil::HasSlashAtEnd(translatedPath);
  
  CStdString result;
  vector<CStdString> tokens;
  CUtil::Tokenize(translatedPath, tokens, "/");
  CStdString file;
  DIR* dir;
  struct dirent* de;

  for (unsigned int i = 0; i < tokens.size(); i++)
  {
    file = result + "/" + tokens[i];
    if (stat(file.c_str(), &stat_buf) == 0)
    {
      result += "/" + tokens[i];
    }
    else
    {
      dir = opendir(result.c_str());
      if (dir)
      {
        while ((de = readdir(dir)) != NULL)
        {
          // check if there's a file with same name but different case
          if (strcasecmp(de->d_name, tokens[i]) == 0)
          {
            result += "/";
            result += de->d_name;
            break;
          }
        }

        // if we did not find any file that somewhat matches, just
        // fallback but we know it's not gonna be a good ending
        if (de == NULL)
          result += "/" + tokens[i];

        closedir(dir);
      }
      else
      { // this is just fallback, we won't succeed anyway...
        result += "/" + tokens[i];
      }
    }
  }

  {
    CSingleLock lock(m_mapPathsGuard);
    
    if(needToAddSlashAtEnd)
    {
      CUtil::AddSlashAtEnd(result);
    }

    m_cachePaths[path] = result;
  }
  
  return result;
#else
  return translatedPath;
#endif
}

CStdString CSpecialProtocol::ReplaceOldPath(const CStdString &oldPath, int pathVersion)
{
  if (pathVersion < 1)
  {
    if (oldPath.Left(2).CompareNoCase("P:") == 0)
      return CUtil::AddFileToFolder("special://profile/", oldPath.Mid(2));
    else if (oldPath.Left(2).CompareNoCase("Q:") == 0)
      return CUtil::AddFileToFolder("special://xbmc/", oldPath.Mid(2));
    else if (oldPath.Left(2).CompareNoCase("T:") == 0)
      return CUtil::AddFileToFolder("special://masterprofile/", oldPath.Mid(2));
    else if (oldPath.Left(2).CompareNoCase("U:") == 0)
      return CUtil::AddFileToFolder("special://home/", oldPath.Mid(2));
    else if (oldPath.Left(2).CompareNoCase("Z:") == 0)
      return CUtil::AddFileToFolder("special://temp/", oldPath.Mid(2));
  }
  return oldPath;
}

void CSpecialProtocol::LogPaths()
{
  CLog::Log(LOGNOTICE, "special://xbmc/ is mapped to: %s", GetPath("xbmc").c_str());
  CLog::Log(LOGNOTICE, "special://masterprofile/ is mapped to: %s", GetPath("masterprofile").c_str());
  CLog::Log(LOGNOTICE, "special://home/ is mapped to: %s", GetPath("home").c_str());
  CLog::Log(LOGNOTICE, "special://temp/ is mapped to: %s", GetPath("temp").c_str());
  CLog::Log(LOGNOTICE, "special://userhome/ is mapped to: %s", GetPath("userhome").c_str());
}

// private routines, to ensure we only set/get an appropriate path
void CSpecialProtocol::SetPath(const CStdString &key, const CStdString &path)
{
  m_pathMap[key] = path;
}

CStdString CSpecialProtocol::GetPath(const CStdString &key)
{
  map<CStdString, CStdString>::iterator it = m_pathMap.find(key);
  if (it != m_pathMap.end())
    return it->second;
  assert(false);
  return "";
}

void CSpecialProtocol::ClearCacheMap()
{
  CSingleLock lock(m_mapPathsGuard);
  m_cachePaths.clear();
}

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

#include "DynamicDll.h"
#include "SectionLoader.h"
#include "FileSystem/File.h"
#include "utils/log.h"

using namespace XFILE;

DllDynamic::DllDynamic()
{
  m_dll=NULL;
  m_DelayUnload=true;
}

DllDynamic::DllDynamic(const CStdString& strDllName)
{
  m_strDllName=strDllName;
  m_dll=NULL;
  m_DelayUnload=true;
}

DllDynamic::~DllDynamic()
{
  Unload();
}

bool DllDynamic::Load()
{
  if (m_dll)
    return true;

  if (!(m_dll=CSectionLoader::LoadDLL(m_strDllName, m_DelayUnload, LoadSymbols())))
    return false;

  if (!ResolveExports())
  {
    CLog::Log(LOGERROR, "Unable to resolve exports from dll %s", m_strDllName.c_str());
    Unload();
    return false;
  }

  return true;
}

void DllDynamic::Unload()
{
  if(m_dll)
    CSectionLoader::UnloadDLL(m_strDllName);
  m_dll=NULL;
}

bool DllDynamic::CanLoad()
{
  return CFile::Exists(m_strDllName);
}

bool DllDynamic::EnableDelayedUnload(bool bOnOff)
{
  if (m_dll)
    return false;

  m_DelayUnload=bOnOff;

  return true;
}

bool DllDynamic::SetFile(const CStdString& strDllName)
{
  if (m_dll)
    return false;

  m_strDllName=strDllName;
  return true;
}



bool LibStatic::Load()
{
  if (!ResolveExports())
  {
    CLog::Log(LOGERROR, "Unable to resolve exports from lib.");
    Unload();
    return false;
  }

  m_is_loaded = true;
  return true;
}


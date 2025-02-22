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

#include "StdString.h"

#include <vector>

class CCriticalSection;

class CDNSNameCache
{
public:
  class CDNSName
  {
  public:
    CStdString m_strHostName;
    CStdString m_strIpAdres;
  };
  CDNSNameCache(void);
  virtual ~CDNSNameCache(void);
  static bool Lookup(const CStdString& strHostName, CStdString& strIpAdres);
  static void Add(const CStdString& strHostName, const CStdString& strIpAdres);

protected:
  static bool GetCached(const CStdString& strHostName, CStdString& strIpAdres);
  static CCriticalSection m_critical;
  std::vector<CDNSName> m_vecDNSNames;
  typedef std::vector<CDNSName>::iterator ivecDNSNames;
};

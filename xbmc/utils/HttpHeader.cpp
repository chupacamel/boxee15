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

#include "HttpHeader.h"

CHttpHeader::CHttpHeader()
{
  m_params.clear();
}

CHttpHeader::~CHttpHeader()
{
}

void CHttpHeader::Parse(const CStdString &strData)
{
  unsigned int iIter = 0;
  int iValueStart = 0;
  int iValueEnd = 0;
  
  CStdString strParam;
  CStdString strValue;
  
  while (iIter < strData.size())
  {
    iValueStart = strData.Find(":", iIter);
    iValueEnd = strData.Find("\r\n", iIter);
    
    if (iValueEnd < 0) break;
    
    if (iValueStart > 0)
    {
      strParam = strData.substr(iIter, iValueStart);
      strValue = strData.substr(iValueStart + 1, iValueEnd - iValueStart - 1);
      
      /*
      CUtil::Lower(strParam.c_str()
      // trim left and right
      {
        string::size_type pos = strValue.find_last_not_of(' ');
        if(pos != string::npos)
        {
          strValue.erase(pos + 1);
          pos = strValue.find_first_not_of(' ');
          if(pos != string::npos) strValue.erase(0, pos);
        }
        else strValue.erase(strValue.begin(), strValue.end());
      }*/
      strParam.Trim();
      strParam.ToLower();
      
      strValue.Trim();
      
      
      m_params[strParam] = strValue;
    }
    else
    {
      strParam = strData.substr(iIter, iValueEnd);
      strParam.Trim();

      if (strParam.Left(7) == "HTTP/1.")
      {
        m_protoLine = strData;
        m_params.clear();
      }
    }

    iIter = iValueEnd + 2;
  }
}

CStdString CHttpHeader::GetValue(CStdString strParam) const
{
  strParam.ToLower();
  
  HeaderParamsIter pIter = m_params.find(strParam);
  if (pIter != m_params.end()) return pIter->second;
  
  return "";
}

void CHttpHeader::GetHeader(CStdString& strHeader)
{
  strHeader.clear();
  
  HeaderParamsIter iter = m_params.begin();
  while (iter != m_params.end())
  {
    strHeader += ((*iter).first + ": " + (*iter).second + "\n");
  }
  
  strHeader += "\n";
}

void CHttpHeader::Clear()
{
  m_params.clear();
}

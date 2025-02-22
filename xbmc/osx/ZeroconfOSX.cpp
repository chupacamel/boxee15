/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include "ZeroconfOSX.h"

#include <string>
#include <SingleLock.h>
#include <utils/log.h>

CZeroconfOSX::CZeroconfOSX():m_runloop(0)
{
  //aquire the main threads event loop
  EventLoopRef ref = GetMainEventLoop();
  m_runloop = (CFRunLoopRef)GetCFRunLoopFromEventLoop(ref);
}

CZeroconfOSX::~CZeroconfOSX()
{
  doStop();
}

//methods to implement for concrete implementations
bool CZeroconfOSX::doPublishService(const std::string& fcr_identifier,
                      const std::string& fcr_type,
                      const std::string& fcr_name,
                      unsigned int f_port,
                      const std::map<std::string, std::string>& txt)
{
  CLog::Log(LOGDEBUG, "CZeroconfOSX::doPublishService identifier: %s type: %s name:%s port:%i", fcr_identifier.c_str(),
            fcr_type.c_str(), fcr_name.c_str(), f_port);

  CFStringRef name = CFStringCreateWithCString (NULL,
                                                fcr_name.c_str(),
                                                kCFStringEncodingUTF8
                                                );
  CFStringRef type = CFStringCreateWithCString (NULL,
                                                fcr_type.c_str(),
                                                kCFStringEncodingUTF8
                                                );
  CFNetServiceRef netService = CFNetServiceCreate(NULL, CFSTR(""), type, name, f_port);
  CFRelease(name);
  CFRelease(type);

  //now register it
  CFNetServiceClientContext clientContext = { 0, this, NULL, NULL, NULL };

  CFStreamError error;
  CFNetServiceSetClient(netService, registerCallback, &clientContext);
  CFNetServiceScheduleWithRunLoop(netService, m_runloop, kCFRunLoopCommonModes);

  Boolean result = CFNetServiceRegisterWithOptions (netService, 0, &error);
  if (result == false)
  {
    // Something went wrong so lets clean up.
    CFNetServiceUnscheduleFromRunLoop(netService, m_runloop, kCFRunLoopCommonModes);
    CFNetServiceSetClient(netService, NULL, NULL);
    CFRelease(netService);
    netService = NULL;
    CLog::Log(LOGERROR, "CZeroconfOSX::doPublishService CFNetServiceRegister returned (domain = %d, error = %ld)\n", error.domain, error.error);
  } else
  {
    CSingleLock lock(m_data_guard);
    m_services.insert(make_pair(fcr_identifier, netService));
  }

  CFMutableDictionaryRef dict = CFDictionaryCreateMutable(0, 0,
      &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);

  if (txt.size() > 0)
  {
    for (std::map<std::string, std::string>::const_iterator itr = txt.begin(); itr != txt.end(); ++itr)
    {
      CFStringRef cfKey = CFStringCreateWithCString(NULL, (*itr).first.c_str(), kCFStringEncodingMacRoman);
      CFStringRef cfValue = CFStringCreateWithCString(NULL, (*itr).second.c_str(), kCFStringEncodingMacRoman);
      CFDictionaryAddValue(dict, cfKey, cfValue);
    }

    CFDataRef txtData = CFNetServiceCreateTXTDataWithDictionary(NULL, (CFMutableDictionaryRef) dict);
    if (!CFNetServiceSetTXTData(netService, txtData))
    {
      CLog::Log(LOGERROR, "CZeroconfOSX::doPublishService CFNetServiceSetTXTData returned false");
    }
  }

  return result;
}

bool CZeroconfOSX::doRemoveService(const std::string& fcr_ident)
{
  CSingleLock lock(m_data_guard);
  tServiceMap::iterator it = m_services.find(fcr_ident);
  if(it != m_services.end())
  {
    cancelRegistration(it->second);
    m_services.erase(it);
    return true;
  } else
    return false;
}

void CZeroconfOSX::doStop()
{
  CSingleLock lock(m_data_guard);
  for(tServiceMap::iterator it = m_services.begin(); it != m_services.end(); ++it)
    cancelRegistration(it->second);
  m_services.clear();
}


void CZeroconfOSX::registerCallback(CFNetServiceRef theService, CFStreamError* error, void* info)
{
  if (error->domain == kCFStreamErrorDomainNetServices)
  {
    CZeroconfOSX* p_this = reinterpret_cast<CZeroconfOSX*>(info);
    switch(error->error) {
      case kCFNetServicesErrorCollision:
        CLog::Log(LOGERROR, "CZeroconfOSX::registerCallback name collision occured");
        break;
      default:
        CLog::Log(LOGERROR, "CZeroconfOSX::registerCallback returned (domain = %d, error = %ld)\n", error->domain, error->error);
        break;
    }
    p_this->cancelRegistration(theService);
    //remove it
    CSingleLock lock(p_this->m_data_guard);
    for(tServiceMap::iterator it = p_this->m_services.begin(); it != p_this->m_services.end(); ++it)
    {
      if(it->second == theService)
        p_this->m_services.erase(it);
    }
  }
}

void CZeroconfOSX::cancelRegistration(CFNetServiceRef theService)
{
  assert(theService != NULL);
  CFNetServiceUnscheduleFromRunLoop(theService, m_runloop, kCFRunLoopCommonModes);
  CFNetServiceSetClient(theService, NULL, NULL);
  CFNetServiceCancel(theService);
  CFRelease(theService);
}

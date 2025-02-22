/*
* UPnP Support for XBMC
* Copyright (c) 2006 c0diq (Sylvain Rebaud)
* Portions Copyright (c) by the authors of libPlatinum
*
* http://www.plutinosoft.com/blog/category/platinum/
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#pragma once

/* forward declarations */
class PLT_UPnP;
class PLT_SyncMediaBrowser;
class CDeviceHostReferenceHolder;
class CCtrlPointReferenceHolder;
class CRendererReferenceHolder;
class CUPnPRenderer;
class CUPnPServer;
class PLT_MediaObject;
class PLT_MediaItemResource;

namespace MUSIC_INFO {
class CMusicInfoTag;
}

class CVideoInfoTag;


class CUPnP
{
public:
    CUPnP();
    ~CUPnP();

    // server
    void StartServer();
    void StopServer();

    // client
    void StartClient();
    void StopClient();
    bool IsClientStarted() { return (m_MediaBrowser != NULL); }

    // renderer
    void StartRenderer();
    void StopRenderer();
    void UpdateState();

    // methods
    static int PopulateTagFromObject(MUSIC_INFO::CMusicInfoTag& tag,
                                     PLT_MediaObject&           object,
                                     PLT_MediaItemResource*     resource = NULL);
    static int PopulateTagFromObject(CVideoInfoTag&             tag,
                                     PLT_MediaObject&           object,
                                     PLT_MediaItemResource*     resource = NULL);

    // class methods
    static CUPnP* GetInstance();
    static void   ReleaseInstance(bool bWait);
    static bool   IsInstantiated() { return upnp != NULL; }

    // fences for m_MediaBrowser class methods; ugly because the class is defined in the cpp.
    void AddFriendlyName( const CStdString& device_id, const CStdString& object_id, const CStdString& friendly_name );
    bool GetFriendlyName( const CStdString& device_id, const CStdString& object_id, CStdString& friendly_name );
    bool GetFriendlyPath( const CStdString& path, CStdString& friendly_path );
  
  
private:
    // methods
    CUPnPRenderer* CreateRenderer(int port = 0);
    CUPnPServer*   CreateServer(int port = 0);

public:
    PLT_SyncMediaBrowser*       m_MediaBrowser;

private:
    CStdString                  m_IP;
    PLT_UPnP*                   m_UPnP;
    CDeviceHostReferenceHolder* m_ServerHolder;
    CRendererReferenceHolder*   m_RendererHolder;
    CCtrlPointReferenceHolder*  m_CtrlPointHolder;    


    static CUPnP* upnp;
    static bool   broadcast;
};

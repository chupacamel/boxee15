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

#include "../../xbmc/Application.h"
#include "../../xbmc/AdvancedSettings.h"
#include "WIN32Util.h"
#include "UpdateSourceFile.h"
#include "RemoteWrapper.h"
#include "RemoteControl.h"
#include "shellapi.h"
#include "dbghelp.h"
#include "../ErrorHandler.h"

// Add support for minidumps
#ifdef USE_MINI_DUMPS
#include "MiniDumps.h"
#endif

GlobalErrorHandler geh;

//-----------------------------------------------------------------------------
// Resource defines
//-----------------------------------------------------------------------------

#include "resource.h"

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

#ifdef USE_MINI_DUMPS
CMiniDumps g_MiniDumps;
#endif
CApplication g_application;

// Minidump creation function 
LONG WINAPI CreateMiniDump( EXCEPTION_POINTERS* pEp ) 
{
  // Create the dump file where the xbmc.exe resides
  CStdString errorMsg;
  CStdString dumpFile;
  dumpFile.Format("xbmc.r%s.dmp", SVN_REV);
  HANDLE hFile = CreateFile(dumpFile.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

  // Call MiniDumpWriteDump api with the dump file
  if ( hFile && ( hFile != INVALID_HANDLE_VALUE ) ) 
  {
    // Load the DBGHELP DLL
    HMODULE hDbgHelpDll = ::LoadLibrary("DBGHELP.DLL");       
    if (hDbgHelpDll)
    {
      MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDbgHelpDll, "MiniDumpWriteDump");
      if (pDump)
      {
        // Initialize minidump structure
        MINIDUMP_EXCEPTION_INFORMATION mdei; 
        mdei.ThreadId           = CThread::GetCurrentThreadId();
        mdei.ExceptionPointers  = pEp; 
        mdei.ClientPointers     = FALSE; 

        // Call the minidump api with normal dumping
        // We can get more detail information by using other minidump types but the dump file will be
        // extermely large.
        BOOL rv = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, 0, NULL); 
        if( !rv ) 
        {
          errorMsg.Format("MiniDumpWriteDump failed with error id %d", GetLastError());
          MessageBox(NULL, errorMsg.c_str(), "XBMC: Error", MB_OK|MB_ICONERROR); 
        } 
      }
      else
      {
        errorMsg.Format("MiniDumpWriteDump failed to load with error id %d", GetLastError());
        MessageBox(NULL, errorMsg.c_str(), "XBMC: Error", MB_OK|MB_ICONERROR);
      }
      
      // Close the DLL
      FreeLibrary(hDbgHelpDll);
    }
    else
    {
      errorMsg.Format("LoadLibrary 'DBGHELP.DLL' failed with error id %d", GetLastError());
      MessageBox(NULL, errorMsg.c_str(), "XBMC: Error", MB_OK|MB_ICONERROR);
    }

    // Close the file 
    CloseHandle( hFile ); 
  }
  else 
  {
    errorMsg.Format("CreateFile '%s' failed with error id %d", dumpFile.c_str(), GetLastError());
    MessageBox(NULL, errorMsg.c_str(), "XBMC: Error", MB_OK|MB_ICONERROR); 
  }

  return pEp->ExceptionRecord->ExceptionCode;;
}


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR commandLine, INT )
{
#ifdef WIN32_MEMORY_LEAK_DETECT
	// Turn on the heap checking
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF       |
					_CRTDBG_DELAY_FREE_MEM_DF	|
                     _CRTDBG_LEAK_CHECK_DF       ) ;
	// Not adding the _CRTDBG_CHECK_ALWAYS_DF flag either - since it makes Boxee run VERY slow.

#endif

#ifdef USE_MINI_DUMPS
  g_MiniDumps.Install();
#endif

  CStdStringW strcl(commandLine);
  if (strcl.Find(L"-usf") >= 0)
  {        
    CUpdateSourceFile UpdateSourceFile;
    bool success = UpdateSourceFile.UpdateProfilesSourceFile();
    
    if(success)
    {
      printf("main - Successfully updated the profiles source files\n");
    }
    else
    {
      printf("main - Failed to update profiles source files\n");
    }

    success = UpdateSourceFile.UpgradeSettings();

    if(success)
    {
      printf("main - Successfully updated GUI Settings\n");
    }
    else
    {
      printf("main - Failed to update GUI Settings\n");
    }
    
    exit(0);
  }

  // Initializes CreateMiniDump to handle exceptions.
  SetUnhandledExceptionFilter( CreateMiniDump );

  // check if Boxee is already running
  bool AlreadyRunning;
  HANDLE hMutexOneInstance = ::CreateMutex( NULL, FALSE,
    _T("BOXEE-4B71588F-DDBB-40da-AD86-3187B70AA5A3"));

  AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
    ::GetLastError() == ERROR_ACCESS_DENIED);
  // The call fails with ERROR_ACCESS_DENIED if the Mutex was 
  // created in a different users session because of passing
  // NULL for the SECURITY_ATTRIBUTES on Mutex creation);
#ifndef _DEBUG
  if ( AlreadyRunning )
  {
    HWND m_hwnd = FindWindow("Boxee","Boxee");
    if(m_hwnd != NULL)
    {
      // switch to the running instance
      ShowWindow(m_hwnd,SW_RESTORE);
      SetForegroundWindow(m_hwnd);
    }
   
    return 0;
  }
#endif

  if(CWIN32Util::GetDesktopColorDepth() < 32)
  {
    //FIXME: replace it by a SDL window for all ports
    MessageBox(NULL, "Desktop Color Depth isn't 32Bit", "BOXEE: Fatal Error", MB_OK|MB_ICONERROR);
    return 0;
  }

  // parse the command line
  LPWSTR *szArglist;
  int nArgs;

  g_advancedSettings.m_startFullScreen = false;
  szArglist = CommandLineToArgvW(strcl.c_str(), &nArgs);
  if(szArglist != NULL)
  {
    for(int i=0;i<nArgs;i++)
    {
      CStdStringW strArgW(szArglist[i]);
      if(strArgW.Equals(L"-fs"))
        g_advancedSettings.m_startFullScreen = true;
      else if(strArgW.Equals(L"-p") || strArgW.Equals(L"--portable"))
        g_application.EnablePlatformDirectories(false);
      else if(strArgW.Equals(L"-d"))
      {
        if(++i < nArgs)
        {
          int iSleep = _wtoi(szArglist[i]);
          if(iSleep > 0 && iSleep < 360)
            Sleep(iSleep*1000);
          else
            --i;
}
      }
      // this flag indicate boxee was started from the registry on user login
      // We create a delay to make sure initialization succeed.
      else if (strArgW.Equals(L"-startup"))
      {
        Sleep(5000);
    }
    }
    LocalFree(szArglist);
  }

// we don't want to see the "no disc in drive" windows message box
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // Create and run the app
  if (!SUCCEEDED(g_application.Create(NULL)))
  {
    return 1;
  }  

  g_application.Run();

  char *a = new char[50];
  return 0;
}

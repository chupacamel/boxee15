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

#include "emu_kernel32.h"
#include "emu_dummy.h"
#include "utils/log.h"
#include "Thread.h"

#include "xbox/IoSupport.h"

#ifndef _LINUX
#include <process.h>
#endif

#include "../dll_tracker.h"
#include "FileSystem/SpecialProtocol.h"

#include "ThreadPolicy.h"

#ifdef _LINUX
#include "../../../linux/PlatformInclude.h"
#define __except catch
#endif

using namespace std;

vector<string> m_vecAtoms;

//#define API_DEBUG

extern "C" HANDLE xboxopendvdrom()
{
  return CIoSupport::OpenCDROM();
}

extern "C" UINT WINAPI dllGetAtomNameA( ATOM nAtom, LPTSTR lpBuffer, int nSize)
{
  if (nAtom < 1 || nAtom > m_vecAtoms.size() ) return 0;
  nAtom--;
  string& strAtom = m_vecAtoms[nAtom];
  strcpy(lpBuffer, strAtom.c_str());
  return strAtom.size();
}

extern "C" ATOM WINAPI dllFindAtomA( LPCTSTR lpString)
{
  for (int i = 0; i < (int)m_vecAtoms.size(); ++i)
  {
    string& strAtom = m_vecAtoms[i];
    if (strAtom == lpString) return i + 1;
  }
  return 0;
}

extern "C" ATOM WINAPI dllAddAtomA( LPCTSTR lpString)
{
  m_vecAtoms.push_back(lpString);
  return m_vecAtoms.size();
}
/*
extern "C" ATOM WINAPI dllDeleteAtomA(ATOM nAtom)
{
}*/

extern "C" BOOL WINAPI dllFindClose(HANDLE hFile)
{
  return FindClose(hFile);
}

#ifdef _WIN32
#define CORRECT_SEP_STR(str) \
  if (strstr(str, "://") == NULL) \
  { \
    int iSize_##str = strlen(str); \
    for (int pos = 0; pos < iSize_##str; pos++) \
      if (str[pos] == '/') str[pos] = '\\'; \
  } \
  else \
  { \
    int iSize_##str = strlen(str); \
    for (int pos = 0; pos < iSize_##str; pos++) \
      if (str[pos] == '\\') str[pos] = '/'; \
  }
#else
#define CORRECT_SEP_STR(str)
#endif

extern "C" HANDLE WINAPI dllFindFirstFileA(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
  char* p = strdup(lpFileName);
  CORRECT_SEP_STR(p);
  
  // change default \\*.* into \\* which the xbox is using
  char* e = strrchr(p, '.');
  if (e != NULL && strlen(e) > 1 && e[1] == '*')
  {
    e[0] = '\0';
  }

  HANDLE res = FindFirstFile(_P(p).c_str(), lpFindFileData);
  free(p);
  return res;
}

// should be moved to CFile! or use CFile::stat
extern "C" DWORD WINAPI dllGetFileAttributesA(LPCSTR lpFileName)
{
  char str[1024];

  if (!strcmp(lpFileName, "\\Device\\Cdrom0")) return (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY);

  // move to CFile classes
  if (strncmp(lpFileName, "\\Device\\Cdrom0", 14) == 0)
  {
    // replace "\\Device\\Cdrom0" with "D:"
    strcpy(str, "D:");
    strcat(str, lpFileName + 14);
  }
  else strcpy(str, lpFileName);

#ifndef _LINUX
  // convert '/' to '\\'
  char *p = str;
  while (p = strchr(p, '/')) * p = '\\';
  return GetFileAttributesA(str);
#else
  return GetFileAttributes(str);
#endif
}

struct SThreadWrapper
{
  LPTHREAD_START_ROUTINE lpStartAddress;
  LPVOID lpParameter;
  PCHAR lpDLL;
};

#ifdef _DEBUG
#define MS_VC_EXCEPTION 0x406d1388
typedef struct tagTHREADNAME_INFO 
{ 
  DWORD dwType; // must be 0x1000 
  LPCSTR szName; // pointer to name (in same addr space) 
  DWORD dwThreadID; // thread ID (-1 caller thread) 
  DWORD dwFlags; // reserved for future use, most be zero 
} THREADNAME_INFO;
#endif

#ifdef _LINUX
int dllThreadWrapper(LPVOID lpThreadParameter)
#else
unsigned int __stdcall dllThreadWrapper(LPVOID lpThreadParameter)
#endif
{
  SThreadWrapper *param = (SThreadWrapper*)lpThreadParameter;
  DWORD result;

#if defined(_DEBUG) && !defined(_LINUX)  
  THREADNAME_INFO info; 
  info.dwType = 0x1000; 
  info.szName = "DLL"; 
  info.dwThreadID = ::GetCurrentThreadId(); 
  info.dwFlags = 0; 
  __try 
  { 
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD *)&info); 
  } 
  __except (EXCEPTION_CONTINUE_EXECUTION) 
  { 
  }  
#endif

  __try
  {
    result = param->lpStartAddress(param->lpParameter);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {    
    CLog::Log(LOGERROR, "DLL:%s - Unhandled exception in thread created by dll", param->lpDLL );
    result = 0;
  }

  delete param;
  return result;

}

extern "C" HANDLE WINAPI dllCreateThread(
  LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
  DWORD dwStackSize,                        // initial stack size
  LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
  LPVOID lpParameter,                       // thread argument
  DWORD dwCreationFlags,                    // creation option
  LPDWORD lpThreadId                        // thread identifier
)
{
  uintptr_t loc = (uintptr_t)_ReturnAddress();
  
  SThreadWrapper *param = new SThreadWrapper;
  param->lpStartAddress = lpStartAddress;
  param->lpParameter = lpParameter;
  param->lpDLL = tracker_getdllname(loc);

  return (HANDLE)_beginthreadex(lpThreadAttributes, dwStackSize, dllThreadWrapper, param, dwCreationFlags, (unsigned int *)lpThreadId);
}


extern "C" BOOL WINAPI dllTerminateThread(HANDLE tHread, DWORD dwExitCode)
{
  not_implement("kernel32.dll fake function TerminateThread called\n");  //warning
  return TRUE;
}

extern "C" void WINAPI dllSleep(DWORD dwTime)
{
  return ::Sleep(dwTime);
}

extern "C" HANDLE WINAPI dllGetCurrentThread(void)
{
  HANDLE retval = GetCurrentThread();
  return retval;
}

extern "C" DWORD WINAPI dllGetCurrentProcessId(void)
{
#ifndef _XBOX
#ifdef _LINUX
  return (DWORD)getppid();
#else
  return GetCurrentProcessId();
#endif
#else
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetCurrentProcessId(void) => 31337");
#endif
  return 31337;
#endif
}

extern "C" BOOL WINAPI dllGetProcessTimes(HANDLE hProcess, LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime)
{
  // since the xbox has only one process, we just take the current thread
  HANDLE h = GetCurrentThread();
  BOOL res = GetThreadTimes(h, lpCreationTime, lpExitTime, lpKernelTime, lpUserTime);
  
  return res;
}

extern "C" int WINAPI dllDuplicateHandle(HANDLE hSourceProcessHandle,   // handle to source process
      HANDLE hSourceHandle,          // handle to duplicate
      HANDLE hTargetProcessHandle,   // handle to target process
      HANDLE* lpTargetHandle,       // duplicate handle
      DWORD dwDesiredAccess,         // requested access
      int bInheritHandle,           // handle inheritance option
      DWORD dwOptions               // optional actions
                                          )
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "DuplicateHandle(%p, %p, %p, %p, 0x%x, %d, %d) called\n",
            hSourceProcessHandle, hSourceHandle, hTargetProcessHandle,
            lpTargetHandle, dwDesiredAccess, bInheritHandle, dwOptions);
#endif
#if defined (_XBOX) || defined (_LINUX)
  *lpTargetHandle = hSourceHandle;
  return 1;
#else
  return DuplicateHandle(hSourceProcessHandle, hSourceHandle, hTargetProcessHandle, lpTargetHandle, dwDesiredAccess, bInheritHandle, dwOptions);
#endif
}

extern "C" BOOL WINAPI dllDisableThreadLibraryCalls(HMODULE h)
{
#ifdef _WIN32
  return DisableThreadLibraryCalls(h);
#endif
  not_implement("kernel32.dll fake function DisableThreadLibraryCalls called\n"); //warning
  return TRUE;
}

#ifndef _LINUX
static void DumpSystemInfo(const SYSTEM_INFO* si)
{
  CLog::Log(LOGDEBUG, "  Processor architecture %d\n", si->wProcessorArchitecture);
  CLog::Log(LOGDEBUG, "  Page size: %d\n", si->dwPageSize);
  CLog::Log(LOGDEBUG, "  Minimum app address: %d\n", si->lpMinimumApplicationAddress);
  CLog::Log(LOGDEBUG, "  Maximum app address: %d\n", si->lpMaximumApplicationAddress);
  CLog::Log(LOGDEBUG, "  Active processor mask: 0x%x\n", si->dwActiveProcessorMask);
  CLog::Log(LOGDEBUG, "  Number of processors: %d\n", si->dwNumberOfProcessors);
  CLog::Log(LOGDEBUG, "  Processor type: 0x%x\n", si->dwProcessorType);
  CLog::Log(LOGDEBUG, "  Allocation granularity: 0x%x\n", si->dwAllocationGranularity);
  CLog::Log(LOGDEBUG, "  Processor level: 0x%x\n", si->wProcessorLevel);
  CLog::Log(LOGDEBUG, "  Processor revision: 0x%x\n", si->wProcessorRevision);
}
#endif

extern "C" void WINAPI dllGetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetSystemInfo(0x%x) =>", lpSystemInfo);
#endif
#ifdef _WIN32
  // VS 2003 complains about x even so it's defined
  lpSystemInfo->wProcessorArchitecture = 0; //#define PROCESSOR_ARCHITECTURE_INTEL 0
#else
  lpSystemInfo->x.wProcessorArchitecture = 0; //#define PROCESSOR_ARCHITECTURE_INTEL 0
#endif
  lpSystemInfo->dwPageSize = 4096;   //Xbox page size
  lpSystemInfo->lpMinimumApplicationAddress = (void *)0x00000000;
  lpSystemInfo->lpMaximumApplicationAddress = (void *)0x7fffffff;
  lpSystemInfo->dwActiveProcessorMask = 1;
  lpSystemInfo->dwNumberOfProcessors = 1;
  lpSystemInfo->dwProcessorType = 586;  //#define PROCESSOR_INTEL_PENTIUM 586
  lpSystemInfo->wProcessorLevel = 6;
  //lpSystemInfo->wProcessorLevel = 5;
  lpSystemInfo->wProcessorRevision = 0x080A;
  lpSystemInfo->dwAllocationGranularity = 0x10000; //virtualalloc reserve block size
#ifdef API_DEBUG
  DumpSystemInfo(lpSystemInfo);
#endif
}  //hardcode for xbox processor type;

extern "C" UINT WINAPI dllGetPrivateProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault,
    LPCSTR lpFileName)
{
  not_implement("kernel32.dll fake function GetPrivateProfileIntA called\n"); //warning
  return nDefault;
}

//globals for memory leak hack, no need for well-behaved dlls call init/del in pair
//We can free the sections if applications does not call deletecriticalsection
//need to initialize the list head NULL at mplayer_open_file, and free memory at close file.
std::map<LPCRITICAL_SECTION, LPCRITICAL_SECTION> g_mapCriticalSection;

extern "C" void WINAPI dllDeleteCriticalSection(LPCRITICAL_SECTION cs)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "DeleteCriticalSection(0x%x)", cs);
#endif
  if (g_mapCriticalSection.find(cs) != g_mapCriticalSection.end())
  {
    LPCRITICAL_SECTION cs_new = g_mapCriticalSection[cs];
    DeleteCriticalSection(cs_new);
    delete cs_new;
    g_mapCriticalSection.erase(cs);
  }
}

extern "C" void WINAPI dllInitializeCriticalSection(LPCRITICAL_SECTION cs)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "InitializeCriticalSection(0x%x)", cs);
#endif
  LPCRITICAL_SECTION cs_new = new CRITICAL_SECTION;
  memset(cs_new, 0, sizeof(CRITICAL_SECTION));
  InitializeCriticalSection(cs_new);
  
  // just take the first member of the CRITICAL_SECTION to save ourdata in, this will be used to 
  // get fast access to the new critial section in dllLeaveCriticalSection and dllEnterCriticalSection
  ((LPCRITICAL_SECTION*)cs)[0] = cs_new;
  g_mapCriticalSection[cs] = cs_new;
}

extern "C" void WINAPI dllLeaveCriticalSection(LPCRITICAL_SECTION cs)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "LeaveCriticalSection(0x%x) %p\n", ((LPCRITICAL_SECTION*)cs)[0]);
#endif
  LeaveCriticalSection(((LPCRITICAL_SECTION*)cs)[0]);
}

extern "C" void WINAPI dllEnterCriticalSection(LPCRITICAL_SECTION cs)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "EnterCriticalSection(0x%x) %p\n", cs, ((LPCRITICAL_SECTION*)cs)[0]);
#endif
#ifndef _LINUX
  if (!(LPCRITICAL_SECTION)cs->OwningThread)
  {
#ifdef API_DEBUG
    CLog::Log(LOGDEBUG, "entered uninitialized critisec!\n");
#endif
    dllInitializeCriticalSection(cs);
  }
#endif
  EnterCriticalSection(((LPCRITICAL_SECTION*)cs)[0]);
}

extern "C" DWORD WINAPI dllGetVersion()
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetVersion() => 0xC0000004 (Windows 95)\n");
#endif
  //return 0x0a280105; //Windows XP
  return 0xC0000004; //Windows 95
}

extern "C" BOOL WINAPI dllGetVersionExA(LPOSVERSIONINFO lpVersionInfo)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetVersionExA()\n");
#endif
  lpVersionInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  lpVersionInfo->dwMajorVersion = 4;
  lpVersionInfo->dwMinorVersion = 0;
  lpVersionInfo->dwBuildNumber = 0x4000457;
  // leave it here for testing win9x-only codecs
  lpVersionInfo->dwPlatformId = 1; //VER_PLATFORM_WIN32_WINDOWS
  lpVersionInfo->szCSDVersion[0] = 0;
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "  Major version: %d\n  Minor version: %d\n  Build number: %x\n"
            "  Platform Id: %d\n Version string: '%s'\n", 
            lpVersionInfo->dwMajorVersion, lpVersionInfo->dwMinorVersion, 
            lpVersionInfo->dwBuildNumber, lpVersionInfo->dwPlatformId, lpVersionInfo->szCSDVersion);
#endif
  return TRUE;
}

extern "C" BOOL WINAPI dllGetVersionExW(LPOSVERSIONINFOW lpVersionInfo)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetVersionExW()\n");
#endif
  if(!dllGetVersionExA((LPOSVERSIONINFO)lpVersionInfo))
    return FALSE;
  
  lpVersionInfo->szCSDVersion[0] = 0;
  lpVersionInfo->szCSDVersion[1] = 0;
  return TRUE;
}

extern "C" UINT WINAPI dllGetProfileIntA(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault)
{
  //  CLog::Log(LOGDEBUG,"GetProfileIntA:%s %s %i", lpAppName,lpKeyName,nDefault);
  not_implement("kernel32.dll fake function GetProfileIntA called\n"); //warning
  return nDefault;
}

extern "C" BOOL WINAPI dllFreeEnvironmentStringsW(LPWSTR lpString)
{
  // we don't have anything to clean up here, just return.
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "FreeEnvironmentStringsA(0x%x) => 1", lpString);
#endif
  return true;
}

extern "C" HMODULE WINAPI dllGetOEMCP()
{
  not_implement("kernel32.dll fake function GetOEMCP called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllRtlUnwind(PVOID TargetFrame OPTIONAL, PVOID TargetIp OPTIONAL, PEXCEPTION_RECORD ExceptionRecord OPTIONAL, PVOID ReturnValue)
{
  not_implement("kernel32.dll fake function RtlUnwind called\n"); //warning
  return NULL;
}
extern "C" LPTSTR WINAPI dllGetCommandLineA()
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetCommandLineA() => \"c:\\xbmc.xbe\"\n");
#endif
  return (LPTSTR)"c:\\xbmc.xbe";
}

extern "C" HMODULE WINAPI dllExitProcess(UINT uExitCode)
{
  not_implement("kernel32.dll fake function ExitProcess called\n"); //warning
  return NULL;
}
extern "C" HMODULE WINAPI dllTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
  not_implement("kernel32.dll fake function TerminateProcess called\n"); //warning
  return NULL;
}
extern "C" HANDLE WINAPI dllGetCurrentProcess()
{
#if !defined (_XBOX) && !defined(_LINUX)
  return GetCurrentProcess();
#else
#ifdef _WIN32
  return GetCurrentProcess();
#endif
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetCurrentProcess(void) => 9375");
#endif
  return (HANDLE)9375;
#endif
}

extern "C" UINT WINAPI dllGetACP()
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetACP() => 0");
#endif
  return CP_ACP;
}

extern "C" UINT WINAPI dllSetHandleCount(UINT uNumber)
{
  //Under Windows NT and Windows 95, this function simply returns the value specified in the uNumber parameter.
  //return uNumber;
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "SetHandleCount(0x%x) => 1\n", uNumber);
#endif
  return uNumber;
}

extern "C" HANDLE WINAPI dllGetStdHandle(DWORD nStdHandle)
{
  switch (nStdHandle)
  {
  case STD_INPUT_HANDLE: return (HANDLE)0;
  case STD_OUTPUT_HANDLE: return (HANDLE)1;
  case STD_ERROR_HANDLE: return (HANDLE)2;
  }
  SetLastError( ERROR_INVALID_PARAMETER );
  return INVALID_HANDLE_VALUE;
}

#ifdef _XBOX
#define FILE_TYPE_UNKNOWN       0
#define FILE_TYPE_DISK          1
#define FILE_TYPE_CHAR          2
#endif

extern "C" DWORD WINAPI dllGetFileType(HANDLE hFile)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetFileType(0x%x) => 0x3 = pipe", hFile);
#endif
  return 3;
}

extern "C" int WINAPI dllGetStartupInfoA(LPSTARTUPINFOA lpStartupInfo)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetStartupInfoA(0x%x) => 1\n");
#endif
  lpStartupInfo->cb = sizeof(_STARTUPINFOA);
  lpStartupInfo->cbReserved2 = 0;
  lpStartupInfo->dwFillAttribute = 0;
  lpStartupInfo->dwFlags = 0;
  lpStartupInfo->dwX = 50; //
  lpStartupInfo->dwXCountChars = 0;
  lpStartupInfo->dwXSize = 0;
  lpStartupInfo->dwY = 50; //
  lpStartupInfo->dwYCountChars = 0;
  lpStartupInfo->dwYSize = 0;
  lpStartupInfo->hStdError = (HANDLE)2;
  lpStartupInfo->hStdInput = (HANDLE)0;
  lpStartupInfo->hStdOutput = (HANDLE)1;
  lpStartupInfo->lpDesktop = NULL;
  lpStartupInfo->lpReserved = NULL;
  lpStartupInfo->lpReserved2 = 0;
  lpStartupInfo->lpTitle = (LPTSTR)"XBMC";
  lpStartupInfo->wShowWindow = 0;
  return 1;
}

extern "C" BOOL WINAPI dllFreeEnvironmentStringsA(LPSTR lpString)
{
  // we don't have anything to clean up here, just return.
  return true;
}

static const char ch_envs[] =
  "__MSVCRT_HEAP_SELECT=__GLOBAL_HEAP_SELETED,1\r\n"
  "PATH=C:\\;C:\\windows\\;C:\\windows\\system\r\n";

extern "C" LPVOID WINAPI dllGetEnvironmentStrings()
{
#ifdef _WIN32
  return GetEnvironmentStrings();
#endif
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetEnvironmentStrings() => 0x%x = %p", ch_envs, ch_envs);
#endif
  return (LPVOID)ch_envs;
}

extern "C" LPVOID WINAPI dllGetEnvironmentStringsW()
{
#ifdef _WIN32
  return GetEnvironmentStringsW();
#endif
  return 0;
}

extern "C" int WINAPI dllGetEnvironmentVariableA(LPCSTR lpName, LPSTR lpBuffer, DWORD nSize)
{
#ifdef _WIN32
  return GetEnvironmentVariableA(lpName, lpBuffer, nSize);
#endif
  if (lpBuffer) lpBuffer[0] = 0;

  if (strcmp(lpName, "__MSVCRT_HEAP_SELECT") == 0)
    strcpy(lpBuffer, "__GLOBAL_HEAP_SELECTED,1");
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetEnvironmentVariableA('%s', 0x%x, %d) => %d", lpName, lpBuffer, nSize, strlen(lpBuffer));
#endif
  return strlen(lpBuffer);
}

extern "C" HMODULE WINAPI dllLCMapStringA(LCID Locale, DWORD dwMapFlags, LPCSTR lpSrcStr, int cchSrc, LPSTR lpDestStr, int cchDest)
{
  not_implement("kernel32.dll fake function LCMapStringA called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllLCMapStringW(LCID Locale, DWORD dwMapFlags, LPCWSTR lpSrcStr, int cchSrc, LPWSTR lpDestStr, int cchDest)
{
  not_implement("kernel32.dll fake function LCMapStringW called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllSetStdHandle(DWORD nStdHandle, HANDLE hHandle)
{
  not_implement("kernel32.dll fake function SetStdHandle called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllGetStringTypeA(LCID Locale, DWORD dwInfoType, LPCSTR lpSrcStr, int cchSrc, LPWORD lpCharType)
{
  not_implement("kernel32.dll fake function GetStringTypeA called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllGetStringTypeW(DWORD dwInfoType, LPCWSTR lpSrcStr, int cchSrc, LPWORD lpCharType)
{
  not_implement("kernel32.dll fake function GetStringTypeW called\n"); //warning
  return NULL;
}

extern "C" HMODULE WINAPI dllGetCPInfo(UINT CodePage, LPCPINFO lpCPInfo)
{
  not_implement("kernel32.dll fake function GetCPInfo called\n"); //warning
  return NULL;
}

extern "C" LCID WINAPI dllGetThreadLocale(void)
{
  // primary language identifier, sublanguage identifier, sorting identifier
  return MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
}

extern "C" BOOL WINAPI dllSetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass)
{
  not_implement("kernel32.dll fake function SetPriorityClass called\n"); //warning
  return false;
}

extern "C" DWORD WINAPI dllFormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPTSTR lpBuffer, DWORD nSize, va_list* Arguments)
{
#ifdef _WIN32
  return FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
#else
  not_implement("kernel32.dll fake function FormatMessage called\n"); //warning
  return 0;
#endif  
}

extern "C" DWORD WINAPI dllGetFullPathNameA(LPCTSTR lpFileName, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR* lpFilePart)
{
  if (!lpFileName) return 0;
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetFullPathNameA('%s',%d,%p,%p)\n", lpFileName, nBufferLength, lpBuffer, lpFilePart);
#endif
  if (strrchr(lpFileName, '\\'))
    lpFilePart = (LPSTR*)strrchr((const char *)lpFileName, '\\');
  else
    lpFilePart = (LPTSTR *)lpFileName;

  unsigned int length = strlen(lpFileName);
  if (nBufferLength < (length + 1))
  {
    return length + 1;
  } else {
    strcpy(lpBuffer, lpFileName);
    return length;
  }
}

extern "C" DWORD WINAPI dllExpandEnvironmentStringsA(LPCTSTR lpSrc, LPTSTR lpDst, DWORD nSize)
{
#ifdef _WIN32
  return ExpandEnvironmentStringsA(lpSrc, lpDst, nSize);
#else
  not_implement("kernel32.dll fake function ExpandEnvironmentStringsA called\n"); //warning
#endif
  return 0;
}

extern "C" UINT WINAPI dllGetWindowsDirectoryA(LPTSTR lpBuffer, UINT uSize)
{
  not_implement("kernel32.dll fake function dllGetWindowsDirectory called\n"); //warning
  return 0;
}

extern "C" UINT WINAPI dllGetSystemDirectoryA(LPTSTR lpBuffer, UINT uSize)
{
  //char* systemdir = "special://xbmc/system/mplayer/codecs";
  //unsigned int len = strlen(systemdir);
  //if (len > uSize) return 0;
  //strcpy(lpBuffer, systemdir);
  //not_implement("kernel32.dll incompete function dllGetSystemDirectory called\n"); //warning
  //CLog::Log(LOGDEBUG,"KERNEL32!GetSystemDirectoryA(0x%x, %d) => %s", lpBuffer, uSize, systemdir);
  //return len;
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetSystemDirectoryA(%p,%d)\n", lpBuffer, uSize);
#endif
  if (!lpBuffer) strcpy(lpBuffer, ".");
  return 1;
}


extern "C" UINT WINAPI dllGetShortPathName(LPTSTR lpszLongPath, LPTSTR lpszShortPath, UINT cchBuffer)
{
  if (!lpszLongPath) return 0;
  if (strlen(lpszLongPath) == 0)
  {
    //strcpy(lpszLongPath, "special://xbmc/system/mplayer/codecs/QuickTime.qts");
  }
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "KERNEL32!GetShortPathNameA('%s',%p,%d)\n", lpszLongPath, lpszShortPath, cchBuffer);
#endif
  strcpy(lpszShortPath, lpszLongPath);
  return strlen(lpszShortPath);
}

extern "C" HANDLE WINAPI dllGetProcessHeap()
{
#ifdef  _LINUX
  CLog::Log(LOGWARNING, "KERNEL32!GetProcessHeap() linux cant provide this service!");
  return 0;
#else
  HANDLE hHeap;
  hHeap = GetProcessHeap();
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "KERNEL32!GetProcessHeap() => 0x%x", hHeap);
#endif
  return hHeap;
#endif
}

extern "C" UINT WINAPI dllSetErrorMode(UINT i)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "SetErrorMode(%d) => 0\n", i);
#endif
  return 0;
}

extern "C" BOOL WINAPI dllIsProcessorFeaturePresent(DWORD ProcessorFeature)
{
  BOOL result = 0;
  switch (ProcessorFeature)
  {
  case PF_3DNOW_INSTRUCTIONS_AVAILABLE:
    result = false;
    break;
  case PF_COMPARE_EXCHANGE_DOUBLE:
    result = true;
    break;
  case PF_FLOATING_POINT_EMULATED:
    result = true;
    break;
  case PF_FLOATING_POINT_PRECISION_ERRATA:
    result = false;
    break;
  case PF_MMX_INSTRUCTIONS_AVAILABLE:
    result = true;
    break;
  case PF_PAE_ENABLED:
    result = false;
    break;
  case PF_RDTSC_INSTRUCTION_AVAILABLE:
    result = true;
    break;
  case PF_XMMI_INSTRUCTIONS_AVAILABLE:
    result = true;
    break;
  case 10: //PF_XMMI64_INSTRUCTIONS_AVAILABLE
    result = false;
    break;
  }

#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "IsProcessorFeaturePresent(0x%x) => 0x%x\n", ProcessorFeature, result);
#endif
  return result;
}

extern "C" DWORD WINAPI dllTlsAlloc()
{
  DWORD retval = TlsAlloc();
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "TlsAlloc() => %d\n", retval);
#endif
  return retval;
}

extern "C" BOOL WINAPI dllTlsFree(DWORD dwTlsIndex)
{
  BOOL retval = TlsFree(dwTlsIndex);
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "KERNEL32!TlsFree(%d) => %d", dwTlsIndex, retval);
#endif
  return retval;
}

extern "C" BOOL WINAPI dllTlsSetValue(int dwTlsIndex, LPVOID lpTlsValue)
{
  if (dwTlsIndex == -1) 
    return FALSE;
  BOOL retval = TlsSetValue(dwTlsIndex, lpTlsValue);

#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "KERNEL32!TlsSetValue(%d, 0x%x) => %d", dwTlsIndex, lpTlsValue, retval);
#endif
  return retval;
}

extern "C" LPVOID WINAPI dllTlsGetValue(DWORD dwTlsIndex)
{
  if(dwTlsIndex == (DWORD)(-1)) 
    return NULL;
  LPVOID retval = TlsGetValue(dwTlsIndex);

#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "KERNEL32!TlsGetValue(%d) => 0x%x", dwTlsIndex, retval);
#endif
  return retval;
}

extern "C" UINT WINAPI dllGetCurrentDirectoryA(UINT c, LPSTR s)
{
  char curdir[] = "special://xbmc/";
  int result;
  strncpy(s, curdir, c);
  result = 1 + ((c < strlen(curdir)) ? c : strlen(curdir));
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "GetCurrentDirectoryA(0x%x, %d) => %d\n", s, c, result);
#endif
  return result;
}

extern "C" UINT WINAPI dllSetCurrentDirectoryA(const char *pathname)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "SetCurrentDirectoryA(0x%x = %s) => 1\n", pathname, pathname);
#endif
  return 1;
}

extern "C" int WINAPI dllSetUnhandledExceptionFilter(void* filter)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "SetUnhandledExceptionFilter(0x%x) => 1\n", filter);
#endif
  return 1; //unsupported and probably won't ever be supported
}

extern "C" int WINAPI dllSetEnvironmentVariableA(const char *name, const char *value)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "SetEnvironmentVariableA(%s, %s)\n", name, value);
#endif
  return 0;
}

extern "C" int WINAPI dllCreateDirectoryA(const char *pathname, void *sa)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "CreateDirectory(0x%x = %s, 0x%x) => 1\n", pathname, pathname, sa);
#endif
  return 1;
}

extern "C" DWORD WINAPI dllWaitForSingleObject(HANDLE hHandle, DWORD dwMiliseconds)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "WaitForSingleObject(0x%x, %d)", hHandle, dwMiliseconds);
#endif
  return WaitForSingleObject(hHandle, dwMiliseconds);
}

#ifdef _LINUX
extern "C" DWORD WINAPI dllWaitForMultipleObjects(DWORD nCount, HANDLE *lpHandles, BOOL fWaitAll, DWORD dwMilliseconds)
#else
extern "C" DWORD WINAPI dllWaitForMultipleObjects(DWORD nCount, CONST HANDLE *lpHandles, BOOL fWaitAll, DWORD dwMilliseconds)
#endif
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "WaitForMultipleObjects(..)");
#endif

  return WaitForMultipleObjects(nCount, lpHandles, fWaitAll, dwMilliseconds);
}

extern "C" BOOL WINAPI dllGetProcessAffinityMask(HANDLE hProcess, LPDWORD lpProcessAffinityMask, LPDWORD lpSystemAffinityMask)
{
  CLog::Log(LOGDEBUG, "GetProcessAffinityMask(%p, %p, %p) => 1\n",
            (void*)hProcess, (void*)lpProcessAffinityMask, (void*)lpSystemAffinityMask);
  if (lpProcessAffinityMask)*lpProcessAffinityMask = 1;
  if (lpSystemAffinityMask)*lpSystemAffinityMask = 1;
  return 1;
}

extern "C" int WINAPI dllGetLocaleInfoA(LCID Locale, LCTYPE LCType, LPTSTR lpLCData, int cchData)
{
  if (Locale == LOCALE_SYSTEM_DEFAULT || Locale == LOCALE_USER_DEFAULT)
  {
    if (LCType == LOCALE_SISO639LANGNAME)
    {
      if (cchData > 3)
      {
        strcpy(lpLCData, "eng");
        return 4;
      }
    }
    else if (LCType == LOCALE_SISO3166CTRYNAME)
    {
      if (cchData > 2)
      {
        strcpy(lpLCData, "US");
        return 3;
      }
    }
    else if (LCType == LOCALE_IDEFAULTLANGUAGE)
    {
      if (cchData > 5)
      {
        strcpy(lpLCData, "en-US");
        return 6;
      }
    }
  }
  
  not_implement("kernel32.dll incomplete function GetLocaleInfoA called\n");  //warning
  SetLastError(ERROR_INVALID_FUNCTION);
  return 0;
}

extern "C" UINT WINAPI dllGetConsoleCP()
{
  return 437; // OEM - United States 
}

extern "C" UINT WINAPI dllGetConsoleOutputCP()
{
  return 437; // OEM - United States 
}

// emulated because windows expects different behaviour
// the xbox calculates always 1 character extra for 0 termination
// however, this is only desired when cbMultiByte has the value -1
extern "C" int WINAPI dllMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
  // first fix, on windows cchWideChar and cbMultiByte may be the same.
  // xbox fails, because it expects cbMultiByte to be at least one character bigger
  // solution, create a new buffer to can hold the new data and copy it (without the 0 termination)
  // to lpMultiByteStr. This is needed because we cannot be sure that lpMultiByteStr is big enough
  int destinationBufferSize = cchWideChar;
  LPWSTR destinationBuffer = lpWideCharStr;
  if (cbMultiByte > 0 && cbMultiByte == cchWideChar) {
    destinationBufferSize++;
    destinationBuffer = (LPWSTR)malloc(destinationBufferSize * sizeof(WCHAR));
  }
  
  int ret = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, destinationBuffer, destinationBufferSize);

  if (ret > 0)
  {
    // second fix, but only if cchWideChar == 0, and ofcours ret > 0 indicating the function
    // returned the number of bytes needed, otherwise ret would be 0, meaning a successfull conversion
    if (cchWideChar == 0) {
      ret--;
    }
    
    // revert the first fix again
    if (cbMultiByte > 0 && cbMultiByte == cchWideChar) {
      // the 0 termination character could never have been written on a windows machine
      // because of cchWideChar == cbMultiByte, again xbox added one for it.
      ret--;
      
      memcpy(lpWideCharStr, destinationBuffer, ret * sizeof(WCHAR));
      free(destinationBuffer);
    }
  }
  
  return ret;
}

// same reason as above
extern "C" int WINAPI dllWideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
  // first fix, on windows cchWideChar and cbMultiByte may be the same.
  // xbox fails, because it expects cbMultiByte to be at least one character bigger
  // solution, create a new buffer to can hold the new data and copy it (without the 0 termination)
  // to lpMultiByteStr. This is needed because we cannot be sure that lpMultiByteStr is big enough
  int destinationBufferSize = cbMultiByte;
  LPSTR destinationBuffer = lpMultiByteStr;
  if (cchWideChar > 0 && cchWideChar == cbMultiByte) {
    destinationBufferSize++;
    destinationBuffer = (LPSTR)malloc(destinationBufferSize * sizeof(char));
  }
  
  int ret = WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, destinationBuffer, destinationBufferSize, lpDefaultChar, lpUsedDefaultChar);

  if (ret > 0)
  {
    // second fix, but only if cbMultiByte == 0, and ofcours ret > 0 indicating the function
    // returned the number of bytes needed, otherwise ret would be 0, meaning a successfull conversion
    if (cbMultiByte == 0) {
      ret--;
    }
    
    // revert the first fix again
    if (cchWideChar > 0 && cchWideChar == cbMultiByte) {
      // the 0 termination character could never have been written on a windows machine
      // because of cchWideChar == cbMultiByte, again xbox added one for it.
      ret--;
      
      memcpy(lpMultiByteStr, destinationBuffer, ret);
      free(destinationBuffer);
    }
  }
  
  return ret;
}

extern "C" UINT WINAPI dllSetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine, BOOL Add)
{
#ifdef _WIN32
  return SetConsoleCtrlHandler(HandlerRoutine, Add);
#else
  // no consoles exists on the xbox, do nothing
  not_implement("kernel32.dll fake function SetConsoleCtrlHandler called\n");  //warning
  SetLastError(ERROR_INVALID_FUNCTION);
  return 0;
#endif
}

typedef struct _SFlsSlot
{
  LONG lInUse; 
  PVOID	pData;
  PFLS_CALLBACK_FUNCTION pCallback;
}
SFlsSlot, *LPSFlsSlot;

#define FLS_NUM_SLOTS 5
#if defined (_XBOX) || defined (_LINUX)
#define FLS_OUT_OF_INDEXES (DWORD)0xFFFFFFFF
#endif
SFlsSlot flsSlots[FLS_NUM_SLOTS] = { { false, NULL, NULL } };

extern "C" DWORD WINAPI dllFlsAlloc(PFLS_CALLBACK_FUNCTION lpCallback)
{
  DWORD i;
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "FlsAlloc(0x%x)\n", lpCallback);
#endif
  for (i = 0; i < FLS_NUM_SLOTS; i++) {    
    if( InterlockedCompareExchange(&flsSlots[i].lInUse, 1, 0) == 0 ) {      
      flsSlots[i].pData = NULL;
      flsSlots[i].pCallback = lpCallback;
      return i;
    }
  }
  SetLastError(ERROR_INVALID_PARAMETER);
  CLog::Log(LOGERROR, " - Out of fls slots");
  return FLS_OUT_OF_INDEXES; // "
}

static LPSFlsSlot FlsGetSlot(DWORD dwFlsIndex)
{
  if (dwFlsIndex >= FLS_NUM_SLOTS) {
    SetLastError(ERROR_INVALID_PARAMETER);
    return NULL;
  }
  if (flsSlots[dwFlsIndex].lInUse != 1) {
    SetLastError(ERROR_INVALID_PARAMETER); // actually ERROR_NO_MEMORY would be correct
    return NULL;
  }
  return &(flsSlots[dwFlsIndex]);
}

extern "C" BOOL WINAPI dllFlsSetValue(DWORD dwFlsIndex, PVOID lpFlsData)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "FlsSetValue(%d, 0x%x) => 0x%x\n", dwFlsIndex, lpFlsData);
#endif
  LPSFlsSlot slot = FlsGetSlot(dwFlsIndex);
  if (slot == NULL)
    return false;
  slot->pData = lpFlsData;
  return true;
}

extern "C" PVOID WINAPI dllFlsGetValue(DWORD dwFlsIndex)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "FlsGetValue(%d)\n", dwFlsIndex);
#endif
  LPSFlsSlot slot = FlsGetSlot(dwFlsIndex);
  return (slot == NULL) ? NULL : slot->pData;
}

extern "C" BOOL WINAPI dllFlsFree(DWORD dwFlsIndex)
{
#ifdef API_DEBUG
  CLog::Log(LOGDEBUG, "FlsFree(%d)\n", dwFlsIndex);
#endif
  LPSFlsSlot slot = FlsGetSlot(dwFlsIndex);
  if (slot == NULL)
    return false;

  if( slot->pCallback )
    slot->pCallback(slot->pData);
  
  slot->pData = NULL;  
  slot->lInUse = 0;

  return true;
}


extern "C" PVOID WINAPI dllEncodePointer(PVOID ptr)
{
  return ptr;
}

extern "C" PVOID WINAPI dllDecodePointer(PVOID ptr)
{
  return ptr;
}


extern "C" HANDLE WINAPI dllCreateFileA(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    )
{
  bool safeToOpen = true; 
  char mode[2];
  ThreadIdentifier tid;
#if defined(_LINUX) && !defined(__APPLE__)
  tid = gettid();
#else
  tid = CThread::GetCurrentThreadId();
#endif 	
  if(dwDesiredAccess & GENERIC_READ)
    mode[0] = 'r';
  else if(dwDesiredAccess & GENERIC_WRITE)
    mode[0] = 'w';

  mode[1] = 'b';

  FileSystemItem item;
  item.fileName = lpFileName;
  item.accessMode = mode;

  if(TPApplyPolicy(tid, FILE_SYSTEM, &item, &safeToOpen) && !safeToOpen)
  {
    CLog::Log(LOGDEBUG, "%s: Access denied for %s", __FUNCTION__, lpFileName);
#if defined(_WIN32)
    SetLastError(ERROR_ACCESS_DENIED);
#endif
    return NULL;
  }
  return CreateFileA(_P(lpFileName), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

extern "C" BOOL WINAPI dllLockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOffBytesToLockLow, DWORD nNumberOffBytesToLockHigh)
{
  //return LockFile(hFile, dwFileOffsetLow, dwFileOffsetHigh, nNumberOffBytesToLockLow, nNumberOffBytesToLockHigh);
  return TRUE;
}

extern "C" BOOL WINAPI dllLockFileEx(HANDLE hFile, DWORD dwFlags, DWORD dwReserved, DWORD nNumberOffBytesToLockLow, DWORD nNumberOffBytesToLockHigh, LPOVERLAPPED lpOverlapped)
{
  //return LockFileEx(hFile, dwFlags, nNumberOffBytesToLockLow, nNumberOffBytesToLockHigh, lpOverlapped);
  return TRUE;
}

extern "C" BOOL WINAPI dllUnlockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh)
{
  //return UnlockFile(hFile, dwFileOffsetLow, dwFileOffsetHigh, nNumberOfBytesToUnlockLow, nNumberOfBytesToUnlockHigh);
  return TRUE;
}

extern "C" DWORD WINAPI dllGetTempPathA(DWORD nBufferLength, LPTSTR lpBuffer)
{
  // If the function succeeds, the return value is the length, in TCHARs, of the string copied to lpBuffer,
  // not including the terminating null character. If the return value is greater than nBufferLength,
  // the return value is the size of the buffer required to hold the path.
  const char* tempPath = "special://temp/temp/";
  unsigned int len = strlen(tempPath);
  
  if (nBufferLength > len)
  {
    strcpy(lpBuffer, tempPath);
  }

  return len;
}

extern "C" HGLOBAL WINAPI dllLoadResource(HMODULE hModule, HRSRC hResInfo)
{
  not_implement("kernel32.dll fake function LoadResource called\n");
  return NULL;
}

extern "C" HRSRC WINAPI dllFindResourceA(HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType)
{
  not_implement("kernel32.dll fake function FindResource called\n");
  return NULL;
}


/*

The following routine was hacked up by JM while looking at why the DVD player was failing
in the middle of the movie.  The symptoms were:

1. DVD player returned error about expecting a NAV packet but none found.
2. Resulted in DVD player closing.
3. Always occured in the same place.
4. Occured on every DVD I tried (originals)
5. Approximately where I would expect the layer change to be (ie just over half way
   through the movie)
6. Resulted in the last chunk of the requested data to be NULL'd out completely.  ReadFile()
   returns correctly, but the last chunk is completely zero'd out.

This routine checks the last chunk for zeros, and re-reads if necessary.
*/
#define DVD_CHUNK_SIZE 2048

extern "C" BOOL WINAPI dllDVDReadFileLayerChangeHack(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
  BOOL ret = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
  if (!ret || !lpNumberOfBytesRead || *lpNumberOfBytesRead < DVD_CHUNK_SIZE) return ret;
  DWORD numChecked = *lpNumberOfBytesRead;
  while (numChecked >= DVD_CHUNK_SIZE)
  {
    BYTE *p = (BYTE *)lpBuffer + numChecked - DVD_CHUNK_SIZE;
    // test for a NULL block
    while (*p == 0 && p < (BYTE *)lpBuffer + numChecked)
      p++;
    if (p == (BYTE *)lpBuffer + numChecked)
    { // fully NULL block - reread
#ifdef _WIN32
      LONG low = 0;
      LONG high = 0;
#else
      int32_t low = 0;
      int32_t high = 0;
#endif 
      low = SetFilePointer(hFile, low, &high, FILE_CURRENT);
      CLog::Log(LOGWARNING,
                "DVDReadFile() warning - "
                "invalid data read from block at %i (%i) - rereading",
                low, high);
      SetFilePointer(hFile, (int)numChecked - (int)*lpNumberOfBytesRead - DVD_CHUNK_SIZE, NULL, FILE_CURRENT);
      DWORD numRead;
      ret = ReadFile(hFile, (BYTE *)lpBuffer + numChecked - DVD_CHUNK_SIZE, DVD_CHUNK_SIZE, &numRead, lpOverlapped);
      if (!ret) return FALSE;
      SetFilePointer(hFile, low, &high, FILE_BEGIN);
    }
    numChecked -= DVD_CHUNK_SIZE;
  }
  return ret;
}

extern "C" LPVOID WINAPI dllLockResource(HGLOBAL hResData)
{
#ifdef _WIN32
  return LockResource(hResData);
#else
  not_implement("kernel32.dll fake function LockResource called\n"); //warning
  return 0;
#endif
}

extern "C" SIZE_T WINAPI dllGlobalSize(HGLOBAL hMem)
{
#ifdef _WIN32
  return GlobalSize(hMem);
#else
  not_implement("kernel32.dll fake function GlobalSize called\n"); //warning
  return 0;
#endif
}

extern "C" DWORD WINAPI dllSizeofResource(HMODULE hModule, HRSRC hResInfo)
{
#ifdef _WIN32
  return SizeofResource(hModule, hResInfo);
#else
  not_implement("kernel32.dll fake function SizeofResource called\n"); //warning
  return 0;
#endif
}

extern "C" BOOL WINAPI dllCreateProcessA(
  LPCSTR lpApplicationName, 
  LPSTR lpCommandLine, 
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL bInheritHandles,
  DWORD dwCreationFlags,
  LPVOID lpEnvironment,
  LPCSTR lpCurrentDirectory,
  LPSTARTUPINFOA lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
)
{
#ifdef _WIN32
  ThreadIdentifier tid = GetCurrentThreadId();
  bool safeToOpen;

  if(TPApplyPolicy(tid, DISALLOW, (void*)lpApplicationName, &safeToOpen) && !safeToOpen)
  {
    CLog::Log(LOGDEBUG, "%s: Access denied for %s", __FUNCTION__, lpApplicationName ? lpApplicationName : lpCommandLine);
    SetLastError(ERROR_ACCESS_DENIED);
    return false;
  }
  return CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
                        dwCreationFlags, lpEnvironment,lpCurrentDirectory, lpStartupInfo, lpProcessInformation);  
#else
  not_implement("kernel32.dll fake function dllCreateProcessA called\n"); //warning
  return 0;  
#endif
}


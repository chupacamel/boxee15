#ifndef Timidity_CODEC_H_
#define Timidity_CODEC_H_

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

#include "ICodec.h"
#include "DllTimidity.h"

class TimidityCodec : public ICodec
{
public:
  TimidityCodec();
  virtual ~TimidityCodec();

  virtual bool Init(const CStdString &strFile, unsigned int filecache);
  virtual void DeInit();
  virtual __int64 Seek(__int64 iSeekTime);
  virtual int ReadPCM(BYTE *pBuffer, int size, int *actualsize);
  virtual bool CanInit();
  static bool IsSupportedFormat(const CStdString& strExt);

private:
  LibraryLoader* m_loader;
  CStdString m_loader_name;

  typedef int (__cdecl *InitMethod) ( const char * soundfont );
  typedef void* (__cdecl *LoadMethod) ( const char* p1);
  typedef int (__cdecl *FillMethod) ( void* p1, char* p2, int p3);
  typedef void  (__cdecl *CleanupMethod)();
  typedef void  (__cdecl *FreeMethod) ( void* p1);
  typedef const char* (__cdecl *ErrorMsgMethod) ();
  typedef unsigned long (__cdecl *LengthMethod) ( void* p1 );
  typedef unsigned long (__cdecl *SeekMethod) ( void* p1, unsigned long p2);

  struct   
  {
    InitMethod Init;
    CleanupMethod Cleanup;
    ErrorMsgMethod ErrorMsg;
    LoadMethod LoadMID;
    FillMethod FillBuffer;
    FreeMethod FreeMID;
    LengthMethod GetLength;
    SeekMethod Seek;
  } m_dll;

  void * m_mid;
  int m_iTrack;
  __int64 m_iDataPos;
};

#endif


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

#ifndef RENDER_SYSTEM_GLES_H
#define RENDER_SYSTEM_GLES_H

#pragma once

#include "RenderSystem.h"

class CRenderSystemGLES : public CRenderSystemBase
{
public:
  CRenderSystemGLES();
  virtual ~CRenderSystemGLES();

  virtual bool InitRenderSystem();
  virtual bool DestroyRenderSystem();
  virtual bool ResetRenderSystem(int width, int height, bool fullScreen, float refreshRate);

  virtual bool BeginRender();
  virtual bool EndRender();
  virtual bool PresentRender();
  virtual bool ClearBuffers(color_t color);
  virtual bool ClearBuffers(float r, float g, float b, float a);
  virtual void ClearStencilBuffer(int val);
  virtual bool IsExtSupported(const char* extension);

  virtual void SetVSync(bool vsync);

  virtual void SetViewPort(CRect& viewPort);
  virtual void GetViewPort(CRect& viewPort);
  
  virtual void CaptureStateBlock();
  virtual void ApplyStateBlock();

  virtual void EnableClipping(bool bEnable);
  virtual void ApplyClippingRect(CRect& clipRect);
  virtual void GetClipRect(CRect& clipRect);
  
  bool IsDeviceReady() const { return true; };
  
  virtual bool TestRender();

  virtual void EnableTexture(bool bEnable);
  virtual void EnableBlending(bool bEnableRGB, bool bEnableAlpha = false);
  virtual void EnableStencil(bool bEnable);
  virtual void EnableDepthTest(bool bEnable);
  virtual void SetStencilFunc(StencilFunc func, int ref, unsigned int mask);
  virtual void SetStencilOp(StencilOp fail_op, StencilOp fail, StencilOp pass);
  virtual void SetColorMask(bool r, bool g, bool b, bool a);

protected:
  virtual void SetVSyncImpl(bool enable) = 0;
  virtual bool PresentRenderImpl() = 0;
  void CalculateMaxTexturesize();

  int        m_iVSyncMode;
  int        m_iVSyncErrors;
  int64_t    m_iSwapStamp;
  int64_t    m_iSwapRate;
  int64_t    m_iSwapTime;
  bool       m_bVsyncInit;
  
  CStdString m_RenderExtensions;
};


#endif // RENDER_SYSTEM_GLES_H

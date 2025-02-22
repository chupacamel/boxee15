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

/*!
\file GUIFont.h
\brief
*/

#pragma once

#include "GUIFontTTF.h"
#include "Shader.h"



/*!
 \ingroup textures
 \brief
 */

#ifdef HAS_GL

class CGUIFontTTFGL : public CGUIFontTTFBase
{
public:
  CGUIFontTTFGL(const CStdString& strFileName);
  virtual ~CGUIFontTTFGL(void);
  static void PrintPixelCount() {}

  virtual void Begin();
  virtual void End();
  virtual bool LoadShaders();

protected:
  virtual CBaseTexture* ReallocTexture(unsigned int& newHeight);
  virtual bool CopyCharToTexture(FT_BitmapGlyph bitGlyph, Character *ch);
  virtual void DeleteHardwareTexture();
  virtual void Render(FontCoordsIndiced& coords, bool useShadow);

  GLint m_uniMatModelView;
  GLint m_uniMatProjection;

  GLint m_uniTexture;
  GLint m_attribVertex;
  GLint m_attribTextureCoord;
  GLint m_attribColor;
};

#endif

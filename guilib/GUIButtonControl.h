/*!
\file GUIButtonControl.h
\brief 
*/

#ifndef GUILIB_GUIBUTTONCONTROL_H
#define GUILIB_GUIBUTTONCONTROL_H

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

#include "GUITexture.h"
#include "GUITextLayout.h"
#include "GUIControl.h"
#include "Key.h"

/*!
 \ingroup controls
 \brief 
 */
class CGUIButtonControl : public CGUIControl
{
public:
  CGUIButtonControl(const CGUIButtonControl &copy);
  CGUIButtonControl(int parentID, int controlID,
                    float posX, float posY, float width, float height,
                    const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus,
                    const CLabelInfo &label);

  virtual ~CGUIButtonControl(void);
  virtual CGUIButtonControl *Clone() const;
  virtual void Render();
  virtual void DoRender(unsigned int currentTime);
  virtual bool OnAction(const CAction &action) ;
  virtual bool OnMouseClick(int button, const CPoint &point);
  virtual bool OnMessage(CGUIMessage& message);
  virtual void AllocResources();
  virtual void FreeResources(bool immediately = false);
  virtual void DynamicResourceAlloc(bool bOnOff);
  virtual void SetPosition(float posX, float posY);
  virtual void SetWidth(float width);
  virtual void SetLabel(const std::string & aLabel);
  virtual void SetLabel2(const std::string & aLabel2);
  void SetClickActions(const std::vector<CGUIActionDescriptor>& clickActions) { m_clickActions = clickActions; };
  const std::vector<CGUIActionDescriptor> &GetClickActions() const { return m_clickActions; };
  void SetFocusActions(const std::vector<CGUIActionDescriptor>& focusActions) { m_focusActions = focusActions; };
  void SetUnFocusActions(const std::vector<CGUIActionDescriptor>& unfocusActions) { m_unfocusActions = unfocusActions; };
  const CLabelInfo& GetLabelInfo() const { return m_label; };
  virtual CStdString GetLabel() const { return GetDescription(); };
  virtual CStdString GetLabel2() const;
  void SetSelected(bool bSelected);
  virtual CStdString GetDescription() const;
  void SetAlpha(unsigned char alpha);

  void PythonSetLabel(const CStdString &strFont, const std::string &strText, color_t textColor, color_t shadowColor, color_t focusedColor);
  void PythonSetDisabledColor(color_t disabledColor);

  void RAMSetTextColor(color_t textColor);
  void SettingsCategorySetTextAlign(uint32_t align);

  virtual void OnClick();
  bool HasClickActions() { return m_clickActions.size() > 0; };
  
  bool IsSelected();

  virtual void UpdateColors();
  
  void SetClickTexture(const CTextureInfo& textureFocus, int visibleDuration);

protected:
  void OnFocus();
  void OnUnFocus();
  virtual void RenderText();

  CGUITexture m_imgFocus;
  CGUITexture m_imgNoFocus;
  CGUITexture* m_imgClick;
  unsigned int  m_focusCounter;
  unsigned char m_alpha;

  CGUIInfoLabel  m_info;
  CGUIInfoLabel  m_info2;
  CLabelInfo m_label;
  CGUITextLayout m_textLayout;
  CGUITextLayout m_textLayout2;

  std::vector<CGUIActionDescriptor> m_clickActions;
  std::vector<CGUIActionDescriptor> m_focusActions;
  std::vector<CGUIActionDescriptor> m_unfocusActions;

  bool m_bSelected;
  
  bool m_clickAnimationStarted;
  CAction m_clickAction;    
  
  int m_clickVisibleDuration;
  unsigned int m_clickTime;
  unsigned int m_renderTime;
};
#endif

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

#include "GUIButtonControl.h"

/*!
 \ingroup controls
 \brief 
 */
class CGUIMultiSelectTextControl : public CGUIControl
{
public:
  CGUIMultiSelectTextControl(int parentID, int controlID,
                    float posX, float posY, float width, float height,
                    const CTextureInfo& textureFocus, const CTextureInfo& textureNoFocus, const CLabelInfo &label, const CGUIInfoLabel &content);

  virtual ~CGUIMultiSelectTextControl(void);
  virtual CGUIMultiSelectTextControl *Clone() const { return new CGUIMultiSelectTextControl(*this); };

  virtual void DoRender(unsigned int currentTime);
  virtual void Render();

  virtual bool OnAction(const CAction &action);
  virtual void OnLeft();
  virtual void OnRight();
  virtual bool HitTest(const CPoint &point) const;
  virtual bool OnMouseOver(const CPoint &point);
  virtual bool OnMouseClick(int button, const CPoint &point);
  virtual void UpdateInfo(const CGUIListItem *item = NULL);

  virtual CStdString GetDescription() const;
  virtual bool CanFocus() const;

  void UpdateText(const CStdString &text);
  bool MoveLeft();
  bool MoveRight();
  void SelectItemFromPoint(const CPoint &point);
  unsigned int GetFocusedItem() const;
  void SetFocusedItem(unsigned int item);

  // overrides to allow all focus anims to translate down to the focus image
  virtual void SetAnimations(const std::vector<CAnimation> &animations);
  virtual void SetFocus(bool focus);
protected:
  virtual void UpdateColors();
  void AddString(const CStdString &text, bool selectable, const CStdString &clickAction = "");
  void PositionButtons();
  unsigned int GetNumSelectable() const;
  int GetItemFromPoint(const CPoint &point) const;
  void ScrollToItem(unsigned int item);

  // the static strings and buttons strings
  class CSelectableString
  {
  public:
    CSelectableString(CGUIFont *font, const CStdString &text, bool selectable, const CStdString &clickAction);
    CGUITextLayout m_text;
    float m_length;
    bool m_selectable;
    CStdString m_clickAction;
  };
  std::vector<CSelectableString> m_items;

  CLabelInfo m_label;
  CGUIInfoLabel  m_info;
  CStdString m_oldText;
  unsigned int m_renderTime;

  // scrolling
  float      m_totalWidth;
  float      m_offset;
  float      m_scrollOffset;
  float      m_scrollSpeed;
  unsigned int m_scrollLastTime;

  // buttons
  CGUIButtonControl m_button;
  unsigned int m_selectedItem;
  std::vector<CGUIButtonControl> m_buttons;
};


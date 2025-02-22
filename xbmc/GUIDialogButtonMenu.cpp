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

#include "GUIDialogButtonMenu.h"
#include "GUILabelControl.h"
#include "GUIButtonControl.h"
#include "GUIBaseContainer.h"
#include "FileItem.h"
#include "GUIDialogYesNo2.h"
#include "LocalizeStrings.h"
#include "Application.h"
#include "BoxeeVersionUpdateManager.h"

#define CONTROL_BUTTON_LIST   10
#define SHUTDOWN_BUTTON_ID    11
#define UPDATE_BUTTON_ID      14
#define CONTROL_BUTTON_LABEL  3100

CGUIDialogButtonMenu::CGUIDialogButtonMenu(void)
    : CGUIDialog(WINDOW_DIALOG_BUTTON_MENU, "DialogButtonMenu.xml")
{
  m_bCanceled = true;
}

CGUIDialogButtonMenu::~CGUIDialogButtonMenu(void)
{

}

void CGUIDialogButtonMenu::OnInitWindow()
{
  CGUIDialog::OnInitWindow();
  m_bCanceled = true;

  SET_CONTROL_FOCUS(CONTROL_BUTTON_LIST,1);
}

bool CGUIDialogButtonMenu::OnMessage(CGUIMessage &message)
{
  bool bRet = CGUIDialog::OnMessage(message);

  if (message.GetMessage() == GUI_MSG_EXECUTE)
  {
    m_bCanceled = false;

    // someone has been clicked - deinit...
    Close();
    return true;
  }
  else if (message.GetMessage() == GUI_MSG_CLICKED)
  {
    if (message.GetSenderId() == CONTROL_BUTTON_LIST)
    {
      CGUIBaseContainer* menuList = (CGUIBaseContainer*) GetControl(CONTROL_BUTTON_LIST);
      CGUIListItemPtr selectedListItem = menuList->GetSelectedItemPtr();
      CFileItem* selectedFileItem = (CFileItem*) selectedListItem.get();
      if (selectedFileItem->m_iprogramCount == SHUTDOWN_BUTTON_ID)
      {
        g_application.getApplicationMessenger().Shutdown();
      }
      else if (selectedFileItem->m_iprogramCount == UPDATE_BUTTON_ID)
      {
        CBoxeeVersionUpdateManager::HandleUpdateVersionButton();
      }
    }
  }

  return bRet;
}

void CGUIDialogButtonMenu::Render()
{
  // get the label control
  CGUILabelControl *pLabel = (CGUILabelControl *)GetControl(CONTROL_BUTTON_LABEL);
  if (pLabel)
  {
    // get the active window, and put it's label into the label control
    const CGUIControl *pControl = GetFocusedControl();
    if (pControl && (pControl->GetControlType() == CGUIControl::GUICONTROL_BUTTON || pControl->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON))
    {
      CGUIButtonControl *pButton = (CGUIButtonControl *)pControl;
      pLabel->SetLabel(pButton->GetLabel());
    }
  }
  CGUIDialog::Render();
}

bool CGUIDialogButtonMenu::IsCanceled() const
{
  return m_bCanceled;
}

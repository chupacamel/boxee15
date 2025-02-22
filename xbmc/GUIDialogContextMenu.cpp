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

#include "system.h"
#include "GUIDialogContextMenu.h"
#include "GUIButtonControl.h"
#include "GUIDialogNumeric.h"
#include "GUIDialogGamepad.h"
#include "GUIDialogFileBrowser.h"
#include "GUIUserMessages.h"
#include "GUIWindowVideoFiles.h"
#include "Autorun.h"
#include "GUIPassword.h"
#include "Util.h"
#include "GUISettings.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogLockSettings.h"
#include "MediaManager.h"
#include "GUIWindowManager.h"
#include "GUIDialogYesNo.h"
#include "FileItem.h"
#include "FileSystem/File.h"
#include "Picture.h"
#include "Settings.h"
#include "LocalizeStrings.h"
#ifdef HAS_HAL
#include "linux/HALManager.h"
#endif

#ifdef _WIN32
#include "WIN32Util.h"
#endif

using namespace std;

#define BACKGROUND_IMAGE       999
#define BACKGROUND_BOTTOM      998
#define BACKGROUND_TOP         997
#define BUTTON_TEMPLATE       1000
#define SPACE_BETWEEN_BUTTONS    2

void CContextButtons::Add(CONTEXT_BUTTON button, const CStdString &label)
{
  push_back(pair<CONTEXT_BUTTON, CStdString>(button, label));
}

void CContextButtons::Add(CONTEXT_BUTTON button, int label)
{
  push_back(pair<CONTEXT_BUTTON, CStdString>(button, g_localizeStrings.Get(label)));
}

CGUIDialogContextMenu::CGUIDialogContextMenu(void):CGUIDialog(WINDOW_DIALOG_CONTEXT_MENU, "DialogContextMenu.xml")
{
  m_iClickedButton = -1;
  m_iNumButtons = 0;
}
CGUIDialogContextMenu::~CGUIDialogContextMenu(void)
{}
bool CGUIDialogContextMenu::OnMessage(CGUIMessage &message)
{
  if (message.GetMessage() == GUI_MSG_CLICKED)
  { // someone has been clicked - deinit...
    m_iClickedButton = message.GetSenderId() - BUTTON_TEMPLATE;
    Close();
    return true;
  }
  return CGUIDialog::OnMessage(message);
}

void CGUIDialogContextMenu::OnInitWindow()
{ // disable the template button control
  CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE);
  if (pControl)
  {
    pControl->SetVisible(false);
  }
  m_iClickedButton = -1;
  // set initial control focus
  m_lastControlID = BUTTON_TEMPLATE + 1;
  CGUIDialog::OnInitWindow();
}

void CGUIDialogContextMenu::ClearButtons()
{ // destroy our buttons (if we have them from a previous viewing)
  for (int i = 1; i <= m_iNumButtons; i++)
  {
    // get the button to remove...
    CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + i);
    if (pControl)
    {
      // remove the control from our list
      RemoveControl(pControl);
      // kill the button
      pControl->FreeResources();
      delete pControl;
    }
  }
  m_iNumButtons = 0;
}

int CGUIDialogContextMenu::AddButton(int iLabel)
{
  return AddButton(g_localizeStrings.Get(iLabel));
}

void CGUIDialogContextMenu::OffsetPosition(float offsetX, float offsetY)
{
  float newX = m_posX + offsetX - GetWidth() * 0.5f;
  float newY = m_posY + offsetY - GetHeight() * 0.5f;
  SetPosition(newX, newY);
}

void CGUIDialogContextMenu::SetPosition(float posX, float posY)
{
  if (posY + GetHeight() > g_settings.m_ResInfo[m_coordsRes].iHeight)
    posY = g_settings.m_ResInfo[m_coordsRes].iHeight - GetHeight();
  if (posY < 0) posY = 0;
  if (posX + GetWidth() > g_settings.m_ResInfo[m_coordsRes].iWidth)
    posX = g_settings.m_ResInfo[m_coordsRes].iWidth - GetWidth();
  if (posX < 0) posX = 0;
  // we currently hack the positioning of the buttons from y position 0, which
  // forces skinners to place the top image at a negative y value.  Thus, we offset
  // the y coordinate by the height of the top image.
  const CGUIControl *top = GetControl(BACKGROUND_TOP);
  if (top)
    posY += top->GetHeight();
  CGUIDialog::SetPosition(posX, posY);
}

int CGUIDialogContextMenu::AddButton(const CStdString &strLabel)
{ // add a button to our control
  CGUIButtonControl *pButtonTemplate = (CGUIButtonControl *)GetFirstFocusableControl(BUTTON_TEMPLATE);
  if (!pButtonTemplate) pButtonTemplate = (CGUIButtonControl *)GetControl(BUTTON_TEMPLATE);
  if (!pButtonTemplate) return 0;
  CGUIButtonControl *pButton = new CGUIButtonControl(*pButtonTemplate);
  if (!pButton) return 0;
  // set the button's ID and position
  m_iNumButtons++;
  int id = BUTTON_TEMPLATE + m_iNumButtons;
  pButton->SetID(id);
  pButton->SetPosition(pButtonTemplate->GetXPosition(), (m_iNumButtons - 1)*(pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
  pButton->SetVisible(true);
  pButton->SetNavigation(id - 1, id + 1, id, id);
  pButton->SetLabel(strLabel);
  AddControl(pButton);
  // and update the size of our menu
  CGUIControl *pControl = (CGUIControl *)GetControl(BACKGROUND_IMAGE);
  if (pControl)
  {
    pControl->SetHeight(m_iNumButtons*(pButtonTemplate->GetHeight() + SPACE_BETWEEN_BUTTONS));
    CGUIControl *pControl2 = (CGUIControl *)GetControl(BACKGROUND_BOTTOM);
    if (pControl2)
      pControl2->SetPosition(pControl2->GetXPosition(), pControl->GetYPosition() + pControl->GetHeight());
  }
  return m_iNumButtons;
}
void CGUIDialogContextMenu::DoModal(int iWindowID /*= WINDOW_INVALID */, const CStdString &param)
{
  // update the navigation of the first and last buttons
  CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + 1);
  if (pControl)
    pControl->SetNavigation(BUTTON_TEMPLATE + m_iNumButtons, pControl->GetControlIdDown(), pControl->GetControlIdLeft(), pControl->GetControlIdRight());
  pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + m_iNumButtons);
  if (pControl)
    pControl->SetNavigation(pControl->GetControlIdUp(), BUTTON_TEMPLATE + 1, pControl->GetControlIdLeft(), pControl->GetControlIdRight());
  // update our default control
  if (m_defaultControl <= BUTTON_TEMPLATE || m_defaultControl > (BUTTON_TEMPLATE + m_iNumButtons))
    m_defaultControl = BUTTON_TEMPLATE + 1;
  // check the default control has focus...
  while (m_defaultControl <= (BUTTON_TEMPLATE + m_iNumButtons) && !(GetControl(m_defaultControl)->CanFocus()))
    m_defaultControl++;
  CGUIDialog::DoModal();
}

int CGUIDialogContextMenu::GetButton()
{
  return m_iClickedButton;
}

float CGUIDialogContextMenu::GetHeight()
{
  const CGUIControl *backMain = GetControl(BACKGROUND_IMAGE);
  if (backMain)
  {
    float height = backMain->GetHeight();
    const CGUIControl *backBottom = GetControl(BACKGROUND_BOTTOM);
    if (backBottom)
      height += backBottom->GetHeight();
    const CGUIControl *backTop = GetControl(BACKGROUND_TOP);
    if (backTop)
      height += backTop->GetHeight();
    return height;
  }
  else
    return CGUIDialog::GetHeight();
}

float CGUIDialogContextMenu::GetWidth()
{
  CGUIControl *pControl = (CGUIControl *)GetControl(BACKGROUND_IMAGE);
  if (pControl)
    return pControl->GetWidth();
  else
    return CGUIDialog::GetWidth();
}
int CGUIDialogContextMenu::GetNumButtons()
{
  return m_iNumButtons;
}
void CGUIDialogContextMenu::EnableButton(int iButton, bool bEnable)
{
  CGUIControl *pControl = (CGUIControl *)GetControl(BUTTON_TEMPLATE + iButton);
  if (pControl) pControl->SetEnabled(bEnable);
}

bool CGUIDialogContextMenu::SourcesMenu(const CStdString &strType, const CFileItemPtr item, float posX, float posY)
{
  // TODO: This should be callable even if we don't have any valid items
  if (!item)
    return false;

  // popup the context menu
  CGUIDialogContextMenu *pMenu = (CGUIDialogContextMenu *)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  if (pMenu)
  {
    // load our menu
    pMenu->Initialize();

    // grab our context menu
    CContextButtons buttons;
    GetContextButtons(strType, item, buttons);

    // add the buttons and execute it
    for (CContextButtons::iterator it = buttons.begin(); it != buttons.end(); it++)
      pMenu->AddButton((*it).second);
    // position it correctly
    pMenu->OffsetPosition(posX, posY);
    pMenu->DoModal();

    // translate our button press
    CONTEXT_BUTTON btn = CONTEXT_BUTTON_CANCELLED;
    if (pMenu->GetButton() > 0 && pMenu->GetButton() <= (int)buttons.size())
      btn = buttons[pMenu->GetButton() - 1].first;

    if (btn != CONTEXT_BUTTON_CANCELLED)
      return OnContextButton(strType, item, btn);
  }
  return false;
}

void CGUIDialogContextMenu::GetContextButtons(const CStdString &type, const CFileItemPtr item, CContextButtons &buttons)
{
  // Add buttons to the ContextMenu that should be visible for both sources and autosourced items
  if (item && item->IsRemovable())
  {
    if (item->IsDVD() || item->IsCDDA())
    {
    // We need to check if there is a detected is inserted!
      if ( g_mediaManager.IsDiscInDrive() )
      buttons.Add(CONTEXT_BUTTON_PLAY_DISC, 341); // Play CD/DVD!
    buttons.Add(CONTEXT_BUTTON_EJECT_DISC, 13391);  // Eject/Load CD/DVD!
  }
    else // Must be HDD
    {
      buttons.Add(CONTEXT_BUTTON_EJECT_DRIVE, 13420);  // Eject Removable HDD!
    }
  }


  // Next, Add buttons to the ContextMenu that should ONLY be visible for sources and not autosourced items
  CMediaSource *share = GetShare(type, item.get());

  if (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() || g_passwordManager.bMasterUser)
  {
    if (share)
    {
      if (!share->m_ignore)
        buttons.Add(CONTEXT_BUTTON_EDIT_SOURCE, 1027); // Edit Source
      buttons.Add(CONTEXT_BUTTON_SET_DEFAULT, 13335); // Set as Default
      if (!share->m_ignore)
        buttons.Add(CONTEXT_BUTTON_REMOVE_SOURCE, 522); // Remove Source

      buttons.Add(CONTEXT_BUTTON_SET_THUMB, 20019);
    }
    if (!GetDefaultShareNameByType(type).IsEmpty())
      buttons.Add(CONTEXT_BUTTON_CLEAR_DEFAULT, 13403); // Clear Default

    buttons.Add(CONTEXT_BUTTON_ADD_SOURCE, 1026); // Add Source
  }
  if (share && LOCK_MODE_EVERYONE != g_settings.m_vecProfiles[0].getLockMode())
  {
    if (share->m_iHasLock == 0 && (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() || g_passwordManager.bMasterUser))
      buttons.Add(CONTEXT_BUTTON_ADD_LOCK, 12332);
    else if (share->m_iHasLock == 1)
      buttons.Add(CONTEXT_BUTTON_REMOVE_LOCK, 12335);
    else if (share->m_iHasLock == 2)
    {
      buttons.Add(CONTEXT_BUTTON_REMOVE_LOCK, 12335);

      bool maxRetryExceeded = false;
      if (g_guiSettings.GetInt("masterlock.maxretries") != 0)
        maxRetryExceeded = (share->m_iBadPwdCount >= g_guiSettings.GetInt("masterlock.maxretries"));
  
      if (maxRetryExceeded)
        buttons.Add(CONTEXT_BUTTON_RESET_LOCK, 12334);
      else
        buttons.Add(CONTEXT_BUTTON_CHANGE_LOCK, 12356);
    }
  }
  if (share && !g_passwordManager.bMasterUser && item->m_iHasLock == 1)
    buttons.Add(CONTEXT_BUTTON_REACTIVATE_LOCK, 12353);
}

bool CGUIDialogContextMenu::OnContextButton(const CStdString &type, const CFileItemPtr item, CONTEXT_BUTTON button)
{
  // Add Source doesn't require a valid share
  if (button == CONTEXT_BUTTON_ADD_SOURCE)
  {
    if (g_settings.m_iLastLoadedProfileIndex == 0)
    {
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;
    }
    else if (!g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
      return false;

    return CGUIDialogMediaSource::ShowAndAddMediaSource(type);
  }

  // buttons that are available on both sources and autosourced items
  if (!item) return false;
  
  switch (button)
  {
  case CONTEXT_BUTTON_EJECT_DRIVE:
#ifdef HAS_HAL
    return g_HalManager.Eject(item->m_strPath);
#elif defined(_WIN32)
    return CWIN32Util::EjectDrive(item->m_strPath[0]);
#else
    return false;
#endif

#ifdef HAS_DVD_DRIVE  
  case CONTEXT_BUTTON_PLAY_DISC:
    return MEDIA_DETECT::CAutorun::PlayDisc();

  case CONTEXT_BUTTON_EJECT_DISC:
#ifdef _WIN32
    CWIN32Util::ToggleTray(g_mediaManager.TranslateDevicePath(item->m_strPath)[0]);
#else
    CIoSupport::ToggleTray();
#endif
#endif  
    return true;
  default:
    break;
  }

  // the rest of the operations require a valid share
  CMediaSource *share = GetShare(type, item.get());
  if (!share) return false;
  switch (button)
  {
  case CONTEXT_BUTTON_EDIT_SOURCE:
    if (g_settings.m_iLastLoadedProfileIndex == 0)
    {
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;
    }
    else if (!g_passwordManager.IsProfileLockUnlocked())
      return false;

    return CGUIDialogMediaSource::ShowAndEditMediaSource(type, *share);
    
  case CONTEXT_BUTTON_REMOVE_SOURCE:
    if (g_settings.m_iLastLoadedProfileIndex == 0)
    {
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;
    }
    else 
    {
      if (!g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsMasterLockUnlocked(false))
        return false;
      if (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
        return false;
    }
    // prompt user if they want to really delete the source
    if (CGUIDialogYesNo::ShowAndGetInput(751, 0, 750, 0))
    { // check default before we delete, as deletion will kill the share object
      CStdString defaultSource(GetDefaultShareNameByType(type));
      if (!defaultSource.IsEmpty())
      {
        if (share->strName.Equals(defaultSource))
          ClearDefault(type);
      }

      // delete this share
      g_settings.DeleteSource(type, share->strName, share->strPath);
      return true;
    }
    break;

  case CONTEXT_BUTTON_SET_DEFAULT:
    if (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
      return false;
    else if (!g_passwordManager.IsMasterLockUnlocked(true))
      return false;

    // make share default
    SetDefault(type, share->strName);
    return true;

  case CONTEXT_BUTTON_CLEAR_DEFAULT:
    if (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
      return false;
    else if (!g_passwordManager.IsMasterLockUnlocked(true))
      return false;
    // remove share default
    ClearDefault(type);
    return true;

  case CONTEXT_BUTTON_SET_THUMB:
    {
      if (g_settings.m_vecProfiles[g_settings.m_iLastLoadedProfileIndex].canWriteSources() && !g_passwordManager.IsProfileLockUnlocked())
        return false;
      else if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;

      // setup our thumb list
      CFileItemList items;

      // add the current thumb, if available
      if (!share->m_strThumbnailImage.IsEmpty())
      {
        CFileItemPtr current(new CFileItem("thumb://Current", false));
        current->SetThumbnailImage(share->m_strThumbnailImage);
        current->SetLabel(g_localizeStrings.Get(20016));
        items.Add(current);
      }
      else if (item->HasThumbnail())
      { // already have a thumb that the share doesn't know about - must be a local one, so we mayaswell reuse it.
        CFileItemPtr current(new CFileItem("thumb://Current", false));
        current->SetThumbnailImage(item->GetThumbnailImage());
        current->SetLabel(g_localizeStrings.Get(20016));
        items.Add(current);
      }
      // see if there's a local thumb for this item
      CStdString folderThumb = item->GetFolderThumb();
      if (XFILE::CFile::Exists(folderThumb))
      { // cache it
        if (CPicture::CreateThumbnail(folderThumb, item->GetCachedProgramThumb()))
        {
          CFileItemPtr local(new CFileItem("thumb://Local", false));
          local->SetThumbnailImage(item->GetCachedProgramThumb());
          local->SetLabel(g_localizeStrings.Get(20017));
          items.Add(local);
        }
      }
      // and add a "no thumb" entry as well
      CFileItemPtr nothumb(new CFileItem("thumb://None", false));
      nothumb->SetIconImage(item->GetIconImage());
      nothumb->SetLabel(g_localizeStrings.Get(20018));
      items.Add(nothumb);

      CStdString strThumb;
      VECSOURCES shares;
      g_mediaManager.GetLocalDrives(shares);
      if (!CGUIDialogFileBrowser::ShowAndGetImage(items, shares, g_localizeStrings.Get(1030), strThumb))
        return false;

      if (strThumb == "thumb://Current")
        return true;

      if (strThumb == "thumb://None")
        strThumb = "";

      if (!share->m_ignore)
      {
        g_settings.UpdateSource(type,share->strName,"thumbnail",strThumb);
        g_settings.SaveSources();
      }
      else if (!strThumb.IsEmpty())
      { // this is icky as we have to cache using a bunch of different criteria
        CStdString cachedThumb;
        if (type == "music")
        {
          cachedThumb = item->m_strPath;
          CUtil::RemoveSlashAtEnd(cachedThumb);
          cachedThumb = CUtil::GetCachedMusicThumb(cachedThumb);
    }
        else if (type == "video")
          cachedThumb = item->GetCachedVideoThumb();
        else if (type == "pictures")
          cachedThumb = item->GetCachedPictureThumb();
        else  // assume "programs"
          cachedThumb = item->GetCachedProgramThumb();
        XFILE::CFile::Cache(strThumb, cachedThumb);
      }

      CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
      g_windowManager.SendThreadMessage(msg);
      return true;
    }

  case CONTEXT_BUTTON_ADD_LOCK:
    {
      // prompt user for mastercode when changing lock settings) only for default user
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;

      CStdString strNewPassword = "";
      if (!CGUIDialogLockSettings::ShowAndGetLock(share->m_iLockMode,strNewPassword))
        return false;
      // password entry and re-entry succeeded, write out the lock data
      share->m_iHasLock = 2;
      g_settings.UpdateSource(type, share->strName, "lockcode", strNewPassword);
      strNewPassword.Format("%i",share->m_iLockMode);
      g_settings.UpdateSource(type, share->strName, "lockmode", strNewPassword);
      g_settings.UpdateSource(type, share->strName, "badpwdcount", "0");
      g_settings.SaveSources();

      CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
      g_windowManager.SendThreadMessage(msg);
      return true;
    }
  case CONTEXT_BUTTON_RESET_LOCK:
    {
      // prompt user for profile lock when changing lock settings
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;

      g_settings.UpdateSource(type, share->strName, "badpwdcount", "0");
      g_settings.SaveSources();
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
      g_windowManager.SendThreadMessage(msg);
      return true;
    }
  case CONTEXT_BUTTON_REMOVE_LOCK:
    {
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;

      if (!CGUIDialogYesNo::ShowAndGetInput(12335, 0, 750, 0))
        return false;

      share->m_iHasLock = 0;
      g_settings.UpdateSource(type, share->strName, "lockmode", "0");
      g_settings.UpdateSource(type, share->strName, "lockcode", "0");
      g_settings.UpdateSource(type, share->strName, "badpwdcount", "0");
      g_settings.SaveSources();
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
      g_windowManager.SendThreadMessage(msg);
      return true;
    }
  case CONTEXT_BUTTON_REACTIVATE_LOCK:
    {
      bool maxRetryExceeded = false;
      if (g_guiSettings.GetInt("masterlock.maxretries") != 0)
        maxRetryExceeded = (share->m_iBadPwdCount >= g_guiSettings.GetInt("masterlock.maxretries"));
      if (!maxRetryExceeded)
      {
        // don't prompt user for mastercode when reactivating a lock
        g_passwordManager.LockSource(type, share->strName, true);
        return true;
      }
      return false;
    }
  case CONTEXT_BUTTON_CHANGE_LOCK:
    {
      if (!g_passwordManager.IsMasterLockUnlocked(true))
        return false;

      CStdString strNewPW;
      CStdString strNewLockMode;
      if (CGUIDialogLockSettings::ShowAndGetLock(share->m_iLockMode,strNewPW))
        strNewLockMode.Format("%i",share->m_iLockMode);
      else
        return false;
      // password ReSet and re-entry succeeded, write out the lock data
      g_settings.UpdateSource(type, share->strName, "lockcode", strNewPW);
      g_settings.UpdateSource(type, share->strName, "lockmode", strNewLockMode);
      g_settings.UpdateSource(type, share->strName, "badpwdcount", "0");
      g_settings.SaveSources();
      CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_UPDATE_SOURCES);
      g_windowManager.SendThreadMessage(msg);
      return true;
    }
  default:
    break;
  }
  return false;
}

CMediaSource *CGUIDialogContextMenu::GetShare(const CStdString &type, const CFileItem *item)
{
  VECSOURCES *shares = g_settings.GetSourcesFromType(type);
  if (!shares) return NULL;
  for (unsigned int i = 0; i < shares->size(); i++)
  {
    CMediaSource &testShare = shares->at(i);
    if (CUtil::IsDVD(testShare.strPath))
    {
      if (!item->IsDVD())
        continue;
    }
    else
    {
      if (!testShare.strPath.Equals(item->m_strPath))
        continue;
    }
    // paths match, what about share name - only match the leftmost
    // characters as the label may contain other info (status for instance)
    if (item->GetLabel().Left(testShare.strName.size()).Equals(testShare.strName))
    {
      return &testShare;
    }
  }
  return NULL;
}

void CGUIDialogContextMenu::OnWindowLoaded()
{
  CGUIDialog::OnWindowLoaded();
  SetInitialVisibility();
}

void CGUIDialogContextMenu::OnWindowUnload()
{
  ClearButtons();
}

CStdString CGUIDialogContextMenu::GetDefaultShareNameByType(const CStdString &strType)
{
  VECSOURCES *pShares = g_settings.GetSourcesFromType(strType);
  CStdString strDefault = g_settings.GetDefaultSourceFromType(strType);

  if (!pShares) return "";

  bool bIsSourceName(false);
  int iIndex = CUtil::GetMatchingSource(strDefault, *pShares, bIsSourceName);
  if (iIndex < 0 || iIndex >= (int)pShares->size())
    return "";

  return pShares->at(iIndex).strName;
}

void CGUIDialogContextMenu::SetDefault(const CStdString &strType, const CStdString &strDefault)
{
  if (strType == "programs")
    g_settings.m_defaultProgramSource = strDefault;
  else if (strType == "files")
    g_settings.m_defaultFileSource = strDefault;
  else if (strType == "music")
    g_settings.m_defaultMusicSource = strDefault;
  else if (strType == "video")
    g_settings.m_defaultVideoSource = strDefault;
  else if (strType == "pictures")
    g_settings.m_defaultPictureSource = strDefault;
  g_settings.SaveSources();
}

void CGUIDialogContextMenu::ClearDefault(const CStdString &strType)
{
  SetDefault(strType, "");
}

void CGUIDialogContextMenu::SwitchMedia(const CStdString& strType, const CStdString& strPath)
{
  // what should we display?
  vector <CStdString> vecTypes;
  if (!strType.Equals("music"))
    vecTypes.push_back(g_localizeStrings.Get(2)); // My Music
  if (!strType.Equals("video"))
    vecTypes.push_back(g_localizeStrings.Get(3)); // My Videos
  if (!strType.Equals("pictures"))
    vecTypes.push_back(g_localizeStrings.Get(1)); // My Pictures
  if (!strType.Equals("files"))
    vecTypes.push_back(g_localizeStrings.Get(7)); // My Files

  // something went wrong
  if (vecTypes.size() != 3)
    return;

  // create menu
  CGUIDialogContextMenu *pMenu = (CGUIDialogContextMenu *)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  pMenu->Initialize();

  // add buttons
  int btn_Type[3];
  for (int i=0; i<3; i++)
  {
    btn_Type[i] = pMenu->AddButton(vecTypes[i]);
  }

  // display menu
  pMenu->CenterWindow();
  pMenu->DoModal();

  // check selection
  int btn = pMenu->GetButton();
  for (int i=0; i<3; i++)
  {
    if (btn == btn_Type[i])
    {
      // map back to correct window
      int iWindow = WINDOW_INVALID;
      if (vecTypes[i].Equals(g_localizeStrings.Get(2)))
        iWindow = WINDOW_MUSIC_FILES;
      else if (vecTypes[i].Equals(g_localizeStrings.Get(3)))
        iWindow = WINDOW_VIDEO_FILES;
      else if (vecTypes[i].Equals(g_localizeStrings.Get(1)))
        iWindow = WINDOW_PICTURES;
      else if (vecTypes[i].Equals(g_localizeStrings.Get(7)))
        iWindow = WINDOW_FILES;

      CUtil::ClearFileItemCache();
      g_windowManager.ChangeActiveWindow(iWindow, strPath);
      return;
    }
  }
  return;
}

int CGUIDialogContextMenu::ShowAndGetChoice(const vector<CStdString> &choices, const CPoint *pos)
{
  // no choices??
  if (choices.size() == 0)
    return 0;

  // popup the context menu
  CGUIDialogContextMenu *pMenu = (CGUIDialogContextMenu *)g_windowManager.GetWindow(WINDOW_DIALOG_CONTEXT_MENU);
  if (pMenu)
  {
    // load our menu
    pMenu->Initialize();

    for (unsigned int i = 0; i < choices.size(); i++)
      pMenu->AddButton(choices[i]);

    // position it correctly
    if (pos)
      pMenu->OffsetPosition(pos->x, pos->y);
    else
      pMenu->PositionAtCurrentFocus();

    pMenu->DoModal();

    if (pMenu->GetButton() > 0)
      return pMenu->GetButton();
  }
  return 0;
}

void CGUIDialogContextMenu::PositionAtCurrentFocus()
{
  CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
  if (window)
  {
    const CGUIControl *focusedControl = window->GetFocusedControl();
    if (focusedControl)
    {
      CPoint pos = focusedControl->GetRenderPosition() + CPoint(focusedControl->GetWidth() * 0.5f, focusedControl->GetHeight() * 0.5f);
      OffsetPosition(pos.x,pos.y);
      return;
    }
  }
  // no control to center at, so just center the window
  CenterWindow();
}


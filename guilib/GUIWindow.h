/*!
\file GUIWindow.h
\brief 
*/

#ifndef GUILIB_GUIWINDOW_H
#define GUILIB_GUIWINDOW_H

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

#include "GUIControlGroup.h"
#include "boost/shared_ptr.hpp"
#include "utils/Stopwatch.h"

class CFileItem; typedef boost::shared_ptr<CFileItem> CFileItemPtr;

#include "GUICallback.h"  // for GUIEvent

#include <map>
#include <vector>

#define ON_CLICK_MESSAGE(i,c,m) \
{ \
 GUIEventHandler<c, CGUIMessage&> clickHandler(this, &m); \
 m_mapClickEvents[i] = clickHandler; \
} \

#define ON_SELECTED_MESSAGE(i,c,m) \
{ \
 GUIEventHandler<c, CGUIMessage&> selectedHandler(this, &m); \
 m_mapSelectedEvents[i] = selectedHandler; \
} \

// forward
class TiXmlNode;
class TiXmlElement;
class TiXmlDocument;

class COrigin
{
public:
  COrigin()
  {
    x = y = 0;
    condition = 0;
  };
  float x;
  float y;
  int condition;
};

class CWindowTimer
{
public:
  CStopWatch timer;
  int interval;
  CGUIActionDescriptor actions;  
};

/*!
 \ingroup winmsg
 \brief 
 */
class CGUIWindow : public CGUIControlGroup
{
public:
  enum WINDOW_TYPE { WINDOW = 0, MODAL_DIALOG, MODELESS_DIALOG, BUTTON_MENU, SUB_MENU };

  CGUIWindow(int id, const CStdString &xmlFile);
  virtual ~CGUIWindow(void);

  bool Initialize();  // loads the window
  bool Load(const CStdString& strFileName, bool bContainsPath = false);
  
  void CenterWindow();
  virtual void Render();

  // Close should never be called on this base class (only on derivatives) - its here so that window-manager can use a general close
  virtual void Close(bool forceClose = false);

  // OnAction() is called by our window manager.  We should process any messages
  // that should be handled at the window level in the derived classes, and any
  // unhandled messages should be dropped through to here where we send the message
  // on to the currently focused control.  Returns true if the action has been handled
  // and does not need to be passed further down the line (to our global action handlers)
  virtual bool OnAction(const CAction &action);

  virtual bool OnMouse(const CPoint &point);
  bool HandleMouse(CGUIControl *pControl, const CPoint &point);
  bool OnMove(int fromControl, int moveAction, int toControl = 0);
  virtual bool OnMessage(CGUIMessage& message);

  bool ControlGroupHasFocus(int groupID, int controlID);
  virtual bool HasID(int id) const { return (id >= m_controlID && id < m_controlID + m_idRange); };
  void SetIDRange(int range) { m_idRange = range; };
  int GetIDRange() const { return m_idRange; };
  int GetPreviousWindow() { return m_previousWindow; };
  CRect GetScaledBounds() const;
  virtual void ClearAll();
  virtual void AllocResources() { AllocResources(false); }
  virtual void AllocResources(bool forceLoad);
  virtual void FreeResources(bool forceUnLoad = false);
  virtual void DynamicResourceAlloc(bool bOnOff);
  virtual bool IsDialog() const { return false; };
  virtual bool IsDialogRunning() const { return false; };
  virtual bool IsModalDialog() const { return false; };
  virtual bool IsMediaWindow() const { return false; };
  virtual bool HasListItems() const { return false; };
  virtual CFileItemPtr GetCurrentListItem(int offset = 0) { return CFileItemPtr(); };
  virtual int GetViewContainerID() const { return 0; };
  virtual bool IsActive() const;
  void SetCoordsRes(RESOLUTION res) { m_coordsRes = res; };
  RESOLUTION GetCoordsRes() const { return m_coordsRes; };
  void SetXMLFile(const CStdString &xmlFile) { m_xmlFile = xmlFile; };
  const CStdString &GetXMLFile() const { return m_xmlFile; };
  void LoadOnDemand(bool loadOnDemand) { m_loadOnDemand = loadOnDemand; };
  bool GetLoadOnDemand() { return m_loadOnDemand; }
  int GetRenderOrder() { return m_renderOrder; };
  virtual void SetInitialVisibility();

  enum OVERLAY_STATE { OVERLAY_STATE_PARENT_WINDOW=0, OVERLAY_STATE_SHOWN, OVERLAY_STATE_HIDDEN };

  OVERLAY_STATE GetOverlayState() const { return m_overlayState; };

  virtual bool IsAnimating(ANIMATION_TYPE animType = ANIM_TYPE_ANY);
  void DisableAnimations();

  virtual void ResetControlStates();
  
  void       SetRunActionsManually();
  void       RunLoadActions();
  void       RunUnloadActions();
  
#ifdef _DEBUG
  void DumpTextureUse();
#endif

  bool HasSaveLastControl() const { return !m_defaultAlways; }
  bool HasDynamicContents() const { return m_isDynamicContents; }
  bool HasColorBufferActive() const { return m_bColorbufferactive; }

protected:
  virtual bool LoadXML(const CStdString& strPath, const CStdString &strLowerPath);  ///< Loads from the given file
  bool Load(TiXmlDocument &xmlDoc);                 ///< Loads from the given XML document
  virtual void LoadAdditionalTags(TiXmlElement *root) {}; ///< Load additional information from the XML document

  virtual void SetDefaults();
  virtual void OnWindowUnload() {}
  virtual void OnWindowLoaded();
  virtual void OnInitWindow();
  virtual void OnDeinitWindow(int nextWindowID);
  virtual bool OnMouseAction();
  virtual bool RenderAnimation(unsigned int time);
  virtual bool CheckAnimation(ANIMATION_TYPE animType);

  CAnimation *GetAnimation(ANIMATION_TYPE animType, bool checkConditions = true);

  // control state saving on window close
  virtual void SaveControlStates();
  virtual void RestoreControlStates();

  // methods for updating controls and sending messages
  void OnEditChanged(int id, CStdString &text);
  bool SendMessage(int message, int id, int param1 = 0, int param2 = 0);

  typedef GUIEvent<CGUIMessage&> CLICK_EVENT;
  typedef std::map<int, CLICK_EVENT> MAPCONTROLCLICKEVENTS;
  MAPCONTROLCLICKEVENTS m_mapClickEvents;

  typedef GUIEvent<CGUIMessage&> SELECTED_EVENT;
  typedef std::map<int, SELECTED_EVENT> MAPCONTROLSELECTEDEVENTS;
  MAPCONTROLSELECTEDEVENTS m_mapSelectedEvents;

  void LoadControl(TiXmlElement* pControl, CGUIControlGroup *pGroup);

//#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
  void ChangeButtonToEdit(int id, bool singleLabel = false);
//#endif

  void RunActions(std::vector<CGUIActionDescriptor>& actions);
  
  int m_idRange;
  bool m_bRelativeCoords;
  OVERLAY_STATE m_overlayState;
  RESOLUTION m_coordsRes; // resolution that the window coordinates are in.
  bool m_needsScaling;
  CStdString m_xmlFile;  // xml file to load
  bool m_windowLoaded;  // true if the window's xml file has been loaded
  bool m_loadOnDemand;  // true if the window should be loaded only as needed
  bool m_isDialog;      // true if we have a dialog, false otherwise.
  bool m_dynamicResourceAlloc;

  int m_renderOrder;      // for render order of dialogs

  std::vector<COrigin> m_origins;  // positions of dialogs depending on base window

  // control states
  int m_lastControlID;
  std::vector<CControlState> m_controlStates;
  int m_previousWindow;
  
  bool m_animationsEnabled;
  
  std::vector<CGUIActionDescriptor> m_loadActions;
  std::vector<CGUIActionDescriptor> m_unloadActions;
  std::vector<CWindowTimer> m_timerActions;
  
  bool m_manualRunActions;
  bool m_isDynamicContents; // when this flag is on, all color buffers optimizations for this dialog is off
  bool m_bColorbufferactive; // when this flag is off, all color buffers optimizations for windows below this dialog is off
};

#endif

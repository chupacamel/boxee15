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

#include "GUIStandardWindow.h"
#include "GUIWindowManager.h"
#include "AdvancedSettings.h"
#include "Key.h"

CGUIStandardWindow::CGUIStandardWindow(void) : CGUIWindow(0, "")
{
}

CGUIStandardWindow::~CGUIStandardWindow(void)
{
}

bool CGUIStandardWindow::OnAction(const CAction &action)
{
  if (action.id == ACTION_PREVIOUS_MENU)
  {
    g_windowManager.PreviousWindow();
    return true;
  }

  if (action.id == ACTION_PARENT_DIR && g_advancedSettings.m_bUseEvilB)
  {
    g_windowManager.PreviousWindow();
    return true;
  }

  return CGUIWindow::OnAction(action);
}

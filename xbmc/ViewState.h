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

#include "SortFileItem.h"

#define DEFAULT_VIEW_AUTO (VIEW_TYPE_AUTO << 16)
#define DEFAULT_VIEW_LIST (VIEW_TYPE_LIST << 16)
#define DEFAULT_VIEW_ICONS (VIEW_TYPE_ICON << 16)
#define DEFAULT_VIEW_BIG_ICONS (VIEW_TYPE_BIG_ICON << 16)
#define DEFAULT_VIEW_MAX (((VIEW_TYPE_MAX - 1) << 16) | 60)

class CViewState
{
public:
  CViewState(int viewMode, SORT_METHOD sortMethod, SORT_ORDER sortOrder, time_t changeTime)
  {
    m_viewMode = viewMode;
    m_sortMethod = sortMethod;
    m_sortOrder = sortOrder;
    m_changeTime = changeTime;
  };
  CViewState()
  {
    m_viewMode = 0;
    m_sortMethod = SORT_METHOD_LABEL;
    m_sortOrder = SORT_ORDER_ASC;
    m_changeTime = 0;
  };

  int m_viewMode;
  SORT_METHOD m_sortMethod;
  SORT_ORDER m_sortOrder;
  time_t m_changeTime;
};

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

#include <set>
#include "GUIWindow.h"
#include "utils/Thread.h"
#include "utils/CriticalSection.h"
#include "SlideShowPicture.h"
#include "DllImageLib.h"

class CFileItemList;

class CGUIWindowSlideShow;

class CBackgroundPicLoader : public CThread
{
public:
  CBackgroundPicLoader();
  ~CBackgroundPicLoader();

  void Create(CGUIWindowSlideShow *pCallback);
  void LoadPic(int iPic, int iSlideNumber, const CStdString &strFileName, const CStdString &strContentType, const int maxWidth, const int maxHeight);
  bool IsLoading() { return m_isLoading;};
  CStdString GetFileName(){ return m_strFileName;};
private:
  void Process();
  int m_iPic;
  int m_iSlideNumber;
  CStdString m_strFileName;
  CStdString m_strContentType;
  int m_maxWidth;
  int m_maxHeight;

  HANDLE m_loadPic;
  bool m_isLoading;

  CGUIWindowSlideShow *m_pCallback;
};

class CGUIWindowSlideShow : public CGUIWindow
{
public:
  CGUIWindowSlideShow(void);
  virtual ~CGUIWindowSlideShow(void);

  void RunSlideShow(const CStdString &strPath, bool bRecursive = false, bool bRandom = false, bool bNotRandom = false, const CStdString& pictureToStart = "", bool bStartInPause = false);
  void RunSlideShow(const CFileItemList &slides, bool bRandom = false, bool bNotRandom = false, const CStdString& pictureToStart = "", bool bStartInPause = false);
  void RunSlideShow(bool bRandom = false, bool bNotRandom = false, const CStdString& pictureToStart = "", bool bStartInPause = false);

  void Reset();

  void Add(const CFileItem *picture);
  void AddFromPath(const CStdString &strPath, bool bRecursive);

  const CFileItemList &GetSlideShowContents();
  const CFileItemPtr GetCurrentSlide();
  int NumSlides() const;
  int CurrentSlide() const;

  bool IsPlaying() const;
  bool IsPaused() const;
  bool InSlideShow() const;
  bool InScreensaver() const;

  void ShowNext();
  void ShowPrevious();
  void Select(const CStdString& strPicture);

  void StartSlideShow(bool screensaver=false);
  void PauseSlideShow();

  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual void Render();
  virtual void FreeResources();

  // Callback for CBackgroundPicLoader
  void OnLoadPic(int iPic, int iSlideNumber, CBaseTexture* pTexture, int iOriginalWidth, int iOriginalHeight, bool bFullSize);

  void Shuffle();
  void ToggleShowSlideInfo();
  bool ShowSlideInfo();
  bool IsBackgroundLoaderLoading(bool localPath);

private:
  typedef std::set<CStdString> path_set;  // set to track which paths we're adding
  void AddItems(const CStdString &strPath, path_set *recursivePaths);
  void RenderPause();
  void RenderErrorMessage();
  void Rotate();
  void Zoom(int iZoom);
  void Move(float fX, float fY);
  void GetCheckedSize(float width, float height, int &maxWidth, int &maxHeight);

  int m_iCurrentSlide;
  int m_iNextSlide;
  int m_iRotate;
  int m_iZoomFactor;

  bool m_bSlideShow;
  bool m_bScreensaver;
  bool m_bPause;
  bool m_bErrorMessage;

  CFileItemList* m_slides;

  CSlideShowPic m_Image[2];

  int m_iCurrentPic;
  // background loader
  CBackgroundPicLoader* m_pBackgroundLoader;
  bool m_bWaitForNextPic;
  bool m_bLoadNextPic;
  bool m_bReloadImage;
  DllImageLib m_ImageLib;
  RESOLUTION m_Resolution;
  CCriticalSection m_slideSection;
  
  bool m_showSlideInfo;
};

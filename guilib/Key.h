/*!
 \file Key.h
 \brief
 */

#ifndef GUILIB_KEY
#define GUILIB_KEY

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

#include "XBIRRemote.h"
#include "common/SDLKeyboard.h"
#include <StdString.h>
#include "XBMC_events.h"

// Analogue - don't change order
#define KEY_BUTTON_A                        256
#define KEY_BUTTON_B                        257
#define KEY_BUTTON_X                        258
#define KEY_BUTTON_Y                        259
#define KEY_BUTTON_BLACK                    260
#define KEY_BUTTON_WHITE                    261
#define KEY_BUTTON_LEFT_TRIGGER             262
#define KEY_BUTTON_RIGHT_TRIGGER            263

#define KEY_BUTTON_LEFT_THUMB_STICK         264
#define KEY_BUTTON_RIGHT_THUMB_STICK        265

#define KEY_BUTTON_RIGHT_THUMB_STICK_UP     266 // right thumb stick directions
#define KEY_BUTTON_RIGHT_THUMB_STICK_DOWN   267 // for defining different actions per direction
#define KEY_BUTTON_RIGHT_THUMB_STICK_LEFT   268
#define KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT  269

// Digital - don't change order
#define KEY_BUTTON_DPAD_UP                  270
#define KEY_BUTTON_DPAD_DOWN                271
#define KEY_BUTTON_DPAD_LEFT                272
#define KEY_BUTTON_DPAD_RIGHT               273

#define KEY_BUTTON_START                    274
#define KEY_BUTTON_BACK                     275

#define KEY_BUTTON_LEFT_THUMB_BUTTON        276
#define KEY_BUTTON_RIGHT_THUMB_BUTTON       277

#define KEY_BUTTON_LEFT_ANALOG_TRIGGER      278
#define KEY_BUTTON_RIGHT_ANALOG_TRIGGER     279

#define KEY_BUTTON_LEFT_THUMB_STICK_UP      280 // left thumb stick directions
#define KEY_BUTTON_LEFT_THUMB_STICK_DOWN    281 // for defining different actions per direction
#define KEY_BUTTON_LEFT_THUMB_STICK_LEFT    282
#define KEY_BUTTON_LEFT_THUMB_STICK_RIGHT   283

#define KEY_BROWSER_MOUSE   0xEFFE
#define KEY_VMOUSE          0xEFFF

// 0xF000 -> 0xF200 is reserved for the keyboard; a keyboard press is either
#define KEY_VKEY            0xF000 // a virtual key/functional key e.g. cursor left
#define KEY_ASCII           0xF100 // a printable character in the range of TRUE ASCII (from 0 to 127) // FIXME make it clean and pure unicode! remove the need for KEY_ASCII
#define KEY_UNICODE         0xF200 // another printable character whose range is not included in this KEY code

#define KEY_INVALID         0xFFFF

// actions that we have defined...
#define ACTION_NONE                    0
#define ACTION_MOVE_LEFT               1
#define ACTION_MOVE_RIGHT              2
#define ACTION_MOVE_UP                 3
#define ACTION_MOVE_DOWN               4
#define ACTION_PAGE_UP                 5
#define ACTION_PAGE_DOWN               6
#define ACTION_SELECT_ITEM             7
#define ACTION_HIGHLIGHT_ITEM          8
#define ACTION_PARENT_DIR              9
#define ACTION_PREVIOUS_MENU          10
#define ACTION_SHOW_INFO              11

#define ACTION_PAUSE                  12
#define ACTION_STOP                   13
#define ACTION_NEXT_ITEM              14
#define ACTION_PREV_ITEM              15
#define ACTION_FORWARD                16 // Can be used to specify specific action in a window, Playback control is handled in ACTION_PLAYER_*
#define ACTION_REWIND                 17 // Can be used to specify specific action in a window, Playback control is handled in ACTION_PLAYER_*

#define ACTION_SHOW_GUI               18 // toggle between GUI and movie or GUI and visualisation.
#define ACTION_ASPECT_RATIO           19 // toggle quick-access zoom modes. Can b used in videoFullScreen.zml window id=2005
#define ACTION_STEP_FORWARD           20 // seek +1% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_STEP_BACK              21 // seek -1% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_FORWARD       22 // seek +10% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_BIG_STEP_BACK          23 // seek -10% in the movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_OSD               24 // show/hide OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_SUBTITLES         25 // turn subtitles on/off. Can b used in videoFullScreen.xml window id=2005
#define ACTION_NEXT_SUBTITLE          26 // switch to next subtitle of movie. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SHOW_CODEC             27 // show information about file. Can b used in videoFullScreen.xml window id=2005 and in slideshow.xml window id=2007
#define ACTION_NEXT_PICTURE           28 // show next picture of slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_PREV_PICTURE           29 // show previous picture of slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_OUT               30 // zoom in picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_IN                31 // zoom out picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_TOGGLE_SOURCE_DEST     32 // used to toggle between source view and destination view. Can be used in myfiles.xml window id=3
#define ACTION_SHOW_PLAYLIST          33 // used to toggle between current view and playlist view. Can b used in all mymusic xml files
#define ACTION_QUEUE_ITEM             34 // used to queue a item to the playlist. Can b used in all mymusic xml files
#define ACTION_REMOVE_ITEM            35 // not used anymore
#define ACTION_SHOW_FULLSCREEN        36 // not used anymore
#define ACTION_ZOOM_LEVEL_NORMAL      37 // zoom 1x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_1           38 // zoom 2x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_2           39 // zoom 3x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_3           40 // zoom 4x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_4           41 // zoom 5x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_5           42 // zoom 6x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_6           43 // zoom 7x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_7           44 // zoom 8x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_8           45 // zoom 9x picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_ZOOM_LEVEL_9           46 // zoom 10x picture during slideshow. Can b used in slideshow.xml window id=2007

#define ACTION_CALIBRATE_SWAP_ARROWS  47 // select next arrow. Can b used in: settingsScreenCalibration.xml windowid=11
#define ACTION_CALIBRATE_RESET        48 // reset calibration to defaults. Can b used in: settingsScreenCalibration.xml windowid=11/settingsUICalibration.xml windowid=10
#define ACTION_ANALOG_MOVE            49 // analog thumbstick move. Can b used in: slideshow.xml window id=2007/settingsScreenCalibration.xml windowid=11/settingsUICalibration.xml windowid=10
#define ACTION_ROTATE_PICTURE         50 // rotate current picture during slideshow. Can b used in slideshow.xml window id=2007
#define ACTION_CLOSE_DIALOG           51 // action for closing the dialog. Can b used in any dialog
#define ACTION_SUBTITLE_DELAY_MIN     52 // Decrease subtitle/movie Delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_SUBTITLE_DELAY_PLUS    53 // Increase subtitle/movie Delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_DELAY_MIN        54 // Increase avsync delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_DELAY_PLUS       55 // Decrease avsync delay.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_AUDIO_NEXT_LANGUAGE    56 // Select next language in movie.  Can b used in videoFullScreen.xml window id=2005
#define ACTION_CHANGE_RESOLUTION      57 // switch 2 next resolution. Can b used during screen calibration settingsScreenCalibration.xml windowid=11

#define REMOTE_0                    58  // remote keys 0-9. are used by multiple windows
#define REMOTE_1                    59  // for example in videoFullScreen.xml window id=2005 you can
#define REMOTE_2                    60  // enter time (mmss) to jump to particular point in the movie
#define REMOTE_3                    61
#define REMOTE_4                    62  // with spincontrols you can enter 3digit number to quickly set
#define REMOTE_5                    63  // spincontrol to desired value
#define REMOTE_6                    64
#define REMOTE_7                    65
#define REMOTE_8                    66
#define REMOTE_9                    67

#define ACTION_PLAY                 68  // Unused at the moment
#define ACTION_OSD_SHOW_LEFT        69  // Move left in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_RIGHT       70  // Move right in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_UP          71  // Move up in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_DOWN        72  // Move down in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_SELECT      73  // toggle/select option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_VALUE_PLUS  74  // increase value of current option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_OSD_SHOW_VALUE_MIN   75  // decrease value of current option in OSD. Can b used in videoFullScreen.xml window id=2005
#define ACTION_SMALL_STEP_BACK      76  // jumps a few seconds back during playback of movie. Can b used in videoFullScreen.xml window id=2005

#define ACTION_PLAYER_FORWARD        77  // FF in current file played. global action, can be used anywhere
#define ACTION_PLAYER_REWIND         78  // RW in current file played. global action, can be used anywhere
#define ACTION_PLAYER_PLAY           79  // Play current song. Unpauses song and sets playspeed to 1x. global action, can be used anywhere

#define ACTION_DELETE_ITEM          80  // delete current selected item. Can be used in myfiles.xml window id=3 and in myvideoTitle.xml window id=25
#define ACTION_COPY_ITEM            81  // copy current selected item. Can be used in myfiles.xml window id=3
#define ACTION_MOVE_ITEM            82  // move current selected item. Can be used in myfiles.xml window id=3
#define ACTION_SHOW_MPLAYER_OSD     83  // toggles mplayers OSD. Can be used in videofullscreen.xml window id=2005
#define ACTION_OSD_HIDESUBMENU      84  // removes an OSD sub menu. Can be used in videoOSD.xml window id=2901
#define ACTION_TAKE_SCREENSHOT      85  // take a screenshot
#define ACTION_RENAME_ITEM          87  // rename item

#define ACTION_VOLUME_UP            88
#define ACTION_VOLUME_DOWN          89
#define ACTION_MUTE                 91

#define ACTION_MOUSE                90
#define ACTION_VOLUME_UP_COND       92
#define ACTION_VOLUME_DOWN_COND     93

#define ACTION_MOUSE_CLICK            100
#define ACTION_MOUSE_LEFT_CLICK       100
#define ACTION_MOUSE_RIGHT_CLICK      101
#define ACTION_MOUSE_MIDDLE_CLICK     102
#define ACTION_MOUSE_XBUTTON1_CLICK   103
#define ACTION_MOUSE_XBUTTON2_CLICK   104

#define ACTION_MOUSE_DOUBLE_CLICK           105
#define ACTION_MOUSE_LEFT_DOUBLE_CLICK      105
#define ACTION_MOUSE_RIGHT_DOUBLE_CLICK     106
#define ACTION_MOUSE_MIDDLE_DOUBLE_CLICK    107
#define ACTION_MOUSE_XBUTTON1_DOUBLE_CLICK  108
#define ACTION_MOUSE_XBUTTON2_DOUBLE_CLICK  109

#define ACTION_BACKSPACE          110
#define ACTION_SCROLL_UP          111
#define ACTION_SCROLL_DOWN        112
#define ACTION_ANALOG_FORWARD     113
#define ACTION_ANALOG_REWIND      114

#define ACTION_MOVE_ITEM_UP       115  // move item up in playlist
#define ACTION_MOVE_ITEM_DOWN     116  // move item down in playlist
#define ACTION_CONTEXT_MENU       117  // pops up the context menu


// stuff for virtual keyboard shortcuts
#define ACTION_SHIFT              118
#define ACTION_SYMBOLS            119
#define ACTION_CURSOR_LEFT        120
#define ACTION_CURSOR_RIGHT       121

#define ACTION_BUILT_IN_FUNCTION  122

#define ACTION_SHOW_OSD_TIME      123 // displays current time, can be used in videoFullScreen.xml window id=2005
#define ACTION_ANALOG_SEEK_FORWARD  124 // seeks forward, and displays the seek bar.
#define ACTION_ANALOG_SEEK_BACK     125 // seeks backward, and displays the seek bar.

#define ACTION_VIS_PRESET_SHOW        126
#define ACTION_VIS_PRESET_LIST        127
#define ACTION_VIS_PRESET_NEXT        128
#define ACTION_VIS_PRESET_PREV        129
#define ACTION_VIS_PRESET_LOCK        130
#define ACTION_VIS_PRESET_RANDOM      131
#define ACTION_VIS_RATE_PRESET_PLUS   132
#define ACTION_VIS_RATE_PRESET_MINUS  133

#define ACTION_SHOW_VIDEOMENU         134
#define ACTION_ENTER                  135

#define ACTION_INCREASE_RATING        136
#define ACTION_DECREASE_RATING        137

#define ACTION_NEXT_SCENE             138 // switch to next scene/cutpoint in movie
#define ACTION_PREV_SCENE             139 // switch to previous scene/cutpoint in movie

#define ACTION_NEXT_LETTER            140 // jump through a list or container by letter
#define ACTION_PREV_LETTER            141

#define ACTION_JUMP_SMS2              142 // jump direct to a particular letter using SMS-style input
#define ACTION_JUMP_SMS3              143
#define ACTION_JUMP_SMS4              144
#define ACTION_JUMP_SMS5              145
#define ACTION_JUMP_SMS6              146
#define ACTION_JUMP_SMS7              147
#define ACTION_JUMP_SMS8              148
#define ACTION_JUMP_SMS9              149

#define ACTION_FILTER_CLEAR           150
#define ACTION_FILTER_SMS2            151
#define ACTION_FILTER_SMS3            152
#define ACTION_FILTER_SMS4            153
#define ACTION_FILTER_SMS5            154
#define ACTION_FILTER_SMS6            155
#define ACTION_FILTER_SMS7            156
#define ACTION_FILTER_SMS8            157
#define ACTION_FILTER_SMS9            158

#define ACTION_FIRST_PAGE             159
#define ACTION_LAST_PAGE              160

#define ACTION_AUDIO_DELAY            161
#define ACTION_SUBTITLE_DELAY         162

#define ACTION_PASTE                  180
#define ACTION_NEXT_CONTROL           181
#define ACTION_PREV_CONTROL           182
#define ACTION_CHANNEL_SWITCH         183

#define ACTION_TOGGLE_FULLSCREEN      199 // switch 2 desktop resolution
#define ACTION_TOGGLE_WATCHED         200 // Toggle watched status (videos)
#define ACTION_SCAN_ITEM              201 // scan item
#define ACTION_TOGGLE_DIGITAL_ANALOG  202 // switch digital <-> analog
#define ACTION_RELOAD_KEYMAPS         203 // reloads CButtonTranslator's keymaps
#define ACTION_GUIPROFILE_BEGIN       204 // start the GUIControlProfiler running

#define ACTION_TELETEXT_RED           215 // Teletext Color buttons to control TopText
#define ACTION_TELETEXT_GREEN         216 //    "       "      "    "     "       "
#define ACTION_TELETEXT_YELLOW        217 //    "       "      "    "     "       "
#define ACTION_TELETEXT_BLUE          218 //    "       "      "    "     "       "

#define ACTION_INCREASE_PAR           219
#define ACTION_DECREASE_PAR           220

#define ACTION_GESTURE_NOTIFY         221
#define ACTION_GESTURE_BEGIN          222
#define ACTION_GESTURE_ZOOM           223
#define ACTION_GESTURE_ROTATE         224
#define ACTION_GESTURE_PAN            225
#define ACTION_GESTURE_END            226
#define ACTION_VSHIFT_UP              227 // shift up video image in DVDPlayer
#define ACTION_VSHIFT_DOWN            228 // shift down video image in DVDPlayer

#define ACTION_PLAYER_PLAYPAUSE       229 // Play/pause. If playing it pauses, if paused it plays.


// Boxee
#define ACTION_POST_ANIM_CLICK        311
#define ACTION_RESET_DEMUXER          312
#define ACTION_SYNC_AV                313
#define ACTION_SET_VIDEO_TIME         314
#define ACTION_BROWSER_FULL_SCREEN_ON 315
#define ACTION_BROWSER_FULL_SCREEN_OFF 316
#define ACTION_BROWSER_BACK           317
#define ACTION_BROWSER_FORWARD        318
#define ACTION_BROWSER_PLAYBACK_SKIP  319
#define ACTION_BROWSER_EXT            320
#define ACTION_BROWSER_SET_MODE       321
#define ACTION_BROWSER_NAVIGATE       322
#define ACTION_BROWSER_TOGGLE_MODE    323
#define ACTION_BROWSER_THUMBNAIL_GENERATE 324
#define ACTION_BROWSER_RELOAD         325
#define ACTION_BROWSER_STOP_LOADING   326
#define ACTION_BROWSER_TOGGLE_QUALITY 327
#define ACTION_OSD_EXT_CLICK          328
#define ACTION_HANDLE_DISCONTINUITY   329
// Boxee

// Window ID defines to make the code a bit more readable
#define WINDOW_INVALID                     9999
#define WINDOW_HOME                       10000
#define WINDOW_PROGRAMS                   10001
#define WINDOW_PICTURES                   10002
#define WINDOW_FILES                      10003
#define WINDOW_SETTINGS_MENU              10004
#define WINDOW_MUSIC                      10005 // virtual window to return the music start window.
#define WINDOW_VIDEOS                     10006
#define WINDOW_SYSTEM_INFORMATION         10007
#define WINDOW_TEST_PATTERN               10008
#define WINDOW_SCREEN_CALIBRATION         10011

#define WINDOW_SETTINGS_MYPICTURES        10012
#define WINDOW_SETTINGS_PARRENTAL         10013
#define WINDOW_SETTINGS_MYPERSONAL        10014
#define WINDOW_SETTINGS_LIVE_TV           10015
#define WINDOW_SETTINGS_SYSTEM            10016
#define WINDOW_SETTINGS_MYVIDEOS          10017
#define WINDOW_SETTINGS_NETWORK           10018
#define WINDOW_SETTINGS_APPEARANCE        10019
#define WINDOW_SETTINGS_MYMEDIA           10017

#define WINDOW_SCRIPTS                    10020
#define WINDOW_VIDEO_GENRE                10021
#define WINDOW_VIDEO_ACTOR                10022
#define WINDOW_VIDEO_YEAR                 10023
#define WINDOW_VIDEO_FILES                10024
#define WINDOW_VIDEO_NAV                  10025
#define WINDOW_VIDEO_PLAYLIST             10028

#define WINDOW_LOGIN_SCREEN               10029
#define WINDOW_SETTINGS_PROFILES          10034

#define WINDOW_DIALOG_YES_NO              10100
#define WINDOW_DIALOG_PROGRESS            10101
#define WINDOW_DIALOG_KEYBOARD            10103
#define WINDOW_DIALOG_VOLUME_BAR          10104
#define WINDOW_DIALOG_SUB_MENU            10105
#define WINDOW_DIALOG_CONTEXT_MENU        10106
#define WINDOW_DIALOG_KAI_TOAST           10107
#define WINDOW_DIALOG_NUMERIC             10109
#define WINDOW_DIALOG_GAMEPAD             10110
#define WINDOW_DIALOG_BUTTON_MENU         10111
#define WINDOW_DIALOG_MUSIC_SCAN          10112
#define WINDOW_DIALOG_MUTE_BUG            10113
#define WINDOW_DIALOG_PLAYER_CONTROLS     10114
#define WINDOW_DIALOG_SEEK_BAR            10115
#define WINDOW_DIALOG_MUSIC_OSD           10120
#define WINDOW_DIALOG_VIS_SETTINGS        10121
#define WINDOW_DIALOG_VIS_PRESET_LIST     10122
#define WINDOW_DIALOG_VIDEO_OSD_SETTINGS  10123
#define WINDOW_DIALOG_AUDIO_OSD_SETTINGS  10124
#define WINDOW_DIALOG_VIDEO_BOOKMARKS     10125
#define WINDOW_DIALOG_FILE_BROWSER        10126
#define WINDOW_DIALOG_NETWORK_SETUP       10128
#define WINDOW_DIALOG_MEDIA_SOURCE        10129
#define WINDOW_DIALOG_PROFILE_SETTINGS    10130
#define WINDOW_DIALOG_LOCK_SETTINGS       10131
#define WINDOW_DIALOG_CONTENT_SETTINGS    10132
#define WINDOW_DIALOG_VIDEO_SCAN          10133
#define WINDOW_DIALOG_FAVOURITES          10134
#define WINDOW_DIALOG_SONG_INFO           10135
#define WINDOW_DIALOG_SMART_PLAYLIST_EDITOR 10136
#define WINDOW_DIALOG_SMART_PLAYLIST_RULE   10137
#define WINDOW_DIALOG_BUSY                10138
#define WINDOW_DIALOG_PICTURE_INFO        10139
#define WINDOW_DIALOG_PLUGIN_SETTINGS     10140
#define WINDOW_DIALOG_ACCESS_POINTS       10141
#define WINDOW_DIALOG_FULLSCREEN_INFO     10142
#define WINDOW_DIALOG_KARAOKE_SONGSELECT  10143
#define WINDOW_DIALOG_KARAOKE_SELECTOR    10144
#define WINDOW_DIALOG_SLIDER              10145
#define WINDOW_DIALOG_SUBTITLE_OSD_SETTINGS  10146
#define WINDOW_DIALOG_MENU_CUSTOMIZATION  10147

// Boxee
#define WINDOW_BOXEE_FEED                 10250
#define WINDOW_DIALOG_BOXEE_LOGGING_IN    10255
#define WINDOW_DIALOG_BOXEE_CREDITS       10256

#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_CHOOSE_USER_TO_ADD         10280
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_ADD_EXISTING_USER          10281
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_ADD_NEW_USER               10282
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_NEW_USER_DETAILS           10283
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_TOU                        10284
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_CONNECT_SOCIAL_SERVICES    10285
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_MENU_CUST                  10286
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_CONFIRMATION               10287
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_QUICK_TIP                  10288
#define WINDOW_DIALOG_BOXEE_LOGIN_EDIT_EXISTING_USER                10289
#define WINDOW_DIALOG_BOXEE_LOGIN_WIZARD_QUICK_TIP_AIRPLAY          10290

#define WINDOW_DIALOG_BOXEE_SHARE         10350
#define WINDOW_DIALOG_BOXEE_RATE          10351
#define WINDOW_DIALOG_BOXEE_USER_INFO     10352
#define WINDOW_DIALOG_BOXEE_FRIENDS_LIST  10353
#define WINDOW_DIALOG_BOXEE_VIDEO_CTX     10354
#define WINDOW_DIALOG_BOXEE_MUSIC_CTX     10355
#define WINDOW_DIALOG_BOXEE_PICTURE_CTX   10356
#define WINDOW_DIALOG_BOXEE_APP_CTX       10357
#define WINDOW_DIALOG_BOXEE_USER_LOGIN	  10358
#define WINDOW_DIALOG_BOXEE_BUFFERING  	  10359
#define WINDOW_DIALOG_BOXEE_CHAPTERS      10360
#define WINDOW_DIALOG_BOXEE_EXIT_VIDEO    10361
#define WINDOW_DIALOG_BOXEE_BROWSER_CTX   10362
#define WINDOW_DIALOG_BOXEE_QUICK_TIP     10363
#define WINDOW_DIALOG_BOXEE_SORT_DROPDOWN 10364
#define WINDOW_DIALOG_TECH_INFO           10365
#define WINDOW_DIALOG_BOXEE_SEEK_BAR      10366
#define WINDOW_DIALOG_BOXEE_PAIR          10367
#define WINDOW_BOXEE_SETTINGS_DEVICES     10368
#define WINDOW_DIALOG_BOXEE_CHANNEL_FILTER 10369
#define WINDOW_DIALOG_WIDGETS_TEST        10400
#define WINDOW_DIALOG_BOXEE_MEDIA_ACTION  10401
#define WINDOW_DIALOG_BOXEE_MANUAL_RESOLVE 10402
#define WINDOW_DIALOG_BOXEE_UPDATE_MESSAGE 10403
#define WINDOW_DIALOG_BOXEE_UPDATE_PROGRESS 10404
#define WINDOW_DIALOG_BOXEE_RSS_FEED_INFO 10405
#define WINDOW_DIALOG_BOXEE_DROPDOWN      10406
#define WINDOW_DIALOG_BOXEE_POST_PLAY  	  10407
#define WINDOW_DIALOG_BOXEE_SEARCH        10408
#define WINDOW_DIALOG_BOXEE_GLOBAL_SEARCH 10409
#define WINDOW_DIALOG_BOXEE_USER_PASSWORD 10410
//#define WINDOW_DIALOG_BOXEE_MANUAL_TYPE   10411
#define WINDOW_DIALOG_BOXEE_MANUAL_MOVIE  10412
#define WINDOW_DIALOG_BOXEE_MANUAL_EPISODE 10413
//#define WINDOW_DIALOG_BOXEE_MANUAL_RESULTS 10414
#define WINDOW_DIALOG_BOXEE_MANUAL_DETAILS 10415
#define WINDOW_DIALOG_BOXEE_MANUAL_ADD_FILES 10416
#define WINDOW_DIALOG_BOXEE_MANUAL_PART_ACTION 10417
#define WINDOW_DIALOG_BOXEE_VIDEO_QUALITY 10418
#define WINDOW_DIALOG_BOXEE_MESSAGE_SCROLL 10419
#define WINDOW_DIALOG_BOXEE_MANUAL_RESOLVE_ALBUM    10420
#define WINDOW_DIALOG_BOXEE_MANUAL_RESOLVE_AUDIO    10421
#define WINDOW_DIALOG_WIRELESS_AUTHENTICATION  10422
#define WINDOW_DIALOG_BOXEE_BROWSE_MENU        10423
#define WINDOW_DIALOG_BOXEE_VIDEO_RESUME       10424
#define WINDOW_DIALOG_BOXEE_SELECTION_LIST     10425
#define WINDOW_DIALOG_BOXEE_BROWSER_DROPDOWN   10426
#define WINDOW_TEST_BAD_PIXELS_MANAGER         10428
#define WINDOW_TEST_BAD_PIXELS                 10429
#define WINDOW_DIALOG_BOXEE_PAYMENT_PRODUCTS   10430
#define WINDOW_DIALOG_BOXEE_PAYMENT_TOU        10431
#define WINDOW_DIALOG_BOXEE_PAYMENT_USERDATA   10432
#define WINDOW_DIALOG_BOXEE_PAYMENT_FAILURE    10433
#define WINDOW_DIALOG_BOXEE_PAYMENT_OK_PLAY    10434
#define WINDOW_DIALOG_BOXEE_PAYMENT_WAIT_FOR_SERVER_APPROVAL    10435

// first time use initializations
#define WINDOW_FTU_CALIBRATION                    10436
#define WINDOW_FTU_BACKGROUND                     10438
#define WINDOW_DIALOG_FTU_CONF_NETWORK            10439
#define WINDOW_DIALOG_FTU_LANG                    10440
#define WINDOW_DIALOG_FTU_WELCOME                 10441
#define WINDOW_DIALOG_FTU_NETWORK_MESSAGE         10442
#define WINDOW_DIALOG_FTU_UPDATE_MESSAGE          10443
#define WINDOW_DIALOG_FTU_UPDATE_PROGRESS         10444
#define WINDOW_DIALOG_FTU_WIRELESS                10445
#define WINDOW_DIALOG_FTU_CONF_WIRELESS_PASSWORD  10446
#define WINDOW_DIALOG_FTU_CONF_WIRELESS_SECURITY  10447
#define WINDOW_DIALOG_FTU_CONF_WIRELESS_SSID      10448
#define WINDOW_DIALOG_FTU_SIMPLE_MESSAGE          10449

// Start wizard dialogs
#define WINDOW_BOXEE_WIZARD_RESOLUTION    10450
#define WINDOW_BOXEE_WIZARD_AUDIO    	  10451
#define WINDOW_BOXEE_WIZARD_NETWORK    	  10452
#define WINDOW_BOXEE_WIZARD_NETWORK_MANUAL 10453
#define WINDOW_BOXEE_WIZARD_TIMEZONE	   10454
#define WINDOW_BOXEE_WIZARD_ADD_SOURCE	   10455
#define WINDOW_BOXEE_WIZARD_ADD_SOURCE_MANUAL   10456
#define WINDOW_BOXEE_WIZARD_SOURCE_MANAGER 10457
#define WINDOW_BOXEE_WIZARD_SOURCE_NAME   10458
#define WINDOW_BOXEE_MEDIA_SOURCES   	        10460
#define WINDOW_BOXEE_MEDIA_SOURCE_LIST   	10461
#define WINDOW_BOXEE_MEDIA_SOURCE_ADD_SHARE   	10462
#define WINDOW_BOXEE_MEDIA_SOURCE_INFO   	10463
#define WINDOW_BOXEE_MEDIA_SOURCE_ADD_FOLDER   	10464
#define WINDOW_BOXEE_MAIN   		        10465
#define WINDOW_BOXEE_BROWSE   		        10466
#define WINDOW_BOXEE_DIALOG_MAIN_MENU       10467
//#define WINDOW_BOXEE_DIALOG_OPTIONS_MENU    10468
#define WINDOW_BOXEE_MEDIA_INFO             10469

#define WINDOW_BOXEE_APPLICATION_SETTINGS   10470
//#define WINDOW_BOXEE_DIALOG_LIBRARY_STATUS  10471
#define WINDOW_BOXEE_DIALOG_SERIES_INPUT  10472
#define WINDOW_BOXEE_ALBUM_INFO           10473
#define WINDOW_DIALOG_YES_NO_2              10474
#define WINDOW_DIALOG_OK_2                  10475
#define WINDOW_BOXEE_DIALOG_APPLICATION_ACTION 10476 
#define WINDOW_BOXEE_SHORTCUT_DIALOG      10477
#define WINDOW_SCREEN_SIMPLE_CALIBRATION  10478
#define WINDOW_BOXEE_BROWSE_LOCAL         10479
#define WINDOW_BOXEE_BROWSE_TVSHOWS       10480
#define WINDOW_BOXEE_BROWSE_MOVIES        10481
#define WINDOW_BOXEE_BROWSE_APPS          10482
#define WINDOW_BOXEE_BROWSE_TVEPISODES    10483
#define WINDOW_BOXEE_BROWSE_ALBUMS        10484
#define WINDOW_BOXEE_BROWSE_TRACKS        10485
#define WINDOW_BOXEE_BROWSE_REPOSITORIES  10486
#define WINDOW_BOXEE_BROWSE_APPBOX        10487
#define WINDOW_BOXEE_BROWSE_PRODUCT       10488
#define WINDOW_BOXEE_BROWSE_QUEUE         10489
#define WINDOW_BOXEE_BROWSE_DISCOVER      10490
#define WINDOW_BOXEE_BROWSE_PHOTOS        10491
#define WINDOW_BOXEE_BROWSE_SHORTCUTS     10492
#define WINDOW_BOXEE_BROWSE_HISTORY       10493

#define WINDOW_BOXEE_BROWSE_SIMPLE_APP    10495
#define WINDOW_BOXEE_LOGING_PROMPT        10498
#define WINDOW_DIALOG_BOXEE_NETWORK_NOTIFICATION 10499
// end Boxee

#define WINDOW_MUSIC_PLAYLIST             10500
#define WINDOW_MUSIC_FILES                10501
#define WINDOW_MUSIC_NAV                  10502
#define WINDOW_MUSIC_PLAYLIST_EDITOR      10503
#define WINDOW_DIALOG_BOXEE_SEARCH_RESULTS 10504

//start Boxee
#define WINDOW_BOXEE_LIVETV                  10505
#define WINDOW_OTA_WELCOME_CONFIGURATION     10506
#define WINDOW_OTA_LOCATION_CONFIRMATION     10507
#define WINDOW_OTA_NO_CHANNELS               10508
#define WINDOW_OTA_CONNECTION_CONFIGURATION  10509
#define WINDOW_OTA_COUNTRIES_CONFIGURATION   10510
#define WINDOW_OTA_CONFIGURATION_MANAGER     10512
#define WINDOW_OTA_ZIPCODE_CONFIGURATION     10513
#define WINDOW_DIALOG_BOXEE_LIVETV_INFO      10514
#define WINDOW_OTA_FACEBOOK_CONNECT          10515
#define WINDOW_OTA_SCANNING                  10516
#define WINDOW_DIALOG_WEB                    10517
#define WINDOW_DIALOG_BOXEE_LIVETV_CTX       10518
#define WINDOW_DIALOG_BOXEE_LIVETV_EDIT_CHANNELS 10519

#define WINDOW_DIALOG_BOXEE_BROWSE_SUBTITLES_SETTINGS  10520
#define WINDOW_DIALOG_BOXEE_BROWSE_LOCAL_SUBTITLES_SETTINGS  10521
#define WINDOW_DIALOG_BOXEE_EJECT            10522
#define WINDOW_DIALOG_BOXEE_GET_STARTED      10523
#define WINDOW_DIALOG_BOXEE_MAKE_SOCIAL      10524

#define WINDOW_DIALOG_BOXEE_GET_FACEBOOK_EXTRA_CRED   10525

//end Boxee
#define WINDOW_DIALOG_OSD_TELETEXT        10600

//#define WINDOW_VIRTUAL_KEYBOARD           11000
#define WINDOW_DIALOG_SELECT              12000
#define WINDOW_MUSIC_INFO                 12001
#define WINDOW_DIALOG_OK                  12002
#define WINDOW_VIDEO_INFO                 12003
#define WINDOW_SCRIPTS_INFO               12004
#define WINDOW_FULLSCREEN_VIDEO           12005
#define WINDOW_VISUALISATION              12006
#define WINDOW_SLIDESHOW                  12007
#define WINDOW_DIALOG_FILESTACKING        12008
#define WINDOW_KARAOKELYRICS              12009
#define WINDOW_WEATHER                    12600
#define WINDOW_SCREENSAVER                12900
#define WINDOW_OSD                        12901
#define WINDOW_VIDEO_MENU                 12902
#define WINDOW_MUSIC_OVERLAY              12903
#define WINDOW_VIDEO_OVERLAY              12904

#define WINDOW_STARTUP                    12999 // for startup animations

// WINDOW_ID's from 13000 to 13099 reserved for Python

#define WINDOW_PYTHON_START               13000
#define WINDOW_PYTHON_END                 13099

#define WINDOW_APPS_START                 14000
#define WINDOW_APPS_END                   15000

#define ICON_TYPE_NONE          101
#define ICON_TYPE_PROGRAMS      102
#define ICON_TYPE_MUSIC         103
#define ICON_TYPE_PICTURES      104
#define ICON_TYPE_VIDEOS        105
#define ICON_TYPE_FILES         106
#define ICON_TYPE_WEATHER       107
#define ICON_TYPE_SETTINGS      109

/*!
  \ingroup actionkeys
  \brief
  */
class CAction
{
public:
  CAction()
  {
    id = 0;
    amount1 = amount2 = repeat = 0;
    buttonCode = 0;
    unicode = 0;
    holdTime = 0;
    originalwID = 0;
  };
  CAction(int actionId)
  {
    id = actionId;
    amount1 = amount2 = repeat = 0;
    buttonCode = 0;
    unicode = 0;
    holdTime = 0;
    originalwID = 0;
  };
  int          id;
  int          originalwID;
  float        amount1;
  float        amount2;
  float        repeat;
  unsigned int buttonCode;
  CStdString strAction;
  wchar_t      unicode; // new feature, does not fit into id like ASCII, wouldn't be good design either!? Will be set whenever ASCII is set into id (for backwards compatibility)
  SVKey kKey;
  unsigned int holdTime; ///< Time the key has been held down (in ms)
};

/*!
  \ingroup actionkeys
  \brief
  */
class CKey
{
public:
  CKey(void);
  CKey(uint32_t buttonCode, uint8_t leftTrigger = 0, uint8_t rightTrigger = 0, float leftThumbX = 0.0f, float leftThumbY = 0.0f, float rightThumbX = 0.0f, float rightThumbY = 0.0f, float repeat = 0.0f);
  CKey(const CKey& key);

  virtual ~CKey(void);
  const CKey& operator=(const CKey& key);
  uint32_t GetButtonCode() const; // for backwards compatibility only
  wchar_t GetUnicode() const; // http api does not really support unicode till now. It only simulates unicode when ascii codes are available:
  uint8_t GetLeftTrigger() const;
  uint8_t GetRightTrigger() const;
  float GetLeftThumbX() const;
  float GetLeftThumbY() const;
  float GetRightThumbX() const;
  float GetRightThumbY() const;
  float GetRepeat() const;
  bool FromKeyboard() const;
  bool IsAnalogButton() const;
  bool IsIRRemote() const;
  void SetFromHttpApi(bool);
  bool GetFromHttpApi() const;

  void SetHeld(unsigned int held);
  unsigned int GetHeld() const;

  void SetModifiers(XBMCMod modifiers);
  XBMCMod GetModifiers() const;

private:
  uint32_t m_buttonCode;
  uint8_t m_leftTrigger;
  uint8_t m_rightTrigger;
  float m_leftThumbX;
  float m_leftThumbY;
  float m_rightThumbX;
  float m_rightThumbY;
  float m_repeat; // time since last keypress
  bool m_fromHttpApi;
  unsigned int m_held;
  XBMCMod m_modifiers;
};
#endif


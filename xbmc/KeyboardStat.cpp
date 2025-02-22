//
// C++ Implementation: CKeyboard
//
// Description:
//
//
// Author: Team XBMC <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

// Comment OUT, if not really debugging!!!:
//#define DEBUG_KEYBOARD_GETCHAR

#include "XBMC_keyboard.h"
#include "KeyboardStat.h"
#include "KeyboardLayoutConfiguration.h"
#include "XBMC_events.h"
#include "utils/TimeUtils.h"

#if defined(HAS_X11_EVENTS)
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#endif

CKeyboardStat g_Keyboard;

#define XBMC_NLK_CAPS 0x01
#define XBMC_NLK_NUM  0x02

/* Global keystate information */
static uint8_t  XBMC_KeyState[XBMCK_LAST];
static XBMCMod XBMC_ModState;
static const char *keynames[XBMCK_LAST];	/* Array of keycode names */

static uint8_t XBMC_NoLockKeys;

struct _XBMC_KeyRepeat {
  int firsttime;    /* if we check against the delay or repeat value */
  int delay;        /* the delay before we start repeating */
  int interval;     /* the delay between key repeat events */
  uint32_t timestamp; /* the time the first keydown event occurred */

  XBMC_Event evt;    /* the event we are supposed to repeat */
} XBMC_KeyRepeat;

int XBMC_EnableKeyRepeat(int delay, int interval)
{
  if ( (delay < 0) || (interval < 0) ) {
    return(-1);
  }
  XBMC_KeyRepeat.firsttime = 0;
  XBMC_KeyRepeat.delay = delay;
  XBMC_KeyRepeat.interval = interval;
  XBMC_KeyRepeat.timestamp = 0;
  return(0);
}

struct XBMC_KeyMapping
{
  int   source;
  BYTE  VKey;
  char  Ascii;
  WCHAR Unicode;
};

// based on the evdev mapped scancodes in /user/share/X11/xkb/keycodes
static XBMC_KeyMapping g_mapping_evdev[] =
{ { 121, 0xad, 0, 0 } // Volume mute
, { 122, 0xae, 0, 0 } // Volume down
, { 123, 0xaf, 0, 0 } // Volume up
, { 135, 0x5d, 0, 0 } // Right click
, { 136, 0xb2, 0, 0 } // Stop
, { 138, 0x49, 0, 0 } // Info
, { 166, 0xa6, 0, 0 } // Browser back
, { 167, 0xa7, 0, 0 } // Browser forward
, { 171, 0xb0, 0, 0 } // Next track
, { 172, 0xb3, 0, 0 } // Play_Pause
, { 173, 0xb1, 0, 0 } // Prev track
, { 174, 0xb2, 0, 0 } // Stop
, { 176, 0x52, 0, 0 } // Rewind
, { 180, 0xac, 0, 0 } // Browser home
, { 181, 0xa8, 0, 0 } // Browser refresh
, { 214, 0x1B, 0, 0 } // Close
, { 215, 0xb3, 0, 0 } // Play_Pause
, { 216, 0x46, 0, 0 } // Forward
//, {167, 0xb3 } // Record
};

// following scancode infos are
// 1. from ubuntu keyboard shortcut (hex) -> predefined
// 2. from unix tool xev and my keyboards (decimal)
// m_VKey infos from CharProbe tool
// Can we do the same for XBoxKeyboard and DirectInputKeyboard? Can we access the scancode of them? By the way how does SDL do it? I can't find it. (Automagically? But how exactly?)
// Some pairs of scancode and virtual keys are only known half

// special "keys" above F1 till F12 on my MS natural keyboard mapped to virtual keys "F13" till "F24"):
static XBMC_KeyMapping g_mapping_ubuntu[] =
{ { 0xf5, 0x7c, 0, 0 } // F13 Launch help browser
, { 0x87, 0x7d, 0, 0 } // F14 undo
, { 0x8a, 0x7e, 0, 0 } // F15 redo
, { 0x89, 0x7f, 0, 0 } // F16 new
, { 0xbf, 0x80, 0, 0 } // F17 open
, { 0xaf, 0x81, 0, 0 } // F18 close
, { 0xe4, 0x82, 0, 0 } // F19 reply
, { 0x8e, 0x83, 0, 0 } // F20 forward
, { 0xda, 0x84, 0, 0 } // F21 send
//, { 0x, 0x85, 0, 0 } // F22 check spell (doesn't work for me with ubuntu)
, { 0xd5, 0x86, 0, 0 } // F23 save
, { 0xb9, 0x87, 0, 0 } // 0x2a?? F24 print
        // end of special keys above F1 till F12

, { 234, 0xa6, 0, 0 } // Browser back
, { 233, 0xa7, 0, 0 } // Browser forward
, { 231, 0xa8, 0, 0 } // Browser refresh
//, { , 0xa9 } // Browser stop
, { 122, 0xaa, 0, 0 } // Browser search
, { 0xe5, 0xaa, 0, 0 } // Browser search
, { 230, 0xab, 0, 0 } // Browser favorites
, { 130, 0xac, 0, 0 } // Browser home
, { 0xa0, 0xad, 0, 0 } // Volume mute
, { 0xae, 0xae, 0, 0 } // Volume down
, { 0xb0, 0xaf, 0, 0 } // Volume up
, { 0x99, 0xb0, 0, 0 } // Next track
, { 0x90, 0xb1, 0, 0 } // Prev track
, { 0xa4, 0xb2, 0, 0 } // Stop
, { 0xa2, 0xb3, 0, 0 } // Play_Pause
, { 0xec, 0xb4, 0, 0 } // Launch mail
, { 129, 0xb5, 0, 0 } // Launch media_select
, { 198, 0xb6, 0, 0 } // Launch App1/PC icon
, { 0xa1, 0xb7, 0, 0 } // Launch App2/Calculator
, { 34, 0xba, 0, 0 } // OEM 1: [ on us keyboard
, { 51, 0xbf, 0, 0 } // OEM 2: additional key on european keyboards between enter and ' on us keyboards
, { 47, 0xc0, 0, 0 } // OEM 3: ; on us keyboards
, { 20, 0xdb, 0, 0 } // OEM 4: - on us keyboards (between 0 and =)
, { 49, 0xdc, 0, 0 } // OEM 5: ` on us keyboards (below ESC)
, { 21, 0xdd, 0, 0 } // OEM 6: =??? on us keyboards (between - and backspace)
, { 48, 0xde, 0, 0 } // OEM 7: ' on us keyboards (on right side of ;)
//, { 0, 0xdf, 0, 0 } // OEM 8
, { 94, 0xe2, 0, 0 } // OEM 102: additional key on european keyboards between left shift and z on us keyboards
//, { 0xb2, 0x } // Ubuntu default setting for launch browser
//, { 0x76, 0x } // Ubuntu default setting for launch music player
//, { 0xcc, 0x } // Ubuntu default setting for eject
, { 117, 0x5d, 0, 0 } // right click
};

// OSX defines unicode values for non-printing keys which breaks the key parser, set m_wUnicode
static XBMC_KeyMapping g_mapping_npc[] =
{ { XBMCK_BACKSPACE, 0x08, 0, 0 }
, { XBMCK_TAB, 0x09, 0, 0 }
, { XBMCK_RETURN, 0x0d, 0, 0 }
, { XBMCK_ESCAPE, 0x1b, 0, 0 }
, { XBMCK_SPACE, 0x20, ' ', L' ' }
, { XBMCK_MENU, 0x5d, 0, 0 }
, { XBMCK_KP0, 0x60, 0, 0 }
, { XBMCK_KP1, 0x61, 0, 0 }
, { XBMCK_KP2, 0x62, 0, 0 }
, { XBMCK_KP3, 0x63, 0, 0 }
, { XBMCK_KP4, 0x64, 0, 0 }
, { XBMCK_KP5, 0x65, 0, 0 }
, { XBMCK_KP6, 0x66, 0, 0 }
, { XBMCK_KP7, 0x67, 0, 0 }
, { XBMCK_KP8, 0x68, 0, 0 }
, { XBMCK_KP9, 0x69, 0, 0 }
, { XBMCK_KP_ENTER, 0x6C, 0, 0 }
, { XBMCK_UP, 0x26, 0, 0 }
, { XBMCK_DOWN, 0x28, 0, 0 }
, { XBMCK_LEFT, 0x25, 0, 0 }
, { XBMCK_RIGHT, 0x27, 0, 0 }
, { XBMCK_INSERT, 0x2D, 0, 0 }
, { XBMCK_DELETE, 0x2E, 0, 0 }
, { XBMCK_HOME, 0x24, 0, 0 }
, { XBMCK_END, 0x23, 0, 0 }
, { XBMCK_F1, 0x70, 0, 0 }
, { XBMCK_F2, 0x71, 0, 0 }
, { XBMCK_F3, 0x72, 0, 0 }
, { XBMCK_F4, 0x73, 0, 0 }
, { XBMCK_F5, 0x74, 0, 0 }
, { XBMCK_F6, 0x75, 0, 0 }
, { XBMCK_F7, 0x76, 0, 0 }
, { XBMCK_F8, 0x77, 0, 0 }
, { XBMCK_F9, 0x78, 0, 0 }
, { XBMCK_F10, 0x79, 0, 0 }
, { XBMCK_F11, 0x7a, 0, 0 }
, { XBMCK_F12, 0x7b, 0, 0 }
, { XBMCK_F13, 0x7c, 0, 0 }
, { XBMCK_F14, 0x7d, 0, 0 }
, { XBMCK_F15, 0x7e, 0, 0 }
, { XBMCK_KP_PERIOD, 0, 46, 46 }
, { XBMCK_KP_MULTIPLY,0, 42, 42 }
, { XBMCK_KP_MINUS, 0, 45, 45 }
, { XBMCK_KP_PLUS,0, 43, 43 }
, { XBMCK_KP_DIVIDE,0, 47, 47 }
, { XBMCK_PAGEUP, 0x21, 0, 0 }
, { XBMCK_PAGEDOWN, 0x22, 0, 0 }
, { XBMCK_PRINT, 0x2a, 0, 0 }
, { XBMCK_LSHIFT, 0xa0, 0, 0 }
, { XBMCK_RSHIFT, 0xa1, 0, 0 }
, { XBMCK_PLAYPAUSE, 0xB3, 0, 0 }
, { XBMCK_RMETA, 0xFB, 0, 0 }
, { XBMCK_VOLUME_UP, 0xAF, 0, 0 }
, { XBMCK_VOLUME_DOWN, 0xAE, 0, 0 }
, { XBMCK_MUTE, 0xAD, 0, 0 }
, { XBMCK_HOME_WINDOW, 0xAC, 0, 0 }
, { XBMCK_PLAY, 0xCF, 0, 0 }
, { XBMCK_PAUSE, 0xCD, 0, 0 }
, { XBMCK_FASTFORWARD, 0xD0, 0, 0 }
, { XBMCK_REWIND, 0xD1, 0, 0 }
, { XBMCK_STOP, 0xb2, 0, 0 }
, { XBMCK_BACK, 0xa6, 0, 0 }
};

static bool LookupKeyMapping(BYTE* VKey, char* Ascii, WCHAR* Unicode, int source, XBMC_KeyMapping* map, int count)
{
  for(int i = 0; i < count; i++)
  {
    if(source == map[i].source)
    {
      if(VKey)
        *VKey    = map[i].VKey;
      if(Ascii)
        *Ascii   = map[i].Ascii;
      if(Unicode)
        *Unicode = map[i].Unicode;
      return true;
    }
  }
  return false;
}

CKeyboardStat::CKeyboardStat()
{
  /* Initialize the tables */
  XBMC_ModState = XBMCKMOD_NONE;
  memset((void*)keynames, 0, sizeof(keynames));
  memset(XBMC_KeyState, 0, sizeof(XBMC_KeyState));

  XBMC_EnableKeyRepeat(0, 0);

  XBMC_NoLockKeys = 0;

  /* Fill in the blanks in keynames */
  keynames[XBMCK_BACKSPACE] = "backspace";
  keynames[XBMCK_TAB] = "tab";
  keynames[XBMCK_CLEAR] = "clear";
  keynames[XBMCK_RETURN] = "return";
  keynames[XBMCK_PAUSE] = "pause";
  keynames[XBMCK_ESCAPE] = "escape";
  keynames[XBMCK_SPACE] = "space";
  keynames[XBMCK_EXCLAIM]  = "!";
  keynames[XBMCK_QUOTEDBL]  = "\"";
  keynames[XBMCK_HASH]  = "#";
  keynames[XBMCK_DOLLAR]  = "$";
  keynames[XBMCK_AMPERSAND]  = "&";
  keynames[XBMCK_QUOTE] = "'";
  keynames[XBMCK_LEFTPAREN] = "(";
  keynames[XBMCK_RIGHTPAREN] = ")";
  keynames[XBMCK_ASTERISK] = "*";
  keynames[XBMCK_PLUS] = "+";
  keynames[XBMCK_COMMA] = ",";
  keynames[XBMCK_MINUS] = "-";
  keynames[XBMCK_PERIOD] = ".";
  keynames[XBMCK_SLASH] = "/";
  keynames[XBMCK_0] = "0";
  keynames[XBMCK_1] = "1";
  keynames[XBMCK_2] = "2";
  keynames[XBMCK_3] = "3";
  keynames[XBMCK_4] = "4";
  keynames[XBMCK_5] = "5";
  keynames[XBMCK_6] = "6";
  keynames[XBMCK_7] = "7";
  keynames[XBMCK_8] = "8";
  keynames[XBMCK_9] = "9";
  keynames[XBMCK_COLON] = ":";
  keynames[XBMCK_SEMICOLON] = ";";
  keynames[XBMCK_LESS] = "<";
  keynames[XBMCK_EQUALS] = "=";
  keynames[XBMCK_GREATER] = ">";
  keynames[XBMCK_QUESTION] = "?";
  keynames[XBMCK_AT] = "@";
  keynames[XBMCK_PERCENT] = "%";
  keynames[XBMCK_LEFTBRACKET] = "[";
  keynames[XBMCK_BACKSLASH] = "\\";
  keynames[XBMCK_RIGHTBRACKET] = "]";
  keynames[XBMCK_CARET] = "^";
  keynames[XBMCK_UNDERSCORE] = "_";
  keynames[XBMCK_BACKQUOTE] = "`";
  keynames[XBMCK_a] = "a";
  keynames[XBMCK_b] = "b";
  keynames[XBMCK_c] = "c";
  keynames[XBMCK_d] = "d";
  keynames[XBMCK_e] = "e";
  keynames[XBMCK_f] = "f";
  keynames[XBMCK_g] = "g";
  keynames[XBMCK_h] = "h";
  keynames[XBMCK_i] = "i";
  keynames[XBMCK_j] = "j";
  keynames[XBMCK_k] = "k";
  keynames[XBMCK_l] = "l";
  keynames[XBMCK_m] = "m";
  keynames[XBMCK_n] = "n";
  keynames[XBMCK_o] = "o";
  keynames[XBMCK_p] = "p";
  keynames[XBMCK_q] = "q";
  keynames[XBMCK_r] = "r";
  keynames[XBMCK_s] = "s";
  keynames[XBMCK_t] = "t";
  keynames[XBMCK_u] = "u";
  keynames[XBMCK_v] = "v";
  keynames[XBMCK_w] = "w";
  keynames[XBMCK_x] = "x";
  keynames[XBMCK_y] = "y";
  keynames[XBMCK_z] = "z";
  keynames[XBMCK_DELETE] = "delete";

  keynames[XBMCK_WORLD_0] = "world 0";
  keynames[XBMCK_WORLD_1] = "world 1";
  keynames[XBMCK_WORLD_2] = "world 2";
  keynames[XBMCK_WORLD_3] = "world 3";
  keynames[XBMCK_WORLD_4] = "world 4";
  keynames[XBMCK_WORLD_5] = "world 5";
  keynames[XBMCK_WORLD_6] = "world 6";
  keynames[XBMCK_WORLD_7] = "world 7";
  keynames[XBMCK_WORLD_8] = "world 8";
  keynames[XBMCK_WORLD_9] = "world 9";
  keynames[XBMCK_WORLD_10] = "world 10";
  keynames[XBMCK_WORLD_11] = "world 11";
  keynames[XBMCK_WORLD_12] = "world 12";
  keynames[XBMCK_WORLD_13] = "world 13";
  keynames[XBMCK_WORLD_14] = "world 14";
  keynames[XBMCK_WORLD_15] = "world 15";
  keynames[XBMCK_WORLD_16] = "world 16";
  keynames[XBMCK_WORLD_17] = "world 17";
  keynames[XBMCK_WORLD_18] = "world 18";
  keynames[XBMCK_WORLD_19] = "world 19";
  keynames[XBMCK_WORLD_20] = "world 20";
  keynames[XBMCK_WORLD_21] = "world 21";
  keynames[XBMCK_WORLD_22] = "world 22";
  keynames[XBMCK_WORLD_23] = "world 23";
  keynames[XBMCK_WORLD_24] = "world 24";
  keynames[XBMCK_WORLD_25] = "world 25";
  keynames[XBMCK_WORLD_26] = "world 26";
  keynames[XBMCK_WORLD_27] = "world 27";
  keynames[XBMCK_WORLD_28] = "world 28";
  keynames[XBMCK_WORLD_29] = "world 29";
  keynames[XBMCK_WORLD_30] = "world 30";
  keynames[XBMCK_WORLD_31] = "world 31";
  keynames[XBMCK_WORLD_32] = "world 32";
  keynames[XBMCK_WORLD_33] = "world 33";
  keynames[XBMCK_WORLD_34] = "world 34";
  keynames[XBMCK_WORLD_35] = "world 35";
  keynames[XBMCK_WORLD_36] = "world 36";
  keynames[XBMCK_WORLD_37] = "world 37";
  keynames[XBMCK_WORLD_38] = "world 38";
  keynames[XBMCK_WORLD_39] = "world 39";
  keynames[XBMCK_WORLD_40] = "world 40";
  keynames[XBMCK_WORLD_41] = "world 41";
  keynames[XBMCK_WORLD_42] = "world 42";
  keynames[XBMCK_WORLD_43] = "world 43";
  keynames[XBMCK_WORLD_44] = "world 44";
  keynames[XBMCK_WORLD_45] = "world 45";
  keynames[XBMCK_WORLD_46] = "world 46";
  keynames[XBMCK_WORLD_47] = "world 47";
  keynames[XBMCK_WORLD_48] = "world 48";
  keynames[XBMCK_WORLD_49] = "world 49";
  keynames[XBMCK_WORLD_50] = "world 50";
  keynames[XBMCK_WORLD_51] = "world 51";
  keynames[XBMCK_WORLD_52] = "world 52";
  keynames[XBMCK_WORLD_53] = "world 53";
  keynames[XBMCK_WORLD_54] = "world 54";
  keynames[XBMCK_WORLD_55] = "world 55";
  keynames[XBMCK_WORLD_56] = "world 56";
  keynames[XBMCK_WORLD_57] = "world 57";
  keynames[XBMCK_WORLD_58] = "world 58";
  keynames[XBMCK_WORLD_59] = "world 59";
  keynames[XBMCK_WORLD_60] = "world 60";
  keynames[XBMCK_WORLD_61] = "world 61";
  keynames[XBMCK_WORLD_62] = "world 62";
  keynames[XBMCK_WORLD_63] = "world 63";
  keynames[XBMCK_WORLD_64] = "world 64";
  keynames[XBMCK_WORLD_65] = "world 65";
  keynames[XBMCK_WORLD_66] = "world 66";
  keynames[XBMCK_WORLD_67] = "world 67";
  keynames[XBMCK_WORLD_68] = "world 68";
  keynames[XBMCK_WORLD_69] = "world 69";
  keynames[XBMCK_WORLD_70] = "world 70";
  keynames[XBMCK_WORLD_71] = "world 71";
  keynames[XBMCK_WORLD_72] = "world 72";
  keynames[XBMCK_WORLD_73] = "world 73";
  keynames[XBMCK_WORLD_74] = "world 74";
  keynames[XBMCK_WORLD_75] = "world 75";
  keynames[XBMCK_WORLD_76] = "world 76";
  keynames[XBMCK_WORLD_77] = "world 77";
  keynames[XBMCK_WORLD_78] = "world 78";
  keynames[XBMCK_WORLD_79] = "world 79";
  keynames[XBMCK_WORLD_80] = "world 80";
  keynames[XBMCK_WORLD_81] = "world 81";
  keynames[XBMCK_WORLD_82] = "world 82";
  keynames[XBMCK_WORLD_83] = "world 83";
  keynames[XBMCK_WORLD_84] = "world 84";
  keynames[XBMCK_WORLD_85] = "world 85";
  keynames[XBMCK_WORLD_86] = "world 86";
  keynames[XBMCK_WORLD_87] = "world 87";
  keynames[XBMCK_WORLD_88] = "world 88";
  keynames[XBMCK_WORLD_89] = "world 89";
  keynames[XBMCK_WORLD_90] = "world 90";
  keynames[XBMCK_WORLD_91] = "world 91";
  keynames[XBMCK_WORLD_92] = "world 92";
  keynames[XBMCK_WORLD_93] = "world 93";
  keynames[XBMCK_WORLD_94] = "world 94";
  keynames[XBMCK_WORLD_95] = "world 95";

  keynames[XBMCK_KP0] = "[0]";
  keynames[XBMCK_KP1] = "[1]";
  keynames[XBMCK_KP2] = "[2]";
  keynames[XBMCK_KP3] = "[3]";
  keynames[XBMCK_KP4] = "[4]";
  keynames[XBMCK_KP5] = "[5]";
  keynames[XBMCK_KP6] = "[6]";
  keynames[XBMCK_KP7] = "[7]";
  keynames[XBMCK_KP8] = "[8]";
  keynames[XBMCK_KP9] = "[9]";
  keynames[XBMCK_KP_PERIOD] = "[.]";
  keynames[XBMCK_KP_DIVIDE] = "[/]";
  keynames[XBMCK_KP_MULTIPLY] = "[*]";
  keynames[XBMCK_KP_MINUS] = "[-]";
  keynames[XBMCK_KP_PLUS] = "[+]";
  keynames[XBMCK_KP_ENTER] = "enter";
  keynames[XBMCK_KP_EQUALS] = "equals";

  keynames[XBMCK_UP] = "up";
  keynames[XBMCK_DOWN] = "down";
  keynames[XBMCK_RIGHT] = "right";
  keynames[XBMCK_LEFT] = "left";
  keynames[XBMCK_DOWN] = "down";
  keynames[XBMCK_INSERT] = "insert";
  keynames[XBMCK_HOME] = "home";
  keynames[XBMCK_END] = "end";
  keynames[XBMCK_PAGEUP] = "page up";
  keynames[XBMCK_PAGEDOWN] = "page down";

  keynames[XBMCK_F1] = "f1";
  keynames[XBMCK_F2] = "f2";
  keynames[XBMCK_F3] = "f3";
  keynames[XBMCK_F4] = "f4";
  keynames[XBMCK_F5] = "f5";
  keynames[XBMCK_F6] = "f6";
  keynames[XBMCK_F7] = "f7";
  keynames[XBMCK_F8] = "f8";
  keynames[XBMCK_F9] = "f9";
  keynames[XBMCK_F10] = "f10";
  keynames[XBMCK_F11] = "f11";
  keynames[XBMCK_F12] = "f12";
  keynames[XBMCK_F13] = "f13";
  keynames[XBMCK_F14] = "f14";
  keynames[XBMCK_F15] = "f15";

  keynames[XBMCK_NUMLOCK] = "numlock";
  keynames[XBMCK_CAPSLOCK] = "caps lock";
  keynames[XBMCK_SCROLLOCK] = "scroll lock";
  keynames[XBMCK_RSHIFT] = "right shift";
  keynames[XBMCK_LSHIFT] = "left shift";
  keynames[XBMCK_RCTRL] = "right ctrl";
  keynames[XBMCK_LCTRL] = "left ctrl";
  keynames[XBMCK_RALT] = "right alt";
  keynames[XBMCK_LALT] = "left alt";
  keynames[XBMCK_RMETA] = "right meta";
  keynames[XBMCK_LMETA] = "left meta";
  keynames[XBMCK_LSUPER] = "left super";	/* "Windows" keys */
  keynames[XBMCK_RSUPER] = "right super";	
  keynames[XBMCK_MODE] = "alt gr";
  keynames[XBMCK_COMPOSE] = "compose";

  keynames[XBMCK_HELP] = "help";
  keynames[XBMCK_PRINT] = "print screen";
  keynames[XBMCK_SYSREQ] = "sys req";
  keynames[XBMCK_BREAK] = "break";
  keynames[XBMCK_MENU] = "menu";
  keynames[XBMCK_POWER] = "power";
  keynames[XBMCK_EURO] = "euro";
  keynames[XBMCK_UNDO] = "undo";

#if 0
  // Temporary map for boxee box, should move to xml file
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_q, XBMCKMOD_LALT)] = XBMCK_1;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_w, XBMCKMOD_LALT)] = XBMCK_2;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_e, XBMCKMOD_LALT)] = XBMCK_3;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_r, XBMCKMOD_LALT)] = XBMCK_4;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_t, XBMCKMOD_LALT)] = XBMCK_5;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_y, XBMCKMOD_LALT)] = XBMCK_6;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_u, XBMCKMOD_LALT)] = XBMCK_7;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_i, XBMCKMOD_LALT)] = XBMCK_8;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_o, XBMCKMOD_LALT)] = XBMCK_9;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_p, XBMCKMOD_LALT)] = XBMCK_0;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_a, XBMCKMOD_LALT)] = XBMCK_EXCLAIM;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_s, XBMCKMOD_LALT)] = XBMCK_AT;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_d, XBMCKMOD_LALT)] = XBMCK_HASH;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_f, XBMCKMOD_LALT)] = XBMCK_DOLLAR;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_g, XBMCKMOD_LALT)] = XBMCK_PERCENT;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_h, XBMCKMOD_LALT)] = XBMCK_AMPERSAND;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_j, XBMCKMOD_LALT)] = XBMCK_PERIOD;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_k, XBMCKMOD_LALT)] = XBMCK_LEFTPAREN;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_l, XBMCKMOD_LALT)] = XBMCK_RIGHTPAREN;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_COMMA, XBMCKMOD_LALT)] = XBMCK_SLASH;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_z, XBMCKMOD_LALT)] = XBMCK_UNDERSCORE;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_x, XBMCKMOD_LALT)] = XBMCK_MINUS;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_c, XBMCKMOD_LALT)] = XBMCK_PLUS;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_v, XBMCKMOD_LALT)] = XBMCK_SEMICOLON;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_b, XBMCKMOD_LALT)] = XBMCK_COLON;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_n, XBMCKMOD_LALT)] = XBMCK_QUOTEDBL;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_m, XBMCKMOD_LALT)] = XBMCK_QUOTE;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_PERIOD, XBMCKMOD_LALT)] = XBMCK_QUESTION;
  m_translationMap[std::pair<XBMCKey, XBMCMod>(XBMCK_MENU, XBMCKMOD_NONE)] = XBMCK_ESCAPE;
#endif

  Reset();
  m_lastKey = XBMCK_UNKNOWN;
  m_keyHoldTime = 0;
  m_bEvdev = true;
}

CKeyboardStat::~CKeyboardStat()
{
}

void CKeyboardStat::Initialize()
{
/* this stuff probably doesn't belong here  *
 * but in some x11 specific WinEvents file  *
 * but for some reason the code to map keys *
 * to specific xbmc vkeys is here           */
#if defined(HAS_X11_EVENTS)
  Display* dpy = XOpenDisplay(NULL);
  if (!dpy)
    return;

  XkbDescPtr desc;
  char* symbols;

  desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
  if(!desc)
  {
    XCloseDisplay(dpy);
    return;
  }

  symbols = XGetAtomName(dpy, desc->names->symbols);
  if(symbols)
  {
    CLog::Log(LOGDEBUG, "CKeyboardStat::Initialize - XKb symbols %s", symbols);
    if(strstr(symbols, "(evdev)"))
      m_bEvdev = true;
  }

  XFree(symbols);
  XkbFreeKeyboard(desc, XkbAllComponentsMask, True);
  XCloseDisplay(dpy);
#endif
}

void CKeyboardStat::Reset()
{
  m_bShift = false;
  m_bCtrl = false;
  m_bAlt = false;
  m_bRAlt = false;
  m_cAscii = '\0';
  m_wUnicode = '\0';
  m_VKey = 0;
  XBMC_ModState = XBMCKMOD_NONE;

  ZeroMemory(XBMC_KeyState, sizeof(XBMC_KeyState));
}

void CKeyboardStat::ResetState()
{
  Reset();
}

unsigned int CKeyboardStat::KeyHeld() const
{
  return m_keyHoldTime;
}

int CKeyboardStat::HandleEvent(XBMC_Event& newEvent)
{
  // First, let's see if we need to translate this event.
  if (m_translationMap.find(std::pair<XBMCKey, XBMCMod>(newEvent.key.keysym.sym, XBMC_ModState)) != m_translationMap.end())
  {
    XBMCKey translatedKey = m_translationMap.find(std::pair<XBMCKey, XBMCMod>(newEvent.key.keysym.sym, XBMC_ModState))->second;
    newEvent.key.keysym.sym = translatedKey;
    newEvent.key.keysym.unicode = translatedKey;
  }

  int repeatable;
  uint16_t modstate;

  /* Set up the keysym */
  XBMC_keysym *keysym = &newEvent.key.keysym;
  modstate = (uint16_t)XBMC_ModState;

  repeatable = 0;

  int state;
  if(newEvent.type == XBMC_KEYDOWN)
    state = XBMC_PRESSED;
  else if(newEvent.type == XBMC_KEYUP)
    state = XBMC_RELEASED;
  else
    return 0;

  if ( state == XBMC_PRESSED ) 
  {
    keysym->mod = (XBMCMod)modstate;
    switch (keysym->sym) 
    {
      case XBMCK_UNKNOWN:
        break;
      case XBMCK_NUMLOCK:
        modstate ^= XBMCKMOD_NUM;
        if ( XBMC_NoLockKeys & XBMC_NLK_NUM )
          break;
        if ( ! (modstate&XBMCKMOD_NUM) )
          state = XBMC_RELEASED;
        keysym->mod = (XBMCMod)modstate;
        break;
      case XBMCK_CAPSLOCK:
        modstate ^= XBMCKMOD_CAPS;
        if ( XBMC_NoLockKeys & XBMC_NLK_CAPS )
          break;
        if ( ! (modstate&XBMCKMOD_CAPS) )
          state = XBMC_RELEASED;
        keysym->mod = (XBMCMod)modstate;
        break;
      case XBMCK_LCTRL:
        modstate |= XBMCKMOD_LCTRL;
        break;
      case XBMCK_RCTRL:
        modstate |= XBMCKMOD_RCTRL;
        break;
      case XBMCK_LSHIFT:
        modstate |= XBMCKMOD_LSHIFT;
        break;
      case XBMCK_RSHIFT:
        modstate |= XBMCKMOD_RSHIFT;
        break;
      case XBMCK_LALT:
        modstate |= XBMCKMOD_LALT;
        break;
      case XBMCK_RALT:
        modstate |= XBMCKMOD_RALT;
        break;
      case XBMCK_LMETA:
        modstate |= XBMCKMOD_LMETA;
        break;
#ifndef HAS_INTELCE
    // for YouTube leanback - we'll use this key to switch between keyboard and mouse modes
      case XBMCK_RMETA:
        modstate |= XBMCKMOD_RMETA;
        break;
#endif
      case XBMCK_MODE:
        modstate |= XBMCKMOD_MODE;
        break;
      default:
        repeatable = 1;
        break;
    }
  } 
  else 
  {
    switch (keysym->sym) 
    {
      case XBMCK_UNKNOWN:
        break;
      case XBMCK_NUMLOCK:
        if ( XBMC_NoLockKeys & XBMC_NLK_NUM )
          break;
        /* Only send keydown events */
        return(0);
      case XBMCK_CAPSLOCK:
        if ( XBMC_NoLockKeys & XBMC_NLK_CAPS )
          break;
        /* Only send keydown events */
        return(0);
      case XBMCK_LCTRL:
        modstate &= ~XBMCKMOD_LCTRL;
        break;
      case XBMCK_RCTRL:
        modstate &= ~XBMCKMOD_RCTRL;
        break;
      case XBMCK_LSHIFT:
        modstate &= ~XBMCKMOD_LSHIFT;
        break;
      case XBMCK_RSHIFT:
        modstate &= ~XBMCKMOD_RSHIFT;
        break;
      case XBMCK_LALT:
        modstate &= ~XBMCKMOD_LALT;
        break;
      case XBMCK_RALT:
        modstate &= ~XBMCKMOD_RALT;
        break;
      case XBMCK_LMETA:
        modstate &= ~XBMCKMOD_LMETA;
        break;
      case XBMCK_RMETA:
        modstate &= ~XBMCKMOD_RMETA;
        break;
      case XBMCK_MODE:
        modstate &= ~XBMCKMOD_MODE;
        break;
      default:
        break;
    }
    keysym->mod = (XBMCMod)modstate;
  }

  if(state == XBMC_RELEASED)
  if ( XBMC_KeyRepeat.timestamp &&
    XBMC_KeyRepeat.evt.key.keysym.sym == keysym->sym ) 
  {
    XBMC_KeyRepeat.timestamp = 0;
  }

  if ( keysym->sym != XBMCK_UNKNOWN ) 
  {
    /* Update internal keyboard state */
    XBMC_ModState = (XBMCMod)modstate;
    XBMC_KeyState[keysym->sym] = state;
  }
  
  newEvent.key.state = state;
  Update(newEvent);
  
  return 0;
}

void CKeyboardStat::Update(XBMC_Event& event)
{
  if (event.type == XBMC_KEYDOWN)
  {
    unsigned int now = CTimeUtils::GetFrameTime();
    if (m_lastKey == event.key.keysym.sym)
      m_keyHoldTime += now - m_lastKeyTime;
    else
      m_keyHoldTime = 0;
    m_lastKey = event.key.keysym.sym;
    m_lastKeyTime = now;

    m_cAscii = 0;
    m_VKey = 0;

    m_wUnicode = event.key.keysym.unicode;

    m_bCtrl = (event.key.keysym.mod & XBMCKMOD_CTRL) != 0;
    m_bShift = (event.key.keysym.mod & XBMCKMOD_SHIFT) != 0;
    m_bAlt = (event.key.keysym.mod & XBMCKMOD_ALT) != 0;
    m_bRAlt = (event.key.keysym.mod & XBMCKMOD_RALT) != 0;

    CLog::Log(LOGDEBUG, "CKeyboardStat, SDLKeyboard: scancode: %d, sym: %d, unicode: %d, modifier: %x", event.key.keysym.scancode, event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod);

    if ((event.key.keysym.unicode >= 'A' && event.key.keysym.unicode <= 'Z') ||
      (event.key.keysym.unicode >= 'a' && event.key.keysym.unicode <= 'z'))
    {
      m_cAscii = (char)event.key.keysym.unicode;
      m_VKey = toupper(m_cAscii);
    }
    else if (event.key.keysym.unicode >= '0' && event.key.keysym.unicode <= '9')
    {
      m_cAscii = (char)event.key.keysym.unicode;
      m_VKey = 0x60 + m_cAscii - '0'; // xbox keyboard routine appears to return 0x60->69 (unverified). Ideally this "fixing"
      // should be done in xbox routine, not in the sdl/directinput routines.
      // we should just be using the unicode/ascii value in all routines (perhaps with some
      // headroom for modifier keys?)
    }
    else
    {
      // see comment above about the weird use of m_VKey here...
      if (event.key.keysym.unicode == ')') { m_VKey = 0x60; m_cAscii = ')'; }
      else if (event.key.keysym.unicode == '!') { m_VKey = 0x61; m_cAscii = '!'; }
      else if (event.key.keysym.unicode == '@') { m_VKey = 0x62; m_cAscii = '@'; }
      else if (event.key.keysym.unicode == '#') { m_VKey = 0x63; m_cAscii = '#'; }
      else if (event.key.keysym.unicode == '$') { m_VKey = 0x64; m_cAscii = '$'; }
      else if (event.key.keysym.unicode == '%') { m_VKey = 0x65; m_cAscii = '%'; }
      else if (event.key.keysym.unicode == '^') { m_VKey = 0x66; m_cAscii = '^'; }
      else if (event.key.keysym.unicode == '&') { m_VKey = 0x67; m_cAscii = '&'; }
      else if (event.key.keysym.unicode == '*') { m_VKey = 0x68; m_cAscii = '*'; }
      else if (event.key.keysym.unicode == '(') { m_VKey = 0x69; m_cAscii = '('; }
      else if (event.key.keysym.unicode == ':') { m_VKey = 0xba; m_cAscii = ':'; }
      else if (event.key.keysym.unicode == ';') { m_VKey = 0xba; m_cAscii = ';'; }
      else if (event.key.keysym.unicode == '=') { m_VKey = 0xbb; m_cAscii = '='; }
      else if (event.key.keysym.unicode == '+') { m_VKey = 0xbb; m_cAscii = '+'; }
      else if (event.key.keysym.unicode == '<') { m_VKey = 0xbc; m_cAscii = '<'; }
      else if (event.key.keysym.unicode == ',') { m_VKey = 0xbc; m_cAscii = ','; }
      else if (event.key.keysym.unicode == '-') { m_VKey = 0xbd; m_cAscii = '-'; }
      else if (event.key.keysym.unicode == '_') { m_VKey = 0xbd; m_cAscii = '_'; }
      else if (event.key.keysym.unicode == '>') { m_VKey = 0xbe; m_cAscii = '>'; }
      else if (event.key.keysym.unicode == '.') { m_VKey = 0xbe; m_cAscii = '.'; }
      else if (event.key.keysym.unicode == '?') { m_VKey = 0xbf; m_cAscii = '?'; } // 0xbf is OEM 2 Why is it assigned here?
      else if (event.key.keysym.unicode == '/') { m_VKey = 0xbf; m_cAscii = '/'; }
      else if (event.key.keysym.unicode == '~') { m_VKey = 0xc0; m_cAscii = '~'; }
      else if (event.key.keysym.unicode == '`') { m_VKey = 0xc0; m_cAscii = '`'; }
      else if (event.key.keysym.unicode == '{') { m_VKey = 0xeb; m_cAscii = '{'; }
      else if (event.key.keysym.unicode == '[') { m_VKey = 0xeb; m_cAscii = '['; } // 0xeb is not defined by MS. Why is it assigned here?
      else if (event.key.keysym.unicode == '|') { m_VKey = 0xec; m_cAscii = '|'; }
      else if (event.key.keysym.unicode == '\\') { m_VKey = 0xec; m_cAscii = '\\'; }
      else if (event.key.keysym.unicode == '}') { m_VKey = 0xed; m_cAscii = '}'; }
      else if (event.key.keysym.unicode == ']') { m_VKey = 0xed; m_cAscii = ']'; } // 0xed is not defined by MS. Why is it assigned here?
      else if (event.key.keysym.unicode == '"') { m_VKey = 0xee; m_cAscii = '"'; }
      else if (event.key.keysym.unicode == '\'') { m_VKey = 0xee; m_cAscii = '\''; }


      /* Check for standard non printable keys */
      if (!m_VKey && !m_cAscii)
        LookupKeyMapping(&m_VKey, NULL, &m_wUnicode
                       , event.key.keysym.sym
                       , g_mapping_npc
                       , sizeof(g_mapping_npc)/sizeof(g_mapping_npc[0]));


      if (!m_VKey && !m_cAscii)
      {
        /* Check for linux defined non printable keys */
        if(m_bEvdev)
          LookupKeyMapping(&m_VKey, NULL, NULL
                         , event.key.keysym.scancode
                         , g_mapping_evdev
                         , sizeof(g_mapping_evdev)/sizeof(g_mapping_evdev[0]));
        else
          LookupKeyMapping(&m_VKey, NULL, NULL
                         , event.key.keysym.scancode
                         , g_mapping_evdev
                         , sizeof(g_mapping_ubuntu)/sizeof(g_mapping_ubuntu[0]));
      }

      if (!m_VKey && !m_cAscii)
      {
        if (event.key.keysym.mod & XBMCKMOD_LSHIFT) m_VKey = 0xa0;
        else if (event.key.keysym.mod & XBMCKMOD_RSHIFT) m_VKey = 0xa1;
        else if (event.key.keysym.mod & XBMCKMOD_LALT) m_VKey = 0xa4;
        else if (event.key.keysym.mod & XBMCKMOD_RALT) m_VKey = 0xa5;
        else if (event.key.keysym.mod & XBMCKMOD_LCTRL) m_VKey = 0xa2;
        else if (event.key.keysym.mod & XBMCKMOD_RCTRL) m_VKey = 0xa3;
        else if (event.key.keysym.unicode > 32 && event.key.keysym.unicode < 128)
          // only TRUE ASCII! (Otherwise XBMC crashes! No unicode not even latin 1!)
          m_cAscii = (char)(event.key.keysym.unicode & 0xff);
      }
    }
  }
  else
  { // key up event
    Reset();
    m_lastKey = XBMCK_UNKNOWN;
    m_keyHoldTime = 0;
  }
}

char CKeyboardStat::GetAscii()
{
  char lowLevelAscii = m_cAscii;
  int translatedAscii = GetUnicode();

#ifdef DEBUG_KEYBOARD_GETCHAR
  CLog::Log(LOGDEBUG, "low level ascii: %c ", lowLevelAscii);
  CLog::Log(LOGDEBUG, "low level ascii code: %d ", lowLevelAscii);
  CLog::Log(LOGDEBUG, "result char: %c ", translatedAscii);
  CLog::Log(LOGDEBUG, "result char code: %d ", translatedAscii);
  CLog::Log(LOGDEBUG, "ralt is pressed bool: %d ", GetRAlt());
  CLog::Log(LOGDEBUG, "shift is pressed bool: %d ", GetShift());
#endif

  if (translatedAscii >= 0 && translatedAscii < 128) // only TRUE ASCII! Otherwise XBMC crashes! No unicode not even latin 1!
    return translatedAscii; // mapping to ASCII is supported only if the result is TRUE ASCII
  else
    return lowLevelAscii; // old style
}

WCHAR CKeyboardStat::GetUnicode()
{
  // More specific mappings, i.e. with scancodes and/or with one or even more modifiers,
  // must be handled first/prioritized over less specific mappings! Why?
  // Example: an us keyboard has: "]" on one key, the german keyboard has "+" on the same key,
  // additionally the german keyboard has "~" on the same key, but the "~"
  // can only be reached with the special modifier "AltGr" (right alt).
  // See http://en.wikipedia.org/wiki/Keyboard_layout.
  // If "+" is handled first, the key is already consumed and "~" can never be reached.
  // The least specific mappings, e.g. "regardless modifiers" should be done at last/least prioritized.

  WCHAR lowLevelUnicode = m_wUnicode;
  BYTE key = m_VKey;

#ifdef DEBUG_KEYBOARD_GETCHAR
  CLog::Log(LOGDEBUG, "low level unicode char: %c ", lowLevelUnicode);
  CLog::Log(LOGDEBUG, "low level unicode code: %d ", lowLevelUnicode);
  CLog::Log(LOGDEBUG, "low level vkey: %d ", key);
  CLog::Log(LOGDEBUG, "ralt is pressed bool: %d ", GetRAlt());
  CLog::Log(LOGDEBUG, "shift is pressed bool: %d ", GetShift());
#endif

  if (GetRAlt())
  {
    if (g_keyboardLayoutConfiguration.containsDeriveXbmcCharFromVkeyWithRalt(key))
    {
      WCHAR resultUnicode = g_keyboardLayoutConfiguration.valueOfDeriveXbmcCharFromVkeyWithRalt(key);
#ifdef DEBUG_KEYBOARD_GETCHAR
      CLog::Log(LOGDEBUG, "derived with ralt to code: %d ", resultUnicode);
#endif
      return resultUnicode;
    }
  }

  if (GetShift())
  {
    if (g_keyboardLayoutConfiguration.containsDeriveXbmcCharFromVkeyWithShift(key))
    {
      WCHAR resultUnicode = g_keyboardLayoutConfiguration.valueOfDeriveXbmcCharFromVkeyWithShift(key);
#ifdef DEBUG_KEYBOARD_GETCHAR
      CLog::Log(LOGDEBUG, "derived with shift to code: %d ", resultUnicode);
#endif
      return resultUnicode;
    }
  }

  if (g_keyboardLayoutConfiguration.containsDeriveXbmcCharFromVkeyRegardlessModifiers(key))
  {
    WCHAR resultUnicode = g_keyboardLayoutConfiguration.valueOfDeriveXbmcCharFromVkeyRegardlessModifiers(key);
#ifdef DEBUG_KEYBOARD_GETCHAR
    CLog::Log(LOGDEBUG, "derived to code: %d ", resultUnicode);
#endif
    return resultUnicode;
  }

  if (GetRAlt())
  {
    if (g_keyboardLayoutConfiguration.containsChangeXbmcCharWithRalt(lowLevelUnicode))
    {
      WCHAR resultUnicode = g_keyboardLayoutConfiguration.valueOfChangeXbmcCharWithRalt(lowLevelUnicode);
#ifdef DEBUG_KEYBOARD_GETCHAR
      CLog::Log(LOGDEBUG, "changed char with ralt to code: %d ", resultUnicode);
#endif
      return resultUnicode;
    };
  }

  if (g_keyboardLayoutConfiguration.containsChangeXbmcCharRegardlessModifiers(lowLevelUnicode))
  {
    WCHAR resultUnicode = g_keyboardLayoutConfiguration.valueOfChangeXbmcCharRegardlessModifiers(lowLevelUnicode);
#ifdef DEBUG_KEYBOARD_GETCHAR
    CLog::Log(LOGDEBUG, "changed char to code: %d ", resultUnicode);
#endif
    return resultUnicode;
  };

  return lowLevelUnicode;
}

XBMCMod CKeyboardStat::GetModState() const
{
  return XBMC_ModState;
}

void CKeyboardStat::SetModState(const XBMCMod& modState)
{
  XBMC_ModState = modState;
}

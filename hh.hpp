// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'hh.pas' rev: 6.00

#ifndef hhHPP
#define hhHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Registry.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Hh
{
//-- type declarations -------------------------------------------------------
#pragma pack(push, 1)
struct tagHH_LAST_ERROR
{
	int cbStruct;
	int hr;
	wchar_t *description;
} ;
#pragma pack(pop)

typedef tagHH_LAST_ERROR  HH_LAST_ERROR;

typedef tagHH_LAST_ERROR  THHLastError;

struct tagHHN_NOTIFY;
typedef tagHHN_NOTIFY *PHHNNotify;

#pragma pack(push, 1)
struct tagHHN_NOTIFY
{
	tagNMHDR hdr;
	char *pszUrl;
} ;
#pragma pack(pop)

typedef tagHHN_NOTIFY  HHN_NOTIFY;

typedef tagHHN_NOTIFY  THHNNotify;

struct tagHH_POPUP;
typedef tagHH_POPUP *PHHPopup;

#pragma pack(push, 1)
struct tagHH_POPUP
{
	int cbStruct;
	unsigned hinst;
	unsigned idString;
	char *pszText;
	Types::TPoint pt;
	unsigned clrForeground;
	unsigned clrBackground;
	Types::TRect rcMargins;
	char *pszFont;
} ;
#pragma pack(pop)

typedef tagHH_POPUP  HH_POPUP;

typedef tagHH_POPUP  THHPopup;

struct tagHH_AKLINK;
typedef tagHH_AKLINK *PHHAKLink;

#pragma pack(push, 1)
struct tagHH_AKLINK
{
	int cbStruct;
	BOOL fReserved;
	char *pszKeywords;
	char *pszUrl;
	char *pszMsgText;
	char *pszMsgTitle;
	char *pszWindow;
	BOOL fIndexOnFail;
} ;
#pragma pack(pop)

typedef tagHH_AKLINK  HH_AKLINK;

typedef tagHH_AKLINK  THHAKLink;

struct tagHH_ENUM_IT;
typedef tagHH_ENUM_IT *PHHEnumIT;

#pragma pack(push, 1)
struct tagHH_ENUM_IT
{
	int cbStruct;
	int iType;
	char *pszCatName;
	char *pszITName;
	char *pszITDescription;
} ;
#pragma pack(pop)

typedef tagHH_ENUM_IT  THHEnumIT;

struct tagHH_ENUM_CAT;
typedef tagHH_ENUM_CAT *PHHEnumCat;

#pragma pack(push, 1)
struct tagHH_ENUM_CAT
{
	int cbStruct;
	char *pszCatName;
	char *pszCatDescription;
} ;
#pragma pack(pop)

typedef tagHH_ENUM_CAT  THHEnumCat;

struct tagHH_SET_INFOTYPE;
typedef tagHH_SET_INFOTYPE *PHHSetInfoType;

#pragma pack(push, 1)
struct tagHH_SET_INFOTYPE
{
	int cbStruct;
	char *pszCatName;
	char *pszInfoTypeName;
} ;
#pragma pack(pop)

typedef tagHH_SET_INFOTYPE  THHSetInfoType;

typedef unsigned HH_INFOTYPE;

typedef unsigned THHInfoType;

typedef unsigned *PHHInfoType;

struct tagHH_FTS_QUERY;
typedef tagHH_FTS_QUERY *PHHFtsQuery;

#pragma pack(push, 1)
struct tagHH_FTS_QUERY
{
	int cbStruct;
	BOOL fUniCodeStrings;
	char *pszSearchQuery;
	int iProximity;
	BOOL fStemmedSearch;
	BOOL fTitleOnly;
	BOOL fExecute;
	char *pszWindow;
} ;
#pragma pack(pop)

typedef tagHH_FTS_QUERY  THHFtsQuery;

struct tagHH_WINTYPE;
typedef tagHH_WINTYPE *PHHWinType;

#pragma pack(push, 1)
struct tagHH_WINTYPE
{
	int cbStruct;
	BOOL fUniCodeStrings;
	char *pszType;
	unsigned fsValidMembers;
	unsigned fsWinProperties;
	char *pszCaption;
	unsigned dwStyles;
	unsigned dwExStyles;
	Types::TRect rcWindowPos;
	int nShowState;
	HWND hwndHelp;
	HWND hwndCaller;
	unsigned *paInfoTypes;
	HWND hwndToolBar;
	HWND hwndNavigation;
	HWND hwndHTML;
	int iNavWidth;
	Types::TRect rcHTML;
	char *pszToc;
	char *pszIndex;
	char *pszFile;
	char *pszHome;
	unsigned fsToolBarFlags;
	BOOL fNotExpanded;
	int curNavType;
	int tabpos;
	int idNotify;
	Byte tabOrder[20];
	int cHistory;
	char *pszJump1;
	char *pszJump2;
	char *pszUrlJump1;
	char *pszUrlJump2;
	Types::TRect rcMinSize;
	int cbInfoTypes;
	char *pszCustomTabs;
} ;
#pragma pack(pop)

typedef tagHH_WINTYPE  HH_WINTYPE;

typedef tagHH_WINTYPE  THHWinType;

struct tagHHNTRACK;
typedef tagHHNTRACK *PHHNTrack;

#pragma pack(push, 1)
struct tagHHNTRACK
{
	tagNMHDR hdr;
	char *pszCurUrl;
	int idAction;
	tagHH_WINTYPE *phhWinType;
} ;
#pragma pack(pop)

typedef tagHHNTRACK  HHNTRACK;

typedef tagHHNTRACK  THHNTrack;

typedef Shortint tagHH_GPROPID;

typedef tagHH_GPROPID HH_GPROPID;

typedef tagHH_GPROPID THHGPropID;

struct tagHH_GLOBAL_PROPERTY;
typedef tagHH_GLOBAL_PROPERTY *PHHGlobalProperty;

struct tagHH_GLOBAL_PROPERTY
{
	tagHH_GPROPID id;
	int Dummy;
	Variant var_;
} ;

typedef tagHH_GLOBAL_PROPERTY  HH_GLOBAL_PROPERTY;

typedef tagHH_GLOBAL_PROPERTY  THHGlobalProperty;

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE unsigned HHCtrlHandle;
extern PACKAGE bool AutoLoadAPI;
#define hhctrlLib "hhctrl.ocx"
extern PACKAGE HWND __stdcall (*HtmlHelpA)(HWND hwndCaller, char * pszFile, unsigned uCommand, unsigned dwData);
extern PACKAGE HWND __stdcall (*HtmlHelpW)(HWND hwndCaller, wchar_t * pszFile, unsigned uCommand, unsigned dwData);
extern PACKAGE HWND __stdcall (*HtmlHelp)(HWND hwndCaller, char * pszFile, unsigned uCommand, unsigned dwData);
static const Shortint ATOM_HTMLHELP_API_ANSI = 0xe;
static const Shortint ATOM_HTMLHELP_API_UNICODE = 0xf;
static const Shortint HH_DISPLAY_TOPIC = 0x0;
static const Shortint HH_HELP_FINDER = 0x0;
static const Shortint HH_DISPLAY_TOC = 0x1;
static const Shortint HH_DISPLAY_INDEX = 0x2;
static const Shortint HH_DISPLAY_SEARCH = 0x3;
static const Shortint HH_SET_WIN_TYPE = 0x4;
static const Shortint HH_GET_WIN_TYPE = 0x5;
static const Shortint HH_GET_WIN_HANDLE = 0x6;
static const Shortint HH_ENUM_INFO_TYPE = 0x7;
static const Shortint HH_SET_INFO_TYPE = 0x8;
static const Shortint HH_SYNC = 0x9;
static const Shortint HH_RESERVED1 = 0xa;
static const Shortint HH_RESERVED2 = 0xb;
static const Shortint HH_RESERVED3 = 0xc;
static const Shortint HH_KEYWORD_LOOKUP = 0xd;
static const Shortint HH_DISPLAY_TEXT_POPUP = 0xe;
static const Shortint HH_HELP_CONTEXT = 0xf;
static const Shortint HH_TP_HELP_CONTEXTMENU = 0x10;
static const Shortint HH_TP_HELP_WM_HELP = 0x11;
static const Shortint HH_CLOSE_ALL = 0x12;
static const Shortint HH_ALINK_LOOKUP = 0x13;
static const Shortint HH_GET_LAST_ERROR = 0x14;
static const Shortint HH_ENUM_CATEGORY = 0x15;
static const Shortint HH_ENUM_CATEGORY_IT = 0x16;
static const Shortint HH_RESET_IT_FILTER = 0x17;
static const Shortint HH_SET_INCLUSIVE_FILTER = 0x18;
static const Shortint HH_SET_EXCLUSIVE_FILTER = 0x19;
static const Shortint HH_INITIALIZE = 0x1c;
static const Shortint HH_UNINITIALIZE = 0x1d;
static const Byte HH_PRETRANSLATEMESSAGE = 0xfd;
static const Byte HH_SET_GLOBAL_PROPERTY = 0xfc;
static const Shortint HHWIN_PROP_TAB_AUTOHIDESHOW = 0x1;
static const Shortint HHWIN_PROP_ONTOP = 0x2;
static const Shortint HHWIN_PROP_NOTITLEBAR = 0x4;
static const Shortint HHWIN_PROP_NODEF_STYLES = 0x8;
static const Shortint HHWIN_PROP_NODEF_EXSTYLES = 0x10;
static const Shortint HHWIN_PROP_TRI_PANE = 0x20;
static const Shortint HHWIN_PROP_NOTB_TEXT = 0x40;
static const Byte HHWIN_PROP_POST_QUIT = 0x80;
static const Word HHWIN_PROP_AUTO_SYNC = 0x100;
static const Word HHWIN_PROP_TRACKING = 0x200;
static const Word HHWIN_PROP_TAB_SEARCH = 0x400;
static const Word HHWIN_PROP_TAB_HISTORY = 0x800;
static const Word HHWIN_PROP_TAB_FAVORITES = 0x1000;
static const Word HHWIN_PROP_CHANGE_TITLE = 0x2000;
static const Word HHWIN_PROP_NAV_ONLY_WIN = 0x4000;
static const Word HHWIN_PROP_NO_TOOLBAR = 0x8000;
static const int HHWIN_PROP_MENU = 0x10000;
static const int HHWIN_PROP_TAB_ADVSEARCH = 0x20000;
static const int HHWIN_PROP_USER_POS = 0x40000;
static const int HHWIN_PROP_TAB_CUSTOM1 = 0x80000;
static const int HHWIN_PROP_TAB_CUSTOM2 = 0x100000;
static const int HHWIN_PROP_TAB_CUSTOM3 = 0x200000;
static const int HHWIN_PROP_TAB_CUSTOM4 = 0x400000;
static const int HHWIN_PROP_TAB_CUSTOM5 = 0x800000;
static const int HHWIN_PROP_TAB_CUSTOM6 = 0x1000000;
static const int HHWIN_PROP_TAB_CUSTOM7 = 0x2000000;
static const int HHWIN_PROP_TAB_CUSTOM8 = 0x4000000;
static const int HHWIN_PROP_TAB_CUSTOM9 = 0x8000000;
static const int HHWIN_TB_MARGIN = 0x10000000;
static const Shortint HHWIN_PARAM_PROPERTIES = 0x2;
static const Shortint HHWIN_PARAM_STYLES = 0x4;
static const Shortint HHWIN_PARAM_EXSTYLES = 0x8;
static const Shortint HHWIN_PARAM_RECT = 0x10;
static const Shortint HHWIN_PARAM_NAV_WIDTH = 0x20;
static const Shortint HHWIN_PARAM_SHOWSTATE = 0x40;
static const Byte HHWIN_PARAM_INFOTYPES = 0x80;
static const Word HHWIN_PARAM_TB_FLAGS = 0x100;
static const Word HHWIN_PARAM_EXPANSION = 0x200;
static const Word HHWIN_PARAM_TABPOS = 0x400;
static const Word HHWIN_PARAM_TABORDER = 0x800;
static const Word HHWIN_PARAM_HISTORY_COUNT = 0x1000;
static const Word HHWIN_PARAM_CUR_TAB = 0x2000;
static const Shortint HHWIN_BUTTON_EXPAND = 0x2;
static const Shortint HHWIN_BUTTON_BACK = 0x4;
static const Shortint HHWIN_BUTTON_FORWARD = 0x8;
static const Shortint HHWIN_BUTTON_STOP = 0x10;
static const Shortint HHWIN_BUTTON_REFRESH = 0x20;
static const Shortint HHWIN_BUTTON_HOME = 0x40;
static const Byte HHWIN_BUTTON_BROWSE_FWD = 0x80;
static const Word HHWIN_BUTTON_BROWSE_BCK = 0x100;
static const Word HHWIN_BUTTON_NOTES = 0x200;
static const Word HHWIN_BUTTON_CONTENTS = 0x400;
static const Word HHWIN_BUTTON_SYNC = 0x800;
static const Word HHWIN_BUTTON_OPTIONS = 0x1000;
static const Word HHWIN_BUTTON_PRINT = 0x2000;
static const Word HHWIN_BUTTON_INDEX = 0x4000;
static const Word HHWIN_BUTTON_SEARCH = 0x8000;
static const int HHWIN_BUTTON_HISTORY = 0x10000;
static const int HHWIN_BUTTON_FAVORITES = 0x20000;
static const int HHWIN_BUTTON_JUMP1 = 0x40000;
static const int HHWIN_BUTTON_JUMP2 = 0x80000;
static const int HHWIN_BUTTON_ZOOM = 0x100000;
static const int HHWIN_BUTTON_TOC_NEXT = 0x200000;
static const int HHWIN_BUTTON_TOC_PREV = 0x400000;
static const Word HHWIN_DEF_BUTTONS = 0x3006;
static const Byte IDTB_EXPAND = 0xc8;
static const Byte IDTB_CONTRACT = 0xc9;
static const Byte IDTB_STOP = 0xca;
static const Byte IDTB_REFRESH = 0xcb;
static const Byte IDTB_BACK = 0xcc;
static const Byte IDTB_HOME = 0xcd;
static const Byte IDTB_SYNC = 0xce;
static const Byte IDTB_PRINT = 0xcf;
static const Byte IDTB_OPTIONS = 0xd0;
static const Byte IDTB_FORWARD = 0xd1;
static const Byte IDTB_NOTES = 0xd2;
static const Byte IDTB_BROWSE_FWD = 0xd3;
static const Byte IDTB_BROWSE_BACK = 0xd4;
static const Byte IDTB_CONTENTS = 0xd5;
static const Byte IDTB_INDEX = 0xd6;
static const Byte IDTB_SEARCH = 0xd7;
static const Byte IDTB_HISTORY = 0xd8;
static const Byte IDTB_FAVORITES = 0xd9;
static const Byte IDTB_JUMP1 = 0xda;
static const Byte IDTB_JUMP2 = 0xdb;
static const Byte IDTB_CUSTOMIZE = 0xdd;
static const Byte IDTB_ZOOM = 0xde;
static const Byte IDTB_TOC_NEXT = 0xdf;
static const Byte IDTB_TOC_PREV = 0xe0;
static const short HHN_FIRST = 0xfffffca4;
static const short HHN_LAST = 0xfffffc91;
static const short HHN_NAVCOMPLETE = 0xfffffca4;
static const short HHN_TRACK = 0xfffffca3;
static const short HHN_WINDOW_CREATE = 0xfffffca2;
static const Shortint HHWIN_NAVTYPE_TOC = 0x0;
static const Shortint HHWIN_NAVTYPE_INDEX = 0x1;
static const Shortint HHWIN_NAVTYPE_SEARCH = 0x2;
static const Shortint HHWIN_NAVTYPE_FAVORITES = 0x3;
static const Shortint HHWIN_NAVTYPE_HISTORY = 0x4;
static const Shortint HHWIN_NAVTYPE_AUTHOR = 0x5;
static const Shortint HHWIN_NAVTYPE_CUSTOM_FIRST = 0xb;
static const Shortint IT_INCLUSIVE = 0x0;
static const Shortint IT_EXCLUSIVE = 0x1;
static const Shortint IT_HIDDEN = 0x2;
static const Shortint HHWIN_NAVTAB_TOP = 0x0;
static const Shortint HHWIN_NAVTAB_LEFT = 0x1;
static const Shortint HHWIN_NAVTAB_BOTTOM = 0x2;
static const Shortint HH_MAX_TABS = 0x13;
static const Shortint HH_TAB_CONTENTS = 0x0;
static const Shortint HH_TAB_INDEX = 0x1;
static const Shortint HH_TAB_SEARCH = 0x2;
static const Shortint HH_TAB_FAVORITES = 0x3;
static const Shortint HH_TAB_HISTORY = 0x4;
static const Shortint HH_TAB_AUTHOR = 0x5;
static const Shortint HH_TAB_CUSTOM_FIRST = 0xb;
static const Shortint HH_TAB_CUSTOM_LAST = 0x13;
static const Shortint HH_MAX_TABS_CUSTOM = 0x9;
static const Shortint HH_FTS_DEFAULT_PROXIMITY = 0xffffffff;
static const Shortint HHACT_TAB_CONTENTS = 0x0;
static const Shortint HHACT_TAB_INDEX = 0x1;
static const Shortint HHACT_TAB_SEARCH = 0x2;
static const Shortint HHACT_TAB_HISTORY = 0x3;
static const Shortint HHACT_TAB_FAVORITES = 0x4;
static const Shortint HHACT_EXPAND = 0x5;
static const Shortint HHACT_CONTRACT = 0x6;
static const Shortint HHACT_BACK = 0x7;
static const Shortint HHACT_FORWARD = 0x8;
static const Shortint HHACT_STOP = 0x9;
static const Shortint HHACT_REFRESH = 0xa;
static const Shortint HHACT_HOME = 0xb;
static const Shortint HHACT_SYNC = 0xc;
static const Shortint HHACT_OPTIONS = 0xd;
static const Shortint HHACT_PRINT = 0xe;
static const Shortint HHACT_HIGHLIGHT = 0xf;
static const Shortint HHACT_CUSTOMIZE = 0x10;
static const Shortint HHACT_JUMP1 = 0x11;
static const Shortint HHACT_JUMP2 = 0x12;
static const Shortint HHACT_ZOOM = 0x13;
static const Shortint HHACT_TOC_NEXT = 0x14;
static const Shortint HHACT_TOC_PREV = 0x15;
static const Shortint HHACT_NOTES = 0x16;
static const Shortint HHACT_LAST_ENUM = 0x17;
static const Shortint HH_GPROPID_SINGLETHREAD = 0x1;
static const Shortint HH_GPROPID_TOOLBAR_MARGIN = 0x2;
static const Shortint HH_GPROPID_UI_LANGUAGE = 0x3;
static const Shortint HH_GPROPID_CURRENT_SUBSET = 0x4;
static const Shortint HH_GPROPID_CONTENT_LANGUAGE = 0x5;
extern PACKAGE AnsiString __fastcall GetPathToHHCtrlOCX();
extern PACKAGE void __fastcall LoadHtmlHelp(void);
extern PACKAGE void __fastcall UnloadHtmlHelp(void);

}	/* namespace Hh */
using namespace Hh;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// hh

/*****************************************************************************
 * DeskList.cpp
 *----------------------------------------------------------------------------
 *
 ****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/
#define DUMMYUNIONNAME

#include <windows.h>
#include <winuser.h>
#include <shlobj.h>
#include <commctrl.h>
#include <stdio.h>

#include "DeskLibs.h"
#include "resource.h"

/*****************************************************************************
 * Definitions
 ****************************************************************************/
/* Launch Directory */
//#define LAUNCH_DIR			"Launch\\"

/* Setting definitions */
#define CLASS_NAME			  "DeskList"
#define SETTING_SECTION_GENERAL "General"
#define SETTING_KEY_STYLE		"Style"
#define SETTING_KEY_LAUNCH_DIR	"LaunchDir"
#define SETTING_KEY_STYLE_EX	"StyleEx"
#define SETTING_KEY_STYLE_ST	"StyleSt"
#define SETTING_KEY_LAYOUT_ST   "LayoutSt"
#define SETTING_KEY_HOOKDSKGRID "HookDskGrid"
#define SETTING_KEY_HOOKSHLGRID "HookShlGrid"
#define SETTING_KEY_HOOKSHLBACK "HookShlBack"
#define SETTING_KEY_HOOKDSKKICK "HookDskKick"

/* Message definitions */
#define WM_TRAY_ACTION	(WM_USER+0)
#define WM_LIST_G		(WM_USER+1)
#define WM_LIST_M		(WM_USER+2)
#define WM_LIST_L		(WM_USER+3)
#define WM_LIST_D		(WM_USER+4)
#define WM_LIST_H		(WM_USER+5)
#define WM_HIDE_HEADER  (WM_USER+6)
#define WM_HIDE_SCROLL  (WM_USER+7)
#define WM_DESKHOOK		(WM_USER+100)
#define WID_TRAY		100

/* Platform definitions */
#define IsWindows95( id )		   (id == VER_PLATFORM_WIN32_WINDOWS)
#define IsWindowsNT( id )		   (id == VER_PLATFORM_WIN32_NT)

/* ListView definitions */
#define IsTypeIcon( style )			((style & LVS_TYPEMASK) == LVS_ICON)
#define IsTypeSmallIcon( style )	((style & LVS_TYPEMASK) == LVS_SMALLICON)
#define IsTypeList( style )			((style & LVS_TYPEMASK) == LVS_LIST)
#define IsTypeReport( style )		((style & LVS_TYPEMASK) == LVS_REPORT)
#define IsTypeThumb( style )		((style & LVS_TYPEMASK) == LVS_SHAREIMAGELISTS)

#define IsVisibleWindow( style )	((style | WS_VISIBLE) == style)
#define IsHiddenHeader( style )		((style | HDS_HIDDEN) == style)
#define IsNoScroll( style )			((style | LVS_NOSCROLL) == style)
#define IsNoColumnHeader( style )	((style & LVS_TYPESTYLEMASK) == LVS_NOCOLUMNHEADER)
#define IsNoSortHeader( style )		((style & LVS_TYPESTYLEMASK) == LVS_NOSORTHEADER)
#define IsGridLines( style )		((style | LVS_EX_GRIDLINES) == style)
#define IsSubItemImages( style )	((style | LVS_EX_SUBITEMIMAGES) == style)
#define IsCheckBoxes( style )		((style | LVS_EX_CHECKBOXES) == style)
#define IsHeaderDragDrop( style)	((style | LVS_EX_HEADERDRAGDROP) == style)
#define IsFullRowSelect( style )	((style | LVS_EX_FULLROWSELECT) == style)
#define IsTrackSelect( style )		((style | LVS_EX_TRACKSELECT) == style)
#define IsOneClickActivate( style )	((style | LVS_EX_ONECLICKACTIVATE) == style)
#define IsTwoClickActivate( style )	((style | LVS_EX_TWOCLICKACTIVATE) == style)
#define IsFlatSB( style )			((style | LVS_EX_FLATSB) == style)
#define IsRegional( style )			((style | LVS_EX_REGIONAL) == style)
#define IsInfoTip( style )			((style | LVS_EX_INFOTIP) == style)
#define IsUnderLineHot( style )		((style | LVS_EX_UNDERLINEHOT) == style)
#define IsUnderLineCold( style )	((style | LVS_EX_UNDERLINECOLD) == style)
#define IsMultiWorkAreas( style )	((style | LVS_EX_MULTIWORKAREAS) == style)

/* type definitions */
typedef LPVOID	(WINAPI* DefVirtualAllocEx)		(HANDLE, LPVOID , DWORD, DWORD, DWORD);
typedef BOOL	(WINAPI* DefVirtualFreeEx)		(HANDLE, LPVOID , DWORD, DWORD);
typedef BOOL	(WINAPI* DefSetLayeredWindowAttributes)	(HWND,COLORREF,BYTE,DWORD);
typedef BOOL	(WINAPI* DefSHGetPathFromIDList)	(LPCITEMIDLIST, LPSTR);
typedef HRESULT	(WINAPI* DefSHGetDesktopFolder)	(LPSHELLFOLDER*);
typedef BOOL	(WINAPI* DefShell_NotifyIcon)	(DWORD, PNOTIFYICONDATA);
typedef BOOL	(WINAPI* DefShellExecuteEx)		(LPSHELLEXECUTEINFO);
typedef DWORD	(WINAPI* DefSHGetFileInfo)		(LPCTSTR, DWORD, SHFILEINFO*, UINT, UINT);
typedef HRESULT	(* DefSHGetMalloc)				(LPMALLOC*);
typedef VOID    (WINAPI* DefSHGetSettings)     (LPSHELLFLAGSTATE, DWORD);

typedef BOOL	(* DefInstallHook)				(HWND);
typedef BOOL	(* DefUnInstallHook)			(VOID);
typedef BOOL	(* DefHookGet)					(VOID);
typedef VOID	(* DefHookSet)				  	(BOOL);
typedef BOOL	(* DefDeskListExecute)			(BOOL, CHAR*);
typedef BOOL	(* DefDeskListGetPath)			(CHAR*, INT);

#define HAVE_NOMODULES			  0
#define HAVE_HOOKFUNCS			  1
#define HAVE_VIRTFUNCS			  2
#define haveHookFuncs( module )	 ((module | HAVE_HOOKFUNCS) == module)
#define haveVirtualEx( module )	 ((module | HAVE_VIRTFUNCS) == module)

/*****************************************************************************
 * Prototypes
 ****************************************************************************/
LRESULT CALLBACK	WndProc( HWND, UINT, WPARAM, LPARAM );
ATOM				InitApplication( HINSTANCE );
BOOL				InitInstance( HINSTANCE, INT );
CHAR*				GetIniPath( HINSTANCE );
VOID				SaveIconLayout( HWND );
VOID				LoadIconLayout( HWND );
VOID				RefreshWindow( HWND );
DWORD				LoadModules( VOID );
VOID				FreeModules( VOID );
VOID				StyleChange( HWND, DWORD, DWORD );
VOID				ShowTaskTrayIcon( HWND, PNOTIFYICONDATA );
HMENU				CreateTaskTrayConfigMenu( VOID );
INT					ShowTaskTrayConfigMenu( HMENU, HWND, HWND );
HMENU				CreateTaskTrayLaunchMenu( VOID );
INT					ShowTaskTrayLaunchMenu( HMENU, HWND );
BOOL				NotifyTaskTrayLaunchMenu( HMENU, DWORD, HWND );
DWORD				GetWindowsVersion( VOID );
CHAR*				GetLaunchPath( VOID );
BOOL				FillDesktopMenu( HWND, HMENU );

BOOL				GetSetting( CHAR*, DWORD* );
BOOL				SetSetting( CHAR*, DWORD  );

//BOOL				FillDesktopMenu( HWND, HMENU );

/*****************************************************************************
 * Variables
 ****************************************************************************/
CHAR		szClassName[] = CLASS_NAME;	/* window class name       */
HINSTANCE	hInstance = NULL;			/* instance of application */
HHOOK		hHook = NULL;				/* handle of hook          */
DWORD		hasModule = 0;				/* flag of having modules  */
DWORD		dwPlatformID = 0;			/* id of platform          */
CHAR		*pIniFile = NULL;			/* path to profile         */

BOOL	bLoadStyle = FALSE;				/* flag of loading style  */
BOOL	bLoadLayout = FALSE;			/* flag of loading layout */

DefVirtualAllocEx	pVirtualAllocEx = NULL;		/* pointer of VirtualAllocEx  */
DefVirtualFreeEx	pVirtualFreeEx = NULL;		/* pointer of VirtualFreeEx   */
DefInstallHook		pInstallHook = NULL;		/* pointer of InstallHook     */
DefUnInstallHook	pUnInstallHook = NULL;		/* pointer of UnInstalHook    */
DefDeskListExecute	pDeskListExecute = NULL;	/* pointer of DeskListExecute */
DefDeskListGetPath	pDeskListGetPath = NULL;	/* pointer of DeskListGetPath */

DefHookSet	pSetHookDskGrid = NULL;	/* pointer of SetHookDskGrid */
DefHookGet	pGetHookDskGrid = NULL;	/* pointer of GetHookDskGrid */
DefHookSet	pSetHookShlGrid = NULL;	/* pointer of SetHookShlGrid */
DefHookGet	pGetHookShlGrid = NULL;	/* pointer of GetHookShlGrid */
DefHookSet	pSetHookShlBack = NULL;	/* pointer of SetHookShlBack */
DefHookGet	pGetHookShlBack = NULL;	/* pointer of GetHookShlBack */
DefHookSet	pSetHookDskKick = NULL;	/* pointer of SetHookDskKick */
DefHookGet	pGetHookDskKick = NULL;	/* pointer of GetHookDskKick */

//DefSetLayeredWindowAttributes	pSetLayeredWindowAttributes = NULL;
DefShell_NotifyIcon		pShell_NotifyIcon = NULL;
DefSHGetDesktopFolder	pSHGetDesktopFolder = NULL;
DefShellExecuteEx		pShellExecuteEx = NULL;
DefSHGetFileInfo		pSHGetFileInfo = NULL;
DefSHGetMalloc			pSHGetMalloc = NULL;
DefSHGetPathFromIDList	pSHGetPathFromIDList = NULL;
DefSHGetSettings		pSHGetSettings = NULL;

HMODULE		hModuleKernel32 = NULL;	/* handle of module          */
HMODULE		hModuleDeskHook = NULL;	/* handle of module          */
HMODULE		hModuleShell32  = NULL; /* handle of module          */

static BOOL				haveHeader = FALSE;	/* flag of having header	*/
static BOOL				haveScroll = FALSE;	/* flag of having scrollbar */
static UINT	LastMsg = WM_LIST_G;

/*****************************************************************************
 * Function GetWindowsVersion
 ****************************************************************************/
DWORD GetWindowsVersion( VOID )
{
	OSVERSIONINFO  osInfo;	/* version information of OS */

	osInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &osInfo );
	return osInfo.dwPlatformId;
}

/*****************************************************************************
 * Function WinMain
 ****************************************************************************/
INT WINAPI WinMain( HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, INT nCmdShow )
{
	MSG message;	/* window message */ 

	/* copy handle of instance */
	hInstance = hCurInst;

	/* search modules */
	hasModule = LoadModules();

	/* get platform-id */
	dwPlatformID = GetWindowsVersion();

	/* get path to profile */
	pIniFile = GetIniPath( hInstance );
	if (!pIniFile) hasModule &= ~HAVE_VIRTFUNCS;

	/* initialize application */
	if (!InitApplication( hInstance )) return FALSE;

	/* initialize instance */
	if (!InitInstance( hInstance, SW_HIDE )) return FALSE;

	/* message loop */
	while (GetMessage( &message, NULL, 0, 0 ))
	{
		TranslateMessage( &message );
		DispatchMessage( &message );
	}

	/* free memory */
	if (pIniFile) free( pIniFile );

	/* free modules */
	FreeModules();

	return message.wParam;
}

/*****************************************************************************
 * Function InitApplication
 ****************************************************************************/
ATOM InitApplication( HINSTANCE hInst )
{
	WNDCLASSEX wc;	/* class of window */
	wc.cbSize		= sizeof(WNDCLASSEX);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	 = hInst;
	wc.hIcon		 = LoadIcon( hInst, MAKEINTRESOURCE(IDI_DESKLIST) );
	wc.hCursor	   = LoadCursor( hInst, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = (LPCSTR)szClassName;
	wc.hIconSm	   = LoadIcon( hInst, MAKEINTRESOURCE(IDI_DESKLIST) );
	return (RegisterClassEx( &wc ));
}

/*****************************************************************************
 * Function InitInstance
 ****************************************************************************/
BOOL InitInstance( HINSTANCE hInst, INT nCmdShow )
{
	HWND hWnd;	/* handle of window */

	/* create hidden window */
	hWnd = CreateWindow( szClassName, "DeskList", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			50, 50,
			NULL, NULL, hInst, NULL );
	if (!hWnd) return FALSE;
	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );
	return TRUE;
}

/*****************************************************************************
 * Function ShowTaskTrayIcon
 ****************************************************************************/
VOID ShowTaskTrayIcon( HWND hWnd, PNOTIFYICONDATA lpni )
{
	HICON hIcon;	/* handle of icon */

	/* load icon for task-tary */
	if (pShell_NotifyIcon)
	{
		hIcon = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_DESKLIST) );
		lpni->cbSize			= sizeof(NOTIFYICONDATA);
		lpni->hIcon				= hIcon;
		lpni->hWnd				= hWnd;
		lpni->uCallbackMessage	= WM_TRAY_ACTION;
		lpni->uFlags			= NIF_ICON | NIF_MESSAGE | NIF_TIP;
		lpni->uID				= WID_TRAY;
		strcpy( lpni->szTip, szClassName );
		pShell_NotifyIcon( NIM_ADD, lpni );
	}
	return;
}

/*****************************************************************************
 * Function CreateTaskTrayLaunchMenu
 ****************************************************************************/
HMENU CreateTaskTrayLaunchMenu( VOID )
{
	HMENU			hMenu;

	/* load menu from resource */
	hMenu = LoadMenu( hInstance, MAKEINTRESOURCE(IDM_DESKLAUNCH) );

	return hMenu;
}

/*****************************************************************************
 * Function GetLaunchDirectory
 ****************************************************************************/
CHAR* GetLaunchPath( VOID )
{
#ifndef LAUNCH_DIR
	CHAR  chrProfileBuf[_MAX_PATH]; /* buffer for profile-pos */
	DWORD dwRet;				 /* return value for API   */ 
#else
	CHAR *pStr;	/* source path */
	CHAR *pDir;	/* search path */
#endif
	CHAR *pPath;   /* dest path   */

	pPath = (CHAR*)malloc( _MAX_PATH );
	if(pPath == NULL) return NULL;
#ifndef LAUNCH_DIR
	memset( chrProfileBuf, 0x00, sizeof(chrProfileBuf) );
	dwRet = GetPrivateProfileString( SETTING_SECTION_GENERAL, SETTING_KEY_LAUNCH_DIR, NULL,
		chrProfileBuf, sizeof(chrProfileBuf), pIniFile );
	if(dwRet == 0)
		return NULL;
	strcpy( pPath, chrProfileBuf );
#else
	GetModuleFileName( hInst, pPath, _MAX_PATH - 1 );
	pStr = pPath;
	while(*pStr++ != 0)
	{
		if(IsDBCSLeadByte(*pStr) == TRUE) continue;
		if(*pStr == '\\') pDir = pStr;
	}
	*pDir = 0;
	strcat( pPath, "\\" );
	strcat( pPath, LAUNCH_DIR );
#endif
	if(pPath[strlen(pPath)-1] != '\\')
		strcat( pPath, "\\" );
	return pPath;
}

/*****************************************************************************
 * Function ShowTaskTrayLaunchMenu
 ****************************************************************************/
INT ShowTaskTrayLaunchMenu( HMENU hMenu, HWND hWnd )
{
	HMENU				hSubMenu;		/* handle of sub-menu	*/
	POINT				pt;				/* mouse point			*/

	/* get mouse point */
	GetCursorPos( &pt );

	/* get sub-menu handle */
	hSubMenu = GetSubMenu( hMenu, 0 );

	/* make destop-icons */
	FillDesktopMenu( hWnd, hSubMenu );

	/* force foreground */
	SetForegroundWindow( hWnd );

	/* show task-tray menu */
	TrackPopupMenu( hSubMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y,
		0, hWnd, NULL );
	//TrackPopupMenuEx( hSubMenu, TPM_VERTICAL, pt.x, pt.y,
	//	hWnd, NULL );

	/* message through */
	PostMessage( hWnd, WM_NULL, 0, 0 );
	return 0;
}

/*****************************************************************************
 * Function CreateTaskTrayConfigMenu
 ****************************************************************************/
HMENU CreateTaskTrayConfigMenu( VOID )
{
#define ADD_ODM( mid, iid ) OwnerDrawSetItemList( &itemList[itemCount++], mid, LoadIcon( hInstance, MAKEINTRESOURCE( iid  ) ), NULL )
	HMENU		hMenu;
	INT		  itemCount = 0;
	CONVERT_ITEMLIST	itemList[13];

	/* load menu from resource */
	hMenu = LoadMenu( hInstance, MAKEINTRESOURCE(IDM_DESKLIST) );

	ADD_ODM( IDM_LIST_G,      IDI_LIST_G  );
	ADD_ODM( IDM_LIST_M,      IDI_LIST_M  );
	ADD_ODM( IDM_LIST_L,      IDI_LIST_L  );
	ADD_ODM( IDM_LIST_D,      IDI_LIST_D  );
	ADD_ODM( IDM_REFRESH,     IDI_REFRESH );
	ADD_ODM( IDM_LAYOUTLOAD,  IDI_LOAD    );
	ADD_ODM( IDM_LAYOUTSAVE,  IDI_SAVE    );
	ADD_ODM( IDM_LAYOUTSTART, IDI_START   );
	ADD_ODM( IDM_STYLELOAD,   IDI_LOAD    );
	ADD_ODM( IDM_STYLESAVE,   IDI_SAVE    );
	ADD_ODM( IDM_STYLESTART,  IDI_START   );
	ADD_ODM( IDM_EXIT,        IDI_EXIT    );
	ADD_ODM( 0,               0           );
	OwnerDrawConvertItem( hInstance, hMenu, itemList );

	return hMenu;
}

/*****************************************************************************
 * Function ShowTaskTrayConfigMenu
 ****************************************************************************/
INT ShowTaskTrayConfigMenu( HMENU hMenu, HWND hWnd, HWND hWndDesk )
{
	HMENU		hSubMenu;		/* handle of sub-menu		*/
	HWND		hWndHead;		/* handle of header-window	*/
	POINT		pt;				/* mouse point				*/
	UINT		uItem = 0;		/* item number				*/
	UINT		uValue;			/* general value			*/
	DWORD		dwDeskStyle;	/* style					*/
	DWORD		dwDeskStyleEx;	/* extension style			*/
	LVCOLUMN	lvColumn;

	/* get mouse point */
	GetCursorPos( &pt );

	/* get sub-menu handle */
	hSubMenu = GetSubMenu( hMenu, 0 );

	/* get header-control */
	hWndHead = FindWindowEx( hWndDesk, 0, "SysHeader32", NULL );

	/* get style of desktop */
	dwDeskStyle   = GetWindowLong( hWndDesk, GWL_STYLE );
	dwDeskStyleEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );

	if (!IsWindows95(dwPlatformID))
	{
		/* style of view */
		CheckMenuItem( hMenu, IDM_LIST_G, MF_BYCOMMAND | MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_LIST_M, MF_BYCOMMAND | MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_LIST_L, MF_BYCOMMAND | MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_LIST_D, MF_BYCOMMAND | MF_UNCHECKED );
		CheckMenuItem( hMenu, IDM_LIST_H, MF_BYCOMMAND | MF_UNCHECKED );
		if (IsTypeIcon(dwDeskStyle))		uItem = IDM_LIST_G;
		if (IsTypeSmallIcon(dwDeskStyle))	uItem = IDM_LIST_M;
		if (IsTypeList(dwDeskStyle))		uItem = IDM_LIST_L;
		if (IsTypeReport(dwDeskStyle))		uItem = IDM_LIST_D;
		if (IsTypeThumb(dwDeskStyle))		uItem = IDM_LIST_H;
		CheckMenuItem( hMenu, uItem, MF_BYCOMMAND | MF_CHECKED );

		/* header visible */
		uValue = (IsNoColumnHeader(dwDeskStyle) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HIDE_HEADER, MF_BYCOMMAND | uValue );
		uValue = (uItem == IDM_LIST_D ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem( hMenu, IDM_HIDE_HEADER, MF_BYCOMMAND | uValue );

		/* scrollbar visible */
		uValue = (IsNoScroll(dwDeskStyle) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HIDE_SCROLL, MF_BYCOMMAND | uValue );
		uValue = (uItem == IDM_LIST_D ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem( hMenu, IDM_HIDE_SCROLL, MF_BYCOMMAND | uValue );

		uValue = (haveHeader == TRUE ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem( hMenu, IDM_LIST_D, MF_BYCOMMAND | uValue );
	}
	else
	{
		switch(LastMsg)
		{
		case WM_LIST_G:
		case WM_LIST_M:
		case WM_LIST_L:
			uValue = MF_GRAYED;
			break;
		case WM_LIST_D:
			uValue = MF_ENABLED;
			break;
		}
		EnableMenuItem( hMenu, IDM_HIDE_HEADER, MF_BYCOMMAND | uValue );
		EnableMenuItem( hMenu, IDM_HIDE_SCROLL, MF_BYCOMMAND | uValue );
	}
	//uValue = (haveHeader == TRUE ? MF_ENABLED : MF_GRAYED);
	//EnableMenuItem( hMenu, IDM_LIST_L, MF_BYCOMMAND | uValue );
	//uValue = (haveHeader == TRUE ? MF_ENABLED : MF_GRAYED);
	//EnableMenuItem( hMenu, IDM_LIST_D, MF_BYCOMMAND | uValue );

	/* grid line */
	uValue = (IsGridLines(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_GRIDLINES, MF_BYCOMMAND | uValue );

	/* subitem images */
	uValue = (IsSubItemImages(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_SUBITEMIMAGES, MF_BYCOMMAND | uValue );

	/* check box */
	uValue = (IsCheckBoxes(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_CHECKBOXES, MF_BYCOMMAND | uValue );

	/* header drag&drop */
	uValue = (IsHeaderDragDrop(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_HEADERDRAGDROP, MF_BYCOMMAND | uValue );

	/* full row select */
	uValue = (IsFullRowSelect(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_FULLROWSELECT, MF_BYCOMMAND | uValue );

	/* track select */
	uValue = (IsTrackSelect(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_TRACKSELECT, MF_BYCOMMAND | uValue );

	/* one click activate */
	uValue = (IsOneClickActivate(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_ONECLICKACTIVATE, MF_BYCOMMAND | uValue );

	/* two click activate */
	uValue = (IsTwoClickActivate(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_TWOCLICKACTIVATE, MF_BYCOMMAND | uValue );

	/* flat scrollbar */
	uValue = (IsFlatSB(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_FLATSB, MF_BYCOMMAND | uValue );

	/* regional */
	uValue = (IsRegional(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_REGIONAL, MF_BYCOMMAND | uValue );

	/* info tip */
	uValue = (IsInfoTip(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_INFOTIP, MF_BYCOMMAND | uValue );

	/* underline hot */
	uValue = (IsUnderLineHot(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_UNDERLINEHOT, MF_BYCOMMAND | uValue );

	/* underline cold */
	uValue = (IsUnderLineCold(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_UNDERLINECOLD, MF_BYCOMMAND | uValue );

	/* multi work areas */
	uValue = (IsMultiWorkAreas(dwDeskStyleEx) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_MULTIWORKAREAS, MF_BYCOMMAND | uValue );

	/* enable hook funcs */
	if (haveHookFuncs(hasModule))
	{
		uValue = (pGetHookDskGrid() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HOOKDSKGRID, MF_BYCOMMAND | uValue );
		uValue = (pGetHookDskGrid() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HOOKSHLGRID, MF_BYCOMMAND | uValue );
		uValue = (pGetHookShlBack() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HOOKSHLBACK, MF_BYCOMMAND | uValue );
		uValue = (pGetHookDskKick() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem( hMenu, IDM_HOOKDSKKICK, MF_BYCOMMAND | uValue );
	}
	else
	{
		EnableMenuItem( hMenu, IDM_HOOKDSKGRID, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, IDM_HOOKSHLGRID, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, IDM_HOOKSHLBACK, MF_BYCOMMAND | MF_GRAYED );
		EnableMenuItem( hMenu, IDM_HOOKDSKKICK, MF_BYCOMMAND | MF_GRAYED );
	}

	/* enable load and save */
	uValue = (haveVirtualEx(hasModule) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem( hMenu, IDM_LAYOUTLOAD,  MF_BYCOMMAND | uValue );
	EnableMenuItem( hMenu, IDM_LAYOUTSAVE,  MF_BYCOMMAND | uValue );
	EnableMenuItem( hMenu, IDM_LAYOUTSTART, MF_BYCOMMAND | uValue );

	uValue = (bLoadLayout ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_LAYOUTSTART, MF_BYCOMMAND | uValue );

	uValue = (bLoadStyle ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem( hMenu, IDM_STYLESTART, MF_BYCOMMAND | uValue );

	/* force foreground */
	SetForegroundWindow( hWnd );

	/* show task-tray menu */
	TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN, pt.x, pt.y,
		TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, hWnd, NULL );
	/* message through */
	PostMessage( hWnd, WM_NULL, 0, 0 );
	return 0;
}

/*****************************************************************************
 * Function NotifyTaskTrayLaunchMenu
 ****************************************************************************/
BOOL NotifyTaskTrayLaunchMenu( HMENU hMenu, DWORD dwCommand, HWND hWnd )
{
	MENUITEMINFO miif;
	ODMENUITEM   *pomi;

	if (!pShellExecuteEx)
		return FALSE;
	miif.cbSize		= sizeof(MENUITEMINFO);
	miif.fMask		= MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_CHECKMARKS | MIIM_DATA | MIIM_SUBMENU;
	miif.dwTypeData = 0;
	miif.cch		= 0;
	//miif.dwItemData	= 0;
	if (GetMenuItemInfo( hMenu, dwCommand, FALSE, &miif ))
	{
		if ((miif.fType & MF_SEPARATOR) == MF_SEPARATOR) return FALSE;
		pomi = (ODMENUITEM*)miif.dwItemData;
		if(!pomi) return FALSE;

		if (pSHGetDesktopFolder)
		{
			CHAR	szPath[MAX_PATH];
			SHELLEXECUTEINFOA sei;
			sei.cbSize = sizeof(SHELLEXECUTEINFOA);
			sei.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS;
			sei.hwnd = hWnd;
			sei.lpVerb = NULL;
			sei.lpFile = NULL;
			sei.lpParameters = NULL;
			sei.lpDirectory = NULL;
			sei.nShow = SW_SHOWNORMAL;
			sei.hInstApp = NULL;
			sei.lpClass = NULL;
			sei.hkeyClass = 0;
			sei.dwHotKey = 0;
			sei.hIcon = NULL;
			sei.lpIDList = pomi->pExInfo;

			if (!pSHGetPathFromIDList( pomi->pExInfo, szPath ))
				sei.lpVerb = "open";
			else
				sei.lpVerb = "explore";
			return pShellExecuteEx(&sei);
		}
		else
			return pDeskListExecute( TRUE, pomi->pszText );
	}
	return FALSE;
}

/*****************************************************************************
 * Function RefreshWindow
 ****************************************************************************/
VOID RefreshWindow( HWND hWnd )
{
	/* make rect invalid */
	InvalidateRect( hWnd, NULL, TRUE );
	/* force event WM_PAINT */
	SendMessage( hWnd, WM_PAINT, 0, 0 );
}

/*****************************************************************************
 * Function StyleChange
 ****************************************************************************/
VOID StyleChange( HWND hWnd, DWORD StyleOld, DWORD StyleNew )
{
	STYLESTRUCT ss;	/* style structure */

	/* set old-style and new-style */
	ss.styleOld = StyleOld;
	ss.styleNew = StyleNew;
	SendMessage( hWnd, WM_STYLECHANGED, (WPARAM)GWL_STYLE, (LPARAM)&ss );
	SetWindowLong( hWnd, GWL_STYLE, StyleNew );
}

/*****************************************************************************
 * Function WndProc
 ****************************************************************************/
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
	static HWND				hWndDesk;			/* handle of desktop-window	*/
	//static HWND				hWndTaskbar;		/* handle of taskbar-window */
	static HMENU			hMenuConfig;		/* handle of config-menu	*/
	static HMENU			hMenuLaunch;		/* handle of launch-menu	*/
	static NOTIFYICONDATA	ni;					/* infomation of task-tray  */
	static DWORD			dwDeskStyleOrg;		/* original style			*/
	static DWORD			dwDeskStyleExOrg;	/* original extension style	*/
	static DWORD			dwDeskStyleOld;	 	/* old style				*/
	static DWORD			dwDeskStyleNew;	 	/* new style				*/
	static DWORD			dwDeskStyleOldEx;	/* old extension style		*/
	static DWORD			dwDeskStyleNewEx;	/* new extension style		*/
	DWORD					dwSetting;			/* setting value			*/
	BOOL					bHookSet;			/* setting of hook-funcs	*/
//	CHAR					TipMessage[256];	/* tool-tip message			*/

	switch (msg)
	{
	case WM_CREATE:
		hWndDesk = FindWindow( "Progman", NULL );
		hWndDesk = FindWindowEx( hWndDesk, 0, "SHELLDLL_DefView", NULL );
		hWndDesk = FindWindowEx( hWndDesk, 0, "SysListView32", NULL );
		if (hWndDesk == INVALID_HANDLE_VALUE)
			SendMessage( hWnd, WM_CLOSE, 0, 0 );
//		hWndTaskbar = FindWindow( "Shell_TrayWnd", NULL );
//		if (hWndTaskbar == INVALID_HANDLE_VALUE)
//			SendMessage( hWnd, WM_CLOSE, 0, 0 );
//		pSetLayeredWindowAttributes = (DefSetLayeredWindowAttributes)GetProcAddress( GetModuleHandle("USER32.DLL"), "SetLayeredWindowAttributes" );
//		if (pSetLayeredWindowAttributes == NULL)
//			SendMessage( hWnd, WM_CLOSE, 0, 0 );
//		pSetLayeredWindowAttributes( hWndTaskbar, 0, 50, 0x00000002 );
		//UpdateWindow( hWndDesk );

		dwDeskStyleOrg = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleExOrg = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
		ShowTaskTrayIcon( hWnd, &ni );

		//haveHeader = (IsNoColumnHeader(dwDeskStyleOrg) ? FALSE : TRUE);
		haveHeader = SendMessage( hWndDesk, LVM_GETCOLUMNWIDTH, 0, 0 );
		haveScroll = (IsNoScroll(dwDeskStyleOrg)       ? FALSE : TRUE);

		hMenuConfig = CreateTaskTrayConfigMenu();
		hMenuLaunch = CreateTaskTrayLaunchMenu();

		if (haveHookFuncs(hasModule))
		{
			pInstallHook( hWnd );
			if (GetSetting( SETTING_KEY_HOOKDSKGRID, &dwSetting ))
				pSetHookDskGrid( (BOOL)dwSetting );
			if (GetSetting( SETTING_KEY_HOOKSHLGRID, &dwSetting ))
				pSetHookShlGrid( (BOOL)dwSetting );
			if (GetSetting( SETTING_KEY_HOOKSHLBACK, &dwSetting ))
				pSetHookShlBack( (BOOL)dwSetting );
			if (GetSetting( SETTING_KEY_HOOKDSKKICK, &dwSetting ))
				pSetHookDskKick( (BOOL)dwSetting );
		}

		if (GetSetting( SETTING_KEY_STYLE_ST, &dwSetting ))
		{
			bLoadStyle = (BOOL)dwSetting;
			if (bLoadStyle)
			{
				SendMessage( hWnd, WM_COMMAND, IDM_STYLELOAD, 0 );
			}
		}

		if (GetSetting( SETTING_KEY_LAYOUT_ST, &dwSetting ))
		{
			bLoadLayout = (BOOL)dwSetting;
			if (bLoadStyle)
			{
				SendMessage( hWnd, WM_COMMAND, IDM_LAYOUTLOAD, 0 );
			}
		}
		if(pSHGetSettings)
		{
			SHELLFLAGSTATE	sfs;
			memset( &sfs, 0x00, sizeof(sfs) );
			pSHGetSettings( &sfs, SSF_DESKTOPHTML );
			if(sfs.fDesktopHTML)
			{
				char ErrMsg[256];
				LoadString( hInstance, IDS_ACTIVEDESKTOP, ErrMsg, sizeof(ErrMsg) );
				MessageBox( 0, ErrMsg, CLASS_NAME, MB_OK | MB_ICONSTOP );
				SendMessage( hWnd, WM_CLOSE, 0, 0 );
			}
		}
		break;
	case WM_TRAY_ACTION:
		SendMessage( hWnd, WM_NULL, 0, 0 );
		if (wp == WID_TRAY)
		{
			switch (lp)
			{
			case WM_RBUTTONUP:
				ShowTaskTrayConfigMenu( hMenuConfig, hWnd, hWndDesk );
				break;
			case WM_LBUTTONUP:
				if (hMenuLaunch)
				{
					OwnerDrawDestroyItem( hMenuLaunch );
					DestroyMenu( hMenuLaunch );
					hMenuLaunch = CreateTaskTrayLaunchMenu();
				}
				ShowTaskTrayLaunchMenu( hMenuLaunch, hWnd );
				break;
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDM_LIST_G:
			if(haveHeader)
				SendMessage( hWnd, WM_LIST_L, 0, 0 );
			haveScroll = FALSE;
			haveHeader = FALSE;
			SendMessage( hWnd, WM_LIST_G, 0, 0 );
			break;
		case IDM_LIST_M:
			if(haveHeader)
				SendMessage( hWnd, WM_LIST_L, 0, 0 );
			haveScroll = FALSE;
			haveHeader = FALSE;
			SendMessage( hWnd, WM_LIST_M, 0, 0 );
			break;
		case IDM_LIST_L:
			haveScroll = TRUE;
			haveHeader = FALSE;
			SendMessage( hWnd, WM_LIST_L, 0, 0 );
			break;
		case IDM_LIST_D:
			haveScroll = TRUE;
			haveHeader = TRUE;
			SendMessage( hWnd, WM_LIST_D, 0, 0 );
			break;
		case IDM_LIST_H:
			SendMessage( hWnd, WM_LIST_H, 0, 0 );
			break;
		case IDM_HIDE_HEADER:
			dwDeskStyleNew = GetWindowLong( hWndDesk, GWL_STYLE );
			if (IsNoColumnHeader(dwDeskStyleNew) || haveHeader == FALSE)
				haveHeader = TRUE;
			else
				haveHeader = FALSE;
			SendMessage( hWnd, LastMsg, 0, 0 );
			RefreshWindow( hWndDesk );
			break;
		case IDM_HIDE_SCROLL:
			dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
			if (IsNoScroll(dwDeskStyleOld) || haveScroll == FALSE)
				haveScroll = TRUE;
			else
				haveScroll = FALSE;
			SendMessage( hWnd, LastMsg, 0, 0 );
			RefreshWindow( hWndDesk );
			break;
		case IDM_GRIDLINES:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsGridLines(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_GRIDLINES;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_GRIDLINES;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_SUBITEMIMAGES:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsSubItemImages(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_SUBITEMIMAGES;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_SUBITEMIMAGES;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_CHECKBOXES:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsCheckBoxes(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_CHECKBOXES;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_CHECKBOXES;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_HEADERDRAGDROP:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsHeaderDragDrop(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_HEADERDRAGDROP;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_HEADERDRAGDROP;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_FULLROWSELECT:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsFullRowSelect(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_FULLROWSELECT;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_FULLROWSELECT;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_TRACKSELECT:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsTrackSelect(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_TRACKSELECT;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_TRACKSELECT;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_ONECLICKACTIVATE:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsOneClickActivate(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_ONECLICKACTIVATE;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_ONECLICKACTIVATE;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_TWOCLICKACTIVATE:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsTwoClickActivate(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_TWOCLICKACTIVATE;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_TWOCLICKACTIVATE;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_FLATSB:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsFlatSB(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_FLATSB;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_FLATSB;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			if(LastMsg == WM_LIST_D)
			{
				haveScroll = TRUE;
				SendMessage( hWnd, LastMsg, 0, 0 );
			}
			break;
		case IDM_REGIONAL:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsRegional(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_REGIONAL;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_REGIONAL;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_INFOTIP:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsInfoTip(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_INFOTIP;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_INFOTIP;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_UNDERLINEHOT:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsUnderLineHot(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_UNDERLINEHOT;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_UNDERLINEHOT;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_UNDERLINECOLD:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsUnderLineCold(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_UNDERLINECOLD;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_UNDERLINECOLD;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_MULTIWORKAREAS:
			dwDeskStyleOldEx = SendMessage( hWndDesk, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
			if (IsMultiWorkAreas(dwDeskStyleOldEx))
				dwDeskStyleNewEx = dwDeskStyleOldEx & ~LVS_EX_MULTIWORKAREAS;
			else
				dwDeskStyleNewEx = dwDeskStyleOldEx | LVS_EX_MULTIWORKAREAS;
			SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleNewEx );
			break;
		case IDM_HOOKDSKGRID:
			bHookSet = pGetHookDskGrid() ? FALSE : TRUE;
			pSetHookDskGrid( bHookSet );
			SetSetting( SETTING_KEY_HOOKDSKGRID, (DWORD)bHookSet );
			break;
		case IDM_HOOKSHLGRID:
			bHookSet = pGetHookShlGrid() ? FALSE : TRUE;
			pSetHookShlGrid( bHookSet );
			SetSetting( SETTING_KEY_HOOKSHLGRID, (DWORD)bHookSet );
			break;
		case IDM_HOOKSHLBACK:
			bHookSet = pGetHookShlBack() ? FALSE : TRUE;
			pSetHookShlBack( bHookSet );
			SetSetting( SETTING_KEY_HOOKSHLBACK, (DWORD)bHookSet );
			break;
		case IDM_HOOKDSKKICK:
			bHookSet = pGetHookDskKick() ? FALSE : TRUE;
			pSetHookDskKick( bHookSet );
			SetSetting( SETTING_KEY_HOOKDSKKICK, (DWORD)bHookSet );
			break;
		case IDM_REFRESH:
			RefreshWindow( hWndDesk );
			break;
		case IDM_LAYOUTLOAD:
			if (haveVirtualEx(hasModule)) LoadIconLayout( hWndDesk );
			break;
		case IDM_LAYOUTSAVE:
			if (haveVirtualEx(hasModule)) SaveIconLayout( hWndDesk );
			break;
		case IDM_LAYOUTSTART:
			bLoadLayout = (bLoadLayout ? FALSE : TRUE);
			SetSetting( SETTING_KEY_LAYOUT_ST, (DWORD)bLoadLayout );
			break;
		case IDM_STYLELOAD:
			if (GetSetting( SETTING_KEY_STYLE,	&dwSetting ))
				StyleChange( hWndDesk, dwDeskStyleOrg, dwSetting );
			if (GetSetting( SETTING_KEY_STYLE_EX, &dwSetting ))
				SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwSetting );
			break;
		case IDM_STYLESAVE:
			SetSetting( SETTING_KEY_STYLE, dwDeskStyleNew );
			SetSetting( SETTING_KEY_STYLE_EX, dwDeskStyleNewEx );
			break;
		case IDM_STYLESTART:
			bLoadStyle = (bLoadStyle ? FALSE : TRUE);
			SetSetting( SETTING_KEY_STYLE_ST, (DWORD)bLoadStyle );
			break;
		case IDM_EXIT:
			SendMessage( hWnd, WM_CLOSE, 0, 0 );
			break;
		case IDM_LAUNCH_SMALL:
			keybd_event( VK_LWIN, 0, 0,			   0 );
			keybd_event( 'M',	 0, 0,			   0 );
			keybd_event( VK_LWIN, 0, KEYEVENTF_KEYUP, 0 );
			break;
		default:
			NotifyTaskTrayLaunchMenu( hMenuLaunch, LOWORD(wp), hWnd );
			break;
		}
		break;
	case WM_LIST_G:
		dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleNew = dwDeskStyleOld;
		dwDeskStyleNew = (dwDeskStyleNew & ~LVS_TYPEMASK) | LVS_ICON;
		if (haveScroll) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOSCROLL;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOSCROLL;
		if (haveHeader) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOCOLUMNHEADER;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOCOLUMNHEADER;
		StyleChange( hWndDesk, dwDeskStyleOld, dwDeskStyleNew );
		LastMsg = WM_LIST_G;
		break;
	case WM_LIST_M:
		dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleNew = dwDeskStyleOld;
		dwDeskStyleNew = (dwDeskStyleNew & ~LVS_TYPEMASK) | LVS_SMALLICON;
		if (haveScroll) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOSCROLL;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOSCROLL;
		if (haveHeader) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOCOLUMNHEADER;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOCOLUMNHEADER;
		StyleChange( hWndDesk, dwDeskStyleOld, dwDeskStyleNew );
		LastMsg = WM_LIST_M;
		break;
	case WM_LIST_L:
		dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleNew = dwDeskStyleOld;
		dwDeskStyleNew = (dwDeskStyleNew & ~LVS_TYPEMASK) | LVS_LIST;
		if (haveScroll) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOSCROLL;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOSCROLL;
		if (haveHeader) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOCOLUMNHEADER;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOCOLUMNHEADER;
		StyleChange( hWndDesk, dwDeskStyleOld, dwDeskStyleNew );
		LastMsg = WM_LIST_L;
		break;
	case WM_LIST_D:
		dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleNew = dwDeskStyleOld;
		dwDeskStyleNew = (dwDeskStyleNew & ~LVS_TYPEMASK) | LVS_REPORT;
		if (haveScroll) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOSCROLL;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOSCROLL;
		if (haveHeader) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOCOLUMNHEADER;
		else            dwDeskStyleNew = dwDeskStyleNew | LVS_NOCOLUMNHEADER;
		//if (haveScroll) dwDeskStyleNew = (dwDeskStyleNew & ~LVS_NOSCROLL);
		//else			dwDeskStyleNew = (dwDeskStyleNew | LVS_NOSCROLL);
		//if (haveHeader) dwDeskStyleNew = dwDeskStyleNew & ~LVS_NOCOLUMNHEADER;
		//else			dwDeskStyleNew = dwDeskStyleNew | LVS_NOCOLUMNHEADER;
		StyleChange( hWndDesk, dwDeskStyleOld, dwDeskStyleNew );
		LastMsg = WM_LIST_D;
		break;
	case WM_LIST_H:
		//SendMessage( hWndDesk, WM_USER+3210, 0, 0x0412E128 );
		break;
	case WM_DESKHOOK:
		PostMessage( hWnd, WM_TRAY_ACTION, WID_TRAY, lp );
		break;
	case WM_CLOSE:
		if (haveHookFuncs(hasModule)) pUnInstallHook();
		if (haveScroll)
			SendMessage( hWnd, WM_LIST_L, 0, 0 );
		dwDeskStyleOld = GetWindowLong( hWndDesk, GWL_STYLE );
		dwDeskStyleNew = dwDeskStyleOrg;
		StyleChange( hWndDesk, dwDeskStyleOld, dwDeskStyleNew );
		SendMessage( hWndDesk, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwDeskStyleExOrg );
		RefreshWindow( hWndDesk );
		if (pShell_NotifyIcon)
			pShell_NotifyIcon( NIM_DELETE, &ni );
		OwnerDrawDestroyItem( hMenuConfig );
		OwnerDrawDestroyItem( hMenuLaunch );
		DestroyMenu( hMenuConfig );
		DestroyMenu( hMenuLaunch );
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	case WM_MEASUREITEM:
		return OwnerDrawMeasureItem( hWnd, (LPMEASUREITEMSTRUCT)lp );
	case WM_DRAWITEM:
		return OwnerDrawDrawItem( (LPDRAWITEMSTRUCT)lp );
	case WM_RBUTTONUP:
		ShowTaskTrayConfigMenu( hMenuConfig, hWnd, hWndDesk );
		break;
	default:
		return (DefWindowProc( hWnd, msg, wp, lp ));
	}
	return 0;
}

/*****************************************************************************
 * Function GetIniPath
 ****************************************************************************/
CHAR* GetIniPath( HINSTANCE hInst )
{
	CHAR *pStr;	/* source path */
	CHAR *pDir;	/* search path */
	CHAR *pPath;   /* dest path   */

	pPath = (CHAR*)malloc( _MAX_PATH );
	if (pPath == NULL) return NULL;
	GetModuleFileName( hInst, pPath, _MAX_PATH - 1 );
	pStr = pPath;
	pDir = pPath;
	while(*pStr++ != 0)
	{
		if(IsDBCSLeadByte(*pStr) == TRUE) continue;
		if(*pStr == '\\') pDir = pStr;
	}
	*pDir = 0;
	strcat( pPath, "\\" );
	strcat( pPath, szClassName );
	strcat( pPath, ".ini" );
	return pPath;
}

/*****************************************************************************
 * Function SaveIconLayout
 ****************************************************************************/
VOID SaveIconLayout( HWND hWnd )
{
	INT	  item_count;				/* count of item			*/
	LPLVITEM plvi;					/* item for local memory	*/
	LPLVITEM plviAp = NULL;			/* item for public memory	*/
	INT	  	count;					/* counter					*/
	DWORD	dwProcessId;			/* process-id				*/
	HANDLE  hProcess = NULL;		/* handle of process		*/
	HANDLE  hMemory = NULL;			/* handle of memory			*/
	DWORD	NumberOfBytesRead;		/* number of read byte		*/
	DWORD	NumberOfBytesWrite;		/* number of write byte		*/
	CHAR	v_ItemTxt[_MAX_FNAME];	/* icon text				*/
	POINT	v_ItemPos;				/* icon position			*/
	VOID	*o_ItemTxt;				/* pointer of text			*/
	VOID	*o_ItemPos;				/* pointer of position		*/
	LONG	s_ItemTxt;				/* size of text				*/
	LONG	s_ItemPos;				/* size of position			*/
	DWORD	dwSize;					/* size of public memory	*/
	CHAR	chrPosBuf[256];			/* buffer for profile-pos	*/
	CHAR	chrUsrBuf[256];			/* buffer for user-name		*/
	DWORD	lngUsrLen;				/* length of user-name		*/

	/* get uesr name */
	lngUsrLen = sizeof(chrUsrBuf);
	GetUserName( chrUsrBuf, &lngUsrLen );

	s_ItemTxt = _MAX_FNAME;
	s_ItemPos = sizeof(POINT);
	dwSize = sizeof(LVITEM) + s_ItemTxt + s_ItemPos;

	if (!IsWindows95(dwPlatformID))
	{
		GetWindowThreadProcessId( hWnd, &dwProcessId );
		hProcess = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId );
		if (!hProcess) return;
		plviAp = (LPLVITEM)pVirtualAllocEx( hProcess, NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if (!plviAp)
		{
			CloseHandle( hProcess );
			return;
		}
		plvi = (LPLVITEM)malloc( dwSize );
	}
	else
	{
		hMemory = CreateFileMapping( (HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dwSize, "DeskList" );
		if (hMemory)
		plviAp = (LPLVITEM)MapViewOfFile( hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
		plvi = plviAp;
	}

	o_ItemTxt = plviAp + sizeof(LVITEM);
	o_ItemPos = plviAp;

	item_count = SendMessage( hWnd, LVM_GETITEMCOUNT, 0, 0 );

	for( count = 0; count < item_count; count++ )
	{
		plvi->cchTextMax = s_ItemTxt;
		plvi->pszText = (LPTSTR)o_ItemTxt;
		plvi->mask = LVIF_TEXT;
		plvi->iItem = count;
		plvi->iSubItem = 0;

		if (!IsWindows95(dwPlatformID))
			WriteProcessMemory( hProcess, plviAp, plvi, dwSize, &NumberOfBytesWrite );

		// Icon Name
		SendMessage( hWnd, LVM_GETITEM, count, (LPARAM)plviAp );
		if (!IsWindows95(dwPlatformID))
			ReadProcessMemory( hProcess, o_ItemTxt, &v_ItemTxt, s_ItemTxt, &NumberOfBytesRead );
		else
			strcpy( v_ItemTxt, (CHAR*)o_ItemTxt );

		// Icon Name
		SendMessage( hWnd, LVM_GETITEMPOSITION, count, (LPARAM)plviAp );
		if (!IsWindows95(dwPlatformID))
			ReadProcessMemory( hProcess, o_ItemPos, &v_ItemPos, s_ItemPos, &NumberOfBytesRead );
		else
			memcpy( &v_ItemPos, (VOID*)o_ItemPos, sizeof( POINT ) );

		sprintf( chrPosBuf, "%d,%d", v_ItemPos.x, v_ItemPos.y );
		WritePrivateProfileString( chrUsrBuf, v_ItemTxt, chrPosBuf, pIniFile );
	}

	if (!IsWindows95(dwPlatformID))
	{
		free( plvi );
		pVirtualFreeEx( hProcess, plviAp, dwSize, MEM_RELEASE );
		CloseHandle( hProcess );
	}
	else
	{
		UnmapViewOfFile( plviAp );
		CloseHandle( hMemory );
	}
}

/*****************************************************************************
 * Function LoadIconLayout
 ****************************************************************************/
VOID LoadIconLayout( HWND hWnd )
{
	INT	  item_count;				/* count of item			*/
	LPLVITEM plvi;					/* item for local memory	*/
	LPLVITEM plviAp = NULL;			/* item for public memory	*/
	INT		count;					/* counter					*/
	DWORD	dwProcessId;			/* process-id				*/
	HANDLE  hProcess = NULL;		/* handle of process		*/
	HANDLE  hMemory = NULL;			/* handle of memory			*/
	DWORD	NumberOfBytesRead;		/* number of read byte		*/
	DWORD	NumberOfBytesWrite;		/* number of write byte		*/
	CHAR	v_ItemTxt[_MAX_FNAME];	/* icon text				*/
	PVOID	o_ItemTxt;				/* pointer of text			*/
	LONG	s_ItemTxt;				/* size of text				*/
	DWORD	dwSize;					/* size of public memory	*/
	LONG	x;						/* x-position of icon		*/
	LONG	y;						/* y-position of icon		*/
	DWORD	dwRet;					/* return value for API 	*/
	CHAR	chrPosBuf[256];	 		/* buffer for profile		*/
	CHAR	chrUsrBuf[256];	 		/* buffer for user-name 	*/
	DWORD	lngUsrLen;				/* length of user-name		*/

	/* get uesr name */
	lngUsrLen = sizeof(chrUsrBuf);
	GetUserName( chrUsrBuf, &lngUsrLen );

	s_ItemTxt = _MAX_FNAME;
	dwSize = sizeof(LVITEM) + s_ItemTxt;

	if (!IsWindows95(dwPlatformID))
	{
		GetWindowThreadProcessId( hWnd, &dwProcessId );
		hProcess = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId );
		if (!hProcess) return;
		plviAp = (LPLVITEM)pVirtualAllocEx( hProcess, NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if (!plviAp)
		{
			CloseHandle( hProcess );
			return;
		}
		plvi = (LPLVITEM)malloc( dwSize );
	}
	else
	{
		hMemory = CreateFileMapping( (HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dwSize, "DeskList" );
		if (hMemory)
		plviAp = (LPLVITEM)MapViewOfFile( hMemory, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
		plvi = plviAp;
	}

	o_ItemTxt = plviAp + sizeof(LVITEM);

	item_count = SendMessage( hWnd, LVM_GETITEMCOUNT, 0, 0 );

	for( count = 0; count < item_count; count++ )
	{
		plvi->cchTextMax = s_ItemTxt;
		plvi->pszText = (LPTSTR)o_ItemTxt;
		plvi->mask = LVIF_TEXT;
		plvi->iItem = count;
		plvi->iSubItem = 0;

		if (!IsWindows95(dwPlatformID))
			WriteProcessMemory( hProcess, plviAp, plvi, dwSize, &NumberOfBytesWrite );

		SendMessage( hWnd, LVM_GETITEM, count, (LPARAM)plviAp );
		if (!IsWindows95(dwPlatformID))
			ReadProcessMemory( hProcess, o_ItemTxt, &v_ItemTxt, s_ItemTxt, &NumberOfBytesRead );
		else
			strcpy( v_ItemTxt, (CHAR*)o_ItemTxt );

		dwRet = GetPrivateProfileString( chrUsrBuf, v_ItemTxt, NULL, chrPosBuf, sizeof(chrPosBuf), pIniFile );
		if (dwRet == 0) continue;
		if (sscanf( chrPosBuf, "%d,%d", &x, &y ) == 2)
		{
			PostMessage( hWnd, LVM_SETITEMPOSITION, count, MAKELPARAM( x, y ) );
		}
	}

	if (!IsWindows95(dwPlatformID))
	{
		free( plvi );
		pVirtualFreeEx( hProcess, plviAp, dwSize, MEM_RELEASE );
		CloseHandle( hProcess );
	}
	else
	{
		UnmapViewOfFile( plviAp );
		CloseHandle( hMemory );
	}
}

/*****************************************************************************
 * Function LoadModules
 ****************************************************************************/
DWORD LoadModules( VOID )
{
	DWORD   dwRet;	   /* return value for API */ 
	INT	 ModuleCount; /* count of module	  */

	dwRet = HAVE_NOMODULES;

	hModuleKernel32 = (HMODULE)LoadLibrary( "KERNEL32.DLL" );
	if (hModuleKernel32)
	{
		ModuleCount = 0;
		pVirtualAllocEx = (DefVirtualAllocEx)GetProcAddress( hModuleKernel32, "VirtualAllocEx" );
		if (pVirtualAllocEx) ModuleCount++;
		pVirtualFreeEx  = (DefVirtualFreeEx) GetProcAddress( hModuleKernel32, "VirtualFreeEx"  );
		if (pVirtualFreeEx) ModuleCount++;

		if (ModuleCount == 2) dwRet |= HAVE_VIRTFUNCS;
	}

	hModuleDeskHook = (HMODULE)LoadLibrary( "DESKHOOK.DLL" );
	if (hModuleDeskHook)
	{
		ModuleCount = 0;
		pInstallHook   = (DefInstallHook)       GetProcAddress( hModuleDeskHook, "InstallHook"      );
		if (pInstallHook) ModuleCount++;
		pUnInstallHook = (DefUnInstallHook)     GetProcAddress( hModuleDeskHook, "UnInstallHook"    );
		if (pUnInstallHook) ModuleCount++;
		pSetHookDskGrid   = (DefHookSet)        GetProcAddress( hModuleDeskHook, "SetHookDskGrid"   );
		if (pSetHookDskGrid) ModuleCount++;
		pGetHookDskGrid   = (DefHookGet)        GetProcAddress( hModuleDeskHook, "GetHookDskGrid"   );
		if (pGetHookDskGrid) ModuleCount++;
		pSetHookShlGrid   = (DefHookSet)        GetProcAddress( hModuleDeskHook, "SetHookShlGrid"   );
		if (pSetHookShlGrid) ModuleCount++;
		pGetHookShlGrid   = (DefHookGet)        GetProcAddress( hModuleDeskHook, "GetHookShlGrid"   );
		if (pGetHookShlGrid) ModuleCount++;
		pSetHookShlBack   = (DefHookSet)        GetProcAddress( hModuleDeskHook, "SetHookShlBack"   );
		if (pSetHookShlBack) ModuleCount++;
		pGetHookShlBack   = (DefHookGet)        GetProcAddress( hModuleDeskHook, "GetHookShlBack"   );
		if (pGetHookShlBack) ModuleCount++;
		pSetHookDskKick   = (DefHookSet)        GetProcAddress( hModuleDeskHook, "SetHookDskKick"   );
		if (pSetHookDskKick) ModuleCount++;
		pGetHookDskKick   = (DefHookGet)        GetProcAddress( hModuleDeskHook, "GetHookDskKick"  );
		if (pGetHookDskKick) ModuleCount++;
		pDeskListExecute  = (DefDeskListExecute)GetProcAddress( hModuleDeskHook, "DeskListExecute" );
		if (pDeskListExecute) ModuleCount++;
		pDeskListGetPath  = (DefDeskListGetPath)GetProcAddress( hModuleDeskHook, "DeskListGetPath" );
		if (pDeskListGetPath) ModuleCount++;

		if (ModuleCount == 12) dwRet |= HAVE_HOOKFUNCS;
	}

	hModuleShell32 = (HMODULE)LoadLibrary( "SHELL32.DLL" );
	if (hModuleShell32)
	{
		pShell_NotifyIcon   = (DefShell_NotifyIcon)  GetProcAddress( hModuleShell32, "Shell_NotifyIconA"  );
		//pSHGetDesktopFolder = (DefSHGetDesktopFolder)GetProcAddress( hModuleShell32, "SHGetDesktopFolder" );
		pShellExecuteEx     = (DefShellExecuteEx)    GetProcAddress( hModuleShell32, "ShellExecuteEx"     );
		pSHGetFileInfo      = (DefSHGetFileInfo)     GetProcAddress( hModuleShell32, "SHGetFileInfoA"     );
		//pSHGetMalloc        = (DefSHGetMalloc)       GetProcAddress( hModuleShell32, "SHGetMalloc"        );
		//pSHGetPathFromIDList = (DefSHGetPathFromIDList)GetProcAddress( hModuleShell32, "SHGetPathFromIDListA" );
		pSHGetSettings      = (DefSHGetSettings)     GetProcAddress( hModuleShell32, "SHGetSettings"      );
	}

	return dwRet;
}

/*****************************************************************************
 * Function FreeModules
 ****************************************************************************/
VOID FreeModules( VOID )
{
	FreeLibrary( hModuleKernel32 );
	FreeLibrary( hModuleShell32 );
	FreeLibrary( hModuleDeskHook );
}

/*****************************************************************************
 * Function GetSetting
 ****************************************************************************/
BOOL GetSetting( CHAR *SettingKey, DWORD *SettingValue )
{
	CHAR  chrProfileBuf[BUFSIZ]; /* buffer for profile-pos */
	DWORD dwRet;				 /* return value for API   */ 

	dwRet = GetPrivateProfileString( SETTING_SECTION_GENERAL, SettingKey, NULL, chrProfileBuf, sizeof(chrProfileBuf), pIniFile );
	if (dwRet == 0) return FALSE;
	*SettingValue = atol(chrProfileBuf);
	return TRUE;
}

/*****************************************************************************
 * Function SetSetting
 ****************************************************************************/
BOOL SetSetting( CHAR *SettingKey, DWORD SettingValue )
{
	CHAR  chrProfileBuf[BUFSIZ]; /* buffer for profile-pos */
	DWORD dwRet;				 /* return value for API   */ 

	sprintf( chrProfileBuf, "%d", SettingValue );
	dwRet = WritePrivateProfileString( SETTING_SECTION_GENERAL, SettingKey, chrProfileBuf, pIniFile );
	if (dwRet == 0) return FALSE;
	return TRUE;
}

/*****************************************************************************
 * Function FillDesktopMenu
 ****************************************************************************/
BOOL FillDesktopMenu( HWND hWnd, HMENU hMenu )
{
	LONG			MenuHeight;

	MenuHeight = (LONG)(GetSystemMetrics( SM_CYSCREEN ) / GetSystemMetrics( SM_CYMENUSIZE )) - 3;

	if (pSHGetDesktopFolder && pSHGetMalloc && pSHGetFileInfo)
	{
		LPMALLOC		lpMalloc;
		LPSHELLFOLDER	lpShellFolder;
		if (SUCCEEDED(pSHGetDesktopFolder(&lpShellFolder)))
		{
			LPENUMIDLIST	lpEnum;
			LPITEMIDLIST	lpItem;
			LPITEMIDLIST	lpItemCopy;
			HRESULT			hResult;
			ULONG			ulFetch;
			STRRET			sName;
			INT				FileCount = 0;
			CONVERT_ITEMLIST*	itemList = NULL;

			hResult = pSHGetMalloc(&lpMalloc);
			if(FAILED(hResult)) return FALSE;

			hResult = lpShellFolder->lpVtbl->EnumObjects( lpShellFolder, hWnd,
				SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &lpEnum );
			if(SUCCEEDED(hResult))
			{
				HIMAGELIST		hImageList;
				HICON			hIcon;
				MENUITEMINFO	miif;
				SHFILEINFO		SHFileInfo;

				while(lpEnum->lpVtbl->Next( lpEnum, 1, &lpItem, &ulFetch ) == S_OK )
				{
					if (SUCCEEDED(lpShellFolder->lpVtbl->GetDisplayNameOf( lpShellFolder, lpItem, SHGDN_NORMAL, &sName )))
					{
						hImageList =
							(HIMAGELIST)pSHGetFileInfo( (LPCSTR)lpItem, 0, &SHFileInfo, sizeof(SHFILEINFO),
							SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME );
						hIcon = ImageList_GetIcon( hImageList, SHFileInfo.iIcon, ILD_NORMAL );
						FileCount++;
						miif.cbSize		= sizeof(MENUITEMINFO);
						miif.fMask		= MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
						miif.cch		= 0;
						miif.fState		= MFS_ENABLED;
						miif.wID		= IDM_LAUNCH_SMALL+FileCount+1;
						miif.hSubMenu   = 0;
						miif.fType		= MFT_STRING;
						miif.dwTypeData	= SHFileInfo.szDisplayName;
						if((FileCount % MenuHeight) == 0)
							miif.fType |= (MFT_MENUBREAK | MFT_MENUBARBREAK);
						InsertMenuItem( hMenu, IDM_LAUNCH_SMALL+FileCount+1, FALSE, &miif );
						if (itemList)
							itemList = (CONVERT_ITEMLIST*)realloc( itemList, sizeof(CONVERT_ITEMLIST) * FileCount );
						else
							itemList = (CONVERT_ITEMLIST*)malloc( sizeof(CONVERT_ITEMLIST) );
						lpItemCopy = (LPITEMIDLIST)lpMalloc->lpVtbl->Alloc( lpMalloc, lpItem->mkid.cb+sizeof(lpItem->mkid.cb) );
						CopyMemory( (PVOID)lpItemCopy, (CONST VOID *)lpItem, lpItem->mkid.cb+sizeof(lpItem->mkid.cb) );
						OwnerDrawSetItemList( &itemList[FileCount-1], IDM_LAUNCH_SMALL+FileCount+1, hIcon, lpItemCopy );
					}
					lpMalloc->lpVtbl->Free( lpMalloc, lpItem );
				}
				FileCount++;
				itemList = realloc( itemList, sizeof(CONVERT_ITEMLIST) * FileCount );
				OwnerDrawSetItemList( &itemList[FileCount-1], 0, NULL, NULL );
				OwnerDrawConvertItem( hInstance, hMenu, itemList );
				if (FileCount) free( itemList );

				if(lpEnum) lpMalloc->lpVtbl->Free( lpMalloc,  lpEnum );
			}
		}
	}
	else
	{
		HANDLE				hFindFirst;
		WIN32_FIND_DATA		FindData;
		INT					FileCount = 0;
		BOOL				hasNext;
		CONVERT_ITEMLIST*	itemList = NULL;
		MENUITEMINFO		miif;
		SHFILEINFO			SHFileInfo;  
		CHAR				strDeskPath[MAX_PATH];
		CHAR				strFilePath[MAX_PATH];
		if (pDeskListGetPath)
			pDeskListGetPath( strDeskPath, sizeof(strDeskPath) );
		sprintf( strFilePath, "%s\\*.*", strDeskPath );
		hFindFirst = FindFirstFile( strFilePath, &FindData );
		if (hFindFirst != INVALID_HANDLE_VALUE)
		{
			hasNext = TRUE;
			while( hasNext )
			{
				if (strcmp( FindData.cFileName, "." ) && strcmp( FindData.cFileName, ".." ) )
				{
					FileCount++;
					sprintf( strFilePath, "%s\\%s", strDeskPath, FindData.cFileName );
					pSHGetFileInfo( strFilePath, 0, &SHFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME );
					miif.cbSize		= sizeof(MENUITEMINFO);
					miif.fMask		= MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
					miif.cch		= 0;
					miif.fState		= MFS_ENABLED;
					miif.wID		= IDM_LAUNCH_SMALL+FileCount+1;
					miif.hSubMenu	= 0;
					miif.fType		= MFT_STRING;
					miif.dwTypeData	= SHFileInfo.szDisplayName;
					miif.dwItemData	= 0;
					if((FileCount % MenuHeight) == 0)
						miif.fType |= (MFT_MENUBREAK | MFT_MENUBARBREAK);
					InsertMenuItem( hMenu, IDM_LAUNCH_SMALL+FileCount+1, FALSE, &miif );
					if (itemList)
						itemList = (CONVERT_ITEMLIST*)realloc( itemList, sizeof(CONVERT_ITEMLIST) * FileCount );
					else
						itemList = (CONVERT_ITEMLIST*)malloc( sizeof(CONVERT_ITEMLIST) );
					OwnerDrawSetItemList( &itemList[FileCount-1], IDM_LAUNCH_SMALL+FileCount+1, SHFileInfo.hIcon, NULL );
				}
				hasNext = FindNextFile( hFindFirst, &FindData );
			}
			FindClose( hFindFirst );

			FileCount++;
			itemList = realloc( itemList, sizeof(CONVERT_ITEMLIST) * FileCount );
			OwnerDrawSetItemList( &itemList[FileCount-1], 0, NULL, NULL );
			OwnerDrawConvertItem( hInstance, hMenu, itemList );
			if (FileCount) free( itemList );
		}
	}
	return TRUE;
}

/*****************************************************************************
 * DeskHook.cpp
 *----------------------------------------------------------------------------
 *
 ****************************************************************************/
#ifdef _BCC_
#pragma option -zR_SDATA
#pragma option -zS_SGROUP
#pragma option -zT_SCLASS
#endif

/*****************************************************************************
 * Includes
 ****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <stdio.h>
#define __SUBCLASS__
#include "DeskHook.h"

/*****************************************************************************
 * Definitions
 ****************************************************************************/
typedef HRESULT (WINAPI* DefSHGetSpecialFolderPath) (HWND, LPTSTR , INT, BOOL);
typedef BOOL	(WINAPI* DefSetLayeredWindowAttributes)	(HWND,COLORREF,BYTE,DWORD);

/*****************************************************************************
 * Prototypes
 ****************************************************************************/
EXTERN_C LRESULT CALLBACK MesgProc( INT, WPARAM, LPARAM );
EXTERN_C LRESULT CALLBACK CallProc( INT, WPARAM, LPARAM );
LONG GetIconSize( VOID );

/*****************************************************************************
 * Variables
 ****************************************************************************/
#ifdef _MSC_VER
#pragma comment(linker, "-section:Shared,rws")
#pragma data_seg("Shared")
#endif
static HWND			hWndApp = INVALID_HANDLE_VALUE;
static HWND			hWndDesk = INVALID_HANDLE_VALUE;
static HHOOK		hHookMesg = INVALID_HANDLE_VALUE;
static HHOOK		hHookCall = INVALID_HANDLE_VALUE;
static HINSTANCE	hInstance = INVALID_HANDLE_VALUE;
static WNDPROC		wProc = NULL;
static BOOL			bOptionDskGrid = FALSE;
static BOOL			bOptionShlGrid = FALSE;
static BOOL			bOptionShlBack = FALSE;
static BOOL			bOptionDskKick = FALSE;
static INT			IconSizeSh;
static INT			IconSpaceH;
static INT			IconSpaceV;
static HMODULE		hModuleShell32 = INVALID_HANDLE_VALUE;
static HMODULE		hModuleUser32 = INVALID_HANDLE_VALUE;
static DefSHGetSpecialFolderPath	pSHGetSpecialFolderPath = NULL;
//static DefSetLayeredWindowAttributes	pSetLayeredWindowAttributes = NULL;
#ifdef _MSC_VER
#pragma data_seg()
#endif

/*****************************************************************************
 * Function DllMain
 ****************************************************************************/
EXTERN_C BOOL WINAPI DllMain( HINSTANCE hInstDLL, DWORD fdReason, PVOID pvReserved )
{
	if (fdReason == DLL_PROCESS_ATTACH)
		hInstance = hInstDLL;
	return TRUE;
}

/*****************************************************************************
 * Function InstallHook
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI InstallHook( HWND hWnd )
{
	//DWORD dwProcessID;

	hWndApp = hWnd;

	hWndDesk = FindWindow( "Progman", NULL );
	hWndDesk = FindWindowEx( hWndDesk, 0, "SHELLDLL_DefView", NULL );
	hWndDesk = FindWindowEx( hWndDesk, 0, "SysListView32", NULL );
	if (hWndDesk == INVALID_HANDLE_VALUE) return FALSE;

	hModuleShell32 = (HMODULE)LoadLibrary( "SHELL32.DLL" );
	if (hModuleShell32)
		pSHGetSpecialFolderPath = (DefSHGetSpecialFolderPath)GetProcAddress( hModuleShell32, "SHGetSpecialFolderPathA" );
	hModuleUser32 = (HMODULE)LoadLibrary( "USER32.DLL" );
//	if (hModuleUser32)
//		pSetLayeredWindowAttributes = (DefSetLayeredWindowAttributes)GetProcAddress( hModuleUser32, "SetLayeredWindowAttributes" );

	//dwProcessID = GetWindowThreadProcessId( hWndDesk, NULL );

	hHookMesg = SetWindowsHookEx( WH_GETMESSAGE,  (HOOKPROC)MesgProc, hInstance, 0 );
	if (hHookMesg == NULL) return FALSE;
	hHookCall = SetWindowsHookEx( WH_CALLWNDPROC, (HOOKPROC)CallProc, hInstance, 0 );
	if (hHookCall == NULL) return FALSE;
	PostMessage( hWndDesk, WM_NULL, 0, 0 );
	return TRUE;
}

/*****************************************************************************
 * Function UnInstallHook
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI UnInstallHook()
{
	if (UnhookWindowsHookEx( hHookMesg ) != 0) return FALSE;
	if (UnhookWindowsHookEx( hHookCall ) != 0) return FALSE;
	FreeLibrary( hModuleShell32 );
	FreeLibrary( hModuleUser32 );
	return TRUE;
}

/*****************************************************************************
 * Function DeskListGetPath
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI DeskListGetPath( CHAR *strPath, INT nBuffer )
{
	CHAR	strDeskPath[MAX_PATH];
	DWORD	dwRet;
	HKEY	hKey;
	DWORD	dwSize;

	memset( strDeskPath, 0x00, sizeof(strDeskPath) );
	if (pSHGetSpecialFolderPath)
	{
		pSHGetSpecialFolderPath( GetDesktopWindow(), strDeskPath, CSIDL_DESKTOPDIRECTORY, FALSE );
	}
	else
	{
		dwRet = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_READ, &hKey );
		if (dwRet != ERROR_SUCCESS)
			return FALSE;
		dwSize = sizeof( strDeskPath );
		dwRet = RegQueryValueEx( hKey, "Desktop", NULL, NULL, (LPBYTE)strDeskPath, &dwSize );
		RegCloseKey( hKey );
		if (dwRet != ERROR_SUCCESS)
			return FALSE;
	}
	if (strlen(strDeskPath) > (unsigned)nBuffer)
		return FALSE;
	strcpy( strPath, strDeskPath );
	return TRUE;
}

/*****************************************************************************
 * Function DeskListExecute
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI DeskListExecute( BOOL IsDesktop, CHAR *strPath )
{
	CHAR	strDeskPath[MAX_PATH];
	HANDLE	hHandle;
	WIN32_FIND_DATA  FindData;

	memset( strDeskPath, 0x00, sizeof(strDeskPath) );
	if (IsDesktop || strPath == NULL)
	{
		if (!DeskListGetPath( strDeskPath, sizeof(strDeskPath) ))
			return FALSE;
		if (strPath)
		{
			strcat( strDeskPath, "\\" );
			strcat( strDeskPath, strPath );
		}
	}
	else
	{
		strcpy( strDeskPath, strPath );
	}
	hHandle = FindFirstFile( strDeskPath, &FindData );
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		strcat( strDeskPath, ".lnk" );
		hHandle = FindFirstFile( strDeskPath, &FindData );
	}
	if (hHandle != INVALID_HANDLE_VALUE)
	{
		FindClose( hHandle );
		if (( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ))
			ShellExecute( GetDesktopWindow(), "open", "explorer", strDeskPath, NULL, SW_SHOWNORMAL );
		else
			ShellExecute( GetDesktopWindow(), "open", strDeskPath, NULL, NULL, SW_SHOWNORMAL );
	}
	else
	{
		ShellExecute( GetDesktopWindow(), "open", strDeskPath, NULL, NULL, SW_SHOWNORMAL );
	}
	return TRUE;
}

/*****************************************************************************
 * Function SetHookDskGrid
 ****************************************************************************/
EXTERN_C VOID SUBCLASSAPI SetHookDskGrid( BOOL bValue )
{
	bOptionDskGrid = bValue;
}

/*****************************************************************************
 * Function SetHookShlGrid
 ****************************************************************************/
EXTERN_C VOID SUBCLASSAPI SetHookShlGrid( BOOL bValue )
{
	bOptionShlGrid = bValue;
}

/*****************************************************************************
 * Function SetHookShlBack
 ****************************************************************************/
EXTERN_C VOID SUBCLASSAPI SetHookShlBack( BOOL bValue )
{
	bOptionShlBack = bValue;
}

/*****************************************************************************
 * Function SetHookDskKick
 ****************************************************************************/
EXTERN_C VOID SUBCLASSAPI SetHookDskKick( BOOL bValue )
{
	bOptionDskKick = bValue;
}

/*****************************************************************************
 * Function GetHookDskGrid
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI GetHookDskGrid()
{
	return bOptionDskGrid;
}

/*****************************************************************************
 * Function GetHookShlGrid
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI GetHookShlGrid()
{
	return bOptionDskGrid;
}

/*****************************************************************************
 * Function GetHookShlBack
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI GetHookShlBack()
{
	return bOptionShlBack;
}

/*****************************************************************************
 * Function GetHookDskKick
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI GetHookDskKick()
{
	return bOptionDskKick;
}

/*****************************************************************************
 * Function GetIconSize
 ****************************************************************************/
LONG GetIconSize( VOID )
{
	HKEY	hKey;
	DWORD	RegSize;
	CHAR	RegValue[256];
	LONG	RegRet;
	LONG	RetValue;

	// Get value of IconSize
	RegSize = sizeof(INT);
	RegOpenKeyEx( HKEY_CURRENT_USER, "Control Panel\\Desktop\\WindowMetrics", 0, KEY_READ, &hKey );
	RegRet = RegQueryValueEx( hKey, "Shell Icon Size", NULL, NULL, (LPBYTE)RegValue, &RegSize );
	RegCloseKey( hKey );
	if (RegRet == ERROR_SUCCESS)
	{
		RetValue = atoi( RegValue );
	}
	else
	{
		RetValue = 32;
	}
	return RetValue;
}

/*****************************************************************************
 * Function MesgProc
 ****************************************************************************/
EXTERN_C LRESULT CALLBACK MesgProc( INT nCode, WPARAM wp, LPARAM lp )
{
	CHAR			className[256];
	MSG				*pmsg;
	LVHITTESTINFO	htif;
	POINT			pt;

	pmsg = (MSG*)lp;
	GetClassName( pmsg->hwnd, className, sizeof(className) );
	if (!strcmp( className, "SysListView32" ))
	{
		if (pmsg->message == WM_LBUTTONDBLCLK)
		{
			GetCursorPos( (LPPOINT)&pt );
			htif.pt = pt;
			ScreenToClient( pmsg->hwnd, &htif.pt );
			SendMessage( pmsg->hwnd, LVM_HITTEST, 0, (LPARAM)&htif );
			if (!(htif.flags & LVHT_ONITEM))
			{
				if (hWndDesk == pmsg->hwnd)
				{
					if (bOptionDskKick == TRUE)
					{
						DeskListExecute( TRUE, NULL );
						return 0;
					}
				}
				else
				{
					if (bOptionShlBack == TRUE)
					{
						PostMessage( pmsg->hwnd, WM_KEYDOWN, VK_BACK, 0 );
						return 0;
					}
				}
			}
		}
#if 0 /* not support */
		else
		if (pmsg->message == WM_LBUTTONUP)
		{
			BOOL	CntrlPressed = FALSE;
			BOOL	ShiftPressed = FALSE;
		    CntrlPressed = (BOOL)( 0x8000 & GetAsyncKeyState(VK_CONTROL) );
		    ShiftPressed = (BOOL)( 0x8000 & GetAsyncKeyState(VK_SHIFT) );
		    if ( CntrlPressed || ShiftPressed )
			{
				GetCursorPos( (LPPOINT)&pt );
				htif.pt = pt;
				ScreenToClient( pmsg->hwnd, &htif.pt );
				SendMessage( pmsg->hwnd, LVM_HITTEST, 0, (LPARAM)&htif );
				if (!(htif.flags & LVHT_ONITEM))
				{
					if (CntrlPressed && !ShiftPressed)
						SendMessage( hWndApp, WM_USER+100, 0, WM_LBUTTONUP );
					else
					if (!CntrlPressed && ShiftPressed)
						SendMessage( hWndApp, WM_USER+100, 0, WM_RBUTTONUP );
					return 0;
				}
			}
		}
#endif
	}
	return CallNextHookEx( hHookMesg, nCode, wp, lp );
}

/*****************************************************************************
 * Function CallProc
 ****************************************************************************/
EXTERN_C LRESULT CALLBACK CallProc( INT nCode, WPARAM wp, LPARAM lp )
{
	CWPSTRUCT   *pcwp;

	pcwp = (CWPSTRUCT *)lp;

	if ((pcwp->hwnd == hWndDesk && bOptionDskGrid == TRUE)
		|| (pcwp->hwnd != hWndDesk && bOptionShlGrid == TRUE))
	{
		if (pcwp->message == LVM_SETITEMPOSITION32)
		{
			INT		 iItem;
			LPPOINT	 pPoint;
			ICONMETRICS icmet;
			RECT		rect;
			LONG		tx, ty;
			LONG		offsetX;
			LONG		offsetY;

			icmet.cbSize = sizeof(ICONMETRICS);
			if (SystemParametersInfo( SPI_GETICONMETRICS, sizeof(ICONMETRICS), &icmet, 0 ))
			{
				IconSizeSh = GetIconSize();
				IconSpaceH = icmet.iHorzSpacing - (32-IconSizeSh);
				IconSpaceV = icmet.iVertSpacing - (32-IconSizeSh);
				pPoint = (LPPOINT)pcwp->lParam;
				iItem = pcwp->wParam;
				SystemParametersInfo( SPI_GETWORKAREA, 0, &rect, SPIF_SENDWININICHANGE );
				tx = pPoint->x - rect.left + (int)(IconSpaceH-IconSizeSh)/2;
				ty = pPoint->y - rect.top  - 2 + IconSpaceV/2;
				tx = ((int)(tx/IconSpaceH)) * IconSpaceH;
				ty = ((int)(ty/IconSpaceV)) * IconSpaceV;
				pPoint->x = tx + rect.left + (int)((IconSpaceH-IconSizeSh)/2);
				pPoint->y = ty + rect.top  + 2;
			}
		}
#if 0
		else
		if(pcwp->message == WM_ERASEBKGND)
		{
			LRESULT	ret;
			ret = CallNextHookEx( hHookMesg, nCode, wp, lp );
			SendMessage( pcwp->hwnd, WM_PAINT, 0, 0 );
			MessageBox( 0, "test", "test", MB_OK );
			return ret;
		}
#endif
	}
	return CallNextHookEx( hHookCall, nCode, wp, lp );
}

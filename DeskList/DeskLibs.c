/*****************************************************************************
 * DeskLibs.cpp
 *----------------------------------------------------------------------------
 *
 ****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/
#include <windows.h>
#include <commctrl.h>

#include "DeskLibs.h"
#include "resource.h"

/*****************************************************************************
 * Variables
 ****************************************************************************/
static HIMAGELIST   hImageList = NULL;

/*****************************************************************************
 * Function OwnerDrawDestroyItem
 ****************************************************************************/
VOID OwnerDrawSetItemList( CONVERT_ITEMLIST *itemList, DWORD itemID, HICON hIcon, VOID *pExInfo )
{
	itemList->itemID  = itemID;
	itemList->hIcon   = hIcon;
	itemList->pExInfo = pExInfo;
}

/*****************************************************************************
 * Function OwnerDrawDestroyItem
 ****************************************************************************/
VOID OwnerDrawDestroyItem( HMENU hMenu )
{
	INT		  count;
	INT		  menunum;
	ODMENUITEM   *pomi;
	MENUITEMINFO miif;
	CHAR		 sBuffer[MENUSTRBUFSIZE];

	menunum = GetMenuItemCount( hMenu );
	for( count = 0; count < menunum; count++ )
	{
		miif.cbSize	 = sizeof(MENUITEMINFO);
		miif.fMask	  = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_CHECKMARKS | MIIM_DATA | MIIM_SUBMENU;
		miif.dwTypeData = sBuffer;
		miif.cch		= sizeof(sBuffer);
		GetMenuItemInfo( hMenu, count, TRUE, &miif );
		if ((miif.fType & MF_SEPARATOR) == MF_SEPARATOR) continue;
		pomi = (ODMENUITEM*)miif.dwItemData;
		if (pomi == NULL) continue;
		if (pomi->pszText) free( pomi->pszText );
		if (pomi->hIcon) DestroyIcon( pomi->hIcon );
		free( (VOID*)miif.dwItemData );
		miif.dwItemData = 0;
		miif.fType &= ~MF_OWNERDRAW;
		SetMenuItemInfo( hMenu, count, TRUE, &miif );
		if (miif.hSubMenu) OwnerDrawDestroyItem( miif.hSubMenu );
	}
}

/*****************************************************************************
 * Function OwnerDrawConvertItem
 ****************************************************************************/
VOID OwnerDrawConvertItem( HINSTANCE hInstance, HMENU hMenu, CONVERT_ITEMLIST itemList[] )
{
	INT		  count;
	INT		  menunum;
	INT		  search;
	ODMENUITEM   *pomi;
	MENUITEMINFO miif;
	CHAR		 sBuffer[MENUSTRBUFSIZE];
	static INT   nIndex = 0;

	if (hImageList == NULL)
	{
		hImageList = ImageList_Create( 16, 16, ILC_COLORDDB | ILC_MASK, 4, 0 );
		ImageList_AddIcon( hImageList, LoadIcon( hInstance, MAKEINTRESOURCE(IDI_CHECK) ) );
		nIndex++;
	}

	menunum = GetMenuItemCount( hMenu );
	for( count = 0; count < menunum; count++ )
	{
		miif.cbSize		= sizeof(MENUITEMINFO);
		miif.fMask		= MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_CHECKMARKS | MIIM_DATA | MIIM_SUBMENU;
		miif.dwTypeData	= sBuffer;
		miif.cch		= sizeof(sBuffer);
		GetMenuItemInfo( hMenu, count, TRUE, &miif );
		if ((miif.fType & MF_SEPARATOR) == MF_SEPARATOR) continue;
		for( search = 0; itemList[search].itemID != 0; search++ )
			if (miif.wID == itemList[search].itemID) break;
		//if (itemList[search].itemID == 0) continue;
		pomi = (ODMENUITEM*)malloc( sizeof(ODMENUITEM) );
		memset( pomi, 0, sizeof(ODMENUITEM) );
		pomi->uID = miif.wID;
		pomi->pExInfo = itemList[search].pExInfo;
		pomi->pszText = (CHAR*)malloc( strlen(sBuffer)+1 );
		strcpy( pomi->pszText, sBuffer );
		if (itemList[search].itemID != 0)
		{
			ImageList_AddIcon( hImageList, itemList[search].hIcon );
			pomi->hIcon = ImageList_GetIcon( hImageList, nIndex, 0 );
			nIndex++;
		}
		miif.fType |= MF_OWNERDRAW;
		miif.dwItemData = (DWORD)pomi;
		SetMenuItemInfo( hMenu, count, TRUE, &miif );
		if (miif.hSubMenu) OwnerDrawConvertItem( hInstance, miif.hSubMenu, itemList );
	}
}

/*****************************************************************************
 * Function OwnerDrawMeasureItem
 ****************************************************************************/
BOOL OwnerDrawMeasureItem( HWND hWnd, LPMEASUREITEMSTRUCT lpms )
{
	HDC			  hDC;
	ODMENUITEM*	  pomi;
	SIZE			 size;
	HFONT			hFont;
	HFONT			hFontOld;
	NONCLIENTMETRICS info;

	hDC = GetDC( hWnd );
	pomi = (ODMENUITEM*)lpms->itemData;

	info.cbSize = sizeof(info);
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0 );
	hFont = CreateFontIndirect( &info.lfMenuFont );
	hFontOld = (HFONT)SelectObject( hDC, hFont );

	GetTextExtentPoint32( hDC, pomi->pszText, lstrlen(pomi->pszText), &size );
	size.cx = size.cx + GetSystemMetrics( SM_CYMENU );
	size.cy = max( size.cy, GetSystemMetrics( SM_CYMENU ) );

	SelectObject( hDC, hFontOld );

	lpms->itemWidth  = size.cx;
	lpms->itemHeight = size.cy;
	ReleaseDC( hWnd, hDC );
	return TRUE;
}

/*****************************************************************************
 * Function OwnerDrawDrawItem
 ****************************************************************************/
BOOL OwnerDrawDrawItem( LPDRAWITEMSTRUCT lpdis )
{
	HICON	   hIcon;
	ODMENUITEM* pomi;
	HBRUSH	  hBrush;
	COLORREF	crNew;
	COLORREF	crOld;
	int		 nOldMode;
	RECT		rcIcon;
	RECT		rcText;

	pomi = (ODMENUITEM*)lpdis->itemData;

	if ((lpdis->itemState & ODS_GRAYED) != ODS_GRAYED)
	{
		hBrush = CreateSolidBrush( GetSysColor( COLOR_MENU) );
		FillRect(lpdis->hDC, &lpdis->rcItem, hBrush );
		DeleteObject(hBrush);
		if ((lpdis->itemState & ODS_SELECTED) == ODS_SELECTED)
		{
			SetRect( &rcText,
				lpdis->rcItem.left, 
				lpdis->rcItem.top,
				lpdis->rcItem.right,
				lpdis->rcItem.bottom );
			DrawEdge( lpdis->hDC, &rcText, BDR_RAISEDINNER, BF_RECT );
		}
	}
	else
	{
		hBrush = CreateSolidBrush( GetSysColor( COLOR_MENU) );
		FillRect(lpdis->hDC, &lpdis->rcItem, hBrush );
		DeleteObject(hBrush);
	}
	nOldMode = SetBkMode( lpdis->hDC, TRANSPARENT );


	SetRect( &rcIcon,
		lpdis->rcItem.left,
		lpdis->rcItem.top,
		lpdis->rcItem.left + GetSystemMetrics(SM_CYMENU),
		lpdis->rcItem.top + GetSystemMetrics(SM_CYMENU) );
	SetRect( &rcText,
		lpdis->rcItem.left + 20, 
		lpdis->rcItem.top,
		lpdis->rcItem.right,
		lpdis->rcItem.bottom );

	if (pomi->hIcon) hIcon = pomi->hIcon;
	else			 hIcon = NULL;
	if ((lpdis->itemState & ODS_CHECKED) == ODS_CHECKED)
	{
		DrawEdge( lpdis->hDC, &rcIcon, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST );
		if (hIcon == NULL)
		{
			hIcon = ImageList_GetIcon( hImageList, 0, 0 );
		}
	}
	if (hIcon)
	{
		if ((lpdis->itemState & ODS_GRAYED) == ODS_GRAYED)
			DrawState( lpdis->hDC, NULL, NULL, (WPARAM)hIcon, 0,
				rcIcon.left, rcIcon.top, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), DST_ICON | DSS_DISABLED );
		else
			DrawState( lpdis->hDC, NULL, NULL, (WPARAM)hIcon, 0,
				rcIcon.left, rcIcon.top, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), DST_ICON );
	}
	if ((lpdis->itemState & ODS_GRAYED) == ODS_GRAYED)
	{
		crNew = GetSysColor( COLOR_GRAYTEXT );
	}
	else
	{
		crNew = GetSysColor( COLOR_MENUTEXT );
	}
	crOld = SetTextColor( lpdis->hDC, crNew );
	DrawText( lpdis->hDC, pomi->pszText, -1, &rcText, DT_SINGLELINE|DT_LEFT|DT_VCENTER );
	SetTextColor( lpdis->hDC, crOld );
	SetBkMode( lpdis->hDC, nOldMode );
	return TRUE;
}


/*****************************************************************************
 * DeskLibs.h
 *----------------------------------------------------------------------------
 *
 ****************************************************************************/
 
 /*****************************************************************************
 * Definitions
 ****************************************************************************/
#define MENUSTRBUFSIZE              256

typedef struct {
    UINT			uID;
    HICON			hIcon;
    CHAR*			pszText;
    VOID*			pExInfo;
} ODMENUITEM;

typedef struct {
    DWORD		itemID;
    HICON		hIcon;
    VOID*		pExInfo;
} CONVERT_ITEMLIST;

/*****************************************************************************
 * Prototypes
 ****************************************************************************/
VOID OwnerDrawSetItemList( CONVERT_ITEMLIST*, DWORD, HICON, VOID* );
VOID OwnerDrawConvertItem( HINSTANCE, HMENU, CONVERT_ITEMLIST[] );
VOID OwnerDrawDestroyItem( HMENU );
BOOL OwnerDrawMeasureItem( HWND, LPMEASUREITEMSTRUCT );
BOOL OwnerDrawDrawItem( LPDRAWITEMSTRUCT );


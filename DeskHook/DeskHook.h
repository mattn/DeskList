/*****************************************************************************
 * DeskHook.cpp
 *----------------------------------------------------------------------------
 *
 ****************************************************************************/

/*****************************************************************************
 * Includes
 ****************************************************************************/

/*****************************************************************************
 * Definitions
 ****************************************************************************/
#ifdef __SUBCLASS__
#ifdef _BCC_
#define SUBCLASSAPI _declspec(dllexport) __stdcall 
#else
#define SUBCLASSAPI _declspec(dllexport) 
#endif
#else
#define SUBCLASSAPI _declspec(dllimport)
#endif

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C
# endif
#endif

/*****************************************************************************
 * Prototypes
 ****************************************************************************/
EXTERN_C BOOL SUBCLASSAPI InstallHook( HWND );
EXTERN_C BOOL SUBCLASSAPI UnInstallHook( VOID );

EXTERN_C BOOL SUBCLASSAPI DeskListExecute( BOOL, CHAR* );
EXTERN_C BOOL SUBCLASSAPI DeskListGetPath( CHAR*, INT );

EXTERN_C BOOL SUBCLASSAPI GetHookDskGrid( VOID );
EXTERN_C VOID SUBCLASSAPI SetHookDskGrid( BOOL );

EXTERN_C BOOL SUBCLASSAPI GetHookShlGrid( VOID );
EXTERN_C VOID SUBCLASSAPI SetHookShlGrid( BOOL );

EXTERN_C BOOL SUBCLASSAPI GetHookShlBack( VOID );
EXTERN_C VOID SUBCLASSAPI SetHookShlBack( BOOL );

EXTERN_C BOOL SUBCLASSAPI GetHookDskKick( VOID );
EXTERN_C VOID SUBCLASSAPI SetHookDskKick( BOOL );


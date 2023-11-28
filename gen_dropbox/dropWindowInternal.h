#ifndef NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_INTERNAL_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./dropWindow.h"
#include "./fileMetaInterface.h"


__interface IFileInfo;
class Document;

BOOL DropboxWindow_RegisterClass(HINSTANCE hInstance);

typedef struct __ACTIVECOLUMN
{
	UINT id;
	INT order;
	INT width;
} ACTIVECOLUMN;


void DropboxWindow_LoadProfileView(HWND hwnd, Profile *profile);
void DropboxWindow_SaveViewSize(HWND hwnd, Profile *profile);
void DropboxWindow_SaveClassInfo(HWND hwnd, Profile *profile);
void DropboxWindow_LoadCreateStructSettings(Profile *profile, CREATESTRUCT *pcs);
HWND DropboxWindow_GetFrame(HWND hwnd);

BOOL DropboxWindow_RegisterProfileCallback(HWND hwnd, Profile *profile);
BOOL DropboxWindow_UnregisterProfileCallback(HWND hwnd);

/*
*	DropWindow List
*/
typedef int (__cdecl *COLUMN_COMPARER)(const void *, const void *);
typedef LPCTSTR (__cdecl *COLUMN_FORMATTER)(IFileInfo * /*pFileInfo*/, LPTSTR /*pszBuffer*/, INT /*cchBufferMax*/);


typedef struct __LISTCOLUMN
{
	UINT				id;
	LPCTSTR				pszName;
	LPCTSTR				pszTitle;
	LPCTSTR				pszTitleLong;
	INT					width;
	UINT				format;
	INT					widthMin;
	INT					widthMax;
	COLUMN_FORMATTER		fnFormatter;
	COLUMN_COMPARER		fnComparer;
	LPCSTR				pszMetaKey; //can be WORD or double null terminated list
} LISTCOLUMN;

extern const LISTCOLUMN szRegisteredColumns[COLUMN_LAST];

#define GET_REGISTERED_COLUMN_NAME(__column) (szRegisteredColumns[__column].pszName)

INT ColumnIdToMetaKey(INT columnId, METAKEY *pMetaKey, INT metaKeyMax); 
/*
*	DropBox Header
*/

#define NWC_DROPBOXHEADERA		"NullsoftDropBoxHeader"
#define NWC_DROPBOXHEADERW		L"NullsoftDropBoxHeader"

#ifdef UNICODE
#define NWC_DROPBOXHEADER	NWC_DROPBOXHEADERW
#else
#define NWC_DROPBOXHEADER	NWC_DROPBOXHEADERA
#endif // !UNICODE

BOOL DropboxHeader_RegisterClass(HINSTANCE hInstance);
BOOL DropboxHeader_ParseStyles(LPCTSTR pszString, DWORD *pStyles);
BOOL DropboxHeader_FormatStyles(LPTSTR pszBuffer, INT cchBufferMax, DWORD styles);


#define NWC_BUSYWINDOWA		"NullsoftBusyWindow"
#define NWC_BUSYWINDOWW		L"NullsoftBusyWindow"

#ifdef UNICODE
#define NWC_BUSYWINDOW	NWC_BUSYWINDOWW
#else
#define NWC_BUSYWINDOW	NWC_BUSYWINDOWA
#endif // !UNICODE

BOOL BusyWindow_RegisterClass(HINSTANCE hInstance);

#define BWS_ENABLEBLUR		0x0001

#define BWM_FIRST			(WM_USER + 200)

#define BWM_SETTARGETWINDOW	(BWM_FIRST + 1)
#define BusyWindow_SetTargetWindow(__hwndBusy, __hwndTarget)\
	(SNDMSG((__hwndBusy), BWM_SETTARGETWINDOW, 0, (LPARAM)(__hwndTarget)))

typedef struct __BWOPERATIONINFO
{
	size_t total;
	size_t processed;
	BOOL cancelable;
} BWOPERATIONINFO;

#define BWM_SETOPERATIONINFO			(BWM_FIRST + 2)
#define BusyWindow_SetOperationInfo(__hwndBusy, __pOperationInfo)\
	((BOOL)SNDMSG((__hwndBusy), BWM_SETOPERATIONINFO, 0, (LPARAM)(__pOperationInfo)))

#define BWM_CANCELOPERATION			(BWM_FIRST + 3)
#define BusyWindow_CancelOperation(__hwndBusy)\
	(SNDMSG((__hwndBusy), BWM_CANCELOPERATION, 0, 0L))

#define BWM_BKIMAGEREADY				(BWM_FIRST + 4)
#define BusyWindow_BkImageReady(__hwndBusy, __bkImage)\
	((BOOL)SNDMSG((__hwndBusy), BWM_BKIMAGEREADY, 0, (LPARAM)(__bkImage)))

#define BWM_GETTARGETWINDOW			(BWM_FIRST + 5)
#define BusyWindow_GetTargetWindow(__hwndBusy)\
	((HWND)SNDMSG((__hwndBusy), BWM_GETTARGETWINDOW, 0, 0L))

#define BWM_GETBKCOLOR				(BWM_FIRST + 6)
#define BusyWindow_GetBkColor(__hwndBusy)\
	((COLORREF)SNDMSG((__hwndBusy), BWM_GETBKCOLOR, 0, 0L))

#define BWM_PREPANDSHOW				(BWM_FIRST + 7) 
#define BusyWindow_PrepareAndShow(__hwndBusy, __nCmdShow, __delayMs)\
	(SNDMSG((__hwndBusy), BWM_PREPANDSHOW, (WPARAM)(__nCmdShow), (LPARAM)(__delayMs)))




#define BWWM_UPDATEPROCESSED		(BWM_FIRST + 10)
#define BusyWindowWidget_UpdateProcessed(__hwndBusyWidget, __pszProcessed)\
	(SNDMSG((__hwndBusyWidget), BWWM_UPDATEPROCESSED, 0, (LPARAM)(__pszProcessed)))


/*
*	DropWindow Menu
*/
HMENU DropWindowMenu_Initialize(void);
HMENU DropWindowMenu_GetSubMenu(HWND hwndDB, HMENU hmenuDB, UINT uMenuType);
void DropWindowMenu_ReleaseSubMenu(HWND hwndDB, UINT menuType, HMENU hmenu);

// additional WM_SYSCOMMAND commands
#define SC_DRAGMOVE		0xF012
#define SC_DRAGSIZE_N	0xF003
#define SC_DRAGSIZE_S	0xF006
#define SC_DRAGSIZE_E	0xF002
#define SC_DRAGSIZE_W	0xF001
#define SC_DRAGSIZE_NW	0xF004
#define SC_DRAGSIZE_NE	0xF005
#define SC_DRAGSIZE_SW	0xF007
#define SC_DRAGSIZE_SE	0xF008

/*
*	DropWindow Common Dialogs
*/
BOOL DropboxWindow_SavePlaylistDialog(HWND hwnd, Document *pDoc, LPTSTR pszFileName, INT cchFileNameMax);
BOOL DropboxWindow_OpenPlaylistDialog(HWND hwnd, Document *pDoc, LPTSTR pszFileName, INT cchFileNameMax);
INT_PTR DropboxWindow_RenamePlaylsitDialog(HWND hwnd);

typedef COLORREF (WINAPI *QUERYTHEMECOLOR)(INT index);
typedef HBRUSH (WINAPI *QUERYTHEMEBRUSH)(INT index);

#endif // NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_INTERNAL_HEADER
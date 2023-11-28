#ifndef NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./dropboxClass.h"

class Document;

#define NWC_DROPBOXA		"NullsoftDropBox"
#define NWC_DROPBOXW		L"NullsoftDropBox"

#ifdef UNICODE
#define NWC_DROPBOX		NWC_DROPBOXW
#else
#define NWC_DROPBOX		NWC_DROPBOXA
#endif // !UNICODE

HWND DropBox_CreateWindow(HWND hParent, const DROPBOXCLASSINFO *classInfo);

// DropBox window styles
#define DBS_CAPTION			0x0001	
#define DBS_STATUS			0x0002	
#define DBS_HEADER			(DBS_CAPTION | DBS_STATUS)


#define DBS_SKINWINDOW			0x0100	
#define DBS_WAGLOBAL				0x0200
#define DBS_REGISTERPLAYLIST		0x0400

// DropBoxHeader window style
#define DBHS_ACTIVEHEADER		0x0800


#define DBM_FIRST			(WM_USER + 200)
#define DBM_GETCLASSUID		(DBM_FIRST + 0) // wParam -= not used, lParam = (LPARAM)(UUID*)pGuid. REturns TRUE on success.
#define DropboxWindow_GetClassUid(/*HWND*/ __hwndDB, /* GUID* */__puid)\
	((BOOL)SENDMSG((__hwndDB), DBM_GETCLASSUID, 0, (LPARAM)(__puid)))

#define DBM_REPOSITION		(DBM_FIRST + 1)
#define DBM_ADJUSTRECT		(DBM_FIRST + 2)
#define DBM_GETDROPEFFECT	(DBM_FIRST + 3) // wParam = (WPARAM)(DWORD)keyState, lParam = MAKELPARAM(pt.x, pt.y). return drop effect

#define DBM_GETPROFILE		(DBM_FIRST + 4) // wParam = not used, lParam = not used. returns Profile*.
#define DropboxWindow_GetProfile(/*HWND*/ __hwndDB)\
	((Profile*)SENDMSG((__hwndDB), DBM_GETPROFILE, 0, 0L))

#define DBM_SETPROFILE		(DBM_FIRST + 5) // wParam = not used, lParam = (Profile*). if profile is not null AddRef will be called;
#define DropboxWindow_SetProfile(/*HWND*/ __hwndDB, /*Profile* */ ___profileToSet)\
	((BOOL)SENDMSG((__hwndDB), DBM_SETPROFILE, 0, (LPARAM)(___profileToSet)))


#define DBM_RESIZEFRAME		(DBM_FIRST + 6) // wParam = fAnimate, lParam = MAKELPARAM(clientCX, clientCY)
#define DropboxWindow_ResizeFrame(/*HWND*/ __hwndDB, /*INT*/ __clientCX, /*INT*/ __clientCY, /*BOOL*/__animate)\
	((BOOL)SENDMSG((__hwndDB), DBM_RESIZEFRAME, (WPARAM)(__animate), MAKELPARAM((__clientCX), (__clientCY))))

#define DBM_INSERTENUMERATOR (DBM_FIRST + 7) //wParam = (WPARAM)(UINT)insertBeforeIndex. lParam - (LPARAM)(IFileEnumerator*)pfe; return TRUE if ok
#define DropboxWindow_InsertEnumerator(/*HWND*/ __hwndDB, /*UINT*/ __insertBeforeIndex, /*IFileEnumerator* */__pFileEnumerator)\
	((UINT)SENDMSG((__hwndDB), DBM_INSERTENUMERATOR, (WPARAM)(__insertBeforeIndex), (LPARAM)(__pFileEnumerator)))

#define DBM_CREATEVIEW			(DBM_FIRST + 9) // wParam = (WPARAM)(INT)viewType, lParam = Not used. returns HWND of created view.
#define DropboxWindow_CreateView(/*HWND*/ __hwndDB, /*INT*/ __viewType)\
	((HWND)SENDMSG((__hwndDB), DBM_CREATEVIEW, (WPARAM)(__viewType), 0L))

#define DBM_GETACTIVEVIEW		(DBM_FIRST + 10) // wParam = Not Used, lParam = Not used. returns HWND of current view.
#define DropboxWindow_GetActiveView(/*HWND*/ __hwndDB)\
	((HWND)SENDMSG((__hwndDB), DBM_GETACTIVEVIEW, 0, 0L))

#define DBM_GETITEMCOUNT			(DBM_FIRST + 11) // wParam = Not Used, lParam = Not used. returns UINT items count.
#define DropboxWindow_GetItemCount(/*HWND*/ __hwndDB)\
	((UINT)SENDMSG((__hwndDB), DBM_GETITEMCOUNT, 0, 0L))

#define DBM_DOCUMENTFROMENUMERATOR (DBM_FIRST + 12) //wParam = Not used. lParam - (IFileEnumerator*)pfe;
#define DropboxWindow_DocumentFromEnumerator(/*HWND*/ __hwndDB, /*IFileEnumerator* */__pFileEnumerator)\
	((UINT)SENDMSG((__hwndDB), DBM_DOCUMENTFROMENUMERATOR, 0, (LPARAM)(__pFileEnumerator)))

#define DBM_SETDOCUMENTNAME		(DBM_FIRST + 13) // wParam = Not Used, lParam = (LPARAM)(LPTSTR)pszDocumentName
#define DropboxWindow_SetDocumentName(/*HWND*/ __hwndDB, /*LPCTSTR*/__pszDocumentName)\
	((UINT)SENDMSG((__hwndDB), DBM_SETDOCUMENTNAME, 0, (LPARAM)(__pszDocumentName)))

#define DBMENU_WINDOWCONTEXT		0
#define DBMENU_ITEMCONTEXT		1
#define DBMENU_VIEWCONTEXT		2
#define DBMENU_SELECTIONCONTEXT 3
#define DBMENU_ARRANGEBY			4

#define DBM_GETMENU				(DBM_FIRST + 14) // wParam = (WPARAM)(UINT)uMenuType, lParam = not used. Use DropboxWindow_ReleaseMenu() when done
#define DropboxWindow_GetMenu(/*HWND*/ __hwndDB, /*UINT*/__uMenuType)\
	((HMENU)SENDMSG((__hwndDB), DBM_GETMENU, (WPARAM)(__uMenuType), 0L))

#define DBM_RELEASEMENU			(DBM_FIRST + 15) // wParam = (WPARAM)(UINT)uMenuType, lParam = hMenu
#define DropboxWindow_ReleaseMenu(/*HWND*/ __hwndDB, /*UINT*/__uMenuType, /*HMENU*/ __menu)\
	(SENDMSG((__hwndDB), DBM_RELEASEMENU, (WPARAM)(__uMenuType), (LPARAM)(__menu)))

#define DBM_PROFILENOTIFY		(DBM_FIRST + 16) // wParam = (WPARAM)(UINT)profileEventId, lParam = (LPARAM)(const UUID*)profileUid
#define DropboxWindow_ProfileNotify(/*HWND*/ __hwndDB, /*UINT*/ __profileEventId, /*const UUID* */ __profileUid)\
	(SENDMSG((__hwndDB), DBM_PROFILENOTIFY, (WPARAM)(__profileEventId), (LPARAM)(__profileUid)))


#define DBM_ARRANGEBY		(DBM_FIRST + 17) // wParam = (WPARAM)(INT)columnId, lParam = not used
#define DropboxWindow_ArrangeBy(/*HWND*/ __hwndDB, /*INT*/ __columnId)\
	(SENDMSG((__hwndDB), DBM_ARRANGEBY, (WPARAM)(__columnId), 0L))


#define DBM_GETACTIVEDOCUMENT	(DBM_FIRST + 19) // wParam = not used, lParam = not used
#define DropboxWindow_GetActiveDocument(/*HWND*/ __hwndDB)\
	((Document*)SENDMSG((__hwndDB), DBM_GETACTIVEDOCUMENT, 0, 0L))

#define DBM_CHECKFILEMODIFIED	(DBM_FIRST + 20) // wParam = not used, lParam = not used
#define DropboxWindow_CheckFileModified(/*HWND*/ __hwndDB)\
	((Document*)SENDMSG((__hwndDB), DBM_CHECKFILEMODIFIED, 0, 0L))

#define DBM_SKINCHANGED			(DBM_FIRST + 21) // wParam = not used, lParam = not used
#define DropboxWindow_SkinChanged(/*HWND*/ __hwndDB)\
	SENDMSG((__hwndDB), DBM_SKINCHANGED, 0, 0L)

#define DBM_SKINREFRESHING		(DBM_FIRST + 22) // wParam = not used, lParam = not used. Return 0 to allow, non zero to prevent.
#define DropboxWindow_SkinRefreshing(/*HWND*/ __hwndDB)\
	SENDMSG((__hwndDB), DBM_SKINREFRESHING, 0, 0L)

#define DBM_SKINREFRESHED		(DBM_FIRST + 23) // wParam = not used, lParam = not used.
#define DropboxWindow_SkinRefreshed(/*HWND*/ __hwndDB)\
	SENDMSG((__hwndDB), DBM_SKINREFRESHED, 0, 0L)

#define DBM_PARENTCHANGED		(DBM_FIRST + 24) // wParam = not used, lParam = not used

#define DBM_GETDOCUMENTNAME		(DBM_FIRST + 25) // wPAram = cchMaxBuffer, lParam = (LPCTSTR)pszBuffer
#define DropboxWindow_GetDocumentName(/*HWND*/ __hwndDB, __pszBuffer, __cchBufferMax)\
	((INT)SENDMSG((__hwndDB), DBM_GETDOCUMENTNAME, (WPARAM)(__cchBufferMax), (LPARAM)(__pszBuffer)))

#define DBM_UPDATEDOCUMENTBUSY	(DBM_FIRST + 26) 
#define DropboxWindow_UpdateDocumentBusy(/*HWND*/ __hwndDB)\
	(SENDMSG((__hwndDB), DBM_UPDATEDOCUMENTBUSY, (WPARAM)0, (LPARAM)0L))

#define DBM_CANCELLACTIVEOPERATION	(DBM_FIRST + 27) 
#define DropboxWindow_CancellActiveOperation(/*HWND*/ __hwndDB)\
	(SENDMSG((__hwndDB), DBM_CANCELLACTIVEOPERATION, (WPARAM)0, (LPARAM)0L))

#define DBM_BROADCASTCOMMAND		(DBM_FIRST + 28) 
#define DropboxWindow_BroadcastCommand(/*HWND*/ __hwndDB, /*INT*/ __cmdId, /*HWND*/ __hwndSrc)\
	((BOOL)SENDMSG((__hwndDB), DBM_BROADCASTCOMMAND, (WPARAM)(__cmdId), (LPARAM)(__hwndSrc)))

#define DBM_PROCESSCOMMAND		(DBM_FIRST + 29) 
#define DropboxWindow_ProcessCommand(/*HWND*/ __hwndDB, /*INT*/ __cmdId)\
	((BOOL)SENDMSG((__hwndDB), DBM_PROCESSCOMMAND, (WPARAM)(__cmdId), 0L))


#define DBHM_FIRST				(DBM_FIRST + 100)

#define DBHM_SETDOCUMENT		(DBHM_FIRST + 1) // lParam - Document*
#define DropboxHeader_SetDocument(/*HWND*/ __hwndDBH, /*Document* */ __document)\
	(SENDMSG((__hwndDBH), DBHM_SETDOCUMENT, (WPARAM)0, (LPARAM)(__document)))

#define DBHM_SETVIEW				(DBHM_FIRST + 2) // lParam - DropboxView*
#define DropboxHeader_SetView(/*HWND*/ __hwndDBH, /*DropboxView* */ __view)\
	(SENDMSG((__hwndDBH), DBHM_SETVIEW, (WPARAM)0, (LPARAM)(__view)))

#define DBHM_SHOWTIP				(DBHM_FIRST + 3) // wParam - (WPARAM)(LPCTSTR)pszTipText, lParam - (LPARAM)(const RECT*)prcTipBounds
#define DropboxHeader_ShowTip(/*HWND*/ __hwndDBH, /*LPCTSTR*/ __pszTipText, /*const RECT* */ __prcTipBounds)\
	(SENDMSG((__hwndDBH), DBHM_SHOWTIP, (WPARAM)(__pszTipText), (LPARAM)(__prcTipBounds)))


#define DBHM_POPTIP				(DBHM_FIRST + 4) 
#define DropboxHeader_PopTip(/*HWND*/ __hwndDBH)\
	(SENDMSG((__hwndDBH), DBHM_POPTIP, (WPARAM)0, (LPARAM)0L))


#define DBHM_PROCESSCOMMAND		(DBM_FIRST + 5) 
#define DropboxHeader_ProcessCommand(/*HWND*/ __hwndDBH, /*INT*/ __cmdId)\
	((BOOL)SENDMSG((__hwndDBH), DBHM_PROCESSCOMMAND, (WPARAM)(__cmdId), 0L))

#define DBHM_UPDATEMETRICS		(DBM_FIRST + 6) 
#define DropboxHeader_UpdateMetrics(/*HWND*/ __hwndDBH)\
	(SENDMSG((__hwndDBH), DBHM_UPDATEMETRICS, 0, 0L))


#define DBHP_ERROR		-1 // hittest was outside header
#define DBHP_CLIENT		0  // no part at this point
#define DBHP_MENU		1
#define DBHP_TITLE		2
#define DBHP_METERBAR	3
#define DBHP_TOOLBAR	4

#define DBHM_GETPARTRECT		(DBM_FIRST + 7) // wParam = (WPARAM)(UINT)headerPart - use one of the DBHP_XXX, lParam = (LPARAM)(RECT*)prcPart - will be filled with part rect
#define DropboxHeader_GetPartRect(/*HWND*/ __hwndDBH, /*UINT*/ __headerPart, /*LPCRECT*/ __prcPart)\
	((BOOL)SENDMSG((__hwndDBH), DBHM_GETPARTRECT, (WPARAM)(__headerPart), (LPARAM)(__prcPart)))

#define DBHM_HITTEST		(DBM_FIRST + 8) // wParam = not used, lParam = MAKELPARAM(pt.x, pt.y) - point to test. returns DBHP_XX
#define DropboxHeader_HitTest(/*HWND*/ __hwndDBH, /*LONG*/ __testX, /*LONG*/ __testY)\
	((UINT)SENDMSG((__hwndDBH), DBHM_HITTEST, 0, MAKELPARAM((__testX), (__testY))))

typedef struct __OVERLAYINFO
{
	HINSTANCE hInstance;
	LPCTSTR pszImage;
} OVERLAYINFO;

#define DBHM_SETMENUOVERLAY	(DBM_FIRST + 9) // wParam = not used, lParam = (LPARAM)(OVERLAYINFO*)pOverlayInfo
#define DropboxHeader_SetMenuOverlay(/*HWND*/ __hwndDBH, /*OVERLAYINFO*/ __overlayInfo)\
	((BOOL)SENDMSG((__hwndDBH), DBHM_SETMENUOVERLAY, 0, (LPARAM)(__overlayInfo)))

#define DBHM_REMOVEMENUOVERLAY	(DBM_FIRST + 10) // wParam = not used, lParam = not used
#define DropboxHeader_RemoveMenuOverlay(/*HWND*/ __hwndDBH)\
	(SENDMSG((__hwndDBH), DBHM_REMOVEMENUOVERLAY, 0, 0L))

#define DBHM_PROFILECHANGED	(DBM_FIRST + 11) // wParam = not used, lParam = not used
#define DropboxHeader_ProfileChanged(/*HWND*/ __hwndDBH)\
	(SENDMSG((__hwndDBH), DBHM_PROFILECHANGED, 0, 0L))

#define DBHM_SAVE	(DBM_FIRST + 12) // wParam = not used, lParam = (LPARAM)(Profile*)profile. Return BOOL to indicate success
#define DropboxHeader_Save(/*HWND*/ __hwndDBH, /*Profile* */ __profile)\
	((BOOL)SENDMSG((__hwndDBH), DBHM_SAVE, 0, (LPARAM)(__profile)))



BOOL WINAPI RegisterDropBox(HINSTANCE hInstance);

EXTERN_C const GUID windowSettingsGuid;

#define CFG_ACTIVEVIEW			MAKEINTRESOURCEA(1)
#define CFG_WINDOWSIZE			MAKEINTRESOURCEA(2)

#define MENU_ARRANGEBY_MIN			60100
#define MENU_ARRANGEBY_MAX			60199

typedef enum __LISTCOLUMNID
{
	COLUMN_INVALID = -1,
	COLUMN_FORMATTEDTITLE = 0,
	COLUMN_FILEPATH,
	COLUMN_FILENAME,
	COLUMN_FILESIZE,
	COLUMN_FILETYPE,
	COLUMN_FILEEXTENSION,
	COLUMN_FILEMODIFIED,
	COLUMN_FILECREATED,
	COLUMN_FILEATTRIBUTES,
	COLUMN_TRACKARTIST,
	COLUMN_TRACKALBUM,
	COLUMN_TRACKTITLE,
	COLUMN_TRACKGENRE,
	COLUMN_TRACKLENGTH,
	COLUMN_TRACKBITRATE,
	COLUMN_TRACKNUMBER,
	COLUMN_DISCNUMBER,
	COLUMN_TRACKYEAR,
	COLUMN_TRACKPUBLISHER,
	COLUMN_TRACKCOMPOSER,
	COLUMN_ALBUMARTIST,
	COLUMN_TRACKCOMMENT,
	COLUMN_TRACKBPM,
	COLUMN_EXTENSIONFAMILY,
	COLUMN_LAST,
} LISTCOLUMNID;

#endif // NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_HEADER

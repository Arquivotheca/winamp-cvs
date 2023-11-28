#include "./main.h"
#include "./plugin.h"
#include "./dropWindowInternal.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./dropboxClass.h"
#include "./profile.h"
#include "./filterPolicy.h"
#include "./dropTarget.h"
#include "./fileMetaScheduler.h"
#include "./fileMetaInterface.h"
#include "./fileEnumInterface.h"
#include "./itemView.h"
#include "./itemViewMeta.h"
#include "./itemViewManager.h"
#include "./formatData.h"
#include "./supportedExtensions.h"
#include "../nu/ptrlist.h"
#include "./document.h"
#include "./guiObjects.h"
#include "./winampHook.h"
#include "./messageBoxTweak.h"
#include "./skinWindow.h"
#include "./preferences.h"
#include "../nu/menushortcuts.h"
#include "./documentFilter.h"

#include "../Agave/config/api_config.h"
#include "../Agave/config/ifc_configgroup.h"
#include "../Agave/config/ifc_configitem.h"
#include <api/service/waservicefactory.h>

#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

#define IDC_HEADER		1001
#define IDC_ITEMVIEW		1003
#define IDC_BUSYWINDOW	1004
#define IDC_PROFILE		1005

#define DROPWINDOW_MINWIDTH		96
#define DROPWINDOW_MINHEIGHT		64

#define ACCELERATOR		1

#define ITEMVIEW_MINHEIGHT	96

#define UPDATEBUSYWINDOW_TIMER		40
#define UPDATEBUSYWINDOW_DELAY		250

typedef struct _DROPWND
{
	UUID			classUid;
	Profile			*profile;
	Document			*pDocument;
	HWND				lastFocus;
	HWAHOOK			winampHook;
	HMENU			hMenu;
	HWND				inModalLoop;
	BOOL			checkFile;
	BOOL			updateAsyncOpStatus;
	QUERYTHEMECOLOR GetThemeColor;
	QUERYTHEMEBRUSH GetThemeBrush;
} DROPWND;

#define GetDropWnd(__hwnd) ((DROPWND*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))


static HWND g_hwndInSizeLoop = NULL;
static BOOL g_westDragSize = FALSE;
static HACCEL dropWindowAccelTable = NULL;
static BOOL dropWindowInitialized = FALSE;
static HICON dropBoxIcon = NULL;

static LRESULT CALLBACK DropboxWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK DropboxWindow_OnDocumentNotify(Document *pDocument, UINT eventId, LONG_PTR param, UINT_PTR user);

static void CALLBACK UninitializeDropWindow(void)
{
	if (NULL != dropWindowAccelTable)
	{
		DestroyAcceleratorTable(dropWindowAccelTable);
		dropWindowAccelTable = NULL;
	}
	dropWindowInitialized = FALSE;
}

BOOL DropboxWindow_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX  wc;
	if (GetClassInfoEx(hInstance, NWC_DROPBOX, &wc)) return TRUE;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	if (NULL == dropBoxIcon)
	{
		dropBoxIcon = (HICON)LoadImage(plugin.hDllInstance, MAKEINTRESOURCE(IDI_DROPBOX),
						IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	}

	wc.cbSize		= sizeof(WNDCLASSEX);
	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_DROPBOX;
	wc.lpfnWndProc	= DropboxWindow_WindowProc;
	wc.style			= CS_DBLCLKS;
	wc.hIcon			= dropBoxIcon;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(DROPWND*);
	
	return ( 0 != RegisterClassEx(&wc));
}

BOOL WINAPI RegisterDropBox(HINSTANCE hInstance)
{
	if (!dropWindowInitialized)
	{
		dropWindowInitialized = TRUE;
				
		if (NULL != dropWindowAccelTable)
			DestroyAcceleratorTable(dropWindowAccelTable);

		dropWindowAccelTable = WASABI_API_LOADACCELERATORSW(IDR_DROPWINDOW_ACCELERATORS);
		Plugin_RegisterUnloadCallback(UninitializeDropWindow);
	}

	return (DropboxWindow_RegisterClass(hInstance) && 
			DropboxHeader_RegisterClass(hInstance) &&
			BusyWindow_RegisterClass(hInstance));
}

HWND DropBox_CreateWindow(HWND hParent, const DROPBOXCLASSINFO *classInfo)
{
	if (NULL == classInfo)
		return NULL;

	if (NULL == hParent)
		hParent = plugin.hwndParent;

	if (NULL != hParent)
	{
		HWND hRoot = GetAncestor(hParent, GA_ROOT);
		if (NULL != hRoot) hParent = hRoot;
	}

	RECT rcWindow;	
	if (GetWindowRect(hParent, &rcWindow))
	{
		rcWindow.left += 4;
		rcWindow.top += 4;
		rcWindow.right = rcWindow.left + 180;
		rcWindow.bottom = rcWindow.top + 420;
	}
	else 
		SetRect(&rcWindow, 32, 32, 240, 480);

	CREATESTRUCT cs;
	cs.lpCreateParams = (void*)classInfo;
	cs.hInstance = plugin.hDllInstance;
	cs.hMenu = NULL;
	cs.hwndParent = hParent;
	cs.cy = rcWindow.bottom - rcWindow.top;
	cs.cx = rcWindow.right - rcWindow.left;
	cs.x = (classInfo->x <= -32000) ? rcWindow.left : classInfo->x;
	cs.y = (classInfo->y <= -32000) ? rcWindow.top : classInfo->y;
	cs.style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if (0 != (DBCS_SKINWINDOW & classInfo->style))
		cs.style |= DBS_SKINWINDOW;
	if (0 != (DBCS_WINAMPGROUP & classInfo->style))
		cs.style |= DBS_WAGLOBAL;
	if (0 != (DBCS_REGISTERPLAYLIST & classInfo->style))
		cs.style |= DBS_REGISTERPLAYLIST;
	if (0 != (DBCS_SHOWHEADER & classInfo->style))
		cs.style |= DBS_HEADER;
	
	cs.lpszClass = NWC_DROPBOX;
	cs.dwExStyle = /*WS_EX_CONTROLPARENT |*/ WS_EX_NOPARENTNOTIFY | /*WS_EX_TOOLWINDOW |*/ WS_EX_WINDOWEDGE;

	TCHAR szName[512];
	szName[0] = TEXT('\0');
	
	if (IS_INTRESOURCE(classInfo->pszTitle))
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)classInfo->pszTitle, szName, ARRAYSIZE(szName));
	else
		StringCchCopy(szName, ARRAYSIZE(szName), classInfo->pszTitle);

	if(TEXT('\0') == szName[0])
		StringCchCopy(szName, ARRAYSIZE(szName), TEXT("Dropbox"));
	cs.lpszName = szName;

	HWND hwnd = CreateWindowEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style, 
				cs.x, cs.y, cs.cx, cs.cy, cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);

	if (NULL == hwnd)
		return NULL;
	
	SendMessage(hwnd, DBM_REPOSITION, 0, 0);

	return hwnd;
}

typedef enum
{
	DOCMSGVALUE_TITLE = 0,
	DOCMSGVALUE_PATH = 1,
	DOCMSGVALUE_VALUEINDOCUMENT = 2, // this one is evil. pDoc - actually pointing to the value string or resource id.
} DOCMSGVALUE;

static LPCWSTR FormatDocumentMessage(LPWSTR pszBuffer, INT cchBufferMax, LPCTSTR pszTemplate, Document *pDoc, UINT documentValue)
{
	HRESULT hr = E_FAIL;
	TCHAR szTemplate[1024], szValue[512];
	LPCTSTR pszValue = szValue;
	
	switch(documentValue)
	{
		case DOCMSGVALUE_TITLE:
			if (NULL != pDoc)
				hr = pDoc->GetTitle(szValue, ARRAYSIZE(szValue));
			if (SUCCEEDED(hr)) break;
			// if unable to get title fallback to path
		case DOCMSGVALUE_PATH:
			if (NULL != pDoc)
				hr = pDoc->GetPath(szValue, ARRAYSIZE(szValue));
			break;
		case DOCMSGVALUE_VALUEINDOCUMENT:
			pszValue = (LPCTSTR)pDoc;
			if (IS_INTRESOURCE(pszValue))
			{
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszValue, szValue, ARRAYSIZE(szValue));
				pszValue = szValue;
			}
			hr = S_OK;
			break;
	}

	if (FAILED(hr) || NULL == pszValue)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN, szValue, ARRAYSIZE(szValue));
		pszValue = szValue;
	}
	
	if (IS_INTRESOURCE(pszTemplate))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszTemplate, szTemplate, ARRAYSIZE(szTemplate));
		pszTemplate = szTemplate;
	}
	hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, pszTemplate, pszValue);
	return (SUCCEEDED(hr)) ? pszBuffer : NULL;
}

static void DropboxWindow_SetHeaderText(HWND hwnd, LPCTSTR pszTitle)
{
	TCHAR szText[1024];
	if (IS_INTRESOURCE(pszTitle))
		pszTitle = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszTitle, szText, ARRAYSIZE(szText));
	
	SetDlgItemText(hwnd, IDC_HEADER, pszTitle);
}



HWND DropboxWindow_GetFrame(HWND hwnd)
{
	HWND hFrame = hwnd;
	while (NULL != hFrame && 
		0 != (WS_CHILD & GetWindowLongPtr(hFrame, GWL_STYLE)))
	{
		hFrame = GetAncestor(hFrame, GA_PARENT);
	}
	return hFrame;
}

static BOOL DropboxWindow_PushSize(HWND hwnd, INT cx, INT cy)
{
	SIZE *size = (SIZE*)malloc(sizeof(SIZE));
	if (NULL == size) return NULL;
	size->cx = cx;
	size->cy = cy;
	BOOL result = SetProp(hwnd, TEXT("waDropboxWindowSize"), size);
	if (FALSE == result)
		free(size);
	return result;
}

static BOOL DropboxWindow_PopSize(HWND hwnd, INT *cx, INT *cy)
{
	SIZE *size = NULL;
	size = (SIZE*)GetProp(hwnd, TEXT("waDropboxWindowSize"));
	RemoveProp(hwnd, TEXT("waDropboxWindowSize"));
	if (NULL != size)
	{
		if (NULL != cx) *cx = size->cx;
		if (NULL != cy) *cy = size->cy;
		free(size);
	}
	return (NULL != size);
}

static BOOL DropboxWindow_AdjustViewRect(HWND hwnd, RECT *prcView)
{
	HWND hHeader = GetDlgItem(hwnd, IDC_HEADER);
	if (NULL != hHeader)
	{
		RECT headerRect;
		SetRect(&headerRect, prcView->left, prcView->top, prcView->right, prcView->top + 1000);
		if (0 != SendMessage(hHeader, DBM_ADJUSTRECT, 0, (LPARAM)&headerRect))
			prcView->top -= (headerRect.bottom - headerRect.top);

	}

	if ((prcView->right - prcView->left) < DROPWINDOW_MINWIDTH)
		prcView->right = prcView->left + DROPWINDOW_MINWIDTH;

	if ((prcView->bottom - prcView->top) < DROPWINDOW_MINHEIGHT)
		prcView->bottom = prcView->top + DROPWINDOW_MINHEIGHT;
    		
	return TRUE;
}

static BOOL DropboxWindow_AdjustClientSize(HWND hwnd, INT *cx, INT *cy)
{
	RECT windowRect, clientRect;
	if (!GetWindowRect(hwnd, &windowRect) || !GetClientRect(hwnd, &clientRect))
		return FALSE;
	
	if (NULL != cx) *cx += (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
	if (NULL != cy) *cy += (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
	return TRUE;	
}

static void DropboxWindow_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	DROPWND *pdw = GetDropWnd(hwnd);
	if (NULL == pdw) 
		return;
	
	INT ctrlList[] = { IDC_HEADER, IDC_ITEMVIEW};
	WINDOWPOS szwp[ARRAYSIZE(ctrlList)];

	RECT rc, ri;
	LONG height = 0;

	DWORD styles = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);

	HDWP hdwp = BeginDeferWindowPos(ARRAYSIZE(ctrlList));
	if (NULL == hdwp)
		return;

	GetClientRect(hwnd, &rc);
	height = rc.bottom - rc.top;
	
	for (INT i = 0; i < ARRAYSIZE(ctrlList); i++)
	{
		szwp[i].hwnd = GetDlgItem(hwnd, ctrlList[i]);
		szwp[i].flags = SWP_NOACTIVATE | SWP_NOZORDER | ((bRedraw) ? 0 : (SWP_NOREDRAW | SWP_NOCOPYBITS));

		if (NULL == szwp[i].hwnd && IDC_ITEMVIEW == ctrlList[i])
			szwp[i].hwnd = GetDlgItem(hwnd, IDC_PROFILE);
		
		
		if (NULL != szwp[i].hwnd && 
			height > 0)
		{
			if (IDC_ITEMVIEW == ctrlList[i])
			{
				SetRect(&ri, rc.left, rc.top, rc.right, rc.bottom);
			}
			else
			{
				if (height <= ITEMVIEW_MINHEIGHT)
					SetRect(&ri, rc.left, rc.top, rc.right, rc.top);
				else
				{
					SetRect(&ri, rc.left, rc.top, rc.right, rc.top + height - ITEMVIEW_MINHEIGHT);
					if (0 == SendMessage(szwp[i].hwnd, DBM_ADJUSTRECT, 0, (LPARAM)&ri))
					{
						if (GetWindowRect(szwp[i].hwnd, &ri))
						{
							OffsetRect(&ri, -ri.left, -ri.top);
							if ((ri.bottom - ri.top) > height) 
								ri.bottom = ri.top + height;
							ri.left = rc.left;
							ri.right = rc.right;
						}
						else 
							SetRect(&ri, rc.left, rc.top, rc.right, rc.top);
					}
				}

				if(IDC_HEADER == ctrlList[i])
					rc.top = ri.bottom;
			}

		}
		else 
			SetRect(&ri, rc.left, rc.top, rc.right, rc.top);

		szwp[i].x = ri.left;
		szwp[i].y = ri.top;
		szwp[i].cx = ri.right - ri.left;
		szwp[i].cy = ri.bottom - ri.top;

		height -= szwp[i].cy;
	}


	for(int i = 0; i < ARRAYSIZE(ctrlList) && NULL != hdwp; i++)
	{
		if (NULL == szwp[i].hwnd)
			continue;
		DWORD style = GetWindowStyle(szwp[i].hwnd);
		if ((szwp[i].cy == 0 || 0 == szwp[i].cx) != (0 == (WS_VISIBLE & style)))
		{
			RECT rw;
			GetWindowRect(szwp[i].hwnd, &rw);
			if (((0 == szwp[i].cx) != (0 == (rw.right - rw.left))) ||
				((0 == szwp[i].cy) != (0 == (rw.bottom - rw.top))))
			{
				SetWindowLongPtrW(szwp[i].hwnd, GWL_STYLE, (style & ~WS_VISIBLE) | ((szwp[i].cy == 0 || 0 == szwp[i].cx) ? 0 : WS_VISIBLE));
			}
		}
		hdwp = DeferWindowPos(hdwp, szwp[i].hwnd, NULL, szwp[i].x, szwp[i].y, szwp[i].cx, szwp[i].cy, szwp[i].flags);

	}

	if (NULL != hdwp)
		EndDeferWindowPos(hdwp);
	
	HWND hView = GetDlgItem(hwnd, IDC_ITEMVIEW);
	HWND hctrl = GetDlgItem(hwnd, IDC_BUSYWINDOW);
	if (NULL != hctrl)
	{
		RECT rc;
		if (GetWindowRect(hView, &rc))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rc, 2);
			SetWindowPos(hctrl, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}

static void DropboxWindow_Paint(HWND hwnd, PAINTSTRUCT *pps)
{
	DROPWND *pdw = GetDropWnd(hwnd);
	if (NULL == pdw)
		return;
	
	HDC hdc = pps->hdc;

	if (pps->fErase)
	{		
		SetBkColor(hdc, pdw->GetThemeColor(COLOR_WINDOW));
		ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &pps->rcPaint, L"", 0, 0);
	}
}

static void DropboxWindow_SetChildrenFont(HWND hwnd, HFONT hFont, BOOL bRedraw)
{
	INT ctrlList[] = { IDC_ITEMVIEW};
	HWND hCtrl;
	for (INT i = 0; i < ARRAYSIZE(ctrlList); i++)
	{
		if (NULL != (hCtrl = GetDlgItem(hwnd, ctrlList[i])))
		{
			SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, (LPARAM)bRedraw);
		}
	}
}

static BOOL DropboxWindow_CheckModalState(HWND hwnd, BOOL bVerbal)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->inModalLoop)
		return TRUE;

	if (!IsWindow(pDropWnd->inModalLoop))
	{
		pDropWnd->inModalLoop = NULL;
		return TRUE;
	}
	HWND hRoot = GetAncestor(pDropWnd->inModalLoop, GA_ROOT);
	if (NULL == hRoot)
		hRoot = pDropWnd->inModalLoop;
	
	SetWindowPos(hRoot, HWND_TOP, 0, 0, 0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	if (bVerbal)
	{
		static const GUID accessibilityConfigGroupGUID = 
		{ 0xe2e7f4a, 0x7c51, 0x478f, { 0x87, 0x74, 0xab, 0xbc, 0xf6, 0xd5, 0xa8, 0x57 } };

		#define GetBoolConfig(__group, __itemName, __default)\
			((NULL != (__group)) && NULL != (item = group->GetItem(__itemName)) ? item->GetBool() : (__default))

		waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
		api_config *config = (NULL != serviceFactory) ? (api_config *)serviceFactory->getInterface() : NULL;
		ifc_configgroup *group = (NULL != config) ? config->GetGroup(accessibilityConfigGroupGUID) : NULL;
		ifc_configitem *item;
		
		HWND hPopup = GetWindow(hRoot, GW_ENABLEDPOPUP);
		if (NULL != hPopup && hPopup != hRoot && 
			GetBoolConfig(group, L"modalflash", true))
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize = sizeof(FLASHWINFO);
			flashInfo.dwFlags = FLASHW_CAPTION;
			flashInfo.dwTimeout = 100;
			flashInfo.hwnd = hPopup;
			flashInfo.uCount = 2;
			FlashWindowEx(&flashInfo);
		}

		if (GetBoolConfig(group, L"modalbeep", false))
			MessageBeep(MB_OK);

		if (NULL != config)
			serviceFactory->releaseInterface(config);
	}
	return FALSE;
}




static INT CALLBACK DropboxWindow_FFCallback(embedWindowState *windowState, INT eventId, LPARAM param)
{
	switch(eventId)
	{
		case FFC_CREATEEMBED:
			if(NULL != windowState && NULL != param)
			{			
				INT cx, cy;
				if (DropboxWindow_PopSize(windowState->me, &cx, &cy))
				{
					ifc_window *windowParent = (ifc_window*)param;
					SendMessage(windowParent->gethWnd(), OSWNDHOST_REQUEST_IDEAL_SIZE, cx, cy);
				}
			}
			aTRACE_LINE("**** create embed called ****");
			break;
		case FFC_DESTROYEMBED:
			if(NULL != windowState && NULL != param)
			{
				ifc_window *window = (ifc_window*)param;
				RECT clientRect;
				window->getClientRect(&clientRect);
				DropboxWindow_PushSize(windowState->me, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
			}
		
			aTRACE_LINE("**** destroy embed called ****");
			break;
	}
	return 0;
}

static LRESULT DropboxWindow_SkinWindow(HWND hwnd, BOOL bSkinWindow)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return FALSE;
	
	SendMessage(hwnd, DBM_SKINREFRESHING, 0, 0L);

	HFONT font = NULL;
	if (bSkinWindow)
	{	
		SkinWindow(hwnd, &pDropWnd->classUid, SWF_NOWINDOWMENU, DropboxWindow_FFCallback);
		pDropWnd->GetThemeColor = GetSkinColor;
		pDropWnd->GetThemeBrush = GetSkinBrush;
		font = GetSkinFont();
	}
	else
	{
		pDropWnd->GetThemeColor = GetSystemColor;
		pDropWnd->GetThemeBrush = GetSystemBrush;
		font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		/*SENDWAIPC(hwnd, IPC_UNSKINWINDOW, 0);*/
	}
	if (NULL != font)	
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);

	PostMessage(hwnd, DBM_SKINREFRESHED, 0, 0L);
	return TRUE;
}

static BOOL DropboxWindow_SaveDocument(HWND hwnd, BOOL bSaveAs, BOOL bVerbal)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->pDocument)
		return FALSE;
	TCHAR szBuffer[MAX_PATH];
	HRESULT hr = pDropWnd->pDocument->GetPath(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr) || bSaveAs || TEXT('\0') == szBuffer[0])
	{
		if (!DropboxWindow_CheckModalState(hwnd, bVerbal))
			return FALSE;
		
		pDropWnd->inModalLoop = hwnd;
		BOOL dlgOk = DropboxWindow_SavePlaylistDialog(hwnd, pDropWnd->pDocument, szBuffer, ARRAYSIZE(szBuffer));
		pDropWnd->inModalLoop = NULL;

		if (!dlgOk)
		{		
			if (0 != CommDlgExtendedError())
			{				
				pDropWnd->inModalLoop = hwnd;
				MessageBoxTweak::Show(hwnd, 
					MAKEINTRESOURCE(IDS_ERROR_DIALOGFAILED), MAKEINTRESOURCE(IDS_ERROR_DOCUMENTSAVE), 
					MB_OK | MB_ICONERROR);
				pDropWnd->inModalLoop = NULL;

			}
			return FALSE;
		}

		hr = pDropWnd->pDocument->SetPath(szBuffer);
		if (FAILED(hr))
		{
			pDropWnd->inModalLoop = hwnd;
			MessageBoxTweak::Show(hwnd, 
				MAKEINTRESOURCE(IDS_ERROR_UNKNOWN), MAKEINTRESOURCE(IDS_ERROR_DOCUMENTSAVE), 
				MB_OK | MB_ICONERROR);
			pDropWnd->inModalLoop = NULL;
			return FALSE;
		}
	}
	
	hr = pDropWnd->pDocument->Save(0 != (DBS_REGISTERPLAYLIST & GetWindowLongPtr(hwnd, GWL_STYLE)));
	if (FAILED(hr) && bVerbal)
	{
		TCHAR szMessage[2048], szTemplate[1024];
		pDropWnd->pDocument->GetPath(szBuffer, ARRAYSIZE(szBuffer));
		WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVEFAILED, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szTemplate, szBuffer);

		pDropWnd->inModalLoop = hwnd;
		MessageBoxTweak::Show(hwnd, 
					szMessage, MAKEINTRESOURCE(IDS_ERROR_DOCUMENTSAVE), 
					MB_OK | MB_ICONERROR);
		pDropWnd->inModalLoop = NULL;

	}
	return SUCCEEDED(hr);
}

static void DropboxWindow_DocumentChanged(HWND hwnd, Document *pDocument)
{

	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL != pDropWnd)
	{
		pDropWnd->pDocument = pDocument;
		if (NULL != pDropWnd->pDocument)
			pDropWnd->pDocument->RegisterCallback(DropboxWindow_OnDocumentNotify, (ULONG_PTR)hwnd);
	}

	HWND hctrl = GetDlgItem(hwnd, IDC_ITEMVIEW);
	DropboxView *pView = (NULL != hctrl) ? DropBox_GetItemView(hctrl) : NULL;
	if (NULL != pView) 
	{
		pView->SetDocument(pDocument);
		UpdateWindow(hctrl);
	}

	hctrl = GetDlgItem(hwnd, IDC_HEADER);
	if (NULL != hctrl)
	{
		DropboxHeader_SetDocument(hctrl, pDocument);
		UpdateWindow(hctrl);
	}
}

static void DropboxWindow_ViewChanged(HWND hwnd, DropboxView *pView)
{
	HWND hctrl;
	hctrl = GetDlgItem(hwnd, IDC_HEADER);
	if (NULL != hctrl)
		DropboxHeader_SetView(hctrl, pView);
}

static BOOL DropboxWindow_CloseDocument(HWND hwnd, BOOL bVerbal, BOOL lockApplication)
{	
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd)
		return TRUE;

	if (!DropboxWindow_CheckModalState(hwnd, bVerbal))
		return FALSE;

	if (NULL == pDropWnd->pDocument)
		return TRUE;
	
	if (pDropWnd->pDocument->GetModified())
	{
		HWND hRoot = hwnd;
		DWORD style;
		for(;;)
		{
			style = GetWindowStyle(hRoot);
			if (0 == (WS_VISIBLE & style)) ShowWindow(hRoot, SW_SHOWNA);
			if (0 != (WS_CHILD & style)) hRoot = GetParent(hRoot);
			else break;
		}


		MessageBoxTweak::CTRLOVERRIDE szButtons[] = 
		{
			{IDYES, MAKEINTRESOURCEW(IDS_BUTTON_SAVE)},
			{IDNO, MAKEINTRESOURCEW(IDS_BUTTON_DISCARD)},
			{IDCANCEL, MAKEINTRESOURCEW(IDS_BUTTON_CANCEL)},
		};

		DWORD tweakFlags = MessageBoxTweak::TWEAK_CENTERPARENT;
		if (lockApplication) tweakFlags |= MessageBoxTweak::TWEAK_APPLICATIONMODAL;
		
		TCHAR szMessage[2048];
		FormatDocumentMessage(szMessage, ARRAYSIZE(szMessage), MAKEINTRESOURCE(IDS_DOCUMENT_CLOSEMODIFIED), 
			pDropWnd->pDocument, DOCMSGVALUE_TITLE);
		
		pDropWnd->inModalLoop = hwnd;
		INT result = MessageBoxTweak::ShowEx(hwnd, szMessage, MAKEINTRESOURCE(IDS_DOCUMENT_CONFIRMCLOSE), 
			MB_YESNOCANCEL | MB_ICONQUESTION, tweakFlags, szButtons, ARRAYSIZE(szButtons));

		pDropWnd->inModalLoop = NULL;  
		switch(result)
		{
			case IDYES:
				if (!DropboxWindow_SaveDocument(hwnd, FALSE, bVerbal))
					return FALSE;
				break;
			case IDNO:
				break;
			default:
				return FALSE;
		}
	}

	Document *pDoc = pDropWnd->pDocument;
	pDropWnd->pDocument = NULL;

	DropboxWindow_DocumentChanged(hwnd, NULL);

	pDoc->Close(); // this will call Release() also 

	return TRUE;
}

static BOOL DropboxWindow_NewDocument(HWND hwnd, LPCTSTR pszDocumentName, BOOL bVerbal)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || !DropboxWindow_CheckModalState(hwnd, bVerbal))
		return FALSE;
	if (NULL != pDropWnd->pDocument && !DropboxWindow_CloseDocument(hwnd, bVerbal, FALSE))
		return FALSE;

	HRESULT hr = Document::Create(pszDocumentName, &pDropWnd->pDocument);
	if (FAILED(hr)) 
	{
		if(bVerbal) 
		{
			pDropWnd->inModalLoop = hwnd;
			MessageBoxTweak::Show(hwnd, 
				MAKEINTRESOURCE(IDS_ERROR_UNKNOWN), MAKEINTRESOURCE(IDS_ERROR_DOCUMENTCREATE), 
				MB_OK | MB_ICONERROR);
			pDropWnd->inModalLoop = NULL;
		}
		return FALSE;
	}

	DropboxWindow_DocumentChanged(hwnd, pDropWnd->pDocument);

	return TRUE;
}


static BOOL DropboxWindow_OpenDocumentFromFile(HWND hwnd, LPCTSTR pszFileName, BOOL bVerbal)
{
	TCHAR szBuffer[MAX_PATH];

	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd)
		return FALSE;

	if (NULL != pDropWnd->pDocument && !DropboxWindow_CloseDocument(hwnd, bVerbal, FALSE))
		return FALSE;

	DropboxWindow_SetHeaderText(hwnd, MAKEINTRESOURCE(IDS_PLAYLIST_OPENDIALOGTITLE));
		
	if (NULL == pszFileName || TEXT('\0') == *pszFileName)
	{		
		if (!DropboxWindow_CheckModalState(hwnd, bVerbal))
			return FALSE;

		pDropWnd->inModalLoop = hwnd;
		BOOL dlgOk = DropboxWindow_OpenPlaylistDialog(hwnd, pDropWnd->pDocument, szBuffer, ARRAYSIZE(szBuffer));
		pDropWnd->inModalLoop = NULL;
	
		if (!dlgOk)
		{			
			if (0 != CommDlgExtendedError())
			{
				pDropWnd->inModalLoop = hwnd;
				MessageBoxTweak::Show(hwnd, 
					MAKEINTRESOURCE(IDS_ERROR_DIALOGFAILED), MAKEINTRESOURCE(IDS_ERROR_DOCUMENTLOAD), 
					MB_OK | MB_ICONERROR);
				pDropWnd->inModalLoop = NULL;
			}
			DropboxWindow_NewDocument(hwnd, NULL, FALSE);
			return FALSE;
		}
		pszFileName = szBuffer;
	}

	
	HRESULT hr = Document::OpenFile(pszFileName, &pDropWnd->pDocument);
	if (FAILED(hr)) 
	{
		if (bVerbal)
		{
			TCHAR szMessage[2048];
			FormatDocumentMessage(szMessage, ARRAYSIZE(szMessage), MAKEINTRESOURCE(IDS_ERROR_OPENFAILED), 
				(Document*)pszFileName, DOCMSGVALUE_VALUEINDOCUMENT);

			pDropWnd->inModalLoop = hwnd;
			MessageBoxTweak::Show(hwnd, 
					szMessage, MAKEINTRESOURCE(IDS_ERROR_DOCUMENTLOAD), 
					MB_OK | MB_ICONERROR);
			pDropWnd->inModalLoop = NULL;
		}
		DropboxWindow_NewDocument(hwnd, NULL, FALSE);
		return FALSE;
	}
	
	DropboxWindow_DocumentChanged(hwnd, pDropWnd->pDocument);

	return TRUE;
}

static void DropboxWindow_FileMetaChanged(HWND hwnd, LPCTSTR pszPath)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->pDocument)
		return;
	//size_t fileIndex = pDropWnd->pDocument->FindItemByPath(
}
static void DropboxWindow_ActivateHeader(HWND hwnd, BOOL bActivate)
{
	HWND hHeader = GetDlgItem(hwnd, IDC_HEADER);

	if (NULL != hHeader)
	{
		DWORD currentStyle = GetWindowStyle(hHeader);
		DWORD newStyle = currentStyle & ~DBHS_ACTIVEHEADER;
		if (bActivate) newStyle |= DBHS_ACTIVEHEADER;
		if (newStyle != currentStyle)
			SetWindowLongPtr(hHeader, GWL_STYLE, newStyle);
	}

}
static BOOL DropboxWindow_IsCommandEnabled(HWND hwnd, INT commandId)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return FALSE;

	if (NULL == pDropWnd->profile)
	{
		switch(commandId)
		{
			case ID_WINDOW_CLOSE:
			case ID_PLUGIN_PREFERENCES:
				return TRUE;
		}
		return FALSE;
	}
	
	
	BOOL bAllow = TRUE;
	if (NULL == pDropWnd->pDocument)
	{		
		if (MENU_ARRANGEBY_MIN <= commandId && commandId <= MENU_ARRANGEBY_MAX)
			bAllow = FALSE;
		else
		{			
			switch(commandId)
			{
				case ID_DOCUMENT_SAVE:
				case ID_DOCUMENT_SAVEAS:
				case ID_DOCUMENT_RENAME:
					bAllow = FALSE;
					break;
			}
		}
	}

	if (NULL != pDropWnd->pDocument && pDropWnd->pDocument->QueryAsyncOpInfo(NULL))
	{
		if (MENU_ARRANGEBY_MIN <= commandId && commandId <= MENU_ARRANGEBY_MAX)
			bAllow = FALSE;
		else
		{
			switch(commandId)
			{
				case ID_DOCUMENT_NEW:
				case ID_DOCUMENT_OPEN:
				case ID_DOCUMENT_SAVE:
				case ID_DOCUMENT_SAVEAS:
				case ID_DOCUMENT_RENAME:
					bAllow = FALSE;
					break;
			}
		}
	}

	return bAllow;
}

static void DropboxWindow_UpdateClassProfile(HWND hwnd, Profile *profile)
{
	UUID classUid;
	DROPBOXCLASSINFO *classInfo;
	classInfo = (DropboxWindow_GetClassUid(hwnd, &classUid)) ?
					Plugin_FindRegisteredClass(classUid) : NULL;
	if (NULL == classInfo || 
		DBCS_REMEMBERPROFILE != ((DBCS_REMEMBERPROFILE | DBCS_DONOTSAVE) & classInfo->style)) 
	{
		return;
	}

	
	UUID profileUid;
	if (NULL == profile || FAILED(profile->GetUID(&profileUid)))
		profileUid = GUID_NULL;
	
	classInfo->profileUid = profileUid;
}

static void DropboxWindow_UpdateClassPos(HWND hwnd)
{
	UUID classUid;
	DROPBOXCLASSINFO *classInfo;
	classInfo = (DropboxWindow_GetClassUid(hwnd, &classUid)) ?
					Plugin_FindRegisteredClass(classUid) : NULL;
	if (NULL == classInfo ||
		0 != (DBCS_DONOTSAVE & classInfo->style)) 
	{
		return;
	}

	RECT windowRect;
	if (GetWindowRect(hwnd, &windowRect))
	{
		classInfo->x = windowRect.left;
		classInfo->y = windowRect.top;
	}
	
}

static void DropboxWindow_Save(HWND hwnd)
{
	INT cx, cy;
	if (DropboxWindow_PopSize(hwnd, &cx, &cy))
	{		
		DropboxWindow_AdjustClientSize(hwnd, &cx, &cy);
		SetWindowPos(hwnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
	}
		
	Profile *profile = DropboxWindow_GetProfile(hwnd);
	DropboxWindow_UpdateClassProfile(hwnd, profile);
	DropboxWindow_UpdateClassPos(hwnd);
	DropboxWindow_SaveViewSize(hwnd, profile);
	DropboxWindow_SaveClassInfo(hwnd, profile);

	HWND hControl;
	hControl = GetDlgItem(hwnd, IDC_HEADER);
	if (NULL != hControl) 
		DropboxHeader_Save(hControl, profile);

	hControl = GetDlgItem(hwnd, IDC_ITEMVIEW);
	DropboxView *pView = (NULL != hControl) ? DropBox_GetItemView(hControl) : NULL;
	if (NULL != pView)
	{
		pView->Save(profile);
	}
}

/*
*  document notifications
*/


static void CALLBACK DropboxWindow_OnUpdateBusyWindowTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);

	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL != pDropWnd)
	{		
		PostMessage(hwnd, DBM_UPDATEDOCUMENTBUSY, 0, 0L);
		pDropWnd->updateAsyncOpStatus = FALSE;
	}
}
static void CALLBACK DropboxWindow_OnDocumentNotify(Document *pDocument, UINT eventId, LONG_PTR param, UINT_PTR user)
{
	HWND hwnd = (HWND)user;
	DROPWND *pDropWnd = (NULL != hwnd) ? GetDropWnd(hwnd) : NULL;
	
	switch(eventId)
	{
		case Document::EventCheckFileModified:
			if (NULL != pDropWnd && pDocument == pDropWnd->pDocument)
			{	
				pDropWnd->checkFile = TRUE;
				HWND hRoot = GetAncestor(hwnd, GA_ROOT);
				if (NULL == hRoot) hRoot = hwnd;
				if (hRoot == GetActiveWindow())
					PostMessage(hwnd, DBM_CHECKFILEMODIFIED, 0, 0L);
			}
			break;
		case Document::EventAsyncStarted:
			PostMessage(hwnd, DBM_UPDATEDOCUMENTBUSY, 0, 0L);
			break;
		case Document::EventAsyncFinished:
			PostMessage(hwnd, DBM_UPDATEDOCUMENTBUSY, 0, 0L);
		case Document::EventAsyncStep:
			if (!pDropWnd->updateAsyncOpStatus)
			{
				if (SetTimer(hwnd, UPDATEBUSYWINDOW_TIMER, UPDATEBUSYWINDOW_DELAY, DropboxWindow_OnUpdateBusyWindowTimer))
					pDropWnd->updateAsyncOpStatus = TRUE;
			}
			break;
	}
}
/*
* Message Handlers
*/

static LRESULT CALLBACK DropboxWindow_WinampHook(HWAHOOK hWaHook, UINT whcbId, WPARAM param, ULONG_PTR user)
{
	switch(whcbId)
	{
		case WHCB_OKTOQUIT:
			if (!DropboxWindow_CloseDocument((HWND)user, TRUE, TRUE)) return 0;
			DropboxWindow_Save((HWND)user);
			break;
		case WHCB_SKINCHANGING:
			{
				LRESULT result;
				if (!DropboxWindow_SkinRefreshing((HWND)user))
				{
					result = CallNextWinampHook(hWaHook, whcbId, param);
					PostMessage((HWND)user, DBM_SKINREFRESHED, 0, 0L);
				}
				else
					result = 0;
				
				return result;
			}
			break;
		case WHCB_SKINCHANGED:
			SendNotifyMessage((HWND)user, DBM_SKINCHANGED, 0, 0L);
			break;
		case WHCB_RESETFONT:
			if (0 != (DBS_SKINWINDOW & GetWindowLongPtr((HWND)user, GWL_STYLE)))
				PostMessage((HWND)user, WM_SETFONT, (WPARAM)GetSkinFont(), TRUE);
			break;
		case WHCB_FILEMETACHANGED:
			DropboxWindow_FileMetaChanged((HWND)user, (LPCTSTR)param);
			break;
	}
	return CallNextWinampHook(hWaHook, whcbId, param);
}

static LRESULT DropboxWindow_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{
	HWND hCtrl;
	DROPWND *pdw;

	DROPBOXCLASSINFO *classInfo= (DROPBOXCLASSINFO*)pcs->lpCreateParams;
	if (NULL == classInfo)
		return -1;

	pdw = (DROPWND*)malloc(sizeof(DROPWND));
	if (NULL == pdw)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	ZeroMemory(pdw, sizeof(DROPWND));
	
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)pdw) && ERROR_SUCCESS != GetLastError())
	{
		free(pdw);
		DestroyWindow(hwnd);
		return -1;
	}

	SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

		
	pdw->classUid = classInfo->classUid;
	
	DropTarget::RegisterWindow(hwnd);
	pdw->winampHook = AttachWinampHook(plugin.hwndParent, DropboxWindow_WinampHook, (ULONG_PTR)hwnd);
	DWORD windowStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ((DBS_HEADER | DBS_SKINWINDOW) & pcs->style);
	if (DBS_HEADER & windowStyle) 
	{
		windowStyle |= WS_VISIBLE;
		hCtrl = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, NWC_DROPBOXHEADER, NULL, windowStyle, 
				0, 0, 0, 0, hwnd, (HMENU)IDC_HEADER, NULL, NULL);
	}
	else
		hCtrl = NULL;

	if (NULL != hCtrl)
	{		
		SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetPluginFont(PLUGINFONT_HEADERTITLE), (LPARAM)FALSE);
	}
	
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	if (NULL != hMenu)
	{
		EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND |MF_GRAYED);
		EnableMenuItem(hMenu, SC_MAXIMIZE, MF_BYCOMMAND |MF_GRAYED);
		EnableMenuItem(hMenu, SC_MINIMIZE, MF_BYCOMMAND |MF_GRAYED);
	}

		
	if (NULL != WASABI_API_APP)
	{ 
		if (DBS_WAGLOBAL & pcs->style)
			WASABI_API_APP->app_registerGlobalWindow(hwnd);

		if (NULL != dropWindowAccelTable) 
			WASABI_API_APP->app_addAccelerators(hwnd, &dropWindowAccelTable, 1, TRANSLATE_MODE_CHILD);
	}
	
	DropboxWindow_SetProfile(hwnd, PLUGIN_PROFILEMNGR->LoadProfile(classInfo->profileUid));

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

	DropboxWindow_SkinWindow(hwnd, (0 != (DBS_SKINWINDOW & pcs->style)));

	return FALSE;
}

static void DropboxWindow_OnDestroy(HWND hwnd)
{
	RevokeDragDrop(hwnd);

	DROPWND *pdw = GetDropWnd(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	if (NULL != pdw)
	{
		if (NULL != pdw->pDocument)
		{
			pdw->pDocument->Close();
			pdw->pDocument = NULL;
		}

		if (NULL != pdw->winampHook)
		{
			ReleaseWinampHook(plugin.hwndParent, pdw->winampHook);
			pdw->winampHook = NULL;
		}

		if (NULL != pdw->profile)
		{
			DropboxWindow_UnregisterProfileCallback(hwnd);
			pdw->profile->Release();
			pdw->profile = NULL;
		}

		free(pdw);
	}

	

	if (WASABI_API_APP)
	{
		WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
		WASABI_API_APP->app_removeAccelerators(hwnd);
	}

	HANDLE hSupportedExt = GetDefaultSupportedExtensionsHandle();
	if (NULL != hSupportedExt)
		ReleaseSupportedExtensions(hSupportedExt);
}

static BOOL DropboxWindow_OnProcessCommand(HWND hwnd, INT commandId)
{
	switch (commandId)
	{	
		case ID_DOCUMENT_NEW:
			DropboxWindow_NewDocument(hwnd, NULL, TRUE);
			return TRUE;
		case ID_DOCUMENT_OPEN:
			DropboxWindow_OpenDocumentFromFile(hwnd, NULL, TRUE);
			return TRUE;
		case ID_DOCUMENT_SAVE:
		case ID_DOCUMENT_SAVEAS:
			DropboxWindow_SaveDocument(hwnd, (ID_DOCUMENT_SAVEAS == commandId), TRUE);
			return TRUE;
		case ID_DOCUMENT_RENAME:
			if (DropboxWindow_CheckModalState(hwnd, TRUE))
			{
				DROPWND *pDropWnd = GetDropWnd(hwnd);
				if (NULL != pDropWnd || NULL != pDropWnd->pDocument)
				{
					pDropWnd->inModalLoop = hwnd;
					DropboxWindow_RenamePlaylsitDialog(hwnd);
					pDropWnd->inModalLoop = NULL;
				}
			}
			return TRUE;
	}

	if (MENU_ARRANGEBY_MIN <= commandId && commandId <= MENU_ARRANGEBY_MAX)
	{
		DropboxWindow_ArrangeBy(hwnd, commandId - MENU_ARRANGEBY_MIN);
		return TRUE;
	}
	return FALSE;
}

static BOOL DropboxWindow_OnBroadcastCommand(HWND hwnd, INT commandId, HWND hSource)
{
	if (hSource != hwnd && 
		!DropboxWindow_IsCommandEnabled(hwnd, commandId))
	{
		return FALSE;
	}

	HWND hctrl;
	hctrl = GetDlgItem(hwnd, IDC_ITEMVIEW);
	if (NULL != hctrl && hctrl != hSource)
	{
		DropboxView *pView = DropBox_GetItemView(hctrl);
		if (NULL != pView && S_OK == pView->ProcessCommand(commandId))
			return TRUE;
	}
	
	hctrl = GetDlgItem(hwnd, IDC_HEADER);
	if (NULL != hctrl  && hctrl != hSource &&
		DropboxHeader_ProcessCommand(hctrl, commandId))
	{
		return TRUE;
	}
	
	return (hwnd != hSource) ? DropboxWindow_OnProcessCommand(hwnd, commandId) : FALSE;
}

static void DropboxWindow_OnCommand(HWND hwnd, INT ctrlId, INT eventId, HWND hCtrl)
{
	if (NULL == hCtrl && !DropboxWindow_IsCommandEnabled(hwnd, ctrlId))
		return;
		
	switch (ctrlId)
	{	
		case IDOK:			
			return;
		case IDCANCEL:
		case ID_WINDOW_CLOSE:
			{
				POINT pt;
				GetCursorPos(&pt);
				SendMessage(hwnd, WM_SYSCOMMAND, (WPARAM)SC_CLOSE, MAKELPARAM(pt.x, pt.y));
			}
			return;
		case IDC_PROFILE:
			switch(eventId)
			{
				case PMN_PROFILESELECTED:
					DropboxWindow_SetProfile(hwnd, ProfileManagerView_GetProfile(hCtrl));
					break;
			}
			return;
		case ID_PLUGIN_PREFERENCES:
			if (Plugin_ShowPreferences()) return;
			break;
	}
	DropboxWindow_OnBroadcastCommand(hwnd, ctrlId, NULL);
}

static void DropboxWindow_OnWindowPosChanging(HWND hwnd, WINDOWPOS *pwp)
{
	if (hwnd == g_hwndInSizeLoop && g_westDragSize)
		pwp->flags |= SWP_NOCOPYBITS;
	
	if (pwp->cx < DROPWINDOW_MINWIDTH) pwp->cx = DROPWINDOW_MINWIDTH;
	if (pwp->cy < DROPWINDOW_MINHEIGHT) pwp->cy = DROPWINDOW_MINHEIGHT;
}

static void DropboxWindow_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right) DropboxWindow_Paint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
}

static void DropboxWindow_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & options));
	DropboxWindow_Paint(hwnd, &ps);
}

static void DropboxWindow_OnStyleChanged(HWND hwnd, UINT nStyleType, STYLESTRUCT *pss)
{
	#define ISSTYLECHANGED(__style) ((__style) & pss->styleOld) != ((__style) & pss->styleNew)
	if (GWL_STYLE == nStyleType)
	{
		HWND hCtrl;
		if (ISSTYLECHANGED(DBS_HEADER))
		{
			hCtrl = GetDlgItem(hwnd, IDC_HEADER);
			if (NULL != hCtrl)
			{
				DWORD ctrlStyle = GetWindowStyle(hCtrl);
				ctrlStyle &= ~(DBS_HEADER | WS_VISIBLE);
				ctrlStyle |= (DBS_HEADER & pss->styleNew);
				if (0 != (DBS_HEADER & ctrlStyle)) ctrlStyle |= WS_VISIBLE;
				SetWindowLongPtr(hCtrl, GWL_STYLE, ctrlStyle);
			}
		}
		
		if (ISSTYLECHANGED(WS_CHILD) && 0 != pss->styleNew && 0 != pss->styleOld)
		{
			PostMessage(hwnd, DBM_PARENTCHANGED, 0, 0L);
		}

	}
}
static void DropboxWindow_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;

	DropboxWindow_UpdateLayout(hwnd, 
		((0 == (SWP_NOREDRAW & pwp->flags))));

	if (hwnd == g_hwndInSizeLoop)
	{
		UpdateWindow(hwnd);
		INT szControls[] = {IDC_HEADER, IDC_ITEMVIEW, IDC_PROFILE};
		for (INT i = 0; i < ARRAYSIZE(szControls); i++)
		{
			HWND hControl = GetDlgItem(hwnd, szControls[i]);
			if (NULL != hControl) UpdateWindow(hControl);
		}
	}
}
static void DropboxWindow_OnReposition(HWND hwnd)
{
	HMONITOR hMonitor;
    MONITORINFO mi;
	RECT rcWindow, rcWork;
	DWORD windowStyle;

	windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_CHILD & windowStyle))
		return;

	if (!GetWindowRect(hwnd, &rcWindow))
		return;

	hMonitor = MonitorFromRect(&rcWindow, MONITOR_DEFAULTTONEAREST);
	if (NULL == hMonitor)
		return;

	mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &mi);
	
	CopyRect(&rcWork, &mi.rcWork);

	if (rcWindow.left < rcWork.left ||
		rcWindow.top < rcWork.top ||
		rcWindow.right > rcWork.right ||
		rcWindow.bottom > rcWork.bottom)
	{
		// update hMonitor
	}

	
	POINT pt = *(POINT*)&rcWindow;
	
	if (rcWindow.left < rcWork.left)
	{
		if (rcWindow.right > rcWork.left)
		{
			if ((rcWindow.right - rcWork.left) < 36)
				pt.x = rcWork.left - ((rcWindow.right - rcWindow.left) - 36);
		}
		else 
			pt.x = rcWork.left + 1;
	}
	else if (rcWindow.left >= rcWork.right)
	{
		pt.x = rcWork.right - (rcWindow.right - rcWindow.left) -1;
	}
	else if (rcWindow.left > (rcWork.right - 48))
	{
		pt.x = rcWork.right - 48;
	}


	if (rcWindow.top < rcWork.top)
	{
		pt.y = rcWork.top + 1;
	}
	else if (rcWindow.top >= rcWork.bottom)
	{
		pt.y = rcWork.bottom - (rcWindow.bottom - rcWindow.top) - 1;
	}
	else if (rcWindow.top > (rcWork.bottom - 36))
	{
		pt.y = rcWork.bottom - 36;
	}
	

	if (pt.x != rcWindow.left || pt.y != rcWindow.top)
	{
		if(0 != (WS_OVERLAPPED & windowStyle))
		{
			HWND hOwner = GetWindow(hwnd, GW_OWNER);
			if (NULL != hOwner)
				MapWindowPoints(HWND_DESKTOP, hOwner, &pt, 1);
		}
		SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER); 
	}

	
}
static void DropboxWindow_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw)
{
	DropboxWindow_SetChildrenFont(hwnd, hFont, bRedraw);
}

static LRESULT DropboxWindow_OnGetDropEffect(HWND hwnd, DWORD keyState, POINTS pts)
{
	return DROPEFFECT_NONE;
}


static LRESULT DropboxWindow_OnNotify(HWND hwnd, INT ctrlId, NMHDR *phdr)
{
	DropboxView *pView;
	switch(phdr->idFrom)
	{
		case IDC_ITEMVIEW:
			if (NULL != (pView = DropBox_GetItemView(phdr->hwndFrom)))
			{
				LRESULT result = 0;
				if (SUCCEEDED(pView->ProcessNotification(phdr, &result)))
					return result;
			}
			break;
	}
	return 0;
}
static BOOL DropboxWindow_OnDrawItem(HWND hwnd, INT ctrlId, DRAWITEMSTRUCT *pdis)
{
	switch(ctrlId)
	{
		case IDC_ITEMVIEW:
			{				
				DropboxView *pView = DropBox_GetItemView(pdis->hwndItem);
				if(NULL != pView)
					return (S_OK == pView->DrawItem(pdis)); 
			}
			break;
	}
	return FALSE;
}

static BOOL DropboxWindow_OnMeasureItem(HWND hwnd, INT ctrlId, MEASUREITEMSTRUCT *pmis)
{
	switch(ctrlId)
	{
		case IDC_ITEMVIEW:
			{					
				HWND hView = GetDlgItem(hwnd, ctrlId);
				DropboxView *pView = (NULL != hView) ? DropBox_GetItemView(hView) : NULL;
				if(NULL != pView)
					return (S_OK == pView->MeasureItem(pmis)); 
			}
			break;
	}
	return FALSE;
}

static LRESULT DropboxWindow_OnCreateView(HWND hwnd, INT viewType)
{
	DROPWND *pdw = GetDropWnd(hwnd);
	if (!pdw) return NULL;

	DWORD windowStyle = GetWindowStyle(hwnd);

	HWND hctrl = GetDlgItem(hwnd, IDC_ITEMVIEW);
	if (NULL != hctrl)
	{
		DropboxWindow_ViewChanged(hwnd, NULL);
		DestroyWindow(hctrl);
		hctrl = NULL;
	}

	DropboxViewMeta *viewMeta = PLUGIN_VIEWMNGR->FindById(viewType);
	if (NULL != viewMeta)
		hctrl = viewMeta->CreateView(WS_EX_NOPARENTNOTIFY, 
									WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
									0, 0, 0, 0, 
									hwnd, IDC_ITEMVIEW, plugin.hDllInstance);
	
	DropboxView *pView = NULL;

	if (NULL != hctrl)
	{
		pView = DropBox_GetItemView(hctrl);
		if (NULL != pView)
		{
			if (0 != (DBS_SKINWINDOW & windowStyle))
				pView->SetSkinned(TRUE);
			pView->SetDocument(pdw->pDocument);
		}
		
		HFONT hf = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		if (NULL != hf)
			SendMessage(hctrl, WM_SETFONT, (WPARAM)hf, TRUE);
	}

	SetWindowPos(hwnd, NULL, 0, 0,0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	DropboxWindow_ViewChanged(hwnd, pView);

	return (LRESULT)hctrl;
}

static LRESULT DropboxWindow_OnGetCurrentView(HWND hwnd)
{
	return (LRESULT)GetDlgItem(hwnd, IDC_ITEMVIEW);
}

static LRESULT DropboxWindow_OnGetClassUid(HWND hwnd, UUID *puid)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == puid) return FALSE;
	
	*puid = pDropWnd->classUid;
	return TRUE;
}

static UINT DropboxWindow_OnGetItemCount(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	return (NULL != pDropWnd) ? (UINT)pDropWnd->pDocument->GetItemCount() : 0;
}

static void DropboxWindow_OnActivate(HWND hwnd, UINT uActivate, HWND hwndOther, BOOL bMinimized)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return;

	POINT pt;
	
	switch(uActivate)
	{
		case WA_INACTIVE:
			pDropWnd->lastFocus = GetFocus();
			if (!IsChild(hwnd, pDropWnd->lastFocus))
				pDropWnd->lastFocus = NULL;

			DropboxWindow_ActivateHeader(hwnd, FALSE);
			if (NULL != WASABI_API_APP)
				 WASABI_API_APP->ActiveDialog_Unregister(hwnd);
			break;
		
		case WA_CLICKACTIVE:
			if (GetCursorPos(&pt))
			{
				MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
				HWND hTarget = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
				if (hTarget && hTarget != hwnd) pDropWnd->lastFocus = hTarget;
			}
			
			DropboxWindow_ActivateHeader(hwnd, TRUE);
			
			if (NULL != WASABI_API_APP)
				 WASABI_API_APP->ActiveDialog_Register(hwnd);
			break;

		case WA_ACTIVE:

			DropboxWindow_ActivateHeader(hwnd, TRUE);

			if (NULL != WASABI_API_APP)
				 WASABI_API_APP->ActiveDialog_Register(hwnd);

			break;
	}

	UpdateWindow(hwnd);

	if (pDropWnd->checkFile && WA_INACTIVE != uActivate)
	{
		PostMessage(hwnd, DBM_CHECKFILEMODIFIED, 0, 0L);
	}
}

static void DropboxWindow_OnSetFocus(HWND hwnd, HWND hLost)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	HWND hChild, hTab;
	hChild = FindWindowEx(hwnd, NULL, NULL, NULL);
	hTab = NULL;
	
	if (NULL != pDropWnd)
	{
		while(pDropWnd->lastFocus && IsChild(hwnd, pDropWnd->lastFocus))
		{
			if (IsWindowEnabled(pDropWnd->lastFocus) && IsWindowVisible(pDropWnd->lastFocus) && 
				0 != (WS_TABSTOP & GetWindowLongPtrW(pDropWnd->lastFocus, GWL_STYLE)))
			{
				hTab = pDropWnd->lastFocus;
				break;
			}
			pDropWnd->lastFocus = GetParent(pDropWnd->lastFocus);
		}
	}

	if (NULL == hTab)
	{
		hTab = (hChild) ? GetNextDlgTabItem(hwnd, hChild, FALSE) : hwnd;
	}
	
	if (NULL != hTab && hwnd != hTab && 
		IsWindowEnabled(hTab) && 
		IsWindowVisible(hTab))
	{
		TCHAR szName[128];
		if (NULL != hChild && 
			GetClassName(hChild, szName, ARRAYSIZE(szName)) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szName, -1, TEXT("#32770"), -1))
		{
			if (IsWindowEnabled(hChild))
				PostMessage(hChild, WM_NEXTDLGCTL, (WPARAM)hTab, TRUE);
			else
				DefWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hLost, 0L);
		}
		else 
		{
			SetFocus(hTab);
		}
		return;
	}
	
	DefWindowProc(hwnd, WM_SETFOCUS, (WPARAM)hLost, 0L);
}


static BOOL DropboxWindow_OnSysCommand(HWND hwnd, UINT cmdId, POINTS pts)
{
	switch(cmdId)
	{
		case SC_DRAGSIZE_W:
		case SC_DRAGSIZE_NW:
		case SC_DRAGSIZE_SW:
			g_westDragSize = TRUE;
			DefWindowProc(hwnd, WM_SYSCOMMAND, (WPARAM)cmdId, *(LPARAM*)&pts);
			g_westDragSize = FALSE;
			return TRUE;
	}
	return FALSE;
	
}
static LRESULT DropboxWindow_OnSetDocumentName(HWND hwnd, LPCTSTR pszDocName)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->pDocument ) return FALSE;

	HRESULT hr = pDropWnd->pDocument->SetTitle(pszDocName);
	if (SUCCEEDED(hr))
		pDropWnd->pDocument->FlushTitle();

	else
	{
		pDropWnd->inModalLoop = hwnd;

		MessageBoxTweak::Show(hwnd, 
					MAKEINTRESOURCE(IDS_PLAYLISTNAME_BADFORMAT), MAKEINTRESOURCE(IDS_ERROR_BADPLAYLISTNAME), 
					MB_OK | MB_ICONERROR);

		pDropWnd->inModalLoop = NULL;
	}
	
    return SUCCEEDED(hr);
}


static void DropboxWindow_OnClose(HWND hwnd)
{
	if (!DropboxWindow_CloseDocument(hwnd, TRUE, FALSE))
		return;

	DropboxWindow_Save(hwnd);
	DefWindowProc(hwnd, WM_CLOSE, 0, 0L);
}


static LRESULT DropboxWindow_OnGetMenu(HWND hwnd, UINT menuType)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return NULL;

	if (NULL == pDropWnd->hMenu)
	{
		pDropWnd->hMenu = DropWindowMenu_Initialize();
		if (NULL == pDropWnd->hMenu) return NULL;
	}
	HMENU hMenu = DropWindowMenu_GetSubMenu(hwnd, pDropWnd->hMenu, menuType);
	if (NULL != hMenu)
	{
		UINT uEnable;
		INT itemCount = GetMenuItemCount(hMenu);
		UINT itemId;
		for(INT i = 0; i < itemCount; i++)
		{
			itemId = GetMenuItemID(hMenu, i);
			if (((UINT)-1) != itemId)
			{
				uEnable = (DropboxWindow_IsCommandEnabled(hwnd, itemId)) ? 
							MF_ENABLED : 
							(MF_DISABLED | MF_GRAYED);
				uEnable |= MF_BYPOSITION;
				EnableMenuItem(hMenu, i, uEnable);
			}
		}
			
		if(NULL != dropWindowAccelTable)
			AppendMenuShortcuts(hMenu, &dropWindowAccelTable, 1, MSF_REPLACE | MSF_WALKSUBMENU);
	}
	return (LRESULT)hMenu;
}

static void DropboxWindow_OnReleaseMenu(HWND hwnd, UINT menuType, HMENU hmenu)
{
	DropWindowMenu_ReleaseSubMenu(hwnd, menuType, hmenu);
}
static LRESULT DropboxWindow_OnGetActiveDocument(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	return (LRESULT)((NULL != pDropWnd) ? pDropWnd->pDocument : NULL);
}

static void DropboxWindow_OnCheckFileModified(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd)
		return;

	if (pDropWnd->pDocument->QueryAsyncOpInfo(NULL))
		return; // do not check while async active

	pDropWnd->checkFile = FALSE;
	if (NULL == pDropWnd->pDocument)
		return;
	
	BOOL fileModified;
	if (FAILED(pDropWnd->pDocument->GetFileModified(&fileModified)) || !fileModified)
		return;

	MessageBoxTweak::CTRLOVERRIDE szButtons[] = 
	{
		{IDYES, MAKEINTRESOURCEW(IDS_BUTTON_RELOAD)},
		{IDNO, MAKEINTRESOURCEW(IDS_BUTTON_IGNORE)},
	};

	DWORD tweakFlags = MessageBoxTweak::TWEAK_CENTERPARENT;
	
	TCHAR szMessage[2048];
	FormatDocumentMessage(szMessage, ARRAYSIZE(szMessage), MAKEINTRESOURCE(IDS_DOCUMENT_MODIFIEDOUTSIDE), 
		pDropWnd->pDocument, DOCMSGVALUE_TITLE);
	
	pDropWnd->inModalLoop = hwnd;
	INT result = MessageBoxTweak::ShowEx(hwnd, szMessage, MAKEINTRESOURCE(IDS_DOCUMENT_CONFIRMRELOAD), 
		MB_YESNO | MB_ICONQUESTION, tweakFlags, szButtons, ARRAYSIZE(szButtons));
	pDropWnd->inModalLoop = NULL;  

	if (IDYES == result)
	{
		pDropWnd->pDocument->Reload();
		HWND hctrl = GetDlgItem(hwnd, IDC_ITEMVIEW);
		InvalidateRect(hctrl, NULL, TRUE);
	}
}

static void DropboxWindow_OnSkinChanged(HWND hwnd)
{
	DWORD windowStyle = GetWindowStyle(hwnd);

	if (0 == (DBS_SKINWINDOW & windowStyle))
		return;

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE & ~windowStyle);

	INT ctrlList[] = { IDC_HEADER, IDC_ITEMVIEW, IDC_PROFILE, IDC_BUSYWINDOW};
	for (INT i = 0; i < ARRAYSIZE(ctrlList); i++)
	{
		HWND hctrl = GetDlgItem(hwnd, ctrlList[i]);
		if (NULL != hctrl)
			SendMessage(hctrl, DBM_SKINCHANGED, 0, 0L);
	}

	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
	
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);

}

static LRESULT DropboxWindow_OnSkinRefreshing(HWND hwnd)
{
	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
		DropboxWindow_PushSize(hwnd, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	HWND hRoot = GetAncestor(hwnd, GA_ROOT);
	if (hRoot != hwnd)
	{
		HWND hDlgParent = (HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);
		if (hDlgParent == (HWND)(LONG_PTR)GetWindowLongPtr(hRoot, GWLP_HWNDPARENT))
		{
            SetWindowLongPtr(hRoot, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)plugin.hwndParent);
		}
				
	}
	return 0;
	
}

static void DropboxWindow_OnSkinRefreshed(HWND hwnd)
{
	DropboxWindow_PopSize(hwnd, NULL, NULL);
	
	HWND hFrame = DropboxWindow_GetFrame(hwnd);
	if (hwnd != hFrame)
	{
		UINT state = (IsWindowVisible(hwnd) && IsWindowEnabled(hwnd) && (hFrame == GetActiveWindow())) ? WA_ACTIVE : WA_INACTIVE;
		SendMessage(hwnd, WM_ACTIVATE, state, 0L);
	}

}

static void DropboxWindow_OnParentChanged(HWND hwnd)
{
	HWND hRoot = GetAncestor(hwnd, GA_ROOT);
	DWORD oldStyleEx, newStyleEx;
	
	oldStyleEx = GetWindowStyleEx(hwnd);
	newStyleEx = oldStyleEx;

	
	if (hRoot != hwnd)
	{
		HWND hDlgParent = (HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);
		if (hRoot != plugin.hwndParent && 
			hRoot != hDlgParent)
		{
			HWND hTest = hRoot;
			if (NULL != hDlgParent && 
				plugin.hwndParent == (HWND)(LONG_PTR)GetWindowLongPtr(hRoot, GWLP_HWNDPARENT))
			{
				SetWindowLongPtr(hRoot, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)hDlgParent);
			}

			while (NULL != hTest)
			{
				DWORD styleEx = GetWindowStyleEx(hTest);
				if (WS_EX_ACCEPTFILES & styleEx)
				{
					RevokeDragDrop(hTest);
					if (hTest == hRoot) 
						DropTarget::RegisterWindow(hTest);
					SetWindowLongPtr(hTest, GWL_EXSTYLE, styleEx & ~WS_EX_ACCEPTFILES | WS_EX_CONTROLPARENT);
				}
				hTest = GetWindow(hTest, GW_CHILD);
			}
		}
		newStyleEx |= WS_EX_CONTROLPARENT;
	}
	else
	{
		newStyleEx &= ~WS_EX_CONTROLPARENT;
		if (plugin.hwndParent != (HWND)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT))
		{
			SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONGX86)(LONG_PTR)plugin.hwndParent);
			INT cx, cy;
			if (DropboxWindow_PopSize(hwnd, &cx, &cy))
			{
				DropboxWindow_AdjustClientSize(hwnd, &cx, &cy);
				SetWindowPos(hwnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
			}
		}
		
	}
	
	if (newStyleEx != oldStyleEx)
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyleEx);

	SendMessage(hRoot, WM_UPDATEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);
	
}

static LRESULT DropboxWindow_OnDocumentFromEnumerator(HWND hwnd, IFileEnumerator *pfe)
{
	INT itemsCount = 0;
	IFileInfo *pfi;
	LRESULT result = FALSE;
	HRESULT hr;

	DROPWND *pDropWnd = GetDropWnd(hwnd);

	if (NULL == pfe || NULL == pDropWnd)
		return FALSE;

	if (NULL != pDropWnd->pDocument && !DropboxWindow_CloseDocument(hwnd, TRUE, FALSE))
		return FALSE;

	pfe->AddRef();

	while(S_OK == pfe->Next(1, &pfi, NULL)) 
	{
		itemsCount++;
		pfi->Release();
		if (itemsCount > 1)
			break;
	}

	pfe->Reset();

	pDropWnd->pDocument = NULL;

	if (1 == itemsCount)
	{	
		if (S_OK == pfe->Next(1, &pfi, NULL) && NULL != pfi)
		{			
			DWORD itemType;
			LPCTSTR pszPath;
			
			if (SUCCEEDED(pfi->GetType(&itemType)) &&
				SUCCEEDED(pfi->GetPath(&pszPath)))
			{
				switch(itemType)
				{
					case IItemType::itemTypePlaylistFile:
						if (FAILED(Document::OpenFile(pszPath, &pDropWnd->pDocument)))
						{
							TCHAR szMessage[2048];
							FormatDocumentMessage(szMessage, ARRAYSIZE(szMessage), MAKEINTRESOURCE(IDS_ERROR_OPENFAILED), 
								(Document*)pszPath, DOCMSGVALUE_VALUEINDOCUMENT);

							pDropWnd->inModalLoop = hwnd;
							MessageBoxTweak::Show(hwnd, 
								szMessage, MAKEINTRESOURCE(IDS_ERROR_DOCUMENTLOAD), 
								MB_OK | MB_ICONERROR);
							pDropWnd->inModalLoop = NULL;
							DropboxWindow_NewDocument(hwnd, NULL, FALSE);
						}
						
						break;
					default:
						{
							TCHAR szTitle[512];
							IFileMeta *pMeta;
							szTitle[0] = TEXT('\0');
							if (SUCCEEDED(pfi->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
							{
								IFileMetaReader *pReader;
								METAKEY metaKey = METAKEY_TRACKTITLE;
								hr = pMeta->GetReader(&metaKey, 1, METAREADMODE_NORMAL, &pReader); 
								if (SUCCEEDED(hr))
								{
									pReader->Read();
									pReader->Release();
								}

								METAVALUE metaValue;
								metaValue.type = METATYPE_WSTR;
								
								pMeta->QueryValueHere(METAKEY_TRACKARTIST, &metaValue, szTitle, ARRAYSIZE(szTitle));
								if (TEXT('\0') == szTitle[0])
									pMeta->QueryValueHere(METAKEY_TRACKALBUM, &metaValue, szTitle, ARRAYSIZE(szTitle));
								if (TEXT('\0') == szTitle[0])
									pMeta->QueryValueHere(METAKEY_TRACKTITLE, &metaValue, szTitle, ARRAYSIZE(szTitle));
								
								pMeta->Release();
							}
							if (TEXT('\0') == szTitle[0])
							{
								LPCTSTR pszFile;
								if (SUCCEEDED(pfi->GetFileName(&pszFile)) && NULL != pszFile)
								{
									if (PathIsRoot(pszFile) && 
										0 != GetVolumeInformation(pszFile, szTitle, ARRAYSIZE(szTitle), NULL, NULL, NULL, NULL, NULL))
									{
										hr = S_OK;
									}
									else
										hr = StringCchCopy(szTitle, ARRAYSIZE(szTitle), pszFile);
									
									if (FAILED(hr))
										szTitle[0] = TEXT('\0');
									else
									{
										LPCTSTR ext;
										if (SUCCEEDED(pfi->GetExtension(&ext)))
											PathRemoveExtension(szTitle);
										PathRemoveBlanks(szTitle);
									}
									
								}
							}
							hr = Document::Create((TEXT('\0') != szTitle[0]) ? szTitle : NULL, &pDropWnd->pDocument);
							if (NULL != pDropWnd->pDocument)
							{	
								pfe->Reset();
								DropboxWindow_InsertEnumerator(hwnd, 0, pfe);
							}
						}
						break;

				}
			}
			else
			{
				DropboxWindow_NewDocument(hwnd, NULL, FALSE);
			}
			pfi->Release();
		}
		
	}
	else
	{
		result = DropboxWindow_NewDocument(hwnd, NULL, TRUE);
		if (result && 0 != itemsCount && NULL != pDropWnd->pDocument)
		{
			pfe->Reset();
			DropboxWindow_InsertEnumerator(hwnd, 0, pfe);
		}
	}

	pfe->Release();

	DropboxWindow_DocumentChanged(hwnd, pDropWnd->pDocument);

	return result;
}

static LPCTSTR DropboxWindow_GetAsyncOperationTitle(INT opCode, LPTSTR pszBuffer, INT cchBufferMax)
{
	INT resourceId;

	if (NULL == pszBuffer || cchBufferMax < 0)
		return NULL;

	switch(opCode)
	{
		case Document::AsyncInsert: resourceId = IDS_ASYNCOP_INSERT; break;
		case Document::AsyncOrder: resourceId = IDS_ASYNCOP_ORDER; break;
		default:
			pszBuffer[0] = TEXT('\0');
			return pszBuffer;
	}
	return WASABI_API_LNGSTRINGW_BUF(resourceId, pszBuffer, cchBufferMax);
}

static void DropboxWindow_OnUpdateDocumentBusy(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return;

	HWND hView = GetDlgItem(hwnd, IDC_ITEMVIEW);
	if (NULL == hView) 	return;
	
	Document::ASYNCOPERATION asyncOp;
	BOOL documentBusy = (pDropWnd->pDocument && pDropWnd->pDocument->QueryAsyncOpInfo(&asyncOp));
	
	HWND hBusy = GetDlgItem(hwnd, IDC_BUSYWINDOW);

	if (documentBusy)
	{
		if (NULL == hBusy)
		{
			RECT rc;
			if (GetWindowRect(hView, &rc))
			{
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rc, 2);
				DWORD windowStyle = GetWindowStyle(hwnd);
				DWORD busyStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (DBS_SKINWINDOW & windowStyle) | WS_TABSTOP;

				UUID classUid;
				const DROPBOXCLASSINFO *classInfo = (DropboxWindow_GetClassUid(hwnd, &classUid)) ?
													Plugin_FindRegisteredClass(classUid) : NULL;
				
				if (NULL != classInfo && 0 != (DBCS_ENABLEBLUR & classInfo->style))
					busyStyle |= BWS_ENABLEBLUR;

				hBusy = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, NWC_BUSYWINDOW, NULL, 	busyStyle, 
							rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, 
							(HMENU)IDC_BUSYWINDOW, plugin.hDllInstance, NULL);
								
				if (NULL != hBusy)
				{					
					TCHAR szBuffer[256];
					SetWindowPos(hBusy, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW);
					BusyWindow_SetTargetWindow(hBusy, hView);
					SetWindowText(hBusy, 
						DropboxWindow_GetAsyncOperationTitle(asyncOp.nCode, szBuffer, ARRAYSIZE(szBuffer)));
					PostMessage(hBusy, BWM_PREPANDSHOW, (WPARAM)SW_SHOWNORMAL, 100);
				//	InvalidateRect(hwnd, NULL, TRUE);
				//	UpdateWindow(hBusy);
				}
			}
		}

		if (NULL != hBusy)
		{
			BWOPERATIONINFO operationInfo;
			operationInfo.cancelable = asyncOp.bCancelable;
			operationInfo.total = asyncOp.total;
			operationInfo.processed = asyncOp.processed;
			BusyWindow_SetOperationInfo(hBusy, &operationInfo);
		}

		//DWORD viewStyle = GetWindowLongPtr(hView, GWL_STYLE);
		//if (0 != (WS_VISIBLE & viewStyle))
		//	SetWindowLongPtr(hView, GWL_STYLE, viewStyle & ~WS_VISIBLE);
		EnableWindow(hView, FALSE);
		

	}
	else
	{
		//if (!IsWindowVisible(hView))
		//	ShowWindow(hView, SW_SHOWNORMAL);
		HWND hRoot = GetAncestor(hView, GA_ROOT);	
		if (NULL == hRoot) hRoot = hwnd;
		HWND hActive = GetActiveWindow();
		BOOL bActive =  (NULL != hActive && (hActive == hRoot || IsChild(hRoot, hActive)));
				 
		EnableWindow(hView, TRUE);
		if (bActive)
			SetFocus(hView);
		
		if (NULL != hBusy)
			DestroyWindow(hBusy);
		
	
		pDropWnd->updateAsyncOpStatus = FALSE;

		if (pDropWnd->checkFile && bActive)
			PostMessage(hwnd, DBM_CHECKFILEMODIFIED, 0, 0L);
		
	}
	
}

static LRESULT DropboxWindow_OnGetDocumentName(HWND hwnd, LPTSTR pszBuffer, INT cchBufferMax)\
{
	if (NULL == pszBuffer)
		return 0;

	*pszBuffer = TEXT('\0');

	DROPWND *pDropWnd = GetDropWnd(hwnd);
    if (NULL == pDropWnd || NULL == pDropWnd->pDocument)
		return 0;
	
	HRESULT hr;
	hr = pDropWnd->pDocument->GetTitle(pszBuffer, cchBufferMax);
	
	return SUCCEEDED(hr);
}

static void DropboxWindow_OnCancelActiveOperation(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL != pDropWnd && NULL != pDropWnd->pDocument)
	{
		pDropWnd->pDocument->SignalAsyncAbort();
		PostMessage(hwnd, DBM_UPDATEDOCUMENTBUSY, 0, 0L);
	}
	
}

static LRESULT DropboxWindow_OnGetProfile(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	return (NULL != pDropWnd) ? (LRESULT)pDropWnd->profile : NULL;
}

static BOOL DropboxWindow_OnSetProfile(HWND hwnd, Profile *profile)
{
	HWND hMngr = GetDlgItem(hwnd, IDC_PROFILE);
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd) return FALSE;

	if (NULL != pDropWnd->pDocument && !DropboxWindow_CloseDocument(hwnd, TRUE, FALSE))
		return FALSE;

	if (NULL != pDropWnd->profile)
	{
		DropboxWindow_SaveViewSize(hwnd, pDropWnd->profile);
		DropboxWindow_SaveClassInfo(hwnd, pDropWnd->profile);
		DropboxWindow_UnregisterProfileCallback(hwnd);
		pDropWnd->profile->Release();
		pDropWnd->profile = NULL;
	}
	
	if (NULL == profile)
	{		
		if (NULL == hMngr)
		{
			hMngr = PLUGIN_PROFILEMNGR->CreateView(hwnd, 0, 0, 1, 1, IDC_PROFILE);
			if (NULL == hMngr)
				return FALSE;
			SendMessage(hMngr, WM_SETFONT, (WPARAM)SendMessage(hwnd, WM_GETFONT, 0, 0L), (LPARAM)FALSE);
			
			RECT windowRect;
			SetRectEmpty(&windowRect);
			windowRect.bottom = ProfileManagerView_GetIdealHeight(hMngr);
			if (windowRect.bottom < ITEMVIEW_MINHEIGHT) windowRect.bottom = ITEMVIEW_MINHEIGHT;
			windowRect.right = 267;
			DropboxWindow_AdjustViewRect(hwnd, &windowRect);
			DropboxWindow_ResizeFrame(hwnd, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, FALSE);
		}
        
		//ProfileManagerView_SetProfile(hMngr, NULL);
		
		ShowWindow(hMngr, SW_SHOW);
		DropboxWindow_SetHeaderText(hwnd, MAKEINTRESOURCE(IDS_SELECT_PROFILE));
	}
	else
	{
		profile->AddRef();
		
		if (NULL != hMngr)
			DestroyWindow(hMngr);

		pDropWnd->profile = profile;
		DropboxWindow_RegisterProfileCallback(hwnd, profile);
		DropboxWindow_UpdateClassProfile(hwnd, profile);
		DropboxWindow_LoadProfileView(hwnd, profile);
		
		HWND hHeader = GetDlgItem(hwnd, IDC_HEADER);
		if (NULL != hHeader)
			DropboxHeader_ProfileChanged(hHeader);
		
		DropboxWindow_NewDocument(hwnd, NULL, FALSE);
	}

	
	return TRUE;
}

static BOOL DropboxWindow_OnResizeFrame(HWND hwnd, INT clientCX, INT clientCY, BOOL fAnimate)
{
	RECT windowRect, clientRect;
	if (!GetWindowRect(hwnd, &windowRect) || !GetClientRect(hwnd, &clientRect))
		return FALSE;

	if (clientCX == (clientRect.right - clientRect.left) &&
		clientCY == (clientRect.bottom - clientRect.top))
	{
		return TRUE;
	}

		
	OffsetRect(&windowRect, -windowRect.left, -windowRect.top);

	HWND hParent = GetParent(hwnd);
	HWND hFrame = DropboxWindow_GetFrame(hwnd);

	INT frameCX, frameCY;
	frameCX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
	frameCY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
	
	if (1 || FALSE == fAnimate)
	{
		if (hFrame != hwnd && NULL != hParent)
			SendMessage(hParent, OSWNDHOST_REQUEST_IDEAL_SIZE, clientCX, clientCY);
		else
			SetWindowPos(hFrame, NULL, 0, 0, clientCX + frameCX, clientCY + frameCY, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
		return TRUE;
	}

	INT deltaX = (clientCX - clientRect.right);
	INT deltaY = (clientCY - clientRect.bottom);
	if (0 == deltaX && 0 == deltaY)
		return TRUE;

	INT minDelta = abs(deltaX);
	INT absY = abs(deltaY);
	if (0 == minDelta || (minDelta > absY && absY > 0)) minDelta = absY;
	if (0 == minDelta) return TRUE;

	INT stepCount;
	if (minDelta > 128) stepCount = minDelta/32;
	else if (minDelta > 32) stepCount = minDelta/16;
	else if (minDelta > 8) stepCount = minDelta/4;
	else stepCount = minDelta;

	deltaX = deltaX / stepCount;
	deltaY = deltaY / stepCount;

	INT nx = clientRect.right;
	INT ny = clientRect.bottom;
		
	INT cx, cy;

	for(;;)
	{		
		cx = nx; cy = ny;

		if (cx != clientCX)
		{
			nx = cx + deltaX;
			if (cx < clientCX && nx > clientCX) nx = clientCX;
			else if (cx > clientCX && nx < clientCX) nx = clientCX;
		}
		if (cy != clientCY)
		{
			ny = cy + deltaY;
			if (cy < clientCY && ny > clientCY) ny = clientCY;
			else if (cy > clientCY && ny < clientCY) ny = clientCY;
		}
		
		if (nx == cx && ny == cy)
			return TRUE;
		
		GetClientRect(hwnd, &clientRect);

		if (hFrame != hwnd && NULL != hParent)
		{
			SendMessage(hParent, OSWNDHOST_REQUEST_IDEAL_SIZE, nx, ny);
		}
		else
		{
			SetWindowPos(hFrame, NULL, 0, 0, nx + frameCX, ny + frameCY, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
		}
		
		
		RedrawWindow(hFrame, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN);
		
		if (!GetClientRect(hwnd, &windowRect))
			return FALSE;
		if (windowRect.right == clientRect.right)
			nx = clientCX;
		if (windowRect.bottom == clientRect.bottom)
			ny = clientCY;
		
		Sleep(2);
		
	}
	return TRUE;
}
static BOOL DropboxWindow_OnInsertEnumerator(HWND hwnd, UINT insertBefore, IFileEnumerator *pfe)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->pDocument)
		return FALSE;

	if (NULL == pfe)
		return TRUE;
		
	FilterPolicy *filterPolicy;
	if (NULL == pDropWnd->profile || FAILED(pDropWnd->profile->GetFilterPolicy(&filterPolicy, FALSE)))
		filterPolicy = NULL;
	
	InsertFilter *insertFilter = InsertFilter::CreateInstance(filterPolicy, hwnd);
	
	HRESULT hr = pDropWnd->pDocument->InsertItems(insertBefore, pfe, insertFilter, TRUE);

	if (NULL != insertFilter)
		insertFilter->Release();

	if (NULL != filterPolicy)
		filterPolicy->Release();

	return SUCCEEDED(hr);
}

static void DropboxWindow_OnFilterPolicyChanged(HWND hwnd)
{
	DROPWND *pDropWnd = GetDropWnd(hwnd);
	if (NULL == pDropWnd || NULL == pDropWnd->profile)
		return;

	FilterPolicy *filterPolicy;
	if (SUCCEEDED(pDropWnd->profile->GetFilterPolicy(&filterPolicy, TRUE)))
		filterPolicy->Release();
}

static void DropboxWindow_OnViewChanged(HWND hwnd)
{
	Profile *profile = DropboxWindow_GetProfile(hwnd);
	if (NULL != profile)
	{
		DropboxWindow_SaveViewSize(hwnd, profile);
		DropboxWindow_LoadProfileView(hwnd, profile);
	}
}

static void DropboxWindow_OnViewConfigChanged(HWND hwnd)
{
	HWND hctrl = GetDlgItem(hwnd, IDC_ITEMVIEW);
	DropboxView *pView = (NULL != hctrl) ? DropBox_GetItemView(hctrl) : NULL;
	if (NULL != pView)
	{
		pView->ConfigChanged();
	}
}


static void DropboxWindow_OnProfileNotify(HWND hwnd, UINT eventId, const UUID *profileUid)
{
	switch(eventId)
	{
		case ProfileCallback::eventFilterChanged:
			DropboxWindow_OnFilterPolicyChanged(hwnd);
			break;

		case ProfileCallback::eventViewChanged:
			DropboxWindow_OnViewChanged(hwnd);
			break;

		case ProfileCallback::eventViewConfigChanged:
			DropboxWindow_OnViewConfigChanged(hwnd);
			break;
	}

}

static void DropboxWindow_OnArrangeBy(HWND hwnd, INT columnId)
{
	DROPWND *pdw = GetDropWnd(hwnd);
	if (NULL == pdw || NULL == pdw->pDocument)
		return;

	if (columnId < 0 || columnId >= ARRAYSIZE(szRegisteredColumns))
		return;

	COLUMN_COMPARER primaryComp = szRegisteredColumns[columnId].fnComparer;
	if (NULL == primaryComp)
		return;
	
	INT secondaryId = COLUMN_FILENAME;
	if (columnId == secondaryId)  
		secondaryId = COLUMN_FILEPATH;
        
	COLUMN_COMPARER secondaryComp = (COLUMN_FILEPATH != columnId) ? 
					szRegisteredColumns[secondaryId].fnComparer : 
					NULL;

	METAKEY szMetaKey[32];
	INT metaCount = ColumnIdToMetaKey(columnId, szMetaKey, ARRAYSIZE(szMetaKey));
	pdw->pDocument->ReadAndOrder(primaryComp, secondaryComp, szMetaKey, metaCount);
}

static LRESULT CALLBACK DropboxWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return DropboxWindow_OnCreateWindow(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				DropboxWindow_OnDestroy(hwnd); return 0;
		case WM_PAINT:				DropboxWindow_OnPaint(hwnd); return 0;
		case WM_ACTIVATE:			DropboxWindow_OnActivate(hwnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam)); break;
		case WM_SETFOCUS:			DropboxWindow_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_SETFONT:				DropboxWindow_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); break;
		case WM_PRINTCLIENT:			DropboxWindow_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_COMMAND:				DropboxWindow_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_WINDOWPOSCHANGING:	DropboxWindow_OnWindowPosChanging(hwnd, (WINDOWPOS*)lParam); break;
		case WM_WINDOWPOSCHANGED:	DropboxWindow_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_ENTERSIZEMOVE:		g_hwndInSizeLoop = hwnd; break;
		case WM_EXITSIZEMOVE:		g_hwndInSizeLoop = NULL; break;
		case WM_SYSCOMMAND:			if (DropboxWindow_OnSysCommand(hwnd, (UINT)wParam, MAKEPOINTS(lParam))) return 0; break;
		case WM_CLOSE:				DropboxWindow_OnClose(hwnd); return 0;
		case WM_STYLECHANGED:		DropboxWindow_OnStyleChanged(hwnd, (UINT)wParam, (STYLESTRUCT*)lParam); break;
		case WM_NOTIFY:				return DropboxWindow_OnNotify(hwnd, (INT)wParam, (LPNMHDR)lParam); 
		case WM_DRAWITEM:			return DropboxWindow_OnDrawItem(hwnd, (INT)wParam, (DRAWITEMSTRUCT*)lParam);
		case WM_MEASUREITEM:			return DropboxWindow_OnMeasureItem(hwnd, (INT)wParam, (MEASUREITEMSTRUCT*)lParam);
		case WM_ERASEBKGND:			return 0;
		case DBM_GETCLASSUID	:		return DropboxWindow_OnGetClassUid(hwnd, (UUID*)lParam);
		case DBM_REPOSITION:			DropboxWindow_OnReposition(hwnd); return 0;
		case DBM_GETDROPEFFECT:		return DropboxWindow_OnGetDropEffect(hwnd, (DWORD)wParam, MAKEPOINTS(lParam));
		case DBM_CREATEVIEW:			return DropboxWindow_OnCreateView(hwnd, (INT)wParam);
		case DBM_GETACTIVEVIEW:		return DropboxWindow_OnGetCurrentView(hwnd);
		case DBM_GETITEMCOUNT:		return DropboxWindow_OnGetItemCount(hwnd);
		case DBM_SETDOCUMENTNAME:	return DropboxWindow_OnSetDocumentName(hwnd, (LPCTSTR)lParam);
		case DBM_GETMENU:			return DropboxWindow_OnGetMenu(hwnd, (UINT)wParam);
		case DBM_RELEASEMENU:		DropboxWindow_OnReleaseMenu(hwnd, (UINT)wParam, (HMENU)lParam); return 0;
		case DBM_GETACTIVEDOCUMENT:	return DropboxWindow_OnGetActiveDocument(hwnd);
		case DBM_CHECKFILEMODIFIED:	DropboxWindow_OnCheckFileModified(hwnd); return 0;
		case DBM_SKINCHANGED:		DropboxWindow_OnSkinChanged(hwnd); return 0;
		case DBM_SKINREFRESHING:		return DropboxWindow_OnSkinRefreshing(hwnd);
		case DBM_SKINREFRESHED:		DropboxWindow_OnSkinRefreshed(hwnd); return 0;
		case DBM_PARENTCHANGED:		DropboxWindow_OnParentChanged(hwnd); return 0;
		case DBM_GETDOCUMENTNAME:	return DropboxWindow_OnGetDocumentName(hwnd, (LPTSTR)lParam, (INT)wParam);
		case DBM_DOCUMENTFROMENUMERATOR: return DropboxWindow_OnDocumentFromEnumerator(hwnd, (IFileEnumerator*)lParam);
		case DBM_UPDATEDOCUMENTBUSY:	DropboxWindow_OnUpdateDocumentBusy(hwnd); return 0;
		case DBM_CANCELLACTIVEOPERATION: DropboxWindow_OnCancelActiveOperation(hwnd); return 0;
		case DBM_BROADCASTCOMMAND:	return DropboxWindow_OnBroadcastCommand(hwnd, (INT)wParam, (HWND)lParam);
		case DBM_PROCESSCOMMAND:		return DropboxWindow_OnProcessCommand(hwnd, (INT)wParam);
		case DBM_GETPROFILE:			return DropboxWindow_OnGetProfile(hwnd);
		case DBM_SETPROFILE:			return DropboxWindow_OnSetProfile(hwnd, (Profile*)lParam);
		case DBM_RESIZEFRAME:		return DropboxWindow_OnResizeFrame(hwnd, (INT)(SHORT)LOWORD(lParam), (INT)(SHORT)HIWORD(lParam), (TRUE == wParam));
		case DBM_INSERTENUMERATOR:	return DropboxWindow_OnInsertEnumerator(hwnd, (UINT)wParam, (IFileEnumerator*)lParam);
		case DBM_PROFILENOTIFY:		DropboxWindow_OnProfileNotify(hwnd, (UINT)wParam, (const UUID*)lParam); return 0;
		case DBM_ARRANGEBY:			DropboxWindow_OnArrangeBy(hwnd, (INT)wParam); return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


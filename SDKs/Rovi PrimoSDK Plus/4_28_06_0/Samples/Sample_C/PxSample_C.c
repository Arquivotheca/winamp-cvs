//-----------------------------------------------------------------------------
// pxsample_c.c
// Copyright (c) Sonic Solutions.  All rights reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// PROJECT:         PrimoSDK C Sample
//
// DESCRIPTION:     Mini mastering application as a C example and a guide to
//                  use the PrimoSDK API.
//
//-----------------------------------------------------------------------------
// If you are a licensed user of PrimoSDK you are authorized to
// copy part or entirely the code of this example into your
// application.


#include <windows.h>
#include <commctrl.h>
#include <mbstring.h>
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

#include "primosdk.h"
#include "pxsample_c.h"
#include "misc.h"


//
// APPLICATION GLOBAL VARIABLES
//
typedef struct _DIALOGLIST
{
	struct _DIALOGLIST *Next;
	HWND hDlg;
} DIALOGLIST;
TCHAR       *AppClassName = _T("PrimoSDKSample");       // Application class name
HANDLE      hInstAp = NULL;                              // Application instance
HANDLE      DialogListMutex = NULL;                      // Dialog list mutex
DIALOGLIST *DialogList = NULL;                           // Dialog list
BOOL        Quitting = FALSE;                            // Exiting the app, no more dialogs
BOOL        g_bStreamFsUsed;
CRITICAL_SECTION gCrit;
BOOL        MainWindowCreated = FALSE;



static TCHAR    AppTitle[128] = {0};                    // Window title
static DWORD    dwRel;                                   // DLL version

extern BOOL AudioFileExists();

//-----------------------------------------------------------------------------
//
// Procedure to dispatch message to the appropriate window/dialog
//
//-----------------------------------------------------------------------------
VOID HandleMessage(PMSG msg)
{
	HWND hDlg = NULL;
	DIALOGLIST *Node;

	WaitForSingleObject(DialogListMutex, INFINITE);
	Node = DialogList;
	while (Node)
	{
		if (msg->hwnd == Node->hDlg ||
			IsChild(Node->hDlg, msg->hwnd))
		{
			hDlg = Node->hDlg;
			break;
		}
		Node = Node->Next;
	}
	ReleaseMutex(DialogListMutex);

	if (hDlg == NULL || !IsDialogMessage(hDlg, msg))
	{
		TranslateMessage(msg);
		DispatchMessage(msg);
	}
}

//-----------------------------------------------------------------------------
//
// MAIN WINDOWS PROCEDURE
//
//   Param: hInstance is the Handle to this instance of the application
//
//          hPrev is the Handle to the previous instance (not used)
//
//          pcl is the pointer to the command line (not used)
//
//          iCmdShow is the Show command (not used)
//
//
//   Notes: None.
//
//  Return: Exit status (0 if it could not run)
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, PSTR pcl, int iCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	DWORD dwErr;

	//
	// Save the instance once and for all.
	//
	InitializeCriticalSection(&gCrit);
	hInstAp = hInstance;
	DialogListMutex = CreateMutex(NULL, FALSE, NULL);
	if (DialogListMutex == NULL)
	{
		DisplayError(0,_T("CreateMutex"),NULL);
		return(FALSE);
	}

	_tcscpy(AppTitle, _T("C Sample for PrimoSDK"));

	//
	// If shift key is pressed, we start in trace mode.
	//

	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		PrimoSDK_Trace(1);
		_tcscat(AppTitle, _T(" (Trace mode)"));
	}

	//
	// Initialize and change the title based on what version of SDK we find.
	//
	dwErr = InitializeSDK(&dwRel, AppTitle);

	switch (dwErr)
	{
	case PRIMOSDK_CDDVDVERSION:
		_tcscat(AppTitle, _T(" for CD/DVD"));
		break;
	case PRIMOSDK_DEMOVERSION:
		_tcscat(AppTitle, _T(" Demo"));
		break;
	default:
		DisplayError(dwErr,_T("PrimoSDK_Init"),NULL);
		return(FALSE);
	}


	//
	// Register main window class.
	//

	if (hPrev == NULL)
	{
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)MainWndProc;
		wc.cbClsExtra = wc.cbWndExtra = 0;
		wc.hInstance = hInstAp;
		wc.hIcon = LoadIcon(hInstAp,MAKEINTRESOURCE(IDI_ICON));
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = AppClassName;
		if (!RegisterClass(&wc)) return(FALSE);
	}


	//
	// Create main window. The handle will be saved in hWndMain.
	//

	CreateWindow(AppClassName, AppTitle,
				 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
				 CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
				 (HWND)NULL, (HMENU)NULL, (HANDLE)hInstAp, (LPSTR)NULL);

	if (!MainWindowCreated) return(FALSE);


	//
	// WINDOWS MESSAGE PUMP
	//

	while (GetMessage(&msg,NULL,0,0))
	{
		HandleMessage(&msg);
	}


	//
	// Exiting. Release the handle and terminate.
	//

	UninitializeSDK();

	if ((dwErr = PrimoSDK_End()) != PRIMOSDK_OK)
		DisplayError(dwErr,_T("PrimoSDK_End"),NULL);

	DeleteCriticalSection(&gCrit);

	return(msg.wParam);
}


VOID AddToFileList(LPTSTR Buf, HWND hDlgModeless, PDWORD dwTotEntries)
{
	LRESULT HorizExtent;
	SIZE size;
	SendDlgItemMessage(hDlgModeless,IDC_FILELIST,LB_ADDSTRING,0,(LONG)Buf);
	HorizExtent = SendDlgItemMessage(hDlgModeless,IDC_FILELIST,LB_GETHORIZONTALEXTENT,0,0);
	if (GetTextExtentPoint(GetDC(hDlgModeless), Buf, _tcslen(Buf), &size))
	{
		if (size.cx > HorizExtent)
			SendDlgItemMessage(hDlgModeless,IDC_FILELIST,LB_SETHORIZONTALEXTENT,(WPARAM)size.cx,0);
	}
	dwTotEntries[0]++;
}

//-----------------------------------------------------------------------------
//
// Main Window Callback
//
//
//   Param: hWnd is the Handle to this window
//
//          msg is ???
//
//          wParam is the word parameter - used for accepting dropped files
//
//          lParam is the long parameter - passed on to default window handler
//
//
//   Notes: None.
//
//  Return: ???
//
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT dx, dy, wx, wy, top, left;
	RECT r;
	DWORD dwCount;
	TCHAR Buf[256];
	UINT uiTotToDrop;
	DIALOGLIST *Node, *Prev;
	PGLOBAL g = NULL;
	DWORD dwErr;

	if (msg != WM_CREATE)
		g = GetProp(hWnd, _T("GLOBAL_VARS"));

	switch (msg)
	{

	case WM_CREATE:

		//
		// Create the modeless dialog and center it in the main window.
		//

		AppendMenu(GetSystemMenu(hWnd, 0), MF_STRING, 1, _T("Launch New Window"));

		Node = malloc(sizeof(DIALOGLIST));
		if (Node)
		{
			g = malloc(sizeof(GLOBAL));
			if (g)
			{
				memset(g, 0, sizeof(GLOBAL));
				g->LastIndex = 0xffffffff;
				g->hWndMain = hWnd;
				SetProp(hWnd, _T("GLOBAL_VARS"), g);

				//
				// Get one handle per window. We will always use this handle
				// for every operation in this window
				//

				if ((dwErr = PrimoSDK_GetHandle(&g->dwHandle)) != PRIMOSDK_OK)
				{
					DisplayError(dwErr,_T("PrimoSDK_GetHandle"),NULL);
					return(FALSE);
				}

				Node->hDlg = CreateDialog(hInstAp,_T("DLGMAIN"),hWnd,DlgFnProc);
				g->hDlgModeless = Node->hDlg;

				MainWindowCreated = TRUE;

				GetWindowRect(g->hDlgModeless,&r);
				dx = r.right-r.left;
				dy = r.bottom-r.top;
				wx = dx + GetSystemMetrics(SM_CXEDGE)*2;
				wy = dy + GetSystemMetrics(SM_CYEDGE)*2 + GetSystemMetrics(SM_CYCAPTION);

				// add Node to beginning of list
				WaitForSingleObject(DialogListMutex, INFINITE);
				if (DialogList)
				{
					top = 0;
					left = 0;
				}
				else
				{
					top = (GetSystemMetrics(SM_CXFULLSCREEN)-wx)/2;
					left = (GetSystemMetrics(SM_CYFULLSCREEN)-wy)/2;
				}
				if (!Quitting)
				{
					Node->Next = DialogList;
					DialogList = Node;
				}
				ReleaseMutex(DialogListMutex);

				SetWindowPos(hWnd,NULL,top,left,wx,wy,SWP_NOZORDER);
				MoveWindow(g->hDlgModeless,0,0,dx,dy,TRUE);
			}
		}
		ShowWindow(hWnd,SW_SHOW);
		UpdateWindow(hWnd);


		//
		// Show the DLL version in the window caption.
		//

		GetWindowText(hWnd,Buf,sizeof(Buf));
		wsprintf(&Buf[_tcslen(Buf)],_T(" - PrimoSDK DLL Version %d.%d.%02d.%02d"),
				 HIBYTE(HIWORD(dwRel)),
				 LOBYTE(HIWORD(dwRel)),
				 HIBYTE(LOWORD(dwRel)),
				 LOBYTE(LOWORD(dwRel)));
		SetWindowText(hWnd,Buf);
		break;

	case WM_SETFOCUS:

		//
		// The focus is always on the modeless dialog.
		//

		SetFocus(g->hDlgModeless);
		break;

	case WM_DROPFILES:

		//
		// Handle files or folders that are dropped in the data or audio list box.
		//

		uiTotToDrop = DragQueryFile((HDROP)wParam,(UINT)-1,NULL,0);


		//
		// Handle the dropped files based on the current function.
		//

		switch (g->dwFunction)
		{
		case FUNC_RECORDDATA:
		case FUNC_SAVEDATA:
		case FUNC_SAVEGI:


			for (dwCount = 0; dwCount < uiTotToDrop; dwCount++)
			{
				DragQueryFile((HDROP)wParam,dwCount,Buf,sizeof(Buf));
				AddToFileList(Buf,g->hDlgModeless,&g->dwTotEntries);
			}
			break;

		case FUNC_RECORDAUDIO:  // AUDIO - only drop the .WAV files and ignore others.

			for (dwCount = 0; dwCount < uiTotToDrop; dwCount++)
			{
				DragQueryFile((HDROP)wParam,dwCount,Buf,sizeof(Buf));
				if ((_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".WAV")) == 0) ||
					(_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".WMA")) == 0) ||
					(_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".MP4")) == 0) ||
					(_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".M4A")) == 0) ||
					(_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".MP3")) == 0))
				{
					AddToFileList(Buf,g->hDlgModeless,&g->dwTotEntries);
				}
			}
			break;

		case FUNC_RECORDVIDEO:  // VIDEO - only add valid movie files.

			for (dwCount = 0; dwCount < uiTotToDrop; dwCount++)
			{
				DragQueryFile((HDROP)wParam,dwCount,Buf,sizeof(Buf));
				if (_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".DAT")) == 0 ||
					_tcsicmp(&Buf[_tcslen(Buf)-4],_T(".MPG")) == 0 ||
					_tcsicmp(&Buf[_tcslen(Buf)-5],_T(".MPEG")) == 0)
				{
					AddToFileList(Buf,g->hDlgModeless,&g->dwTotEntries);
				}
			}
			break;

		}
		break;

	case WM_CLOSE:

		//
		// Close the app, but only if we are not in the middle of something!
		//

		if (!g->bBusy)
		{
			DestroyWindow(hWnd);
		}
		else
		{
			//
			// Remind the user to stop the current process before closing the application.
			//

			PxLoadString(IDS_STOPBEFORECLOSE,Buf,sizeof(Buf));
			MessageBox(hWnd,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
		}
		break;

	case WM_DESTROY:
		DestroyWindow(g->hDlgModeless);
		// remove Node
		WaitForSingleObject(DialogListMutex, INFINITE);
		Prev = NULL;
		Node = DialogList;
		while (Node)
		{
			if (Node->hDlg == g->hDlgModeless)
			{
				if (Prev)
					Prev->Next = Node->Next;
				else
					if (Node->Next == NULL)
						Quitting = TRUE;  // no previous or next, done
				if (DialogList == Node)
				{
					DialogList = Node->Next;
				}
				free(Node);
				break;
			}
			Prev = Node;
			Node = Node->Next;
		}
		ReleaseMutex(DialogListMutex);

		if ((dwErr = PrimoSDK_ReleaseHandle(g->dwHandle)) != PRIMOSDK_OK)
			DisplayError(dwErr,_T("PrimoSDK_ReleaseHandle"),NULL);

		if (Quitting)
			PostQuitMessage(0);
		free(g);
		break;

	case WM_SYSCOMMAND:
		if (wParam == 1 && !Quitting)
		{
			CreateWindow(AppClassName, AppTitle,
						 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
						 CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
						 (HWND)NULL, (HMENU)NULL, (HANDLE)hInstAp, (LPSTR)NULL);
		}
		else
			return(DefWindowProc(hWnd, msg, wParam, lParam));

	default:
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}
	return(0);
}



//-----------------------------------------------------------------------------
//
// MODELESS DIALOG CALLBACK
//
//
//   Param: hDlg is the Handle to this window
//
//          msg is ???
//
//          wParam is the word parameter - used for accepting dropped files
//
//          lParam is the long parameter - passed on to default window handler
//
//
//   Notes: This dialog is centered and fixed on the main window.
//
//  Return: ???
//-----------------------------------------------------------------------------
LRESULT CALLBACK DlgFnProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR Buf[2000];
	PGLOBAL g = GetProp(GetParent(hDlg), _T("GLOBAL_VARS"));


	/*
	// USED IN TEST OF PrimoSDK_CDTextInfo
	BYTE szCDTitle[2000],szPerformer[2000],szComposer[2000],szCDTextInfo[6010];
	*/

	switch (msg)
	{

	case WM_INITDIALOG:

		InitMainDialog(hDlg, g);
		break;

	case WM_TIMER:

		//
		// Enter here during asynchronous operations, every second.
		//

		UpdateProgress(hDlg, g);
		break;

	case WM_COMMAND:


		if (LOWORD(wParam) == IDC_FUNCTION && HIWORD(wParam) == CBN_SELCHANGE)
		{
			//
			// The function select slider was changed.
			//

			g->dwFunction = SendDlgItemMessage(hDlg,IDC_FUNCTION,CB_GETCURSEL,0,0) + 1;
			EnableDisable(FALSE, g);
			CheckDlgButton(hDlg, IDC_CHKTAO, (g->dwFunction!=FUNC_RECORDAUDIO));
			break;
		}

		ExecuteCommand(hDlg, wParam, g);

		break;

	case WM_HSCROLL:

		//
		// The data cache slider changed.
		//

		g->dwFileSwap = (INT)SendDlgItemMessage(hDlg,IDC_SLIDERSWAP,TBM_GETPOS,0,0);

		switch (g->dwFileSwap)
		{
		case 0:
			PxLoadString(IDS_NONE,Buf,sizeof(Buf));
			break;
		case 257:
			PxLoadString(IDS_ALLIMAGE,Buf,sizeof(Buf));
			break;
		default:
			wsprintf(Buf,_T("%dKB"),g->dwFileSwap);
		}

		SetDlgItemText(hDlg,IDC_SWAP,Buf);
		break;

	case WM_LBUTTONDOWN:
	{
		BYTE szBigBuf[50000];
		POINT p;
		RECT r;
		UINT i;

		//
		// Show supported units.
		//

		GetCursorPos(&p);
		GetWindowRect(GetDlgItem(g->hDlgModeless,IDC_LITTLEHORSE),&r);
		if (PtInRect(&r,p) && !g->bBusy)
		{
			strcpy(szBigBuf,"xxxx supported recorders:\r\n\r\n");
			i = PrimoSDK_ListSupportedUnits(&szBigBuf[strlen(szBigBuf)]);
			sprintf(szBigBuf,"%4d",i);
			szBigBuf[4] = ' ';
			SetDlgItemTextA(g->hDlgModeless,IDC_RESULT,szBigBuf);
		}
	}
	break;

	default:
		return(FALSE);

	}
	return(TRUE);
}




//-----------------------------------------------------------------------------
//
//   Scan for all available CD/DVD drives.
//
//   Initialize all the controls of the main dialog.
//
//   Param: hDlg is the handle to the dialog window
//
//   Notes: None.
//
//  Return: Exit status (0 if it could not run)
//
//-----------------------------------------------------------------------------
void InitMainDialog(HWND hDlg, PGLOBAL g)
{
	INT i;
	INT iHost, iID;
	TCHAR Buf[50000], Buf2[2048];
	DWORD dwUnit;
	//
	// Load the "functions" ComboBox
	//
	for (i=0; i < FUNC_LASTFUNCTION; i++)
	{
		PxLoadString(IDS_FUNC1+i,Buf,sizeof(Buf));
		SendDlgItemMessage(hDlg,IDC_FUNCTION,CB_ADDSTRING,0,(LPARAM)Buf);
	}
	SendDlgItemMessage(hDlg,IDC_FUNCTION,CB_SETCURSEL,0,0);
	g->dwFunction = FUNC_COPY;

	//
	// Load all present recorders letters in the combo recorders
	//  and in the combo source, saving the unit identifier in the data
	//   field of the combo entry.
	// We loop for all possible conbination and shoot a primosdk_unitinfo,
	//  if there is a drive there, it will answer.
	//

	g->dwHowManyRecorder = g->dwHowManyReader = 0;
	for (iHost = 0;  iHost < 256; iHost++)
	{
		for (iID = 0;  iID < 64; iID++)
		{
			DWORD    dwType;                         // Unit type, all types
			BYTE     szUnitDescr[64+1];              // Unit Vendor, Model and FW. version

			dwUnit = (iID << 16) | (iHost << 24);
			if (PrimoSDK_UnitInfo(g->dwHandle,&dwUnit,&dwType,szUnitDescr,NULL) == PRIMOSDK_OK)
			{
				Buf2[0] = 0x00;

				//
				// There is still the risk that the drive is there but
				// does not have a drive letter.
				//

				if (LOBYTE(LOWORD(dwUnit)))
				{
					wsprintf(&Buf2[_tcslen(Buf2)],_T("%c: "),LOBYTE(LOWORD(dwUnit)));
				}
#ifdef _UNICODE
				wsprintf(&Buf2[_tcslen(Buf2)],_T("(h%d id%d) %S"),
						 HIBYTE(HIWORD(dwUnit)),LOBYTE(HIWORD(dwUnit)),szUnitDescr);
#else
				wsprintf(&Buf2[_tcslen(Buf2)],_T("(h%d id%d) %s"),
						 HIBYTE(HIWORD(dwUnit)),LOBYTE(HIWORD(dwUnit)),szUnitDescr);
#endif

				//
				// Now add the entry to the combo(s) and save the unit ID in the item data
				//

				if (dwType == PRIMOSDK_CDR || dwType == PRIMOSDK_CDRW ||
					dwType == PRIMOSDK_DVDRAM || dwType == PRIMOSDK_DVDR ||
					dwType == PRIMOSDK_DVDRW || dwType == PRIMOSDK_DVDPRW)
				{
					//
					// This unit is some type of recorder - add it to the recorder list.
					//

					SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_ADDSTRING,0,(LPARAM)Buf2);
					SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_SETITEMDATA,
									   g->dwHowManyRecorder,(LPARAM)dwUnit);
					g->dwHowManyRecorder++;
				}

				if (dwType != PRIMOSDK_OTHER)
				{
					//
					// Add all (non-"other") drives to the source list.
					//

					SendDlgItemMessage(hDlg,IDC_LETTERSOURCE,CB_ADDSTRING,0,(LPARAM)Buf2);
					SendDlgItemMessage(hDlg,IDC_LETTERSOURCE,CB_SETITEMDATA,
									   g->dwHowManyReader,(LPARAM)dwUnit);
					g->dwHowManyReader++;
				}
			}
		}
	}

	//
	// Check if we have any drives.
	//

	if (g->dwHowManyReader)
	{
		SendDlgItemMessage(hDlg,IDC_LETTERSOURCE,CB_SETCURSEL,0,0);
	}
	else // no drives!
	{
		//
		// Tell the user.
		//

		PxLoadString(IDS_NOCDFOUND,Buf,sizeof(Buf));
		MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);

		//
		// Close the application
		//

		PostMessage(g->hWndMain,WM_CLOSE,0,0);

		///////
		return;
		///////
	}

	if (g->dwHowManyRecorder)
	{
		SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_SETCURSEL,0,0);
	}
	else // no recorders!
	{
		//
		// Warn user. Keep the app open, we can still read data.
		//

		PxLoadString(IDS_NOCDRFOUND,Buf,sizeof(Buf));
		MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
	}

	//
	// Set parameters to their default values
	//

	SetDlgItemInt(hDlg,IDC_EDITTRACK,1,FALSE);
//      CheckDlgButton(hDlg,IDC_RADIOISOLEVEL1,FALSE);
//      CheckDlgButton(hDlg,IDC_RADIOJOLIET,TRUE);
	CheckDlgButton(hDlg,IDC_RADIOMODE1,TRUE);
	CheckDlgButton(hDlg,IDC_RADIOMODE2,FALSE);
	CheckDlgButton(hDlg, IDC_CHKTAO, TRUE);
	SendDlgItemMessage(hDlg,IDC_SLIDERSWAP,TBM_SETRANGE,0,MAKELPARAM(0,257));
	SendDlgItemMessage(hDlg,IDC_SLIDERSWAP,TBM_SETPOS,TRUE,g->dwFileSwap);
	SendMessage(hDlg,WM_HSCROLL,0,0);
	SetDlgItemInt(hDlg,IDC_LOAD,0,FALSE);

	//
	// Load all the possible file systems in the file system combobox
	//

	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("UDF 1.02"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("UDF 2.0"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("UDF 2.5"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("UDF 2.6"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("Microsoft Joliet"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("ISO Level-1"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("ISO Level-2"));
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_ADDSTRING,0,(LPARAM)_T("ISO Level-3"));

	// Load the numeric values for each string ( in order ).
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,0,PRIMOSDK_UDF);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,1,PRIMOSDK_UDF201);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,2,PRIMOSDK_UDF250);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,3,PRIMOSDK_UDF260);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,4,PRIMOSDK_JOLIET);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,5,PRIMOSDK_ISOLEVEL1);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,6,PRIMOSDK_ISOLEVEL2);
	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETITEMDATA,7,PRIMOSDK_ISOLEVEL3);

	SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_SETCURSEL,0,0);

	//
	// Load all the possible speeds in the speed combobox
	//

	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("Max."));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("1x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("2x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("4x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("6x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("8x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("10x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("12x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("16x"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_ADDSTRING,0,(LPARAM)_T("Best"));
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,0,PRIMOSDK_MAX);
	// Load the numeric values for each string ( in order ).
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,1,1);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,2,2);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,3,4);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,4,6);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,5,8);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,6,10);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,7,12);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,8,16);
	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETITEMDATA,9,PRIMOSDK_BEST);

	SendDlgItemMessage(hDlg,IDC_SPEED,CB_SETCURSEL,0,0);

	//
	// Volume name is only 11 characters here.
	//

	SendDlgItemMessage(hDlg,IDC_VOLUMENAME,EM_SETLIMITTEXT,32,0);
	CheckDlgButton(hDlg,IDC_CHECKTEST,FALSE);

	g->hDlgModeless=hDlg;
	EnableDisable(FALSE, g);
}



//-----------------------------------------------------------------------------
//
//  Enable or disable the main dialog controls of dialog box based
//    on the selected function dwFunction and the bBusy Status.
//
//   Param: bBusyParam - TRUE if there is an operation in progress.
//
//   Notes: Depending on the function, some of the controls are hidden or grayed out.
//
//          Also sets the global "bBusy" based on the parameter passed in.
//
//          Uses the following globals:
//
//          dwFunction - the user-selected function - one of the FUNC_XXX constants
//
//          hDlgModeless - Handle to the main dialog
//
//
//  Return: None.
//
//
//-----------------------------------------------------------------------------
VOID EnableDisable(BOOL bBusyParam, PGLOBAL g)
{
	BOOL bSel;
	INT iShow;

	if (bBusyParam)
		// wait 3 seconds after an operation completes.
		g->dwStatusCounter = 3;

	//
	// Set the status in the global variable.
	//

	g->bBusy = bBusyParam;

	//
	// OPERATION SECTION
	//

	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_FUNCTION),!g->bBusy);

	//
	// Disable the "Test" checkbox for things that don't run in test mode.
	// Disable "Test" checkbox when we are busy.
	//

	bSel = g->dwFunction!=FUNC_BUILDGI          &&
		   g->dwFunction!=FUNC_SAVEDATA         &&
		   g->dwFunction!=FUNC_VERIFYDISC       &&
		   g->dwFunction!=FUNC_VERIFYGI         &&
		   g->dwFunction!=FUNC_AUDIOEXTRACT     &&
		   g->dwFunction!=FUNC_VERIFYOTHERIMAGE &&
		   g->dwFunction!=FUNC_SAVEGI           &&
		   g->dwFunction!=FUNC_GIINFO           &&
		   !g->bBusy;

	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CHECKTEST),bSel);

	//
	// Hide the "Verify" checkbox for things that don't verify.
	// Disable "Verify" checkbox when we are busy.
	//

	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_CHECKVERIFY),(g->dwFunction==FUNC_RECORDDATA));
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CHECKVERIFY),!g->bBusy);

	//
	// Enable the speed and "record to all" controls if we need a recorder.
	//

	bSel = NEEDS_RECORDER(g->dwFunction) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_SPEED),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CHECKRECTOALL),bSel);
	SetDlgItemText(g->hDlgModeless,IDC_GO,g->bBusy?_T("St&op"):_T("G&o !"));

	//
	// The erase disc can't be aborted.
	//

	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_GO),!(g->bBusy && g->dwFunction==FUNC_ERASEDISC));

	SetDlgItemText(g->hDlgModeless,IDC_RESULT,g->bBusy?_T("Working..."):_T(""));
	DragAcceptFiles(g->hWndMain,(g->dwFunction == FUNC_RECORDDATA ||
								 g->dwFunction == FUNC_SAVEDATA ||
								 g->dwFunction == FUNC_RECORDAUDIO ||
								 g->dwFunction == FUNC_RECORDVIDEO ||
								 g->dwFunction == FUNC_SAVEGI) && !g->bBusy);

	//
	// RECORDER SECTION
	//


	//
	// For functions that don't need the recorder (or if we don't have any),
	// deselect the recorder dialog items.
	//

	bSel = NEEDS_RECORDER(g->dwFunction)  && !g->bBusy && g->dwHowManyRecorder;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_LETTERREC),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_EJECTREC),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CLOSETRAYREC),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_UNITINFOREC),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_DISCINFOREC),bSel);

	//
	// SOURCE DRIVE SECTION
	//

	//
	// For functions that need a source drive, enable the source drive dialog items.
	//

	bSel = NEEDS_SOURCE(g->dwFunction)  && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_LETTERSOURCE),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_EJECTSOURCE),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CLOSETRAYSOURCE),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_UNITINFOSOURCE),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_DISCINFOSOURCE),bSel);
	iShow = (g->dwFunction<FUNC_RECORDGIORTRACK || g->dwFunction==FUNC_VERIFYDISC ||
			 g->dwFunction==FUNC_AUDIOEXTRACT) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_SOURCESTRING),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_LETTERSOURCE),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_EJECTSOURCE),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_CLOSETRAYSOURCE),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_UNITINFOSOURCE),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_DISCINFOSOURCE),iShow);

	//
	// AUDIO EXTRACT TRACK
	//

	bSel = g->dwFunction==FUNC_AUDIOEXTRACT && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_EDITTRACK),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOEXTRACTSTRING),bSel);
	iShow = (g->dwFunction == FUNC_AUDIOEXTRACT) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_EDITTRACK),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOEXTRACTSTRING),iShow);

	//
	// FILE LIST SECTION
	//

	bSel = g->dwFunction==FUNC_RECORDDATA  ||
		   g->dwFunction==FUNC_SAVEDATA      ||
		   g->dwFunction==FUNC_RECORDAUDIO ||
		   g->dwFunction==FUNC_RECORDVIDEO ||
		   g->dwFunction==FUNC_SAVEGI;

	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_FILELISTSTRING), bSel && !g->bBusy);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_FILELIST), bSel && !g->bBusy);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CLEARDATA), bSel && !g->bBusy);
	iShow = bSel ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_FILELISTSTRING),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_FILELIST),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_CLEARDATA),iShow);

	//
	// DATA DISC PARAMETERS SECTION
	//

	bSel = (g->dwFunction==FUNC_RECORDDATA) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_TOTALONLY),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_TOTALONLY),iShow);

	bSel = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction==FUNC_SAVEDATA|| g->dwFunction == FUNC_SAVEGI) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_VOLUMENAMESTRING),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_VOLUMENAME),bSel);
//   EnableWindow(GetDlgItem(hDlgModeless,IDC_RADIOISOLEVEL1),bSel);
//   EnableWindow(GetDlgItem(hDlgModeless,IDC_RADIOJOLIET),bSel);
//   EnableWindow(GetDlgItem(hDlgModeless,IDC_RADIOUDF),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_FS_COMBO),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction==FUNC_SAVEDATA|| g->dwFunction == FUNC_SAVEGI) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_VOLUMENAMESTRING),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_VOLUMENAME),iShow);
//   ShowWindow(GetDlgItem(hDlgModeless,IDC_RADIOISOLEVEL1),iShow);
//   ShowWindow(GetDlgItem(hDlgModeless,IDC_RADIOJOLIET),iShow);
//   ShowWindow(GetDlgItem(hDlgModeless,IDC_RADIOUDF),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_FS_COMBO),iShow);

	bSel = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction == FUNC_SAVEGI) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOMODE1),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOMODE2),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_LOADSTRING),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_LOAD),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction == FUNC_SAVEGI) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOMODE1),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_RADIOMODE2),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_LOADSTRING),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_LOAD),iShow);

	bSel = g->dwFunction==FUNC_RECORDDATA  && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_SLIDERSWAPSTRING),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_SLIDERSWAP),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_SWAP),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_SLIDERSWAPSTRING),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_SLIDERSWAP),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_SWAP),iShow);

	bSel = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction==FUNC_RECORDAUDIO || g->dwFunction == FUNC_SAVEGI) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CLOSEDISC),bSel);
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_DVDPRQUICK),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction==FUNC_RECORDAUDIO || g->dwFunction == FUNC_SAVEGI) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_CLOSEDISC),iShow);
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_DVDPRQUICK),iShow);

	bSel = (g->dwFunction == FUNC_SAVEGI) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_DVDIMAGE),bSel);
	iShow = (g->dwFunction == FUNC_SAVEGI) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_DVDIMAGE),iShow);

	if (g->dwFunction == FUNC_RECORDDATA && g_bStreamFsUsed == FALSE)
	{
		CheckDlgButton(g->hDlgModeless,IDC_STREAMFUNC,FALSE);
		EnableWindow(GetDlgItem(g->hDlgModeless,IDC_STREAMFUNC),FALSE);
	}
	else
	{
		bSel = (g->dwFunction == FUNC_RECORDDATA || g->dwFunction == FUNC_RECORDAUDIO) && !g->bBusy;
		EnableWindow(GetDlgItem(g->hDlgModeless,IDC_STREAMFUNC),bSel);
	}
	iShow = (IsStreamEnabled() && (g->dwFunction == FUNC_RECORDDATA || g->dwFunction == FUNC_RECORDAUDIO)) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_STREAMFUNC),iShow);

	bSel = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction == FUNC_SAVEGI || g->dwFunction==FUNC_RECORDAUDIO) && !g->bBusy;
	EnableWindow(GetDlgItem(g->hDlgModeless,IDC_CHKTAO),bSel);
	iShow = (g->dwFunction==FUNC_RECORDDATA || g->dwFunction == FUNC_SAVEGI || g->dwFunction==FUNC_RECORDAUDIO) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(g->hDlgModeless,IDC_CHKTAO),iShow);

	if (g->dwFunction==FUNC_SAVEDATA)
	{
		CheckDlgButton(g->hDlgModeless,IDC_RADIOMODE1,TRUE);
		SetDlgItemInt(g->hDlgModeless,IDC_LOAD,0,FALSE);
		g->dwFileSwap = 0;
		SendDlgItemMessage(g->hDlgModeless,IDC_SLIDERSWAP,TBM_SETPOS,TRUE,g->dwFileSwap);
		SendMessage(g->hDlgModeless,WM_HSCROLL,0,0);
	}
}



//-----------------------------------------------------------------------------
//
//  Get the drive address (triple) from the passed combo box.
//
//   Param: hDlg - Handle to the main dialog
//
//          uiIDCombo - ID of the combo box
//
//   Notes: None.
//
//
//  Return: The drive address packed into a DWORD, or
//          NO_DRIVE if there is any error
//
//-----------------------------------------------------------------------------
DWORD GetAddressFromCombo(HWND hDlg, UINT uiIDCombo)
{
	INT i;
	DWORD dwUnit;

	i = SendDlgItemMessage(hDlg,uiIDCombo,CB_GETCURSEL,0,0);
	if (i == CB_ERR) return(NO_DRIVE);
	dwUnit = (DWORD)SendDlgItemMessage(hDlg,uiIDCombo,CB_GETITEMDATA,(WPARAM)i,0);
	return(dwUnit);
}


//-----------------------------------------------------------------------------
//
// Process Windows messages to allow UI to be updated.
//
//   Param: None
//
//   Notes: This message pump is called to make sure the UI gets serviced
//           during long procedures
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID ProcessMessages(VOID)
{
	MSG msg;

	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		HandleMessage(&msg);
	}
}



//-----------------------------------------------------------------------------
//
//   Update the Progess information for the current function
//
//   Param: hDlg - Handle to the main dialog
//
//   Notes: This is called once a second via a WM_TIMER message.
//
//          Timer is only active during long operations.
//
//          Uses global dwFunction to decide what data to display
//
//  Return: None.
//
//
//-----------------------------------------------------------------------------
VOID UpdateProgress(HWND hDlg, PGLOBAL g)
{

	INT i;
	DWORD dwErr, dwRepStat;
	DWORD dwCommand, dwSense, dwASC, dwASCQ;
	DWORD dwSize, dwTotal;
	TCHAR Buf[50000], Buf2[2048];

	//
	// Determine the operation status.
	//

	dwRepStat = PrimoSDK_RunningStatus(g->dwHandle,g->bStop?PRIMOSDK_ABORT:PRIMOSDK_GETSTATUS,
									   &dwSize,&dwTotal);

	//
	// Display the appropriate progress message.
	//

	switch (g->dwFunction)
	{

	case FUNC_BUILDGI:      // Building a Global Image.

		PxLoadString(IDS_BUILDINGGI,Buf2,sizeof(Buf2));
		wsprintf(Buf,Buf2,dwSize,dwTotal);

		//
		// Get the status of the reading drive.
		//

		if ((dwErr = PrimoSDK_UnitStatus(g->dwHandle,&g->dwUnitSource,
										 &dwCommand,&dwSense,&dwASC,&dwASCQ)) == PRIMOSDK_OK)
		{
			wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - OK"),
					 LOBYTE(LOWORD(g->dwUnitSource)),
					 HIBYTE(HIWORD(g->dwUnitSource)),
					 LOBYTE(HIWORD(g->dwUnitSource)));
		}
		else
		{
			wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - Err:%d"),
					 LOBYTE(LOWORD(g->dwUnitSource)),
					 HIBYTE(HIWORD(g->dwUnitSource)),
					 LOBYTE(HIWORD(g->dwUnitSource)),
					 dwErr);
			wsprintf(&Buf2[_tcslen(Buf2)],_T(" - Cmd:%02X Sense:%02X ASC:%02X ASCQ:%02X"),
					 dwCommand,   dwSense,  dwASC,     dwASCQ);
			g->SenseCode           = dwASC;
			g->SenseQual           = dwASCQ;
			g->DeviceErrorDetected = TRUE;
		}

		_tcscat(Buf,Buf2);
		break;


	case FUNC_SAVEDATA:

		PxLoadString(IDS_BUILDINGIMAGE,Buf2,sizeof(Buf2));
		wsprintf(Buf,Buf2,dwSize,dwTotal);
		break;

	case FUNC_VERIFYDISC:
	case FUNC_VERIFYGI:
	case FUNC_VERIFYOTHERIMAGE:

		PxLoadString(IDS_VERIFING,Buf2,sizeof(Buf2));
		wsprintf(Buf,Buf2,dwSize,dwTotal);

		//
		// Loop for every drive engaged in the operation and get its status.
		//

		for (i=0; g->dwUnitsRec[i] != NO_DRIVE; i++)
		{
			if ((dwErr = PrimoSDK_UnitStatus(g->dwHandle,&g->dwUnitsRec[i],
											 &dwCommand,&dwSense,&dwASC,&dwASCQ)) == PRIMOSDK_OK)
			{
				wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - OK"),
						 LOBYTE(LOWORD(g->dwUnitsRec[i])),
						 HIBYTE(HIWORD(g->dwUnitsRec[i])),
						 LOBYTE(HIWORD(g->dwUnitsRec[i])));
			}
			else // Not OK.
			{
				wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - Verify error"),
						 LOBYTE(LOWORD(g->dwUnitsRec[i])),
						 HIBYTE(HIWORD(g->dwUnitsRec[i])),
						 LOBYTE(HIWORD(g->dwUnitsRec[i])));

			}
			_tcscat(Buf,Buf2);
		}
		break;

	case FUNC_AUDIOEXTRACT:

		PxLoadString(IDS_EXTRACTING,Buf2,sizeof(Buf2));
		wsprintf(Buf,Buf2,dwSize,dwTotal);
		break;

	case FUNC_SAVEGI:

		PxLoadString(IDS_BUILDINGGI,Buf2,sizeof(Buf2));
		wsprintf(Buf,Buf2,dwSize,dwTotal);
		break;

	default:

		if (g->dwFunction == FUNC_ERASEDISC)
		{
			wsprintf(Buf,_T("Erasing - %d of %d"),dwSize,dwTotal);
		}
		else
		{
			if (g->dwFunction == FUNC_VERIFYDATA)
				PxLoadString(IDS_VERIFING,Buf2,sizeof(Buf2));
			else
				PxLoadString(g->dwAction==PRIMOSDK_TEST?IDS_TESTING:IDS_WRITING,Buf2,sizeof(Buf2));
			wsprintf(Buf,Buf2,dwSize,dwTotal);
		}

		//
		// Loop for every drive engaged in the operation and get its status.
		//

		for (i=0; g->dwUnitsRec[i] != NO_DRIVE; i++)
		{
			if ((dwErr = PrimoSDK_UnitStatus(g->dwHandle,&g->dwUnitsRec[i],&dwCommand,
											 &dwSense,&dwASC,&dwASCQ)) == PRIMOSDK_OK)
			{
				wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - OK"),
						 LOBYTE(LOWORD(g->dwUnitsRec[i])),
						 HIBYTE(HIWORD(g->dwUnitsRec[i])),
						 LOBYTE(HIWORD(g->dwUnitsRec[i])));
			}
			else //Not OK
			{
				wsprintf(Buf2,_T("\r\n  %c: (h%d id%d) - Err:%d"),
						 LOBYTE(LOWORD(g->dwUnitsRec[i])),
						 HIBYTE(HIWORD(g->dwUnitsRec[i])),
						 LOBYTE(HIWORD(g->dwUnitsRec[i])),
						 dwErr);
				wsprintf(&Buf2[_tcslen(Buf2)],_T(" - Cmd:%02X Sense:%02X ASC:%02X ASCQ:%02X"),
						 dwCommand,   dwSense,  dwASC,     dwASCQ);

				g->SenseCode           = dwASC;
				g->SenseQual           = dwASCQ;
				g->DeviceErrorDetected = TRUE;
			}
			_tcscat(Buf,Buf2);
		}
		break;

	}

	//
	// Show the progress and status (this happens every timer tick)
	//

	if (g->dwFunction != FUNC_ERASEDISC && dwTotal > 0)
	{

		TCHAR TimeBuf[1000];

		EstimateProgress(&g->progress,dwSize,dwTotal,g);

		SendDlgItemMessage(hDlg,IDC_PROGRESS,PBM_SETPOS,(WPARAM)(g->progress.dwPercentComplete),0);

		// Put estimated time in results
		wsprintf(TimeBuf,_T("\r\nTimeRemaining = %ld, Total = %ld"),g->progress.dwTimeRemaining, g->progress.dwWriteTime);
		_tcscat(Buf,TimeBuf);
	}
	GetDlgItemText(hDlg,IDC_RESULT,Buf2,sizeof(Buf2));

	if (_tcscmp(Buf,Buf2))
		SetDlgItemText(hDlg,IDC_RESULT,Buf);

	//
	// If we are still running, just allow the timer to continue.
	//

	if (dwRepStat != PRIMOSDK_RUNNING)
	{
		if (g->dwStatusCounter != 0)
		{
			g->dwStatusCounter--;
			return;
		}

		//
		// For good or bad, we finished. Stop the timer.
		//

		KillTimer(hDlg,0);

		//
		// Now clear the progress indicator and return in the normal status.
		//

		SendDlgItemMessage(hDlg,IDC_PROGRESS,PBM_SETPOS,0,0);
		SetDlgItemText(hDlg,IDC_RESULT,_T(""));
		EnableDisable(FALSE, g);
	}
}


//-----------------------------------------------------------------------------
//
//  Execute the command that is passed in wCommand
//
//   Update the Progess information for the current function
//
//   Param: hDlg - Handle to the main dialog
//
//   Notes: Called when a button is pressed
//
//          Most of the logic is concerned with the "Go" button. (IDC_GO)
//            Based on the current function in dwFunction, dispatch to
//            the appropriate function.
//
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID ExecuteCommand(HWND hDlg, UINT wCommand, PGLOBAL g)
{

	INT  i, j, k;
	TCHAR Buf[50000], Buf2[2048];
	BYTE szBuf2[2048], szBuf3[1024];
	BYTE szUnitDescr[512], szUnitDescr2[512];
	DWORD dwDVDProtected, dwDVDPRQuick;
	DWORD dwCount;
	DWORD dwUnit;
	DWORD dwFSMode, dwCDMode, dwDateMode, dwCloseDisc, dwSpeed = 0, dwLoad, dwDVDImage;
	DWORD dwErr;
	BYTE szImageName[260];
	TCHAR *Files[2000]; // pointers to file names to be written to CD
	BOOL SkipClear = FALSE;

	switch (wCommand)
	{
	case IDC_CHECKTEST:
		if (IsDlgButtonChecked(hDlg,IDC_CHECKVERIFY) &&
			IsDlgButtonChecked(hDlg, IDC_CHECKTEST))
			CheckDlgButton(hDlg,IDC_CHECKVERIFY,FALSE);
		break;

	case IDC_CHECKVERIFY:
		if (IsDlgButtonChecked(hDlg,IDC_CHECKTEST) &&
			IsDlgButtonChecked(hDlg, IDC_CHECKVERIFY))
			CheckDlgButton(hDlg,IDC_CHECKTEST,FALSE);
		break;

	case IDC_EJECTREC:
	case IDC_EJECTSOURCE:

		//
		// Eject the caddy or open the tray.
		//

		dwUnit = GetAddressFromCombo(hDlg,wCommand==IDC_EJECTREC?IDC_LETTERREC:IDC_LETTERSOURCE);

		if (dwUnit == NO_DRIVE)
			break;

		dwErr = PrimoSDK_MoveMedium(g->dwHandle,&dwUnit,PRIMOSDK_OPENTRAYEJECT|PRIMOSDK_IMMEDIATE);

		if (dwErr != PRIMOSDK_OK)
			DisplayError(dwErr,_T("PrimoSDK_MoveMedium"),NULL);
		break;

	case IDC_CLOSETRAYREC:
	case IDC_CLOSETRAYSOURCE:

		//
		// Close the tray.
		//

		dwUnit = GetAddressFromCombo(hDlg,wCommand==IDC_CLOSETRAYREC?IDC_LETTERREC:IDC_LETTERSOURCE);

		if (dwUnit == NO_DRIVE)
			break;

		dwErr = PrimoSDK_MoveMedium(g->dwHandle,&dwUnit,PRIMOSDK_CLOSETRAY|PRIMOSDK_IMMEDIATE);

		if (dwErr != PRIMOSDK_OK)
			DisplayError(dwErr,_T("PrimoSDK_MoveMedium"),NULL);
		break;

	case IDC_UNITINFOREC:
	case IDC_UNITINFOSOURCE:

		//
		// Request the unit's information
		//

		dwUnit = GetAddressFromCombo(hDlg,wCommand==IDC_UNITINFOREC?IDC_LETTERREC:IDC_LETTERSOURCE);

		if (dwUnit == NO_DRIVE)
			break;

		DisplayUnitInfo(dwUnit, g->dwHandle);

		break;

	case IDC_DISCINFOREC:
	case IDC_DISCINFOSOURCE:

		//
		// Request the disc's info
		//

		dwUnit = GetAddressFromCombo(hDlg,wCommand==IDC_DISCINFOREC?IDC_LETTERREC:IDC_LETTERSOURCE);

		if (dwUnit == NO_DRIVE) break;

		dwErr = PrimoSDK_UnitReady(g->dwHandle,&dwUnit);

		if (dwErr != PRIMOSDK_OK)
		{
			DisplayError(dwErr,_T("PrimoSDK_UnitReady"),NULL);
			break;
		}

		DisplayDiscInfo(dwUnit, g->dwHandle);

		break;

	case IDC_GO:

		//
		// Write! (or test, or whatever function is selected)
		//

		//
		// Wait a moment... if we were running, this is an abort request.
		//

		if (g->bBusy)
		{
			g->bStop = TRUE;
			EnableWindow(GetDlgItem(hDlg,IDC_GO),FALSE);
			break;
		}


		//
		// If we need a recorder then fill dwUnitsRec with the unit(s) to record to.
		//

		if (NEEDS_RECORDER(g->dwFunction))
		{
			ZeroMemory(g->dwUnitsRec,sizeof(g->dwUnitsRec));

			if (IsDlgButtonChecked(hDlg,IDC_CHECKRECTOALL))
			{
				// "To all similar to the selected": (We check just the first part of the drive
				//  description in the combo, but, of course, they could have incompatible firmware.)

				BOOL ForceMatched = FALSE;
				if (g->dwFunction == FUNC_AUDIOEXTRACT ||
					g->dwFunction == FUNC_RECORDVIDEO  ||
					g->dwFunction == FUNC_RECORDAUDIO)
					// these don't use Stream FS, so continue to force matched devices
					ForceMatched = TRUE;

				i = (INT)SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_GETCURSEL,0,0);
				dwUnit = (DWORD)SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_GETITEMDATA,(WPARAM)i,0);

				PrimoSDK_UnitInfo(g->dwHandle,&dwUnit,NULL,szUnitDescr,NULL);

				szUnitDescr[strlen(szUnitDescr)-4] = 0x00;

				j = (INT)SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_GETCOUNT,0,0);

				for (i=0, k=0; i < j; i++)
				{
					dwUnit = (DWORD)SendDlgItemMessage(hDlg,IDC_LETTERREC,CB_GETITEMDATA,(WPARAM)i,0);

					PrimoSDK_UnitInfo(g->dwHandle,&dwUnit,NULL,szUnitDescr2,NULL);

					//
					// Do not check the firmware version and add to the list if brand-name is the same
					// Allow any mix of drives if StreamFs is in use
					//

					szUnitDescr2[strlen(szUnitDescr2)-4] = 0x00;

					if (strcmp(szUnitDescr,szUnitDescr2) == 0  ||  // matched device
						(!ForceMatched && g_bStreamFsUsed))        // using Stream FS and not forcing matched device
						g->dwUnitsRec[k++] = dwUnit;
				}
				g->dwUnitsRec[k] = NO_DRIVE;
			}
			else // record only to one unit.
			{
				g->dwUnitsRec[0] = GetAddressFromCombo(hDlg,IDC_LETTERREC);
				g->dwUnitsRec[1] = NO_DRIVE; // mark the end of the list with all F's
			}
		}


		if (NEEDS_SOURCE(g->dwFunction))
		{
			g->dwUnitSource = GetAddressFromCombo(hDlg,IDC_LETTERSOURCE);
		}

		//
		// Set a global to remember if it was a test or a real recording,
		//  just to ask only once, for simplicity.
		//

		if (IsDlgButtonChecked(hDlg,IDC_CHECKTEST))
		{
			g->dwAction = PRIMOSDK_TEST;
		}
		else
		{
			g->dwAction = PRIMOSDK_WRITE | PRIMOSDK_BURNPROOF;
		}

		//
		// Clear the abort flag to allow function to begin/continue.
		//

		g->bStop = FALSE;

		//
		// Prepare the speed.
		//

		if (g->dwFunction != FUNC_BUILDGI && g->dwFunction != FUNC_SAVEDATA &&
			g->dwFunction < FUNC_AUDIOEXTRACT)
		{
			i = SendDlgItemMessage(hDlg,IDC_SPEED,CB_GETCURSEL,0,0);
			dwSpeed = (DWORD)SendDlgItemMessage(hDlg,IDC_SPEED,CB_GETITEMDATA,(WPARAM)i,0);
		}

		switch (g->dwFunction)
		{

		case FUNC_COPY:

			//
			//  COPY DISC !
			//  (sourceDisk ==> destinationDisk(s))
			//

			CopyDisc(dwSpeed, g);

			break;

		case FUNC_BUILDGI:

			//
			//  BUILD A GLOBAL IMAGE (GI) !
			//  (sourceDisk ==> Global Image file)
			//

			//
			// Ask for the GI name.
			//

		{
			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			PxLoadString(IDS_GIEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars in the resource string with NULLs for the file filter.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

#ifdef _UNICODE
			{
				// tack on ISO as a possible target
				wchar_t *NextStr = (wchar_t*)szBuf2;
				while (NextStr[0] != L'\0')
					NextStr = &NextStr[wcslen(NextStr)+1];
				wcscpy(NextStr, L"Raw image (.ISO)");
				NextStr = &NextStr[wcslen(NextStr)+1];
				wcscpy(NextStr, L"*.ISO");
				NextStr = &NextStr[wcslen(NextStr)+1];
				NextStr[0] = L'\0';
			}
#endif
			ofn.lpstrFilter = (TCHAR*)szBuf2;

			FileName = (TCHAR*)szImageName;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = 256;
			ofn.lpstrDefExt = _T("gi");
			ofn.lpstrTitle = _T("Global Image to Build");
			ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

			if (!GetSaveFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			EnableDisable(TRUE, g);

			BuildGI(FileName, g);

			break;
		}

		case FUNC_RECORDGIORTRACK:

			//
			//  RECORD A GI OR ANY OTHER TRACK !
			//  (Global Image File ==> destination Disk)
			//  (Other Image File ==> destination Disk)
			//

			//
			// Ask the .GI (or .ISO or whatever) file name
			//

		{
			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			PxLoadString(IDS_IMAGEEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars in the resource string with NULLs for the file filter.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

			ofn.lpstrFilter = (TCHAR*)szBuf2;
			FileName = (TCHAR*)szImageName;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = 256;
			ofn.lpstrDefExt = _T("gi");
			ofn.lpstrTitle = _T("Global Image or Other Image to Record");
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

			if (!GetOpenFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			EnableDisable(TRUE, g);

			RecordGIorTrack(FileName, dwSpeed, g);

			break;
		}

		case FUNC_VERIFYDISC:

			//
			//  VERIFY DISC !
			//  Compare sourceDisk with destination Disks
			//

			VerifyDisc(dwSpeed, g);

			break;


		case FUNC_VERIFYGI:

			//
			//  VERIFY GI !
			//  Compare source disk to Global Image file
			//

			//
			// Ask for the file name.
			//

		{
			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			PxLoadString(IDS_IMAGEEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars in the resource string with NULLs for the file filter.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

			ofn.lpstrFilter = (TCHAR*)szBuf2;
			FileName = (TCHAR*)szImageName;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = 256;
			ofn.lpstrDefExt = _T("gi");
			ofn.lpstrTitle = _T("Global Image");
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

			if (!GetOpenFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			//
			// Set the recording status.
			//

			EnableDisable(TRUE, g);

			VerifyGlobalImage(FileName, dwSpeed, g);

			break;
		}

		case  FUNC_VERIFYOTHERIMAGE:

			//
			//  VERIFY OTHER IMAGE !
			//  Compare source disk to ISO or UDI Image file
			//

			//
			// Ask for the file name.
			//

		{
			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			PxLoadString(IDS_IMAGEEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars in the resource string with NULLs for the file filter.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

			ofn.lpstrFilter = (TCHAR*)szBuf2;
			FileName = (TCHAR*)szImageName;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = 256;
			ofn.lpstrDefExt = _T("iso");
			ofn.lpstrTitle = _T("Image File");
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

			if (!GetOpenFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			//
			// Set the recording status.
			//

			EnableDisable(TRUE, g);

			VerifyOtherImage(FileName, dwSpeed, g);

			break;
		}


		case FUNC_RECORDDATA:

			//
			//
			//  DATA DISC !
			// =============
			//  (List of files ==> Disk)

			//
			// Check that the data list is not empty
			//

			if (!g->dwTotEntries)
			{
				PxLoadString(IDS_LISTEMPTY,Buf,sizeof(Buf));
				MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				break;
			}

			//
			// Set the recording status.
			//

			EnableDisable(TRUE, g);
			UpdateWindow(hDlg);

			//
			// Prepare parameters for new image.
			//

			i = SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETCURSEL,0,0);
			dwFSMode = (DWORD)SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETITEMDATA,(WPARAM)i,0);

			if ((dwFSMode == PRIMOSDK_JOLIET) || (dwFSMode == PRIMOSDK_ISOLEVEL1) || (dwFSMode == PRIMOSDK_ISOLEVEL2)
				|| (dwFSMode == PRIMOSDK_ISOLEVEL3))
				dwCDMode = IsDlgButtonChecked(hDlg,IDC_RADIOMODE1)?PRIMOSDK_MODE1:PRIMOSDK_MODE2;
			else if ((dwFSMode == PRIMOSDK_UDF) || (dwFSMode == PRIMOSDK_UDF201) || (dwFSMode == PRIMOSDK_UDF250)|| (dwFSMode == PRIMOSDK_UDF260))
				dwCDMode = PRIMOSDK_MODE1;


			if (IsDlgButtonChecked(hDlg,IDC_CHKTAO))
				dwCDMode |= PRIMOSDK_TAO;
			else
				dwCDMode |= PRIMOSDK_SAO;

			dwDateMode = PRIMOSDK_ORIGDATE;
			dwCloseDisc = IsDlgButtonChecked(hDlg,IDC_CLOSEDISC)?PRIMOSDK_CLOSEDISC:0;
			dwDVDPRQuick = IsDlgButtonChecked(hDlg,IDC_DVDPRQUICK)?PRIMOSDK_DVDPRQUICK:0;
			dwLoad = GetDlgItemInt(hDlg,IDC_LOAD,NULL,FALSE);

			if (dwLoad)
			{
				DWORD dwMedium;

				//
				// The only valid values for dvd+rw media are 0 and 1.
				//

				dwErr = PrimoSDK_DiscInfo2(g->dwHandle,&g->dwUnitsRec[0],&dwMedium,&dwDVDProtected,NULL,NULL,NULL);

				if (dwErr != PRIMOSDK_OK)
				{
					DisplayError(dwErr,_T("PrimoSDK_DiscInfo2"),NULL);
					break;
				}
				if (dwMedium == PRIMOSDK_DVDPRW && (dwLoad != 0 && dwLoad != 1))
				{
					PxLoadString(IDS_INVALIDLOADVALUE,Buf,sizeof(Buf));
					MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				}

				PxLoadString(IDS_LOADING,Buf2,sizeof(Buf2));
				wsprintf(Buf,Buf2,dwLoad);
				SetDlgItemText(hDlg,IDC_RESULT,Buf);
				UpdateWindow(hDlg);
				Sleep(1000);
			}

			GetDlgItemText(hDlg,IDC_VOLUMENAME,Buf2,32+1); // Get volume name

			//
			// Assemble a list of all source files to pass to RecordData
			// Get the name of all source files into szBuf, and pointers to
			// each one in pszFiles.
			//

			Files[0] = Buf; // first file points at beginning of szBuf
			for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
			{
				SendDlgItemMessage(hDlg,IDC_FILELIST,LB_GETTEXT,dwCount,(LONG)(LPSTR)Files[dwCount]);

				//
				// Point the next index past the file name we just read.
				//

				Files[dwCount+1] = &Files[dwCount][_tcslen(Files[dwCount])+1]; //+1 gets past the null in the string.
			}
			Files[g->dwTotEntries] = NULL; //null terminate the list

			//
			// Now pszFiles has pointers to each individual string in szBuf.
			//

			SkipClear = IsDlgButtonChecked(hDlg,IDC_TOTALONLY);
			RecordData(Buf2, Files, dwLoad,  dwFSMode,  dwCDMode,  dwDateMode,  dwCloseDisc,
					   g->dwFileSwap==257?0xFFFFFFFF:g->dwFileSwap, dwDVDPRQuick, dwSpeed,
					   IsDlgButtonChecked(hDlg,IDC_CHECKVERIFY),
					   IsDlgButtonChecked(hDlg,IDC_STREAMFUNC),
					   SkipClear, g);

			break;

		case FUNC_SAVEDATA:

			//
			//  DATA DISC TO IMAGE !
			// (source CD ==>  ISO Image file)
			// (source DVD ==> UDF Image file)

			//
			// Check that the data list is not empty.
			//

			if (!g->dwTotEntries)
			{
				PxLoadString(IDS_LISTEMPTY,Buf,sizeof(Buf));
				MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				break;
			}

			//
			// Ask for the .iso or .udi file.
			//
			{
				OPENFILENAME ofn;
				TCHAR *ExtMask;
				TCHAR *FileName;

				memset(&ofn,0x00,sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hDlg;
				PxLoadString(IDS_ISOUDIEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

				//
				// Replace vertical bars in the resource string with NULLs for the file filter.
				//

				ExtMask = (TCHAR*)szBuf2;
				for (i=0; ExtMask[i]; i++)
					if (ExtMask[i] == '|')
						ExtMask[i] = 0x00;

				ofn.lpstrFilter = (TCHAR*)szBuf2;
				FileName = (TCHAR*)szImageName;
				FileName[0] = 0x00;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = 256;
				ofn.lpstrDefExt = _T("iso");
				ofn.lpstrTitle = _T("Write Image File");
				ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

				if (!GetSaveFileName(&ofn))
					break;

				UpdateWindow(g->hWndMain);

				//
				// Set the recording status.
				//

				EnableDisable(TRUE, g);
				UpdateWindow(hDlg);

				//
				// Prepare parameters for new image.
				//

				i = SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETCURSEL,0,0);
				dwFSMode = (DWORD)SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETITEMDATA,(WPARAM)i,0);


				GetDlgItemText(hDlg,IDC_VOLUMENAME,(TCHAR*)szBuf2,32+1); // get volume name


				//
				// Assemble a list of all source files to pass to RecordData
				// Get the name of all source files into szBuf, and pointers
				// to each one in pszFiles.
				//

				Files[0] = Buf; // first file points at beginning of szBuf
				for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
				{
					SendDlgItemMessage(hDlg,IDC_FILELIST,LB_GETTEXT,dwCount,(LONG)Files[dwCount]);

					//
					// Point the next index past the file name we just read.
					//

					Files[dwCount+1] = &Files[dwCount][_tcslen(Files[dwCount])+1]; //+1 gets past the null in the string.
				}
				Files[g->dwTotEntries] = NULL; // Null terminate the list.

				//
				// Now pszFiles has pointers to each individual string in szBuf
				//

				SaveData(FileName, Files, (TCHAR*)szBuf2, dwFSMode, g);

			}

			break;

		case FUNC_RECORDAUDIO:

			//
			//  AUDIO DISC !
			// (List of Audio files ==>Audio CD)
			//

			//
			// Check that the data list is not empty.
			//

			if (AudioFileExists() == FALSE && !g->dwTotEntries)
			{
				PxLoadString(IDS_LISTEMPTY,Buf,sizeof(Buf));
				MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				break;
			}

			//
			// Set the recording status.
			//

			EnableDisable(TRUE, g);
			dwCloseDisc = IsDlgButtonChecked(hDlg,IDC_CLOSEDISC)?PRIMOSDK_CLOSEDISC:0;


			//
			// Assemble a list of all source files to pass to RecordAudio
			// Get the name of all source files into szBuf, and pointers to each
			// one in pszFiles.
			//

			Files[0] = Buf; // First file points at beginning of szBuf.
			for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
			{
				SendDlgItemMessage(hDlg,IDC_FILELIST,LB_GETTEXT,dwCount,(LONG)Files[dwCount]);

				//
				// point the next index past the file name we just read
				//

				Files[dwCount+1] = &Files[dwCount][_tcslen(Files[dwCount])+1]; //+1 gets past the null in the string.
			}
			Files[g->dwTotEntries] = NULL; //Null terminate the list.

			//
			// Now pszFiles has pointers to each individual string in szBuf
			//

			dwCloseDisc = IsDlgButtonChecked(hDlg,IDC_CLOSEDISC)?PRIMOSDK_CLOSEDISC:0;

			//
			//Use a checkbox to determine whether to use track at once or other method
			//

			if (IsDlgButtonChecked(hDlg,IDC_CHKTAO))
				RecordAudioTrackAtOnce(Files, dwCloseDisc, dwSpeed, IsDlgButtonChecked(hDlg,IDC_STREAMFUNC), g);
			else
				RecordAudio(Files, dwCloseDisc, dwSpeed, IsDlgButtonChecked(hDlg,IDC_STREAMFUNC), g);

			break;

		case FUNC_RECORDVIDEO:

			//
			//  VIDEO CD !
			// (List of video files ==> Video CD)
			//

			//
			// Check that the data list is not empty.
			//

			if (!g->dwTotEntries)
			{
				PxLoadString(IDS_LISTEMPTY,Buf,sizeof(Buf));
				MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				break;
			}

			//
			// Set the recording status.
			//

			EnableDisable(TRUE, g);

			//
			// Assemble a list of all source files to pass to RecordVideo
			// Get the name of all source files into szBuf, and pointers to each
			// one in pszFiles.
			//

			Files[0] = Buf; // first file points at beginning of szBuf
			for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
			{
				SendDlgItemMessage(hDlg,IDC_FILELIST,LB_GETTEXT,dwCount,(LONG)Files[dwCount]);

				//
				// point the next index past the file name we just read
				//

				Files[dwCount+1] = &Files[dwCount][_tcslen(Files[dwCount])+1]; //+1 gets past the null in the string.
			}
			Files[g->dwTotEntries] = NULL; //null terminate the list

			//
			// Now pszFiles has pointers to each individual string in szBuf
			//

			RecordVideo(Files, dwSpeed, g);

			break;

		case FUNC_AUDIOEXTRACT:
		{

			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			//
			//  AUDIO EXTRACT !
			// (Audio Track on CD => .WAV file)
			//

			//
			// Get the track number in dwCount
			//

			dwCount = GetDlgItemInt(hDlg,IDC_EDITTRACK,NULL,FALSE);

			//
			// Ask user for the name of the destination .wav file
			//

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;

			PxLoadString(IDS_WAVEEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars '|' in the resource string with NULLs for the file filter.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

			{
				// tack on RAW
				TCHAR *NextStr = (TCHAR*)szBuf2;
				while (NextStr[0] != _T('\0'))
					NextStr = &NextStr[_tcslen(NextStr)+1];
				_tcscpy(NextStr, _T("Raw PCM (.raw)"));
				NextStr = &NextStr[_tcslen(NextStr)+1];
				_tcscpy(NextStr, _T("*.RAW"));
				NextStr = &NextStr[_tcslen(NextStr)+1];
				NextStr[0] = _T('\0');
			}

			ofn.lpstrFilter = (TCHAR*)szBuf2;

			FileName = (TCHAR*)szBuf3;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = min(256, sizeof(szBuf3)/sizeof(TCHAR));
			ofn.lpstrDefExt = _T("wav");
			ofn.nFileExtension = 2;
			//ofn.11
			ofn.lpstrTitle = _T("Extract to");
			ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

			if (!GetSaveFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			if (_tcsncmp(&FileName[ofn.nFileExtension], _T("RAW"), 3) == 0)
				AudioExtractBuffer(FileName, dwCount, g);
			else
				AudioExtract(FileName, dwCount, g);

			break;
		}

		case FUNC_ERASEDISC:

			//
			//  ERASE DISC !
			//

			dwUnit = GetAddressFromCombo(hDlg,IDC_LETTERREC);

			if (dwUnit == NO_DRIVE)
				break;

			EraseDisc(dwUnit, g);

			break;

		case FUNC_SAVEGI:

			//
			//  DATA DISC !
			//  (List of files ==> Global Image file)
			//

			//
			// Check that the data list is not empty
			//

			if (!g->dwTotEntries)
			{
				PxLoadString(IDS_LISTEMPTY,Buf,sizeof(Buf));
				MessageBox(hDlg,Buf,AppTitle,MB_OK|MB_ICONINFORMATION);
				break;
			}

			//
			// Ask the GI name.
			//

			{

				OPENFILENAME ofn;
				TCHAR *ExtMask;
				TCHAR *FileName;

				memset(&ofn,0x00,sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hDlg;
				PxLoadString(IDS_GIEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

				//
				// Replace vertical bars in the resource string with NULLs for the file filter.
				//

				ExtMask = (TCHAR*)szBuf2;
				for (i=0; ExtMask[i]; i++)
					if (ExtMask[i] == '|')
						ExtMask[i] = 0x00;

				ofn.lpstrFilter = (TCHAR*)szBuf2;
				FileName = (TCHAR*)szImageName;
				FileName[0] = 0x00;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = 256;
				ofn.lpstrDefExt = _T("gi");
				ofn.lpstrTitle = _T("Global Image to Build");
				ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

				if (!GetSaveFileName(&ofn))
					break;

				UpdateWindow(g->hWndMain);

				EnableDisable(TRUE, g);


				//
				// Prepare parameters for new image.
				//

				i = SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETCURSEL,0,0);
				dwFSMode = (DWORD)SendDlgItemMessage(hDlg,IDC_FS_COMBO,CB_GETITEMDATA,(WPARAM)i,0);

				if ((dwFSMode == PRIMOSDK_JOLIET) || (dwFSMode == PRIMOSDK_ISOLEVEL1) || (dwFSMode == PRIMOSDK_ISOLEVEL2)
					|| (dwFSMode == PRIMOSDK_ISOLEVEL3))
					dwCDMode = IsDlgButtonChecked(hDlg,IDC_RADIOMODE1)?PRIMOSDK_MODE1:PRIMOSDK_MODE2;
				else if ((dwFSMode == PRIMOSDK_UDF) || (dwFSMode == PRIMOSDK_UDF201) || (dwFSMode == PRIMOSDK_UDF250)|| (dwFSMode == PRIMOSDK_UDF260))
					dwCDMode = PRIMOSDK_MODE1;

				if (IsDlgButtonChecked(hDlg,IDC_CHKTAO))
					dwCDMode |= PRIMOSDK_TAO;
				else
					dwCDMode |= PRIMOSDK_SAO;

				dwDateMode = PRIMOSDK_ORIGDATE;
				dwCloseDisc = IsDlgButtonChecked(hDlg,IDC_CLOSEDISC)?PRIMOSDK_CLOSEDISC:0;
				dwDVDPRQuick = IsDlgButtonChecked(hDlg,IDC_DVDPRQUICK)?PRIMOSDK_DVDPRQUICK:0;
				dwDVDImage =   IsDlgButtonChecked(hDlg,IDC_DVDIMAGE)?PRIMOSDK_DVDIMAGE:0;
				dwLoad = GetDlgItemInt(hDlg,IDC_LOAD,NULL,FALSE);

				GetDlgItemText(hDlg,IDC_VOLUMENAME,(LPTSTR)szBuf2,32+1); // Get volume name

				//
				// Assemble a list of all source files to pass to SaveGI
				// Get the name of all source files into szBuf, and pointers to each one in pszFiles.
				//

				Files[0] = Buf; // first file points at beginning of szBuf
				for (dwCount=0; dwCount < g->dwTotEntries; dwCount++)
				{
					SendDlgItemMessage(hDlg,IDC_FILELIST,LB_GETTEXT,dwCount,(LONG)Files[dwCount]);

					//
					// Point the next index past the file name we just read.
					//

					Files[dwCount+1] = &Files[dwCount][_tcslen(Files[dwCount])+1]; //+1 gets past the null in the string.
				}
				Files[g->dwTotEntries] = NULL; //null terminate the list

				//
				// Now pszFiles has pointers to each individual string in szBuf.
				//

				SaveGI(FileName, (LPTSTR)szBuf2, Files, dwFSMode|dwCDMode|dwDateMode|dwCloseDisc|dwDVDImage, g);

			}

			break;

		case FUNC_GIINFO:

			//
			//  DISPLAY INFORMATION ON A GI FILE
			//

			//
			// Ask the .GI file name
			//

		{
			OPENFILENAME ofn;
			TCHAR *ExtMask;
			TCHAR *FileName;

			memset(&ofn,0x00,sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			PxLoadString(IDS_IMAGEEXT,(LPTSTR)szBuf2,sizeof(szBuf2));

			//
			// Replace vertical bars with nulls.
			//

			ExtMask = (TCHAR*)szBuf2;
			for (i=0; ExtMask[i]; i++)
				if (ExtMask[i] == '|')
					ExtMask[i] = 0x00;

			ofn.lpstrFilter = (TCHAR*)szBuf2;
			FileName = (TCHAR*)szImageName;
			FileName[0] = 0x00;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = 256;
			ofn.lpstrDefExt = _T("gi");
			ofn.lpstrTitle = _T("Global Image");
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

			if (!GetOpenFileName(&ofn))
				break;

			UpdateWindow(g->hWndMain);

			EnableDisable(TRUE, g);

			GIInfo(FileName, g);

			break;
		}

		break;

		} // end switch (dwFunction)


	case IDC_CLEARDATA:

		//
		// Clear the data listbox.
		//

		if (!SkipClear)
		{
			SendDlgItemMessage(g->hDlgModeless,IDC_FILELIST,LB_RESETCONTENT,0,0L);
			SendDlgItemMessage(g->hDlgModeless,IDC_FILELIST,LB_SETHORIZONTALEXTENT,0,0);
			g->dwTotEntries = 0;
			EnableDisable(FALSE, g);
		}
		break;

	}
}


//-----------------------------------------------------------------------------
//
//  Display resource string in a message box
//
//   Param: resID - Resource ID for the string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID PxMessage(UINT resID)
{
	TCHAR Buf[2000];
	PxLoadString(resID,Buf,sizeof(Buf));
	PxMessageStr(Buf, NULL);
}


//-----------------------------------------------------------------------------
//
//  Display string in a message box
//
//   Param: szMessage - string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID PxMessageStr(LPTSTR Message, HWND hDlgModeless)
{
	MessageBox(hDlgModeless,Message,AppTitle,MB_OK|MB_ICONINFORMATION);
}



//-----------------------------------------------------------------------------
//
//  Ask the user a Yes/No question from a resource
//
//   Param: resID - Resource ID for the string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
int PxMessageYesNo(UINT resID, HWND hDlgModeless)
{
	TCHAR Buf[2000];
	PxLoadString(resID,Buf,sizeof(Buf));
	return PxMessageStrYesNo(Buf, hDlgModeless);
}

//-----------------------------------------------------------------------------
//
//  Ask the user a Yes/No question
//
//   Param: szMessage - string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
int PxMessageStrYesNo(LPTSTR Message, HWND hDlgModeless)
{
	return MessageBox(hDlgModeless,Message,AppTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
}

//-----------------------------------------------------------------------------
//
//  Set the text in the result window by resource ID
//
//   Param: resID - Resource ID for the string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID SetResultText(int resID, PGLOBAL g)
{
	TCHAR Buf[2000];
	PxLoadString(resID,Buf,sizeof(Buf));
	SetResultTextStr(Buf, g);
}

//-----------------------------------------------------------------------------
//
//  Set the text in the result window
//
//   Param: szMessage - string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID SetResultTextStr(LPTSTR Text, PGLOBAL g)
{
	SetDlgItemText(g->hDlgModeless,IDC_RESULT,Text);
	UpdateWindow(g->hWndMain);
}

//-----------------------------------------------------------------------------
//
//  Load a string from the applications resource
//
//   Param: szMessage - string to display
//
//   Notes: None.
//
//  Return: None.
//
//-----------------------------------------------------------------------------
VOID PxLoadString(UINT resID, LPTSTR Buf, int nBufferMax)
{
	LoadString(hInstAp,resID,Buf,nBufferMax);
}
VOID PxLoadStringA(UINT resID, LPSTR szBuf, int nBufferMax)
{
	LoadStringA(hInstAp,resID,szBuf,nBufferMax);
}


//-----------------------------------------------------------------------------
//
//  Display a MessageBox with a text representation of the error.
//
//   Param: cdwError - Error code to be displayed
//
//          szFunction - Function which returned the error
//
//          szAdditionalParam - any additional text to be displayed.
//
//   Notes: Uses the following globals:
//
//          dwUnitSource - address of source drive
//
//          dwUnitsRec -  list of recorder addresses terminated by NO_DRIVE
//
//          hDlgModeless - Main Dialog window
//
//  Return: None.
//
//
//-----------------------------------------------------------------------------
VOID DisplayError(DWORD dwError, LPTSTR Function, LPTSTR AdditionalParam)
{
	TCHAR FormattedErrorMsg[512], ErrorMsg[256], ErrorStr[64];

	//
	// Get text for the error
	//

	switch (dwError)
	{
	case 1 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_CMDSEQUENCE"));
		break;
	case 2 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_NOASPI"));
		break;
	case 3 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_INTERR"));
		break;
	case 4 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_BADPARAM"));
		break;
	case 6 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_ALREADYEXIST"));
		break;
	case 7 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_NOTREADABLE"));
		break;
	case 8 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_NOSPACE"));
		break;
	case 9 :
		_tcscpy(ErrorStr,_T("PRIMOSDK_INVALIDMEDIUM"));
		break;
	case 10:
		_tcscpy(ErrorStr,_T("PRIMOSDK_RUNNING"));
		break;
	case 11:
		_tcscpy(ErrorStr,_T("PRIMOSDK_BUR"));
		break;
	case 12:
		_tcscpy(ErrorStr,_T("PRIMOSDK_SCSIERROR"));
		break;
	case 13:
		_tcscpy(ErrorStr,_T("PRIMOSDK_UNITERROR"));
		break;
	case 14:
		_tcscpy(ErrorStr,_T("PRIMOSDK_NOTREADY"));
		break;
	case 15:
		_tcscpy(ErrorStr,_T("PRIMOSDK_DISKOVERFLOW"));
		break;
	case 16:
		_tcscpy(ErrorStr,_T("PRIMOSDK_INVALIDSOURCE"));
		break;
	case 17:
		_tcscpy(ErrorStr,_T("PRIMOSDK_INCOMPATIBLE"));
		break;
	case 18:
		_tcscpy(ErrorStr,_T("PRIMOSDK_FILEERROR"));
		break;
	case 23:
		_tcscpy(ErrorStr,_T("PRIMOSDK_ITSADEMO"));
		break;
	case 24:
		_tcscpy(ErrorStr,_T("PRIMOSDK_USERABORT"));
		break;
	case 25:
		_tcscpy(ErrorStr,_T("PRIMOSDK_BADHANDLE"));
		break;
	case 26:
		_tcscpy(ErrorStr,_T("PRIMOSDK_BADUNIT"));
		break;
	case 27:
		_tcscpy(ErrorStr,_T("PRIMOSDK_ERRORLOADING"));
		break;
	case 29:
		_tcscpy(ErrorStr,_T("PRIMOSDK_NOAINCONTROL"));
		break;
	case 30:
		_tcscpy(ErrorStr,_T("PRIMOSDK_READERROR"));
		break;
	case 31:
		_tcscpy(ErrorStr,_T("PRIMOSDK_WRITEERROR"));
		break;
	case PRIMOSDK_TMPOVERFLOW: // 32
		_tcscpy(ErrorStr,_T("PRIMOSDK_TMPOVERFLOW"));
		break;
	case PRIMOSDK_DVDSTRUCTERROR: // 33
		_tcscpy(ErrorStr,_T("PRIMOSDK_DVDSTRUCTERROR"));
		break;
	case PRIMOSDK_FILETOOLARGE: // 34
		_tcscpy(ErrorStr,_T("PRIMOSDK_FILETOOLARGE"));
		break;
	case PRIMOSDK_CACHEFULL: // 35
		_tcscpy(ErrorStr,_T("PRIMOSDK_CACHEFULL"));
		break;
	case PRIMOSDK_FEATURE_NOT_SUPPORTED: // 36
		_tcscpy(ErrorStr,_T("PRIMOSDK_FEATURE_NOT_SUPPORTED"));
		break;
	case PRIMOSDK_FEATURE_DISABLED: // 37
		_tcscpy(ErrorStr,_T("PRIMOSDK_FEATURE_DISABLED"));
		break;
	case PRIMOSDK_CALLBACK_ERROR: // 38
		_tcscpy(ErrorStr,_T("PRIMOSDK_CALLBACK_ERROR"));
		break;
	case PRIMOSDK_PROTECTEDWMA: // 39
		_tcscpy(ErrorStr,_T("PRIMOSDK_PROTECTEDWMA"));
		break;
	case PRIMOSDK_LIMITEXPIRED: // 40
		_tcscpy(ErrorStr,_T("PRIMOSDK_LIMITEXPIRED"));
		break;
	case PRIMOSDK_INVALIDPROPERTY: // 41
		_tcscpy(ErrorStr,_T("PRIMOSDK_INVALIDPROPERTY"));
		break;
	case PRIMOSDK_NEEDFULLERASE: // 42
		_tcscpy(ErrorStr,_T("PRIMOSDK_NEEDFULLERASE"));
		break;
	default:
		ErrorStr[0] = 0x00;
	}

	PxLoadString(IDS_FUNCTIONRETURNED,ErrorMsg,sizeof(ErrorMsg));

	_stprintf(FormattedErrorMsg,ErrorMsg,Function,dwError,ErrorStr);

	if (AdditionalParam != NULL)
	{
		_stprintf(&FormattedErrorMsg[_tcslen(FormattedErrorMsg)],_T("\n\n%s"),AdditionalParam);
	}

	MessageBox(NULL,FormattedErrorMsg,AppTitle,MB_OK|MB_ICONINFORMATION);
}


VOID WaitForProcessComplete(PGLOBAL g)
{
	SetTimer(g->hDlgModeless,0,1000,NULL); // 1000 milliseconds, no callback.

	//
	// Let the timer fire off once a second to do progress.
	// When it sees that the procedure is done, it will set the global bBusy.
	//

	while (g->bBusy)
		ProcessMessages();
}


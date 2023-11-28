#include "./main.h"
#include "./dropWindowInternal.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./messageBoxTweak.h"
#include "./extensionFilterList.h"
#include "./supportedExtensions.h"
#include "./fileInfoInterface.h"
#include "./itemTypeInterface.h"
#include "./document.h"
#include "./formatData.h"
#include <shlwapi.h>
#include <strsafe.h>

#define CDM_UPDATEFILENAME (WM_APP + 2)

static HWND DropWindowFromFrame(HWND hwnd)
{
	HWND hDrop = GetParent(hwnd);
	TCHAR szClass[128];
	while(GetClassName(hDrop, szClass, ARRAYSIZE(szClass)))
	{
		if(CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szClass, -1, NWC_DROPBOX, -1))
			break;
		
		hDrop = GetWindow(hDrop, GW_CHILD);
		if (NULL == hDrop)
			break;
	}
	return hDrop;
}

static Document *DlgDocument(HWND hdlg)
{
	HWND hFrame = GetParent(hdlg);
	if (NULL == hFrame) return NULL;
	HWND hDrop = DropWindowFromFrame(hFrame); // thats wrong
	if (NULL == hDrop) return NULL;
	return DropboxWindow_GetActiveDocument(hDrop);
}

static LPCTSTR GetFilterExtension(LPCTSTR pszFilter, INT filterIndex)
{
	if (filterIndex < 1 )
	{
		if ( NULL == pszFilter) return NULL;
		filterIndex = 1;
	}
	INT index = 1;
	for (LPCTSTR p = pszFilter; TEXT('0') != *p; p += (lstrlen(p) + 1), index++)
	{
		if (0 == index%2)
		{
			if (index/2 == filterIndex)
			{
				p = PathFindExtension(p);
				if (TEXT('.') == *p) p++;
				return p;
			}
		}
	}
	return NULL;
}

static INT GetFilterIndex(LPCTSTR pszFilter, LPCTSTR pszFileName)
{
	LPCTSTR pExt = PathFindExtension(pszFileName);
	if (TEXT('.') == *pExt) pExt++;
	else return 0;
	
	INT index = 1;
	for (LPCTSTR p = pszFilter; TEXT('\0') != *p; p += (lstrlen(p) + 1), index++)
	{
		if (0 == index%2)
		{
			p = PathFindExtension(p);
			if (TEXT('.') == *p) p++;
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pExt, -1, p, -1))
				return index/2;	
		}
	}
	return 0;
}

static void GeneratePlaylistNameEx(Document *pDoc, LPCTSTR pszFolder, LPCTSTR pszExtension, LPTSTR pszBufferOut, INT cchBufferMax)
{
	HRESULT hr;
	TCHAR szPath[MAX_PATH], szName[MAX_PATH];
	
	if (NULL != pDoc)
	{
		hr = pDoc->GetTitle(szPath, ARRAYSIZE(szPath));
		hr = Document::FileNameFromTitle(szName, ARRAYSIZE(szName), (SUCCEEDED(hr)) ? szPath : NULL);
	}
	else hr = E_FAIL;

	if (FAILED(hr)) 
		StringCchCopy(szName, ARRAYSIZE(szName), TEXT("Playlist1"));
	
	if (NULL == pszFolder)
		pszFolder = TEXT('\0');

	INT cchName = lstrlen(szName);
	INT cchFolder = lstrlen(pszFolder);
	if ((cchName + cchFolder) >= MAX_PATH)
	{
		cchName = MAX_PATH - cchFolder - 2;
		if (cchName < 0)
			cchName = 0;
		szName[cchName] = TEXT('\0');
	}
	
	PathCombine(szPath, ((NULL != pszFolder) ? pszFolder : TEXT('\0')), szName);
	if (TEXT('\0') != *szPath)
	{
		INT cchLen = lstrlen(szPath);
		if (NULL != pszExtension && TEXT('\0') != pszExtension) 
			StringCchPrintf(&szPath[cchLen], (ARRAYSIZE(szPath) - cchLen), TEXT(".%s"), pszExtension);
		
		INT index = 1;
		
		while(PathFileExists(szPath))
		{
			szPath[cchLen - 1] = TEXT('\0');
			if (NULL != pszExtension && TEXT('\0') != pszExtension) 
				StringCchPrintf(&szPath[cchLen - 1], (ARRAYSIZE(szPath) - cchLen), TEXT("%d.%s"), index, pszExtension);
			else 
				StringCchPrintf(&szPath[cchLen - 1], (ARRAYSIZE(szPath) - cchLen), TEXT("%d"), index);
			index++;
		}
		StringCchCopy(pszBufferOut, cchBufferMax, PathFindFileName(szPath));
	}
	else
	{
		StringCchCopy(pszBufferOut, cchBufferMax, TEXT(""));
	}

}

static void GeneratePlaylistName(HWND hdlg, LPCTSTR pszExtension, LPTSTR pszBufferOut, INT cchBufferMax)
{
	TCHAR szFolder[MAX_PATH];
	szFolder[0] = TEXT('\0');
	HWND hFrame = GetAncestor(hdlg, GA_PARENT);
	if (NULL != hFrame && 
		!SendMessage(hFrame, CDM_GETFOLDERPATH, ARRAYSIZE(szFolder), (LPARAM)szFolder))
		szFolder[0] = TEXT('\0');

	Document *pDoc = DlgDocument(hdlg);
	GeneratePlaylistNameEx(pDoc, szFolder, pszExtension, pszBufferOut, cchBufferMax);
}


static BOOL GetCheckboxRect(HWND hdlg, RECT *prc)
{
	HWND hFrame = GetParent(hdlg);
	if (NULL == hFrame)
		return FALSE;
		
	RECT rcTmp;
	LONG fileboxBottom = 0;
	HWND hctrl;
	
	SetRectEmpty(prc);

	hctrl = GetDlgItem(hFrame, cmb13); // filename box
	if (NULL != hctrl && GetWindowRect(hctrl, &rcTmp))
	{
		fileboxBottom = rcTmp.bottom;
	}

	hctrl = GetDlgItem(hFrame, cmb1); // filter box
	if (NULL == hctrl || !GetWindowRect(hctrl, &rcTmp))
		return FALSE;
	
	if (0 == fileboxBottom || fileboxBottom > rcTmp.top)
		fileboxBottom = rcTmp.top - 6;
	prc->top = rcTmp.bottom + (rcTmp.top - fileboxBottom);
	prc->left = rcTmp.left;
	prc->right = rcTmp.right;
	

	hctrl = GetDlgItem(hdlg, IDC_CHECK_REGISTERPLAYLIST);
	if (NULL == hctrl || !GetWindowRect(hctrl, &rcTmp))
		return FALSE;

	prc->bottom = prc->top + (rcTmp.bottom - rcTmp.top);
	
	MapWindowPoints(HWND_DESKTOP, hdlg, (POINT*)prc, 2);
	return TRUE;
}

static void RecalcFrameSize(HWND hdlg)
{
	HWND hFrame = GetParent(hdlg);
	if (NULL == hFrame)
		return;
		
	RECT rcBox, rcTmp, rcFrame;
	if (!GetCheckboxRect(hdlg, &rcBox))
		return;
	LONG bottomLine = rcBox.bottom;
	
	GetClientRect(hFrame, &rcFrame);
	
	HWND hctrl;
	hctrl = GetDlgItem(hFrame, ctl1);
	if (NULL != hctrl && GetWindowRect(hctrl, &rcTmp))
	{		
		MapWindowPoints(HWND_DESKTOP, hFrame, (POINT*)&rcTmp, 2);
		bottomLine += (rcFrame.bottom - rcTmp.bottom);
	}
	else
		bottomLine += 6;

	GetWindowRect(hdlg, &rcTmp);
	LONG delta = -(rcTmp.bottom - rcTmp.top);


	if (bottomLine > rcFrame.bottom) 
		delta += (bottomLine - rcFrame.bottom);

	if (0 != delta)
	{
		GetWindowRect(hFrame, &rcTmp);
		SetWindowPos(hFrame, NULL, 0, 0, rcTmp.right - rcTmp.left, (rcTmp.bottom - rcTmp.top) + delta, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		if (delta > 0)
		{
			hctrl = GetDlgItem(hFrame, ctl1);
			if (NULL != hctrl && GetWindowRect(hctrl, &rcTmp))
			{		
				MapWindowPoints(HWND_DESKTOP, hFrame, (POINT*)&rcTmp, 2);
				SetWindowPos(hctrl, NULL, 0, 0, rcTmp.right - rcTmp.left, (rcBox.bottom - rcTmp.top), 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
			}

		}
	}
	
}

static void SavePlaylist_OnInit(HWND hdlg, OPENFILENAME *pof)
{
	Document *pDoc = DlgDocument(hdlg);
	
	HWND hFrame = GetParent(hdlg);
	HWND hctrl = GetDlgItem(hdlg, IDC_CHECK_REGISTERPLAYLIST);

	RecalcFrameSize(hdlg);

	if(NULL != hctrl)
	{
		HWND hDrop = DropWindowFromFrame(hFrame);
		
		BOOL registerPlaylist = (NULL != hDrop && 0 != (DBS_REGISTERPLAYLIST & GetWindowLongPtr(hDrop, GWL_STYLE)));
		SendMessage(hctrl, BM_SETCHECK, (WPARAM)((registerPlaylist) ? BST_CHECKED : BST_UNCHECKED), 0L);
		
		if (NULL != pDoc)
		{
			TCHAR szText[256];
			WASABI_API_LNGSTRINGW_BUF(IDS_REGISTER_PLAYLIST, szText, ARRAYSIZE(szText));
			SetWindowText(hctrl, szText);
			EnableWindow(hctrl, TRUE);
			ShowWindow(hctrl, SW_SHOW);
		}
	}
}

static void SavePlaylistDlg_OnInitDone(HWND hdlg, OFNOTIFY *pof)
{	
	RECT rc;
	HWND hFrame = GetParent(hdlg);
	if (NULL != hFrame &&
		GetClientRect(hFrame, &rc))
	{
		SetWindowPos(hdlg, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
	
}

static BOOL SavePlaylistDlg_QueryFileOk(HWND hdlg, OFNOTIFY *pof)
{
	Document *pDoc = DlgDocument(hdlg);

	if (NULL == pDoc)
		return TRUE;

	TCHAR szBuffer[1024], szMessage[ARRAYSIZE(szBuffer) + MAX_PATH];
	if (FAILED(pDoc->GetPath(szBuffer, ARRAYSIZE(szBuffer))))
		szBuffer[0] = TEXT('\0');

	if (0 == GetFilterIndex(pof->lpOFN->lpstrFilter, pof->lpOFN->lpstrFile))
	{
		LPCTSTR pExt = GetFilterExtension(pof->lpOFN->lpstrFilter, pof->lpOFN->nFilterIndex);
		if (NULL == pExt) pExt = TEXT("m3u");

		INT cchLen = lstrlen(pof->lpOFN->lpstrFile);
		if (0 == cchLen)
			GeneratePlaylistName(hdlg, pExt, pof->lpOFN->lpstrFile, pof->lpOFN->nMaxFile);
		else
			StringCchPrintf(&pof->lpOFN->lpstrFile[cchLen], pof->lpOFN->nMaxFile - cchLen, TEXT(".%s"), pExt);
	}


	INT compareResult = CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szBuffer, -1, pof->lpOFN->lpstrFile, -1);
	if (CSTR_EQUAL == compareResult || 0 == compareResult)
		return TRUE;

	LPCTSTR pExtension = PathFindExtension(pof->lpOFN->lpstrFile);
	if (TEXT('.') == *pExtension)
	{
		pExtension++;
		if (!CanSavePlaylistByExtension(pExtension))
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_UNKNOWN_PLAYLISTWRITER, szBuffer, ARRAYSIZE(szBuffer));
			StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szBuffer, pof->lpOFN->lpstrFile);

			HWND hFrame = GetParent(hdlg);
			GetWindowText(hFrame, szBuffer, ARRAYSIZE(szBuffer));
			MessageBoxTweak::Show(hFrame, szMessage, szBuffer, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	
	if (!PathFileExists(pof->lpOFN->lpstrFile))
		return TRUE;
		

	WASABI_API_LNGSTRINGW_BUF(IDS_REPLACE_EXISTING_FILE, szBuffer, ARRAYSIZE(szBuffer));
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szBuffer, pof->lpOFN->lpstrFile);

	HWND hFrame = GetParent(hdlg);
	GetWindowText(hFrame, szBuffer, ARRAYSIZE(szBuffer));

	INT r = MessageBoxTweak::Show(hFrame, szMessage, szBuffer, MB_YESNO | MB_ICONQUESTION);
	return (IDYES == r);
}


static void SavePlaylistDlg_OnFolderChange(HWND hdlg, OFNOTIFY *pof)
{
	if (TEXT('\0') == *pof->lpOFN->lpstrFile)
	{
		TCHAR szExtension[16];
		if (NULL != pof->lpOFN->lpstrDefExt)
		StringCchCopyN(szExtension, ARRAYSIZE(szExtension), pof->lpOFN->lpstrDefExt, 3);
		else
		{
			LPCTSTR pExt = GetFilterExtension(pof->lpOFN->lpstrFilter, pof->lpOFN->nFilterIndex);
			if (NULL == pExt) pExt = TEXT("m3u");
			StringCchCopy(szExtension, ARRAYSIZE(szExtension), pExt);
		}
		
		GeneratePlaylistName(hdlg, szExtension, pof->lpOFN->lpstrFile, pof->lpOFN->nMaxFile);
		if (TEXT('\0') != *pof->lpOFN->lpstrFile)
		{
			SetWindowText(hdlg, pof->lpOFN->lpstrFile);
			PostMessage(hdlg, CDM_UPDATEFILENAME, 0, 0L);
		}
	}
}

static void SavePlaylistDlg_OnTypeChange(HWND hdlg, OFNOTIFY *pof)
{	
	LPCTSTR pExt = GetFilterExtension(pof->lpOFN->lpstrFilter, pof->lpOFN->nFilterIndex);
	if (NULL == pExt) pExt = TEXT("m3u");
	
	HWND hFrame = GetParent(hdlg);

	if (NULL != pExt && TEXT('\0') != *pExt)
	{
		TCHAR szFile[MAX_PATH];
		SendMessage(hFrame, CDM_GETSPEC, (WPARAM)ARRAYSIZE(szFile), (LPARAM)szFile);
		
		if (0 !=GetFilterIndex(pof->lpOFN->lpstrFilter, szFile))
			PathRemoveExtension(szFile);

		INT cchLen = lstrlen(szFile);

		if (0 == cchLen)
			GeneratePlaylistName(hdlg, pExt, szFile, ARRAYSIZE(szFile));
		else
			StringCchPrintf(&szFile[cchLen], ARRAYSIZE(szFile) - cchLen, TEXT(".%s"), pExt);
	
		SendMessage(hFrame, CDM_SETCONTROLTEXT, (WPARAM)edt1, (LPARAM)PathFindFileName(szFile));
	}
	

}

static void SavePlaylistDlg_OnDestroy(HWND hdlg)
{
	Document *pDoc = DlgDocument(hdlg);
	if (NULL == pDoc)
		return;

	HWND hFrame = GetParent(hdlg);
	HWND hDrop = DropWindowFromFrame(hFrame);
	HWND hctrl = GetDlgItem(hdlg, IDC_CHECK_REGISTERPLAYLIST);
	if(NULL != hctrl && NULL != hDrop)
	{	
		DWORD windowStyle = GetWindowStyle(hDrop);
		DWORD newStyle = windowStyle & ~DBS_REGISTERPLAYLIST;
		
		if (BST_CHECKED == SendMessage(hctrl, BM_GETCHECK, 0, 0L))
			newStyle |= DBS_REGISTERPLAYLIST;

		if (newStyle != windowStyle)
			SetWindowLongPtr(hDrop, GWL_STYLE, newStyle);
	}
}

static void SavePlaylistDlg_OnWindowPosChanged(HWND hdlg, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOSIZE & pwp->flags))
	{
		RECT rcBox;
		HWND hctrl = GetDlgItem(hdlg, IDC_CHECK_REGISTERPLAYLIST);
		if (NULL != hctrl && 
			GetCheckboxRect(hdlg, &rcBox))
		{				
			SetWindowPos(hctrl, NULL, rcBox.left, rcBox.top, rcBox.right - rcBox.left, rcBox.bottom - rcBox.top,
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}
}
static UINT_PTR CALLBACK SavePlaylistDlg_HookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uiMsg)
	{
		case WM_INITDIALOG:
			SavePlaylist_OnInit(hdlg, (OPENFILENAME*)lParam);
			return TRUE;
		case WM_WINDOWPOSCHANGED:
			SavePlaylistDlg_OnWindowPosChanged(hdlg, (WINDOWPOS*)lParam);
			return TRUE;
		case WM_NOTIFY:
			switch(((NMHDR*)lParam)->code)
			{
				case CDN_INITDONE:		SavePlaylistDlg_OnInitDone(hdlg, (OFNOTIFY*)lParam); break;
				case CDN_FOLDERCHANGE:	SavePlaylistDlg_OnFolderChange(hdlg, (OFNOTIFY*)lParam); break;
				case CDN_TYPECHANGE:		SavePlaylistDlg_OnTypeChange(hdlg, (OFNOTIFY*)lParam); break;
				case CDN_FILEOK:	
					if (!SavePlaylistDlg_QueryFileOk(hdlg, (OFNOTIFY*)lParam))
					{
						SetWindowLongPtr(hdlg, DWLP_MSGRESULT, 1); 
						return TRUE;
					}
					break;
			}
			break;
		case WM_DESTROY:		SavePlaylistDlg_OnDestroy(hdlg); break;
		case CDM_UPDATEFILENAME:
			{
				TCHAR szBuffer[MAX_PATH];
				if (GetWindowText(hdlg, szBuffer, ARRAYSIZE(szBuffer)))
				{
					HWND hFrame = GetParent(hdlg);
					SendMessage(hFrame, CDM_SETCONTROLTEXT, (WPARAM)cmb13, (LPARAM)szBuffer);
					SetWindowText(hdlg, TEXT('\0'));
				}
			}
			break;
	}
	return 0;
}

static void OpenPlaylistDlg_OnInitDone(HWND hdlg, OFNOTIFY *pof)
{
	HWND hFrame = GetParent(hdlg);
	SendMessage(hFrame, CDM_HIDECONTROL, (WPARAM)chx1, 0L);
}

static UINT CALLBACK OpenPalaylistDlg_ExtParserCb(LPCTSTR pszKeyword, INT cchKeyword, LPVOID user)
{
	LPCTSTR end = pszKeyword + cchKeyword;
	
	while(pszKeyword != end && TEXT('.') != *pszKeyword) pszKeyword++;
	if (TEXT('.') == *pszKeyword) pszKeyword++;
	
	if (TEXT('*') == *pszKeyword ||
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszKeyword, (INT)(INT_PTR)(end - pszKeyword), (LPCTSTR)user, -1))
		return KWPARSER_FOUND | KWPARSER_ABORT;

	return KWPARSER_CONTINUE;
}

static BOOL OpenPlaylistDlg_QueryFileOk(HWND hdlg, OFNOTIFY *pof)
{

	HWND hFrame = GetParent(hdlg);
	TCHAR szBuffer[512], szMessage[512];

	LPCTSTR pExt = PathFindExtension(pof->lpOFN->lpstrFile);
	if (TEXT('.') != *pExt || !PathFileExists(pof->lpOFN->lpstrFile))
	{		
		if (SUCCEEDED(StringCchCopy(szBuffer,ARRAYSIZE(szBuffer), pof->lpOFN->lpstrFile)))
		{
			StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT(".*"));
			WIN32_FIND_DATA findData;
						
			LPCTSTR extList;
			
			INT filterIndex = ((0 == pof->lpOFN->nFilterIndex) ? 1 : pof->lpOFN->nFilterIndex) * 2;
			for (extList = pof->lpOFN->lpstrFilter; TEXT('\0') != *extList; extList += (lstrlen(extList) + 1), filterIndex--)
			{
				if (1 == filterIndex)
					break;
			}
			
			INT cchExtList = lstrlen(extList);

			HANDLE hFind = FindFirstFile(szBuffer, &findData);
			if (INVALID_HANDLE_VALUE != hFind)
			{
				do
				{
					if (0 == (FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes))
					{
						pExt = PathFindExtension(findData.cFileName);
						if (TEXT('.') == *pExt) pExt++;
						INT found = ParseKeywords(extList, cchExtList, TEXT(";"), TRUE, OpenPalaylistDlg_ExtParserCb, (void*)pExt);
						if (found > 0)
						{
							PathRemoveFileSpec(szBuffer);
							PathCombine(pof->lpOFN->lpstrFile, szBuffer, findData.cFileName);
							pExt = PathFindExtension(pof->lpOFN->lpstrFile);
							break;
						}
					}
				} while(FindNextFile(hFind, &findData));
				FindClose(hFind);
			}
			
		}
	}
	
	if (!PathFileExists(pof->lpOFN->lpstrFile))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_PLAYLIST_FILENOTFOUND, szBuffer, ARRAYSIZE(szBuffer));
		StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szBuffer, pof->lpOFN->lpstrFile);
		
		GetWindowText(hFrame, szBuffer, ARRAYSIZE(szBuffer));
		MessageBoxTweak::Show(hFrame, szMessage, szBuffer, MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	HANDLE hse = GetDefaultSupportedExtensionsHandle();
	if (NULL == hse)
		return TRUE;

	if (TEXT('.') == *pExt)
		pExt++;

	IItemType *type = GetTypeByExtension(hse, pExt, NULL);
	if(NULL != type && IItemType::itemTypePlaylistFile == type->GetId()) 
		return TRUE;
	
	
	WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_UNKNOWN_PLAYLISTLOADER, szBuffer, ARRAYSIZE(szBuffer));
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szBuffer, pof->lpOFN->lpstrFile);

	GetWindowText(hFrame, szBuffer, ARRAYSIZE(szBuffer));
	MessageBoxTweak::Show(hFrame, szMessage, szBuffer, MB_OK | MB_ICONERROR);
	
	return FALSE;
}

static UINT_PTR CALLBACK OpenPlaylistDlg_HookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_NOTIFY:
			switch(((NMHDR*)lParam)->code)
			{
				case CDN_INITDONE:		OpenPlaylistDlg_OnInitDone(hdlg, (OFNOTIFY*)lParam); break;
				case CDN_FILEOK:	
					if (!OpenPlaylistDlg_QueryFileOk(hdlg, (OFNOTIFY*)lParam))
					{
						SetWindowLongPtr(hdlg, DWLP_MSGRESULT, 1); 
						return TRUE;
					}
					break;
			}
			break;
	}
	return 0;
}

BOOL DropboxWindow_SavePlaylistDialog(HWND hwnd, Document *pDoc, LPTSTR pszFileName, INT cchFileNameMax)
{
	TCHAR szFilter[1024];
	OPENFILENAME of;
	ZeroMemory(&of, sizeof(OPENFILENAME));
	
	Document::GetFilterList(szFilter, ARRAYSIZE(szFilter), TRUE, &of.nFilterIndex);
			
	if (NULL != pszFileName)
	{
		if (TEXT('\0') != *pszFileName)
		{
			INT index = GetFilterIndex(szFilter, pszFileName);
			if (0 != index) 
				of.nFilterIndex = index;
		}
	}

	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hwnd;
	of.lpstrFilter = szFilter;
	of.lpstrFile = pszFileName;
	of.nMaxFile = cchFileNameMax;
	of.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	of.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_PLAYLIST_SAVEDIALOGTITLE);
	//of.lpstrDefExt = TEXT("m3u8");
	of.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_PATHMUSTEXIST | 
				OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
	of.lpfnHook = SavePlaylistDlg_HookProc;
	of.lpTemplateName = MAKEINTRESOURCE(IDD_SAVEDLGEXTENSION);
	of.hInstance = plugin.hDllInstance;
	
	return GetSaveFileName(&of);
}


BOOL DropboxWindow_OpenPlaylistDialog(HWND hwnd, Document *pDoc, LPTSTR pszFileName, INT cchFileNameMax)
{
	TCHAR szFilter[1024];
	OPENFILENAME of;
	ZeroMemory(&of, sizeof(OPENFILENAME));
	
	pszFileName[0] = TEXT('\0');
	
	Document::GetFilterList(szFilter, ARRAYSIZE(szFilter), FALSE, &of.nFilterIndex);
							
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hwnd;
	of.lpstrFilter = szFilter;
	of.lpstrFile = pszFileName;
	of.nMaxFile = cchFileNameMax;
	of.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	of.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_PLAYLIST_OPENDIALOGTITLE);
	//of.lpstrDefExt = TEXT("m3u");
	of.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_ENABLEHOOK;
	of.lpfnHook = OpenPlaylistDlg_HookProc;
	
	return GetOpenFileName(&of);
}
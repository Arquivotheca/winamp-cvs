#include "main.h"
#include "CurrentPlaylist.h"
#include "Playlist.h"
#include "../winamp/wa_ipc.h"
#include <shlobj.h>
#include <shlwapi.h>
#include "../nu/AutoWide.h"
#include "PlaylistDirectoryCallback.h"
#include "api.h"
#include <strsafe.h>

extern Playlist currentPlaylist;

bool currentPlaylist_ImportFromDisk(HWND hwnd)
{
	wchar_t oldCurPath[MAX_PATH];
	wchar_t newCurPath[MAX_PATH];

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	wchar_t temp[1024];
	wchar_t filter[1024];
	AGAVE_API_PLAYLISTMANAGER->GetFilterList(filter, 1024);
	temp[0] = 0;
	OPENFILENAMEW l = {sizeof(l), };
	l.hwndOwner = hwnd;
	l.lpstrFilter = filter;
	l.lpstrFile = temp;
	l.nMaxFile = 1023;
	l.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_IMPORT_PLAYLIST);
	l.lpstrDefExt = L"m3u";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();

	l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;

	bool ret = false;
	if (GetOpenFileNameW(&l))
	{
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		
		wchar_t titleStr[32];
		int w = currentPlaylist.GetNumItems() == 0 ? IDYES :
			MessageBox(hwnd, WASABI_API_LNGSTRINGW(IDS_APPEND_IMPORTED_PLAYLIST),
					   WASABI_API_LNGSTRINGW_BUF(IDS_LIBRARY_QUESTION,titleStr,32),
					   MB_YESNOCANCEL | MB_ICONQUESTION);

		if (w != IDCANCEL)
		{
			if (w == IDNO)
				currentPlaylist.Clear();

			AGAVE_API_PLAYLISTMANAGER->Load(temp, &currentPlaylist);
			ret = true;
		}
	}
	SetCurrentDirectoryW(oldCurPath);
	return ret;
}

bool currentPlaylist_ImportFromWinamp(HWND hwnd)
{
	wchar_t titleStr[32];
	int w = currentPlaylist.GetNumItems() == 0 ? IDNO :
	        MessageBox(hwnd, WASABI_API_LNGSTRINGW(IDS_APPEND_ACTIVE_PLAYLIST),
					   WASABI_API_LNGSTRINGW_BUF(IDS_LIBRARY_QUESTION,titleStr,32),
					   MB_YESNOCANCEL | MB_ICONQUESTION);

	if (w != IDCANCEL)
	{
		if (w == IDNO)
			currentPlaylist.Clear();

		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITEPLAYLIST);
		wchar_t *m3udir = (wchar_t *) SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW);
		wchar_t s[MAX_PATH];
		PathCombineW(s, m3udir, L"winamp.m3u8");

		AGAVE_API_PLAYLISTMANAGER->Load(s, &currentPlaylist);
		return true;
	}
	return false;
}

bool CurrentPlaylist_DeleteMissing()
{
	bool ret = false;
	size_t x = currentPlaylist.GetNumItems();
	while (x--)
	{
		wchar_t fn[1024];
		currentPlaylist.GetItem(x, fn, 1024);
		if (!wcsstr(fn, L"://") && !wcsstr(fn, L":\\\\") && !(PathFileExistsW(fn)))
		{
			currentPlaylist.Remove(x);
			ret = true;
		}
	}
	return ret;
}

void CurrentPlaylist_Export(HWND dlgparent)
{
	wchar_t oldCurPath[MAX_PATH],
			newCurPath[MAX_PATH],
			filterStr[MAX_PATH] = {0},
			*fStr = filterStr;

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	wchar_t temp[MAX_PATH] = {0};
	OPENFILENAMEW l = {sizeof(OPENFILENAMEW), 0};
	lstrcpynW(temp, (wchar_t*)GetPropW(dlgparent, L"TITLE"), MAX_PATH);
	l.hwndOwner = dlgparent;
	l.hInstance = plugin.hDllInstance;
	l.nFilterIndex = g_config->ReadInt("filter", 3);

	l.lpstrFilter = WASABI_API_LNGSTRINGW_BUF(IDS_PLAYLIST_FILTER_STRING,filterStr,MAX_PATH);
	// store the filter string with | separators so that parts can be translated as needed
	// (\0 or more \0000 will work but it's not reliable to work with that - the pipe is a
	// a work around from mfc apps wanting to do what we want for language support)
	while(fStr && *fStr)
	{
		wchar_t* rfsStr = 0;
		if(*fStr == L'|')
		{
			rfsStr = fStr;
		}
		fStr = CharNextW(fStr);
		if(rfsStr)
		{
			*rfsStr = 0;
		}
	}
	l.lpstrFile = temp;
	l.nMaxFile = MAX_PATH;
	l.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_EXPORT_PLAYLIST);
	l.lpstrDefExt = L"m3u";
	l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	if (GetSaveFileNameW(&l))
	{
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);
		AGAVE_API_PLAYLISTMANAGER->Save(temp, &currentPlaylist);
	}
	g_config->WriteInt("filter", l.nFilterIndex);
	SetCurrentDirectoryW(oldCurPath);
}

bool CurrentPlaylist_AddLocation(HWND hwndDlg)
{
	bool ret = false;
	char *p = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)hwndDlg, IPC_OPENURLBOX);
	if (p)
	{
		size_t s = currentPlaylist.GetNumItems();
		AutoWide wideFn(p);
		if (AGAVE_API_PLAYLISTMANAGER->Load(wideFn, &currentPlaylist) != PLAYLISTMANAGER_SUCCESS)
		{
			wchar_t title[400];
			int length;
			mediaLibrary.GetFileInfo(wideFn, title, 400, &length);
			currentPlaylist.AppendWithInfo(wideFn, title, length*1000);
		}
		ret = true;

		// TODO: if (GetPrivateProfileInt("winamp", "rofiob", 1, WINAMP_INI)&1) PlayList_sort(2, s);

		GlobalFree((HGLOBAL)p);
	}
	return ret;
}

static INT_PTR CALLBACK browseCheckBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		int rofiob = GetPrivateProfileIntA("winamp", "rofiob", 1, mediaLibrary.GetWinampIni());
		if (!(rofiob & 2))
			CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);
	}
	if (uMsg == WM_COMMAND)
	{
		if (LOWORD(wParam) == IDC_CHECK1)
		{
			int rofiob = GetPrivateProfileIntA("winamp", "rofiob", 1, mediaLibrary.GetWinampIni());
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1))
				rofiob &= ~2;
			else
				rofiob |= 2;

			char blah[32];
			StringCchPrintfA(blah, 32, "%d", rofiob);
			WritePrivateProfileStringA("winamp", "rofiob", blah, mediaLibrary.GetWinampIni());
		}
	}
	return 0;
}

static int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			SetWindowTextW(hwnd, WASABI_API_LNGSTRINGW(IDS_ADD_DIR_TO_PLAYLIST));
			SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)WASABI_API_APP->path_getWorkingPath());

			HWND h2 = FindWindowEx(hwnd, NULL, NULL, L"__foo2");
			if (h2) ShowWindow(h2, SW_HIDE);

			HWND h = WASABI_API_CREATEDIALOGW(IDD_BROWSE_PLFLD, hwnd, browseCheckBoxProc);
			SetWindowPos(h, 0, 4, 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			ShowWindow(h, SW_SHOWNA);
		}
	}
	return 0;
}

bool CurrentPlaylist_AddDirectory(HWND hwndDlg)
{
	BROWSEINFOW bi;
	ITEMIDLIST *idlist;
	wchar_t name[MAX_PATH];
	bi.hwndOwner = hwndDlg;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;
	bi.lpszTitle = L"__foo2";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = 0;
	idlist = SHBrowseForFolderW(&bi);
	if (idlist)
	{
		size_t s = currentPlaylist.GetNumItems();
		wchar_t path[MAX_PATH];
		SHGetPathFromIDListW(idlist, path);
		WASABI_API_APP->path_setWorkingPath(path);
		extern void Shell_Free(void *p);
		Shell_Free(idlist);
		WASABI_API_APP->path_setWorkingPath(path);

		PlaylistDirectoryCallback dirCallback(mediaLibrary.GetExtensionList(), mediaLibrary.GetWinampIni());

		AGAVE_API_PLAYLISTMANAGER->LoadDirectory(path, &currentPlaylist, &dirCallback);

		//int rofiob = GetPrivateProfileInt("winamp", "rofiob", 1, WINAMP_INI);
		// TODO: if (rofiob&1) PlayList_sort(2, s);
		return true;
	}
	return false;
}

bool CurrentPlaylist_AddFiles(HWND hwndDlg)
{
	wchar_t oldCurPath[MAX_PATH];
	wchar_t newCurPath[MAX_PATH];

	GetCurrentDirectoryW(MAX_PATH, oldCurPath);

	const int len = 256 * 1024 - 128;
	wchar_t *temp;
	OPENFILENAMEW l = {sizeof(l),};
	static int q;
	if (q) return false;
	q = 1;
	temp = (wchar_t *)GlobalAlloc(GPTR, sizeof(wchar_t) * len);
	l.hwndOwner = hwndDlg;
	wchar_t *fsb = (wchar_t *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_GET_EXTLISTW);

	l.lpstrFilter = fsb;
	l.lpstrFile = temp;
	l.nMaxFile = len - 1;
	l.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_ADD_FILES_TO_PLAYLIST);
	l.lpstrDefExt = L"";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();

	l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

	bool ret = false;
	if (GetOpenFileNameW(&l))
	{
		GetCurrentDirectoryW(MAX_PATH, newCurPath);
		WASABI_API_APP->path_setWorkingPath(newCurPath);

		wchar_t *p = temp;
		size_t sp = currentPlaylist.GetNumItems();
		if (temp[lstrlenW(temp) + 1])
		{
			AGAVE_API_PLAYLISTMANAGER->LoadFromDialog(temp, &currentPlaylist);
			ret = true;
			// TODO: if (GetPrivateProfileInt("winamp", "rofiob", 1, WINAMP_INI)&1) PlayList_sort(2, sp);
		}
		else
		{
			if (AGAVE_API_PLAYLISTMANAGER->Load(temp, &currentPlaylist) != PLAYLISTMANAGER_SUCCESS)
			{
				wchar_t title[400];
				int length;
				mediaLibrary.GetFileInfo(temp, title, 400, &length);
				currentPlaylist.AppendWithInfo(temp, title, length*1000);
				ret = true;
			}
		}
	}
	SetCurrentDirectoryW(oldCurPath);
	GlobalFree(fsb);
	GlobalFree(temp);
	q = 0;
	return ret;
}
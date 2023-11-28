#include "main.h"
#include "../nu/AutoWide.h"
#define LANG_STATIC_BUFFER_SIZE 1024

extern "C" void ResolveEnvironmentVariables2(wchar_t *string, wchar_t *destString, size_t stringSize);
static UINT WM_TASKBARCREATED;

HINSTANCE g_hInstance;

// winamp2/5
wchar_t ini_file[MAX_PATH];
wchar_t wa2ConfigDir[MAX_PATH];
wchar_t winampClassName[MAX_PATH];
wchar_t winampaLngPath[MAX_PATH];
wchar_t icon_tmp[MAX_PATH];

// whatever we decided on
wchar_t winamp_exe_file[MAX_PATH];
wchar_t bm_file[MAX_PATH];

static HINSTANCE winampaLng, nativeLng;
static HICON m_icon;

typedef HRESULT(WINAPI *CHANGEWINDOWMESSAGEFILTER)(UINT message, DWORD dwFlag);
static CHANGEWINDOWMESSAGEFILTER changeWMFilter;

int config_iconidx = -1;
int config_systray_icon = 1;

static wchar_t ini_sec[] = L"WinampAgent"; 

int ReadStr(HANDLE hFile, char *str, int len)
{
	while (1)
	{
		DWORD l;
		ReadFile(hFile, str, 1, &l, 0);
		if (l != 1 || *str == '\r' || *str == '\n')
		{
			DWORD t;
			ReadFile(hFile, str, 1, &t, 0);
			*str = 0;
			return (l == 1);
		}
		str++;
		if (--len < 1)
		{
			*str = 0;
			return 1;
		}
	}
}

static BOOL LoadWMFilter(void){
	if (!changeWMFilter){
		changeWMFilter = (CHANGEWINDOWMESSAGEFILTER)GetProcAddress(GetModuleHandle("USER32"), "ChangeWindowMessageFilter");
	}
	return (!!changeWMFilter);
}

void LoadWinampaLng(void){
	winampaLng = LoadLibraryExW(winampaLngPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
}

void UnloadWinampaLng(void){
	if(winampaLng){
		FreeLibrary(winampaLng);
		winampaLng = 0;
	}
}

wchar_t* GetStringW(UINT uID)
{
	static wchar_t *buf;
	if (!buf) 	
		buf = (wchar_t *)GlobalAlloc(LPTR,(LANG_STATIC_BUFFER_SIZE*sizeof(buf[0])));

	if (!LoadStringW(winampaLng, uID, buf, LANG_STATIC_BUFFER_SIZE))
	{
		if (winampaLng == nativeLng || !LoadStringW(nativeLng, uID, buf, LANG_STATIC_BUFFER_SIZE))
		{
			lstrcpynW(buf, L"Error loading string", LANG_STATIC_BUFFER_SIZE);
		}
	}
	return buf;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int force_icon = 0;
	if (WM_TASKBARCREATED && uMsg == WM_TASKBARCREATED)
	{
		uMsg = WM_USER + 1;
		force_icon = 1;
	}

	switch (uMsg)
	{
		case WM_CREATE:
			SendMessage(hwnd, WM_USER + 1, 0, 0);
			return TRUE;

		case WM_USER + 1:
		{
			int iconidx;
			int isintray;

			config_systray_icon = ini_file[0] ? GetPrivateProfileIntW(ini_sec, L"is_intray", 1, ini_file) : 0;
			iconidx = ini_file[0] ? GetPrivateProfileIntW(L"Winamp", L"sticon", 0, ini_file) : 0;

			isintray = !!systray_isintray();

			if ((isintray && (force_icon || iconidx != config_iconidx)) ||
				 isintray != (config_systray_icon))
			{
				HICON m_oldicon = m_icon;
				m_icon = 0;
				if (config_systray_icon)
				{
					if (iconidx != 0)
					{
						HMODULE h = LoadLibraryExW(winamp_exe_file, NULL, LOAD_LIBRARY_AS_DATAFILE);
						if (h)
						{
							int geticonid(int x); // in winampicon.cpp

							int icon_to_use = geticonid(iconidx);
							if(icon_to_use != -666)
							{
								m_icon = (HICON)LoadImage(h,MAKEINTRESOURCE(icon_to_use),IMAGE_ICON,16,16,0);
							}
							else
							{
								if(PathFileExistsW(icon_tmp))
								{
									m_icon = (HICON)LoadImageW(0,icon_tmp,IMAGE_ICON,16,16,LR_LOADFROMFILE);
								}
							}
							FreeLibrary(h);
						}
					}
					if (!m_icon) m_icon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
					if (isintray)
						systray_mod(hwnd, m_icon, 0);
					systray_add(hwnd, m_icon, GetStringW(IDS_WINAMP_AGENT));
				}
				else systray_del(hwnd);

				if (m_oldicon) DestroyIcon(m_oldicon);
			}
			config_iconidx = iconidx;
		}
			return 0;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;

		case WM_ENDSESSION:  // JF JAN01001 added
			if (wParam)
			{
				ExitProcess(0);
				return 0;
			}
			break;

		case WM_USER + 8:
			if (LOWORD(lParam) == WM_MOUSEMOVE)
			{
				static DWORD last_t;
				HWND hwnd2;
				if (GetTickCount() - last_t > 250)
				{
					last_t = GetTickCount();
					if ((hwnd2 = FindWindowW(winampClassName, NULL)))
					{
						wchar_t buf[64];
						GetWindowTextW(hwnd2, buf, 64);
						systray_mod(hwnd, 0, buf);
					}
					else
					{
						systray_mod(hwnd, 0, GetStringW(IDS_WINAMP_AGENT));
					}
				}
			}

			if (LOWORD(lParam) == WM_LBUTTONUP)
			{
				if(!(GetAsyncKeyState(VK_SHIFT)&0x8000))
				{
					HWND hwnd2;
					if ((hwnd2 = FindWindowW(winampClassName, NULL)))
					{
						SetForegroundWindow(hwnd2);
						SendMessage(hwnd2, WM_USER + 1, 0, WM_LBUTTONUP);
					}
					else if ((hwnd2 = FindWindow("STUDIO", NULL)))
					{
						SetForegroundWindow(hwnd2);
					}
					else
					{
						ShellExecuteW(NULL, L"open", winamp_exe_file, L"", L".", SW_SHOW );
					}
				}
				else
				{
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}

			if (LOWORD(lParam) == WM_RBUTTONUP)
			{
				HWND hwnd2;
				if ((hwnd2 = FindWindowW(winampClassName, NULL)) && !(GetAsyncKeyState(VK_CONTROL)&0x8000))
				{
					SetForegroundWindow(hwnd2);
					SendMessage(hwnd2, WM_USER + 1, 0, WM_RBUTTONUP);
				}
				else
				{
					HMENU hMenu = CreatePopupMenu();
					MENUITEMINFOW i = {sizeof(i), };
					int x;
					// for bookmarks menu
					int num_bookmarks = 0;
					// for audio cd entries
					wchar_t g_audiocdletter[4] = {0};
					int g_audiocdletters = 0;
					int drivemask = 0;
					DWORD drives = GetLogicalDrives();

					char fn[1024];
					char ft[1024];
					POINT p;
					HANDLE hFile;
					HMENU sm;
					fn[0] = ft[0] = 0;
					GetCursorPos(&p);
					i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
					i.fType = MFT_STRING;
					i.wID = 1;
					i.dwTypeData = GetStringW(IDS_OPEN_WINAMP);
					i.cch = lstrlenW((wchar_t*)i.dwTypeData);
					InsertMenuItemW(hMenu, 0, TRUE, &i);
					i.wID = 0;
					i.fType = MFT_SEPARATOR;
					InsertMenuItemW(hMenu, 1, TRUE, &i);

					i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
					i.fType = MFT_STRING;
					i.wID = 2;
					i.dwTypeData = GetStringW(IDS_DISABLE_WINAMP_AGENT);
					i.cch = lstrlenW((wchar_t*)i.dwTypeData);
					InsertMenuItemW(hMenu, 2, TRUE, &i);
					i.wID = 3;
					i.dwTypeData = GetStringW(IDS_CLOSE_WINAMP_AGENT);
					i.cch = lstrlenW((wchar_t*)i.dwTypeData);
					InsertMenuItemW(hMenu, 3, TRUE, &i);

					SetMenuDefaultItem(hMenu,!(GetAsyncKeyState(VK_SHIFT)&0x8000)?0:3,1);

					i.wID = 0;
					i.fType = MFT_SEPARATOR;
					InsertMenuItemW(hMenu, 4, TRUE, &i);

					i.wID = 10;
					for (drivemask = 0; drivemask < 32; drivemask++)
					{
						if (drives&(1 << drivemask))
						{
							wchar_t str[256];
							StringCchPrintfW(str, 256, L"%c:\\", 'A' + drivemask);
							if (GetDriveTypeW(str) == DRIVE_CDROM)
							{
								int old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
								DWORD system_flags = 0, max_file_len = 0;
								wchar_t drives[4] = {L" :\\"}, c = L'A' + drivemask, vol_buf[40] = {0}, empty[64];
								drives[0] = g_audiocdletter[g_audiocdletters] = c;
								
								GetVolumeInformationW(drives,vol_buf,sizeof(vol_buf),0,&max_file_len,&system_flags,0,0);
								SetErrorMode(old_error_mode);

								lstrcpynW(empty,GetStringW(IDS_EMPTY),64);
								StringCchPrintfW(str, 256, GetStringW(IDS_AUDIO_CD),c,(vol_buf[0]?vol_buf:empty));
								i.fType = MFT_STRING;
								i.dwTypeData = str;
								i.cch = wcslen(str);
								InsertMenuItemW(hMenu, 5 + g_audiocdletters, TRUE, &i);
								i.wID++;
								g_audiocdletters++;
								if (g_audiocdletters == 4) break;
							}
						}
					}

					if(g_audiocdletters)
					{
						i.wID = 0;
						i.fType = MFT_SEPARATOR;
						InsertMenuItemW(hMenu, 5 + g_audiocdletters, TRUE, &i);
					}

					i.fType = MFT_STRING;
					i.dwTypeData = GetStringW(IDS_BOOKMARKS);
					i.cch = lstrlenW((wchar_t*)i.dwTypeData);
					sm = i.hSubMenu = CreatePopupMenu();
					i.fMask |= MIIM_SUBMENU;
					i.wID = 0;
					InsertMenuItemW(hMenu, 6 + g_audiocdletters, TRUE, &i);

					// have to keep this ansi since winamp.bm doesn't support unicode
					hFile = CreateFileW(bm_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						MENUITEMINFOW ib = {sizeof(i), };
						ib.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
						ib.fType = MFT_STRING;
						i.wID = ib.wID = 20;
						while (1)
						{
							if (!ReadStr(hFile, fn, MAX_PATH)) break;
							if (!ReadStr(hFile, ft, 2048)) break;
							ib.dwTypeData = AutoWideDup(ft, CP_UTF8);
							ib.cch = lstrlenW(ib.dwTypeData);
							InsertMenuItemW(sm, num_bookmarks, TRUE, &ib);
							ib.wID++;
							i.wID++;
							num_bookmarks++;
						}
					}

					if(i.wID == 20 || !i.wID)
					{
						i.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
						i.fType = MFT_STRING;
						i.dwTypeData = GetStringW(IDS_NO_BOOKMARKS_FOUND);
						i.cch = lstrlenW((wchar_t*)i.dwTypeData);
						InsertMenuItemW(sm, num_bookmarks, TRUE, &i);
						EnableMenuItem(sm, i.wID, MF_BYCOMMAND | MF_GRAYED);
					}

					SetForegroundWindow(hwnd);
					x = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY, p.x, p.y, 0, hwnd, NULL);
					if (x == 1)
					{
						ShellExecuteW(NULL, L"open", winamp_exe_file, L"", L".", SW_SHOW);
					}
					else if (x == 2 || x == 3)
					{
						if (x == 2)  // disable
						{
							HKEY key;
							if (RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
							{
								RegDeleteValueW(key, L"WinampAgent");
								RegCloseKey(key);
							}
						}
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
					else if(x >= 10 && x < 10 + g_audiocdletters)
					{
						wchar_t ftW[1024];
						StringCchPrintfW(ftW, 1024, L"\"cda://%c\"", g_audiocdletter[x - 10]);
						ShellExecuteW(NULL, L"open", winamp_exe_file, ftW, L".", SW_SHOW );
					}
					else if (x >= 20 && x < 20 + num_bookmarks && hFile != INVALID_HANDLE_VALUE)
					{
						int r;
						x -= 20;
						SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
						for (r = 0; r <= x; r ++)
						{
							if (!ReadStr(hFile, fn, MAX_PATH)) break;
							if (!ReadStr(hFile, ft, 2048)) break;
						}
						if (r == (x + 1))
						{
							wchar_t ftW[1024];
							StringCchPrintfW(ftW, 1024, L"\"%s\"", AutoWide(fn, CP_UTF8));
							ShellExecuteW(NULL, L"open", winamp_exe_file, ftW, L".", SW_SHOW );
						}
					}
					DestroyMenu(hMenu);
					if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
				}
			}
			return 0;

		case WM_USER + 16:
			// do this on load/unload requests just incase something went wrong
			UnloadWinampaLng();
			if(!wParam) LoadWinampaLng();
			return 0;

		case WM_DESTROY:
			if (systray_isintray()) systray_del(hwnd);
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static char szClassName[] = "WinampAgentMain";
static char szErrorTitle[] = "Winamp Agent Error";

void queryPath(wchar_t *out, wchar_t *in, int out_len)
{
	wchar_t buf[MAX_PATH];
	HKEY key;
	if (RegOpenKeyW(HKEY_CLASSES_ROOT, in, &key) == ERROR_SUCCESS)
	{
		DWORD s = sizeof(buf);
		if (RegQueryValueExW(key, NULL, 0, NULL, (LPBYTE)buf, &s) == ERROR_SUCCESS)
		{
			if (buf[0] == L'\"')
			{
				wchar_t *p = buf + 1;
				while (*p != L'\"' && *p) p=CharNextW(p);
				if (*p) *p = 0;
				while (p > buf && *p != L'\\') p = CharPrevW(buf,p);
				if (*p) *p = 0;
				lstrcpynW(out, buf + 1, out_len);
			}
			else
			{
				wchar_t *p = buf;
				while (*p) p=CharNextW(p);
				while (p > buf && *p != L'\\') p = CharPrevW(buf,p);
				if (*p) *p = 0;
				lstrcpynW(out, buf, out_len);
			}
		}
		RegCloseKey(key);
	}
}

void BuildDirectories()
{
	// get ini_file from reg
	wchar_t winamp2Folder[MAX_PATH];
	wchar_t pathsIni[MAX_PATH];
	wchar_t iniFileName[MAX_PATH];

	winamp2Folder[0] = 0;
	wa2ConfigDir[0] = 0;
	winampClassName[0] = 0;
	ini_file[0] = 0;
	winamp_exe_file[0] = 0;
	bm_file[0] = 0;
	icon_tmp[0] = 0;

	// attempt to get the winamp folder from the play then the enqueue and then just revert to current folder (wa2/5)
	queryPath(winamp2Folder, L"Winamp.File\\shell\\play\\command", MAX_PATH);
	if(!winamp2Folder[0]) queryPath(winamp2Folder, L"Winamp.File\\shell\\enqueue\\command", MAX_PATH);
	if(!winamp2Folder[0])
	{
		wchar_t buf[MAX_PATH], *p = buf;
		GetModuleFileNameW(GetModuleHandleW(NULL), buf, sizeof(buf));
		while (*p) p=CharNextW(p);
		while (p > buf && *p != L'\\') p=CharPrevW(buf,p);
		if (*p) *p = 0;
		lstrcpynW(winamp2Folder, buf, sizeof(winamp2Folder));
	}

	if (winamp2Folder[0]) // wa2/5
	{
		wchar_t profileString[MAX_PATH];
		StringCchPrintfW(pathsIni, MAX_PATH, L"%s\\paths.ini", winamp2Folder);

		GetPrivateProfileStringW(L"Winamp", L"inidir", L"", profileString, MAX_PATH, pathsIni);
		if (profileString[0])
			ResolveEnvironmentVariables2(profileString, wa2ConfigDir, MAX_PATH);
		else
			lstrcpynW(wa2ConfigDir, winamp2Folder, MAX_PATH);

		GetPrivateProfileStringW(L"Winamp", L"class", L"", profileString, MAX_PATH, pathsIni);
		if (profileString[0])
			ResolveEnvironmentVariables2(profileString, winampClassName, MAX_PATH);

		GetPrivateProfileStringW(L"Winamp", L"inifile", L"", profileString, MAX_PATH, pathsIni);
		if (profileString[0])
			ResolveEnvironmentVariables2(profileString, iniFileName, MAX_PATH);
		else
			lstrcpynW(iniFileName, L"winamp.ini", MAX_PATH);

		StringCchPrintfW(ini_file, MAX_PATH, L"%s\\%s", wa2ConfigDir, iniFileName);

		// winamp.exe should extract this out for us when a new wlz is loaded so we
		// don't have to bloat up winampa - just have to deal with wlz changes instead
		StringCchPrintfW(winampaLngPath, MAX_PATH, L"%s\\winampa.lng", wa2ConfigDir);
		StringCchPrintfW(icon_tmp, MAX_PATH, L"%s\\winamp.ico", wa2ConfigDir);

		StringCchPrintfW(winamp_exe_file, MAX_PATH, L"%s\\winamp.exe", winamp2Folder);
		StringCchPrintfW(bm_file, MAX_PATH, L"%s\\winamp.bm8", wa2ConfigDir);
		// just make sure if a winamp.bm8 doesn't exist then
		// go make one from winamp.bm - implemented for 5.58+
		if(!PathFileExistsW(bm_file))
		{
			wchar_t tmp[MAX_PATH];
			StringCchPrintfW(tmp, MAX_PATH, L"%s\\winamp.bm", wa2ConfigDir);
			CopyFileW(tmp,bm_file,FALSE);
		}
	}

	if (!winampClassName[0])
		lstrcpynW(winampClassName, L"Winamp v1.x", MAX_PATH);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg = {0};
	static WNDCLASS wc;

	if (FindWindow(szClassName, NULL))
	{
		ExitProcess(0);
	}

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

	// add in a UIPI filter so we can get required notifications from winamp.exe such
	// as when we're started from an elevation request e.g. via prefs dialog options.
	if(LoadWMFilter()){
		changeWMFilter(WM_USER+1, 1/*MSGFLT_ADD*/);
		changeWMFilter(WM_USER+16, 1/*MSGFLT_ADD*/);
	}

	wc.lpfnWndProc = WndProc;
	g_hInstance = wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = szClassName;

	BuildDirectories();

	// attempt to load winampa.lng if present (if extracted from the current wlz if there is one)
	nativeLng = hInstance;
	LoadWinampaLng();

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "Cannot register window class!", szErrorTitle, MB_OK | MB_ICONSTOP);
		return 0;
	}

	if (!(CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, szClassName, "", 0,
	                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	                     NULL, NULL, g_hInstance, NULL)))
	{
		MessageBox(NULL, "Cannot create window!", szErrorTitle, MB_OK | MB_ICONSTOP);
		return 0;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
	} // while(GetMessage...

	UnloadWinampaLng();
	ExitProcess(0);
	return 0;
}

#ifdef DO_LOG
void do_log_print(char *p)
{
	HANDLE h = CreateFile("C:\\winampa.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD l;
		SetFilePointer(h, 0, NULL, FILE_END);
		WriteFile(h, p, lstrlen(p), &l, NULL);
		CloseHandle(h);
	}
}
#endif

char *scanstr_back(char *str, char *toscan, char *defval)
{
	char *s = str + lstrlen(str) - 1;
	if (lstrlen(str) < 1) return defval;
	if (lstrlen(toscan) < 1) return defval;
	while (1)
	{
		char *t = toscan;
		while (*t)
			if (*t++ == *s) return s;
		t = CharPrev(str, s);
		if (t == s) return defval;
		s = t;
	}
}

char *extension(char *fn)
{
	char *s = scanstr_back(fn, ".\\", fn - 1);
	if (s < fn) return "";
	if (*s == '\\') return "";
	return (s + 1);
}
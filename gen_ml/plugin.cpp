#include "main.h"
#include "ml.h"
#include "itemlist.h"
#include "gen.h"
#include "config.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "resource.h"
#include "comboskin.h"
#include "../winamp/wa_dlg.h"
#include "childwnd.h"
#include "sendto.h"
#include "api.h"
#include "../nu/autowide.h"
#include "../Elevator/IFileTypeRegistrar.h"
#include <time.h>
#include <shlwapi.h>
#include <strsafe.h>

extern winampGeneralPurposePlugin plugin;

C_ItemList m_plugins;

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id){
HWND ctrl = GetDlgItem(hwndDlg, id);
	SetPropW(ctrl, L"link_proc",
			(HANDLE)(LONG_PTR)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONGX86)(LONG_PTR)link_handlecursor));
}

void link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DRAWITEM)
	{
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON)
		{
			wchar_t wt[123];
			int y;
			RECT r;
			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, (INT)wParam, wt, 123);

			// draw text
			SetTextColor(di->hDC, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			r = di->rcItem;
			r.left += 2;
			DrawTextW(di->hDC, wt, -1, &r, DT_VCENTER | DT_SINGLELINE);

			memset(&r, 0, sizeof(r));
			DrawTextW(di->hDC, wt, -1, &r, DT_SINGLELINE | DT_CALCRECT);

			// draw underline
			y = di->rcItem.bottom - ((di->rcItem.bottom - di->rcItem.top) - (r.bottom - r.top)) / 2 - 1;
			hPen = CreatePen(PS_SOLID, 0, (di->itemState & ODS_SELECTED) ? RGB(220, 0, 0) : RGB(0, 0, 220));
			hOldPen = (HPEN) SelectObject(di->hDC, hPen);
			MoveToEx(di->hDC, di->rcItem.left + 2, y, NULL);
			LineTo(di->hDC, di->rcItem.right + 2 - ((di->rcItem.right - di->rcItem.left) - (r.right - r.left)), y);
			SelectObject(di->hDC, hOldPen);
			DeleteObject(hPen);

		}
	}
}

int inline PluginColumnWidth(HWND hwndDlg, HWND control, const wchar_t *str, int width){
SIZE size;
HDC hdc = GetDC(control);
// get and select parent dialog's font so that it'll calculate things correctly
HFONT font = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0), oldfont = (HFONT)SelectObject(hdc, font);
	GetTextExtentPoint32W(hdc, str, lstrlenW(str)+1, &size);
	SelectObject(hdc, oldfont);
	ReleaseDC(control, hdc);
	return size.cx;
}

/* In Winamp's preferences, Plugins->Media Library  */
INT_PTR CALLBACK PluginsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		extern BOOL init2();
		if (!g_hwnd) init2();
		int x;
		WIN32_FIND_DATA d;
		HANDLE h;
		char dirstr[MAX_PATH];
		HWND listWindow;
		
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if (NULL != listWindow)
		{
			int tabs[] = {160};
			SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

			wchar_t filename[MAX_PATH], description[512];
			for (x = 0; x < m_plugins.GetSize(); x ++)
			{
				winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(x);
				
				if (mlplugin)
				{					
					if(mlplugin->hDllInstance)
					{
						GetModuleFileNameW(mlplugin->hDllInstance, filename,  ARRAYSIZE(filename));
						PathStripPathW(filename);
						if (mlplugin->version == MLHDR_VER)
							StringCchPrintfW(description, ARRAYSIZE(description), L"%s\t%s", 
											 mlplugin->description, filename);
						else
							StringCchPrintfW(description, ARRAYSIZE(description), L"%s\t%s", 
											 AutoWide(mlplugin->description), filename);

						tabs[0] = PluginColumnWidth(hwndDlg, listWindow, filename, tabs[0]);
					}
					else
					{
						if (mlplugin->version == MLHDR_VER)
							StringCchCopyW(description, ARRAYSIZE(description), (wchar_t*)mlplugin->description);
						else
							StringCchCopyW(description, ARRAYSIZE(description), AutoWide(mlplugin->description));
					}

					int pos = SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM)description);
					SendMessageW(listWindow, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)x);
				}
			}

			// convert and slightly padded to character units
			RECT r;
			GetWindowRect(listWindow, &r);
			tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
			tabs[0] /= 10;
			tabs[0] *= 10;
			SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

			PathCombine(dirstr, pluginPath, "ML_*.DLL");
			h = FindFirstFile(dirstr, &d);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					PathCombine(dirstr, pluginPath, d.cFileName);
					HMODULE b = LoadLibraryEx(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
					for (x = 0; b && (x != m_plugins.GetSize()); x ++)
					{
						winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(x);
						if (mlplugin->hDllInstance == b)
						{
							break;
						}
					}

					if (x == m_plugins.GetSize() || !b)
					{
						StringCchPrintfW(description, MAX_PATH, WASABI_API_LNGSTRINGW(IDS_NOT_LOADED), d.cFileName);
						x = SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM) description);
						SendMessageW(listWindow, LB_SETITEMDATA, (WPARAM)x, (LPARAM)-2);
					}
					FreeLibrary(b);
				}
				while (FindNextFile(h, &d));
				FindClose(h);
			}

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow;
		listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if (NULL != listWindow && NULL != WASABI_API_APP)
			WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND) 
	{
		switch (LOWORD(wParam))
		{
			case IDC_UNINST:
				{
					int which = (INT)SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0), which_sel = which;
					if (which != LB_ERR)
					{
						winampMediaLibraryPlugin *mlplugin;
						wchar_t title[32];
						int msgBox = MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UNINSTALL_PROMPT),
												 WASABI_API_LNGSTRINGW_BUF(IDS_UINSTALL_CONFIRMATION,title,32),
												 MB_YESNO | MB_ICONEXCLAMATION);

						which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, (WPARAM)which, 0);
						if (which >= 0 && which < m_plugins.GetSize() && (mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(which)) && msgBox == IDYES)
						{
							int ret = 0;
							int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

							*(void**)&pr = (void*)GetProcAddress(mlplugin->hDllInstance, "winampUninstallPlugin");
							if (pr)ret = pr(mlplugin->hDllInstance, hwndDlg, 0);
							// ok to uninstall but do with a full restart (default/needed in subclassing cases)
							if (ret == ML_PLUGIN_UNINSTALL_REBOOT)
							{
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(mlplugin->hDllInstance, buf, MAX_PATH);
								WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
								WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
								PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
							}
							// added from 5.37+ so we can do true on-the-fly removals (will fall back to default if fails)
							else if (ret == ML_PLUGIN_UNINSTALL_NOW)
							{
								// get the filename before we free the dll otherwise things may go boom
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(mlplugin->hDllInstance, buf, MAX_PATH);

								mlplugin->quit();
								//if (mlplugin->hDllInstance) FreeLibrary(mlplugin->hDllInstance);
								m_plugins.Del(which);

								// try to use the elevator to do this
								IFileTypeRegistrar *registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_FILEREGISTRAR_OBJECT);
								if(registrar && (registrar != (IFileTypeRegistrar*)1)) {
									if(registrar->DeleteItem(buf) != S_OK){
										WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
										WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
										PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
									}
									else
										SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_DELETESTRING, which_sel, 0);
									registrar->Release();
								}
							}
						}
						// will cope with not loaded plug-ins so we can still remove them, etc
						else if (which == -2 && msgBox == IDYES)
						{
							wchar_t buf[1024], base[1024];
							GetModuleFileNameW(plugin.hDllInstance, base, sizeof(base)/sizeof(wchar_t));
							SendDlgItemMessageW(hwndDlg, IDC_GENLIB, LB_GETTEXT, which_sel, (LPARAM)buf);
							wchar_t *p = wcschr(buf, L'.');
							if (p && *p == L'.')
							{
								p += 4;
								*p = 0;
								PathRemoveFileSpecW(base);
								PathAppendW(base, buf);
							}

							// try to use the elevator to do this
							IFileTypeRegistrar *registrar = (IFileTypeRegistrar*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_FILEREGISTRAR_OBJECT);
							if(registrar && (registrar != (IFileTypeRegistrar*)1)) {
								if(registrar->DeleteItem(base) != S_OK){
									WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
									WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
									PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
								}
								else
									SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_DELETESTRING, which_sel, 0);
								registrar->Release();
							}
							// otherwise revert to a standard method
							else {
								WritePrivateProfileStringW(L"winamp", L"remove_genplug", buf, WINAMP_INI);
								WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", WINAMP_INI);
								PostMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_RESTARTWINAMP);
							}
						}

						// fakes a selection and resets the focus to the listbox so it'll keep ui response working
						SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_GENLIB, LBN_SELCHANGE), 0);
						SetFocus(GetDlgItem(hwndDlg, IDC_GENLIB));
					}
				}
				return FALSE;
			case IDC_GENLIB:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0);
					if (which != LB_ERR)
					{
						which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, (WPARAM)which, 0);
						int loaded = (which != -2);
						if (loaded)
						{
							winampMediaLibraryPlugin *mlplugin;
							if (which >= 0 && which < m_plugins.GetSize() &&
								(mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(which)))
							{
								EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), !!mlplugin->hDllInstance);

								// enables / disables the config button as applicable instead of the
								// "This plug-in has no configuration implemented" message (opt-in)
								EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), (!mlplugin->MessageProc(ML_MSG_NO_CONFIG, 0, 0, 0)));
							}
						}
						else
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), 0);
							EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 1);
						}
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), 0);
					}
				}
				if (HIWORD(wParam) != LBN_DBLCLK) break;
			case IDC_GENCONF:
				{
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_GENCONF)))
					{
						int which = (INT)SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0);
						if (which != LB_ERR)
						{
							which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, (WPARAM)which, 0);
							if (which >= 0 && which < m_plugins.GetSize())
							{
								winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(which);
								if (mlplugin && mlplugin->MessageProc && mlplugin->MessageProc(ML_MSG_CONFIG, (INT_PTR)hwndDlg, 0, 0))
								{}
								else
								{
									wchar_t title[128];
									MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_NO_CONFIG_PRESENT),
												WASABI_API_LNGSTRINGW_BUF(IDS_ML_PLUGIN_INFO,title,128), MB_OK);
								}
							}
						}
					}
				}
				return FALSE;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.winamp.com/plugins?loadaddons=plugins",L"http://www.winamp.com/plugins");
				return TRUE;
		}
	}
	
	link_handledraw(hwndDlg, uMsg, wParam, lParam);
	return 0;
}

typedef struct _PLUGINORDER
{
	LPCSTR	name;
	bool	found;
} PLUGINORDER;
static PLUGINORDER preload[] =
{
	{ "ml_nowplaying.dll", false },
	{ "ml_cloud.dll", false },
	{ "ml_local.dll", false },
	{ "ml_playlists.dll", false },
	{ "ml_online.dll", false },
	{ "ml_devices.dll", false },
	{ "ml_pmp.dll", false },
	{ "ml_addons.dll", false },
	{ "ml_wire.dll", false },
	{ "ml_bookmarks.dll", false },
	{ "ml_history.dll", false },
	{ "ml_disc.dll", false },
	{ "ml_downloads.dll", false },
};

void LoadPlugin(const char *filename)
{
	char file[MAX_PATH];
	PathCombine(file, pluginPath, filename);
	HINSTANCE m = LoadLibrary(file);
	if (m)
	{
		winampMediaLibraryPlugin *(*gp)();
		gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(m, "winampGetMediaLibraryPlugin");
		if (!gp)
		{
			FreeLibrary(m);
			return ;
		}
		winampMediaLibraryPlugin *mlplugin = gp();
		if (!mlplugin || (mlplugin->version != MLHDR_VER && mlplugin->version != MLHDR_VER_OLD))
		{
			FreeLibrary(m);
			return ;
		}

		if (g_safeMode != 1)
		{
			if (g_safeMode == 2)
			{
				FreeModule(m);
				return ;
			}

			char desc[128];
			lstrcpyn(desc, mlplugin->description, sizeof(desc));
			if (desc[0] && !memcmp(desc, "nullsoft(", 9))
			{
				char* p = strrchr(desc, ')');
				if (p)
				{
					*p = 0;
					if(stricmp(filename, (desc+9)))
					{
						FreeModule(m);
						return ;
					}
				}
			}
			else
			{
				FreeModule(m);
				return ;
			}
		}

		mlplugin->hDllInstance = m;
		mlplugin->hwndLibraryParent = g_hwnd;
		mlplugin->hwndWinampParent = plugin.hwndParent;
		int index = m_plugins.GetSize();
		m_plugins.Add((void *)mlplugin);  // thats looks weird but it is convinient to have plugin in the plugin list before it will start it's initialization

		if (mlplugin->init())
		{
			FreeLibrary(m);
			m_plugins.Del(index);
			return ;
		}
	}
}

void loadMlPlugins()
{
	HANDLE h;
	WIN32_FIND_DATA d;
	char tofind[MAX_PATH];
	int i, count;

	count = sizeof(preload)/sizeof(PLUGINORDER);
	for (i = 0 ;i < count; i++) LoadPlugin(preload[i].name);

	PathCombine(tofind, pluginPath, "ML_*.DLL");

	h = FindFirstFile(tofind, &d);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			for (i = 0 ; i < count && (preload[i].found || lstrcmpiA(preload[i].name, d.cFileName)); i++);
			if (i == count) LoadPlugin(d.cFileName);
			else preload[i].found = true;
		}
		while (FindNextFile(h, &d));
		FindClose(h);
	}
}

void unloadMlPlugins()
{
	int i = m_plugins.GetSize();
	OutputDebugString("\r\n");
	while (i-- > 0)  // reverse order to aid in not fucking up subclassing shit
	{
		winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(i);
		/*wchar_t a[256];
		DWORD now = GetTickCount();*/
		if (mlplugin->quit) mlplugin->quit();	// deals with 'virtual' ml items on restart
		//if (mlplugin->hDllInstance) FreeLibrary(mlplugin->hDllInstance);
		/*if (mlplugin->version == MLHDR_VER)
			swprintf(a, L"%s - %dms\r\n", mlplugin->description, (GetTickCount() - now));
		else
			swprintf(a, L"%hs - %dms\r\n", mlplugin->description, (GetTickCount() - now));
		OutputDebugStringW(a);*/
		m_plugins.Del(i);
	}
	OutputDebugString("\r\n");
}

INT_PTR plugin_SendMessage(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	for (int i = 0; i < m_plugins.GetSize(); i++)
	{
		winampMediaLibraryPlugin *mlplugin = (winampMediaLibraryPlugin *)m_plugins.Get(i);
		if (mlplugin && mlplugin->MessageProc)
		{
			INT_PTR h = mlplugin->MessageProc(message_type, param1, param2, param3);
			if (h) return h;
		}
	}
	return 0;
}

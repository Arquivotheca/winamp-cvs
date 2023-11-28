/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include "Options.h"
#include "gen.h"
#include "PtrList.h"
#include "main.hpp"
#include "../nu/AutoWide.h"

extern nu::PtrList<winampGeneralPurposePlugin> gen_plugins;
// gen tab procedure
INT_PTR CALLBACK GenProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hi helpinfo[] = {
		{IDC_GENLIB, IDS_P_GEN_LIB},
	    {IDC_GENCONF, IDS_P_GEN_CONF},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		size_t x;
		WIN32_FIND_DATAW d;
		HANDLE h;
		wchar_t dirstr[MAX_PATH];
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);
		HWND listWindow;

		listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);

		int tabs[] = {160};
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		for (x = 0; x != gen_plugins.size(); x ++)
		{
			// only try to add if the plugin is still there
			// (as 5.5+ allows for dynamic unloads if supported)
			if(gen_plugins[x])
			{
				wchar_t filename[MAX_PATH];
				GetModuleFileNameW(gen_plugins[x]->hDllInstance, filename, MAX_PATH);
				PathStripPathW(filename);

				wchar_t description[512];
				if (gen_plugins[x]->version == GPPHDR_VER_U)
					StringCchPrintfW(description, 512, L"%s\t%s", gen_plugins[x]->description, filename);
				else
					StringCchPrintfW(description, 512, L"%s\t%s", AutoWide(gen_plugins[x]->description), filename);
				int pos = SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM) description);
				SendMessageW(listWindow, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)x);

				tabs[0] = PluginColumnWidth(hwndDlg, listWindow, filename, tabs[0]);
			}
		}

		// convert and slightly padded to character units
		RECT r;
		GetClientRect(listWindow, &r);
		tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
		tabs[0] /= 10; tabs[0] *= 9;
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		PathCombineW(dirstr, PLUGINDIR, L"GEN_*.DLL");
		h = FindFirstFileW(dirstr, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				PathCombineW(dirstr, PLUGINDIR, d.cFileName);
				HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
				for (x = 0; b && (x != gen_plugins.size()); x ++)
				{
					if (gen_plugins[x]->hDllInstance == b)
					{
						break;
					}
				}

				if (x == gen_plugins.size() || !b)
				{
					StringCchPrintfW(dirstr, MAX_PATH, getStringW(IDS_NOT_LOADED, NULL, 0), d.cFileName);
					x = SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM) dirstr);
					SendMessageW(listWindow, LB_SETITEMDATA, (WPARAM)x, (LPARAM)-2);
				}
				FreeLibrary(b);
			}
			while (FindNextFileW(h, &d));
			FindClose(h);
		}

		DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
	}

	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow;
		listWindow = GetDlgItem(hwndDlg, IDC_GENLIB);
		if (NULL != listWindow)
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_UNINST:
				{
					int which = (ULONG_PTR) SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0), which_sel = which;
					if (which != LB_ERR && LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL,IDS_P_PLUGIN_UNINSTALL_CONFIRM, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						int ret = GEN_PLUGIN_UNINSTALL_REBOOT, x;
						int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

						for (x = 0; x != gen_plugins.size(); x ++);

						which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, (WPARAM)which, 0);
						if (which != -2 && which < x)
						{
							*(void**)&pr = (void*)GetProcAddress(gen_plugins[which]->hDllInstance, "winampUninstallPlugin");
							if (pr) ret = pr(gen_plugins[which]->hDllInstance, hwndDlg, 0);
						}

						if (ret == GEN_PLUGIN_UNINSTALL_REBOOT)
						{
							if (which >= x)
							{
								wchar_t buf[1024], *p;
								StringCchCopyW(buf, 1024, PLUGINDIR);
								PathAddBackslashW(buf);
								SendDlgItemMessageW(hwndDlg, IDC_GENLIB, LB_GETTEXT, which, (LPARAM)buf + wcslen(buf));
								p = wcsstr(buf, L" |");
								if (p)
								{
									*p = 0;
									IFileTypeRegistrar *registrar=0;
									if (GetRegistrar(&registrar) == 0 && registrar)
									{
										registrar->DeleteItem(buf);
										registrar->Release();
									}
									SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_RESETCONTENT, 0, 0);
									SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);
								}
							}
							else // unload live plug-in
							{
								if (which == -2)
								{
									// TODO remove non-loaded plug-ins
								}
								else
								{
									extern void _w_s(char *name, char *data);
									wchar_t buf[MAX_PATH];
									GetModuleFileNameW(gen_plugins[which]->hDllInstance, buf, MAX_PATH);
									_w_sW("remove_genplug", buf);
									_w_i("show_prefs", 35);
									PostMessage(hMainWindow, WM_USER, 0, IPC_RESTARTWINAMP);
								}
							}
						}
						// do dynamic unload if the plugin is able to support it (5.5+)
						else if (ret == GEN_PLUGIN_UNINSTALL_NOW) {
							wchar_t buf[MAX_PATH];
							GetModuleFileNameW(gen_plugins[which]->hDllInstance, buf, MAX_PATH);
							gen_plugins[which]->quit();
							FreeModule(gen_plugins[which]->hDllInstance);
							gen_plugins[which]=0;

							IFileTypeRegistrar *registrar=0;
							if (GetRegistrar(&registrar) == 0 && registrar)
							{
								registrar->DeleteItem(buf);
								registrar->Release();
							}

							SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_DELETESTRING, which_sel, 0);

							// fakes a selection and resets the focus to the listbox so it'll keep ui response working
							SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_GENLIB,LBN_SELCHANGE),0);
							SetFocus(GetDlgItem(hwndDlg,IDC_GENLIB));
						}
					}
				}
				return FALSE;
			case IDC_GENLIB:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0);
					if (sel != LB_ERR)
					{
						int loaded = (SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, sel, 0) != -2);
						EnableWindow(GetDlgItem(hwndDlg, IDC_GENCONF), loaded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINST), loaded);
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
					size_t which = (ULONG_PTR)SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETCURSEL, 0, 0), x;
					for (x = 0; x != gen_plugins.size(); x ++);
					if (which < x)
					{
						if (which != LB_ERR)
						{
							which = SendDlgItemMessage(hwndDlg, IDC_GENLIB, LB_GETITEMDATA, (WPARAM)which, 0);
							if (which < gen_plugins.size())
							{
								if (!(config_no_visseh&4))
								{
									try {
										gen_plugins[which]->config();
									}
									catch(...)
									{
										LPMessageBox(hwndDlg, IDS_PLUGINERROR, IDS_ERROR, MB_OK | MB_ICONEXCLAMATION);
									}
								}
								else
								{
									gen_plugins[which]->config();
								}
							}
						}
					}
					else
					{}}
					return FALSE;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.winamp.com/plugins?loadaddons=plugins", L"http://www.winamp.com/plugins");
				break;
		}
	}
	else
		link_handledraw(hwndDlg, uMsg, wParam, lParam);

	return FALSE;
} //gen
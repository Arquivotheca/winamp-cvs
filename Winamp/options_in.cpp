/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "resource.h"
#include "Options.h"
#include "main.hpp"
#include "PtrList.h"
#include "../nu/AutoWide.h"

extern nu::PtrList<In_Module> in_modules;

INT_PTR CALLBACK InputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_INPUTS,IDS_P_IN_INPUTS},
		{IDC_CONF,IDS_P_IN_CONF},
		{IDC_ABOUT,IDS_P_IN_ABOUT},
		{IDC_UNINSTINPUT,IDS_P_IN_UNINST},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		HWND hw=GetDlgItem(hwndDlg,IDC_INPUTS);
		size_t x;
		WIN32_FIND_DATAW d;
		HANDLE h;
		wchar_t dirstr[MAX_PATH];
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		SendMessage(hw, WM_SETREDRAW, 0, 0);
		int tabs[] = {160};
		SendMessage(hw, LB_SETTABSTOPS, 1, (LPARAM)tabs);
		SendMessage(hw, LB_RESETCONTENT, 0, 0);

		for (x = 0; x < in_modules.size(); x ++)
		{
			// only try to add if the plugin is still there
			// (as 5.5+ allows for dynamic unloads if supported)
			if(in_modules[x])
			{
				wchar_t buf[MAX_PATH],buf2[MAX_PATH],*p;
				GetModuleFileNameW(in_modules[x]->hDllInstance,buf2,MAX_PATH);
				p = PathFindFileNameW(buf2);

				if ((in_modules[x]->version & ~IN_UNICODE) == IN_VER_U)
					StringCchPrintfW(buf, MAX_PATH, L"%s\t%s", in_modules[x]->description, p);
				else
					StringCchPrintfW(buf, MAX_PATH, L"%hs\t%s", in_modules[x]->description, p);

				int pos = SendMessageW(hw, LB_ADDSTRING, 0, (LPARAM)buf);
				SendMessageW(hw, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)x);

				tabs[0] = PluginColumnWidth(hwndDlg, hw, p, tabs[0]);
			}
		}

		// convert and slightly padded to character units
		RECT r;
		GetClientRect(hw, &r);
		tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
		tabs[0] /= 10; tabs[0] *= 9;
		SendMessage(hw, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		PathCombineW(dirstr, PLUGINDIR, L"IN_*.DLL");
		h = FindFirstFileW(dirstr, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				PathCombineW(dirstr, PLUGINDIR, d.cFileName);
				HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
				for (x = 0; b && (x != in_modules.size()); x ++)
				{
					if (in_modules[x]->hDllInstance == b)
					{
						break;
					}
				}

				if (x == in_modules.size() || !b)
				{
					StringCchPrintfW(dirstr, MAX_PATH, getStringW(IDS_NOT_LOADED, NULL, 0), d.cFileName);
					x = SendMessageW(hw, LB_ADDSTRING, 0, (LPARAM) dirstr);
					SendMessageW(hw, LB_SETITEMDATA, (WPARAM)x, (LPARAM)-2);
				}
				FreeLibrary(b);
			}
			while (FindNextFileW(h, &d));
			FindClose(h);
		}

		SendMessage(hw,WM_SETREDRAW,1,0);
		DirectMouseWheel_EnableConvertToMouseWheel(hw, TRUE);
	}
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow;
		listWindow = GetDlgItem(hwndDlg, IDC_INPUTS);
		if (NULL != listWindow)
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_INPUTS:
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					SendMessage(hwndDlg,WM_COMMAND,IDC_CONF,0);
				}
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_INPUTS, LB_GETCURSEL, 0, 0);
					if (sel != LB_ERR)
					{
						int loaded = (SendDlgItemMessage(hwndDlg, IDC_INPUTS, LB_GETITEMDATA, sel, 0) != -2);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONF), loaded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ABOUT), loaded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTINPUT), loaded);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONF), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ABOUT), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTINPUT), 0);
					}
				}
				break;
			case IDC_ABOUT:
				{
					size_t x = SendMessage(GetDlgItem(hwndDlg, IDC_INPUTS), LB_GETCURSEL, 0, 0);
					if (x != LB_ERR)
					{
						x = SendDlgItemMessage(hwndDlg, IDC_INPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
						if (x >= 0 && x < in_modules.size()) in_modules[x]->About(hwndDlg);
					}
				}
				break;
			case IDC_CONF:
				{
					size_t x = SendMessage(GetDlgItem(hwndDlg, IDC_INPUTS), LB_GETCURSEL, 0, 0);
					if (x != LB_ERR)
					{
						x = SendDlgItemMessage(hwndDlg, IDC_INPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
						if (x >= 0 && x < in_modules.size()) in_modules[x]->Config(hwndDlg);
					}
				}
				break;
			case IDC_UNINSTINPUT:
				{
					size_t x;
					x = SendMessage(GetDlgItem(hwndDlg,IDC_INPUTS),LB_GETCURSEL,0,0);
					if (x != LB_ERR && x < in_modules.size())
					{
						if (LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL, IDS_P_PLUGIN_UNINSTALL_CONFIRM, MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
						{
							int ret = IN_PLUGIN_UNINSTALL_REBOOT;
							int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);
							x = SendDlgItemMessage(hwndDlg, IDC_INPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
							*(void**)&pr = (void*)GetProcAddress(in_modules[x]->hDllInstance,"winampUninstallPlugin");
							if (pr)ret=pr(in_modules[x]->hDllInstance,hwndDlg,0);
							if (ret == IN_PLUGIN_UNINSTALL_REBOOT)
							{
								extern void _w_s(char *name, char *data);
								char buf[1024];
								GetModuleFileName(in_modules[x]->hDllInstance,buf,sizeof(buf));
								_w_s("remove_genplug",buf);
								_w_i("show_prefs",31);
								PostMessage(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
							}
							// do dynamic unload if the plugin is able to support it (5.5+)
							else if (ret == IN_PLUGIN_UNINSTALL_NOW)
							{
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(in_modules[x]->hDllInstance,buf,MAX_PATH);
								in_modules[x]->Quit();
								FreeModule(in_modules[x]->hDllInstance);
								in_modules[x]=0;

								IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar) == 0 && registrar)
								{
									registrar->DeleteItem(buf);
									registrar->Release();
								}

								SendDlgItemMessage(hwndDlg,IDC_INPUTS,LB_DELETESTRING,x,0);

								// fakes a selection and resets the focus to the listbox so it'll keep ui response working
								SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_INPUTS,LBN_SELCHANGE),0);
								SetFocus(GetDlgItem(hwndDlg,IDC_INPUTS));
							}
						}
					}
				}
				break;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.winamp.com/plugins?loadaddons=plugins",L"http://www.winamp.com/plugins");
				break;
		}

	}
	
	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return FALSE;
} //input
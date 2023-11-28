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
#include "../nu/AutoWide.h"

// output tab procedure
static int last_sel = -1;
INT_PTR CALLBACK OutputProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	hi helpinfo[]=
	{
		{IDC_OUTPUTS,IDS_P_OUT_OUTPUTS},
		{IDC_CONF2,IDS_P_OUT_CONF},
		{IDC_ABOUT2,IDS_P_OUT_ABOUT},
		{IDC_UNINSTOUT,IDS_P_OUT_UNINST},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		HWND hw=GetDlgItem(hwndDlg,IDC_OUTPUTS);
		int x;
		WIN32_FIND_DATAW d;
		HANDLE h;
		wchar_t dirstr[MAX_PATH];
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);
		SendMessage(hw, WM_SETREDRAW, 0, 0);
		SendMessage(hw, LB_RESETCONTENT, 0, 0);
		int tabs[] = {160};
		SendMessage(hw, LB_SETTABSTOPS, 1, (LPARAM)tabs);
		if (out_modules[0])
		{		
			EnableWindow(GetDlgItem(hwndDlg,IDC_CONF2), 1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ABOUT2), 1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_UNINSTOUT), 1);
		}
		for (x = 0; out_modules[x]; x ++)
		{
			wchar_t buf[MAX_PATH+1024],buf2[MAX_PATH],*p;
			GetModuleFileNameW(out_modules[x]->hDllInstance,buf2,sizeof(buf2)/sizeof(*buf2));
			p = PathFindFileNameW(buf2);

			if (out_modules[x]->version == OUT_VER_U)
				StringCbPrintfW(buf, sizeof(buf2), L"%s\t%s", out_modules[x]->description, p);
			else
				StringCbPrintfW(buf, sizeof(buf2), L"%hs\t%s", out_modules[x]->description, p);

			int pos = SendMessageW(hw, LB_ADDSTRING, 0, (LPARAM)buf);
			SendMessageW(hw, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)x);

			if (!_stricmp(config_outname, (char*)out_modules[x]->id))
				SendMessage(hw, LB_SETCURSEL, (last_sel = pos), 0);

			tabs[0] = PluginColumnWidth(hwndDlg, hw, p, tabs[0]);
		}

		// convert and slightly padded to character units
		RECT r;
		GetClientRect(hw, &r);
		tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
		tabs[0] /= 10; tabs[0] *= 10;
		SendMessage(hw, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		PathCombineW(dirstr, PLUGINDIR, L"OUT*.DLL");
		h = FindFirstFileW(dirstr, &d);
		if (h != INVALID_HANDLE_VALUE)
		{
			do
			{
				PathCombineW(dirstr, PLUGINDIR, d.cFileName);
				HMODULE b = LoadLibraryExW(dirstr, NULL, LOAD_LIBRARY_AS_DATAFILE);
				bool found = false;
				for (x = 0; b && out_modules[x]; x ++)
				{
					if (out_modules[x]->hDllInstance == b)
					{
						found = true;
						break;
					}
				}

				if (!found)
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
		listWindow = GetDlgItem(hwndDlg, IDC_OUTPUTS);
		if (NULL != listWindow)
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND) 
	{
		switch (LOWORD(wParam))
		{
			case IDC_OUTPUTS:
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					int sel = SendDlgItemMessage(hwndDlg, IDC_OUTPUTS, LB_GETCURSEL, 0, 0);
					if (sel != LB_ERR)
					{
						sel = SendDlgItemMessage(hwndDlg, IDC_OUTPUTS, LB_GETITEMDATA, (WPARAM)sel, 0);
						int loaded = (sel != -2);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CONF2), loaded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ABOUT2), loaded);
						EnableWindow(GetDlgItem(hwndDlg, IDC_UNINSTOUT), loaded);

						if (loaded && out_modules[sel])
						{
							StringCbCopy(config_outname, sizeof(config_outname), (char *)out_modules[sel]->id);
							if (last_sel != -1 && out_modules[last_sel]) out_changed(out_modules[last_sel]->hDllInstance, OUT_UNSET);
							out_changed(out_modules[sel]->hDllInstance, OUT_SET);
							PostMessage(hMainWindow, WM_WA_IPC, (WPARAM)config_outname, IPC_CB_OUTPUTCHANGED);
						}
					}
				}
				else if (HIWORD(wParam) == LBN_DBLCLK)
					SendMessage(hwndDlg,WM_COMMAND,IDC_CONF2,0);

				return FALSE;
			case IDC_ABOUT2:
			{
				int x = SendMessage(GetDlgItem(hwndDlg, IDC_OUTPUTS), LB_GETCURSEL, 0, 0);
				if (x != LB_ERR)
				{
					x = SendDlgItemMessage(hwndDlg, IDC_OUTPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
					if(out_modules[x]) out_modules[x]->About(hwndDlg);
				}
			}
			return FALSE;
			case IDC_CONF2:
			{
				int x = SendMessage(GetDlgItem(hwndDlg, IDC_OUTPUTS), LB_GETCURSEL, 0, 0);
				if (x != LB_ERR)
				{
					x = SendDlgItemMessage(hwndDlg, IDC_OUTPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
					if(out_modules[x]) out_modules[x]->Config(hwndDlg);
				}
			}
			return FALSE;
			case IDC_UNINSTOUT:
			{
				int x = SendMessage(GetDlgItem(hwndDlg,IDC_OUTPUTS),LB_GETCURSEL,0,0);
				if (x != LB_ERR && out_modules[x])
				{
					if (LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL,IDS_P_PLUGIN_UNINSTALL_CONFIRM,MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
					{
						int ret=0;
						int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);

						x = SendDlgItemMessage(hwndDlg, IDC_OUTPUTS, LB_GETITEMDATA, (WPARAM)x, 0);
						*(void**)&pr = (void*)GetProcAddress(out_modules[x]->hDllInstance,"winampUninstallPlugin");
						// changed 28/11/2010 so that even if non-zero is returned then the plug-in will uninstall
						if (pr)/*ret=*/pr(out_modules[x]->hDllInstance,hwndDlg,0);
						if (!ret)
						{
							char buf[1024];
							int w=-1;
							char *p=buf;
							int y;
							GetModuleFileName(out_modules[x]->hDllInstance,buf,sizeof(buf));
							_w_s("remove_genplug",buf);
							_w_i("show_prefs",32);

							buf[0]=0;

							for (y = 0; out_modules[y]; y ++)
							{
								if (x != y)
								{
									GetModuleFileName(out_modules[y]->hDllInstance,buf,sizeof(buf));
									p = PathFindFileName(buf);

									if (!_stricmp(p,"out_wave.dll") && w < 1) w=1;
									if (!_stricmp(p,"out_ds.dll") && w < 0) w=0;

									if (w>0) break;
								}
							}

							if (w==1) StringCbCopy(config_outname, sizeof(config_outname), "out_wave.dll");
							else if (w==0) StringCbCopy(config_outname,sizeof(config_outname), "out_ds.dll");
							else StringCbCopy(config_outname,sizeof(config_outname),p);
							PostMessage(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
						}
					}
				}
			}
			break;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg,L"http://www.winamp.com/plugins?loadaddons=plugins", L"http://www.winamp.com/plugins");
				break;
		}
	}
	else
	{
		link_handledraw(hwndDlg,uMsg,wParam,lParam);
	}


	return FALSE;
} //output
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
#include "vis.h"
#include "main.hpp"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

// vis tab procedure
INT_PTR CALLBACK VisProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	hi helpinfo[]={
		{IDC_VISLIB,IDS_P_VIS_LIB},
		{IDC_VISMOD,IDS_P_VIS_MOD},
		{IDC_VISCONF,IDS_P_VIS_CONF},
		{IDC_VISSTOP,IDS_P_VIS_STOP},
		{IDC_VISTEST,IDS_P_VIS_START},
		{IDC_UNINSTVIS,IDS_P_VIS_UNINST},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		HWND listWindow;
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);
		listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);

		SendMessageW(listWindow, WM_SETREDRAW,0,0);
		int tabs[] = {160};
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);
		SendMessageW(listWindow, LB_INITSTORAGE, 256, 63356);
		{
			wchar_t dirstr[MAX_PATH+128];	
			HANDLE h;
			WIN32_FIND_DATAW d;
			PathCombineW(dirstr,VISDIR,L"*.DLL");
			h = FindFirstFileW(dirstr,&d);
			if (h != INVALID_HANDLE_VALUE) 
			{
				do 
				{
					wchar_t namestr[MAX_PATH+256];
					if (_wcsnicmp(d.cFileName,L"dsp_",4)
						&& _wcsnicmp(d.cFileName,L"in_",3) 
						&& _wcsnicmp(d.cFileName,L"gen_",4) 
						&& _wcsnicmp(d.cFileName,L"ml_",3) 
						&& _wcsnicmp(d.cFileName,L"enc_",4) 
						&& _wcsnicmp(d.cFileName,L"CDDB",4)
						&& _wcsnicmp(d.cFileName,L"out_",4))
					{
						HINSTANCE hLib;
						wchar_t b[1024];
						PathCombineW(b, VISDIR, d.cFileName);
						hLib = LoadLibraryW(b);
						if (hLib)
						{
							winampVisGetHeaderType pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
							if (pr)
							{
								if (!g_safeMode)
								{
									StringCchCopyW(namestr,MAX_PATH+256,AutoWide(pr(hMainWindow)->description));
								}
								else
								{
									StringCchPrintfW(dirstr, MAX_PATH, getStringW(IDS_NOT_LOADED, NULL, 0), d.cFileName);
									SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM) dirstr);
								}
							} 
							else{
								StringCchCopyW(namestr,MAX_PATH+256,L"!");
							}
							FreeModule(hLib);
						}
						else
						{
							StringCchCopyW(namestr,MAX_PATH+256,L"!");
						}

						if (wcscmp(namestr,L"!"))
						{
							int offs=lstrlenW(namestr)+1;
							StringCchPrintfW(namestr+lstrlenW(namestr), MAX_PATH+256-lstrlenW(namestr), L"\t%s", d.cFileName);
							SendMessageW(listWindow,LB_SETITEMDATA, (WPARAM)SendMessageW(listWindow,LB_ADDSTRING,0,(LPARAM)namestr), (LPARAM)offs);

							tabs[0] = PluginColumnWidth(hwndDlg, listWindow, d.cFileName, tabs[0]);
						}
					}								
				} while (FindNextFileW(h,&d));

				FindClose(h);

					int t = SendMessageW(listWindow,LB_GETCOUNT,0,0);
				if (t != LB_ERR)
				{
					int found = 0;
					for (int x = 0; x < t; x ++)
					{
						wchar_t fn[FILENAME_SIZE];
						wchar_t *libname;
						SendMessageW(listWindow,LB_GETTEXT,x,(LPARAM)fn);
						libname = fn + (int) SendMessage(listWindow,LB_GETITEMDATA,x,0);
						if (!_wcsicmp(libname,config_visplugin_name))
						{
							found = 1;
							SendMessageW(listWindow,LB_SETCURSEL,x,0);
							// delay this so everything will work it's way through (5.58+/unicode changes)
							PostMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_VISLIB,LBN_SELCHANGE),0);
							PostMessage(listWindow,CB_SETCURSEL,config_visplugin_num,0);
							break;
						}
					}

					if (!found)
						PostMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_VISLIB,LBN_SELCHANGE),0);
				}
			}
		}
		// [5.55+] only send this to correct the page state if we have no valid vis plugins available and loaded
		if(!SendMessageW(listWindow,LB_GETCOUNT,0,0) || g_safeMode) 
			SendMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_VISLIB,LBN_SELCHANGE),0);

		// convert and slightly padded to character units
		RECT r;
		GetClientRect(listWindow, &r);
		tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
		tabs[0] /= 10; tabs[0] *= 10;
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		SendMessageW(listWindow,WM_SETREDRAW,1,0);
		DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
	}
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow;
		listWindow = GetDlgItem(hwndDlg, IDC_VISLIB);
		if (NULL != listWindow)
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_VISLIB:
				if (HIWORD(wParam) == LBN_DBLCLK)
				{
					SendMessage(hwndDlg,WM_COMMAND,IDC_VISTEST,0);
				}
				if (HIWORD(wParam) == LBN_SELCHANGE)
				{
					wchar_t b[1024];
					int which=CB_ERR;
					if (!g_safeMode)
					{
						HINSTANCE hLib;
						wchar_t fn[FILENAME_SIZE];
						wchar_t *libname;
						which=SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETCURSEL,0,0);
						if (which != CB_ERR){
							SendDlgItemMessageW(hwndDlg,IDC_VISLIB,LB_GETTEXT,which,(LPARAM)fn);
							libname = fn + (int)SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETITEMDATA,which,0);
							if (wcscmp(config_visplugin_name,libname)) config_visplugin_num=0;
							StringCchCopyW(config_visplugin_name,MAX_PATH,libname);
							SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_RESETCONTENT,0,0);
							PathCombineW(b, VISDIR, libname);
							hLib = LoadLibraryW(b);
							if (hLib)
							{
								int i;
								winampVisGetHeaderType pr;
								winampVisModule *module;
								pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
								if (pr)
								{
									i=0;
									for(;;)
									{
										module = pr(hMainWindow)->getModule(i++);
										if (!module) break;
										SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_ADDSTRING,0,(LPARAM)module->description);
									}
									SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_DELETESTRING,which,0);
									StringCchPrintfW(b,1024,L"%hs\t%s",pr(hMainWindow)->description,libname);
									which=SendDlgItemMessageW(hwndDlg,IDC_VISLIB,LB_INSERTSTRING,which,(LPARAM) b);
									SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_SETITEMDATA,which,(LPARAM) (lstrlen(pr(hMainWindow)->description)+1));
									SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_SETCURSEL,which,0);
								}
								FreeModule(hLib);
							}
							SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_SETCURSEL,0,0);
						}
					}

					int enable = (which != CB_ERR);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VISTEST),enable);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VISSTOP),enable);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VISCONF),enable);
					EnableWindow(GetDlgItem(hwndDlg,IDC_UNINSTVIS),enable);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VISMOD),enable);
				}
				return FALSE;
			case IDC_VISMOD:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					config_visplugin_num = (unsigned char) SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_GETCURSEL,0,0);
					if (config_visplugin_num == CB_ERR) config_visplugin_num =0;
				}
				return FALSE;
			case IDC_UNINSTVIS:
				vis_stop();
				{
					int which = SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETCURSEL,0,0);
					if (which != CB_ERR)
					{
						wchar_t buf[MAX_PATH];
						wchar_t b[MAX_PATH];
						wchar_t fn[MAX_PATH*2];
						wchar_t *libname;
						HINSTANCE hLib;
						SendDlgItemMessageW(hwndDlg,IDC_VISLIB,LB_GETTEXT,which,(LPARAM)fn);
						libname = fn + (int) SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETITEMDATA,which,0);
						PathCombineW(b, VISDIR, libname);
						hLib = LoadLibraryW(b);
						if (hLib)
						{
							GetModuleFileNameW(hLib, buf, MAX_PATH);
							int ret=0;
							int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);
							*(void**)&pr = (void*)GetProcAddress(hLib,"winampUninstallPlugin");
							if (pr)ret=pr(hLib,hwndDlg,0);
							FreeLibrary(hLib);
							if (!ret)
							{
								IFileTypeRegistrar *registrar=0;
								if (GetRegistrar(&registrar) == 0 && registrar)
								{
									registrar->DeleteItem(buf);
									registrar->Release();
								}
								SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_DELETESTRING,which,0);
								SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_RESETCONTENT,0,0);
							}
						}
					}
				}
				return FALSE;
			case IDC_VISCONF:
				{
					int which=SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETCURSEL,0,0);
					if (which != CB_ERR)
					{
						wchar_t b[MAX_PATH];
						wchar_t fn[MAX_PATH*2];
						wchar_t *libname;
						HINSTANCE hLib;
						SendDlgItemMessageW(hwndDlg,IDC_VISLIB,LB_GETTEXT,which,(LPARAM)fn);
						libname = fn + (int) SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETITEMDATA,which,0);
						PathCombineW(b, VISDIR, libname);
						hLib = LoadLibraryW(b);
						if (hLib)
						{
							winampVisGetHeaderType pr;
							winampVisModule *module;
							pr = (winampVisGetHeaderType) GetProcAddress(hLib,"winampVisGetHeader");
							module = pr(hMainWindow)->getModule(SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_GETCURSEL,0,0));
							if (module)
							{
								module->hDllInstance = hLib;
								module->hwndParent = hMainWindow;
								if (!(config_no_visseh&1))
								{
									try {
										module->Config(module);
									}
									catch(...)
									{
										LPMessageBox(hwndDlg, IDS_PLUGINERROR, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
									}	
								}
								else
								{
									module->Config(module);
								}
							}
							else
							{
								LPMessageBox(hwndDlg, IDS_ERRORLOADINGPLUGIN, IDS_ERROR, MB_OK);
							}
							FreeLibrary(hLib);
						}
						else
						{
							LPMessageBox(hwndDlg, IDS_ERRORLOADINGPLUGIN, IDS_ERROR, MB_OK);
						}
					}
				}
				return FALSE;
			case IDC_VISSTOP:
				{
					vis_stop();
				}
				return FALSE;
			case IDC_VISTEST:
				{
					int which=SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETCURSEL,0,0);
					if (which != CB_ERR)
					{
						wchar_t fn[MAX_PATH*2];
						wchar_t *libname;
						SendDlgItemMessageW(hwndDlg,IDC_VISLIB,LB_GETTEXT,which,(LPARAM)fn);
						libname = fn + (int)SendDlgItemMessage(hwndDlg,IDC_VISLIB,LB_GETITEMDATA,which,0);
						StringCchCopyW(config_visplugin_name,MAX_PATH,libname);
						config_visplugin_num = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_VISMOD,CB_GETCURSEL,0,0);
					}
					vis_start(hMainWindow,NULL);
				}
				return FALSE;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.winamp.com/visualizations?loadaddons=visualizations", L"http://www.winamp.com/visualizations");
				return TRUE;
		}
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return FALSE;
} //vis
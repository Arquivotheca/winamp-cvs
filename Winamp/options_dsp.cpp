/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
#include "dsp.h"
#include "Options.h"
#include "main.hpp"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"

static bool pressed = 0;
LRESULT pressed_proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_KEYDOWN){
		pressed = 1;
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"pressed_proc"), hwndDlg, uMsg, wParam, lParam);
}

// dsp tab procedure
INT_PTR CALLBACK DspProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	static int noreset=0;
	hi helpinfo[]={
		{IDC_DSPLIB,IDS_P_DSP_LIB},
		{IDC_DSPMOD,IDS_P_DSP_MOD},
		{IDC_DSPCONF,IDS_P_DSP_CONF},
	};

	DO_HELP();

	if (uMsg == WM_INITDIALOG)
	{
		HWND listWindow;

		noreset=1;
		int selset=0;
		link_startsubclass(hwndDlg, IDC_PLUGINVERS);

		listWindow = GetDlgItem(hwndDlg, IDC_DSPLIB);
		SetPropW(listWindow, L"pressed_proc", (HANDLE)SetWindowLongPtrW(listWindow, GWLP_WNDPROC, (LONG_PTR)pressed_proc));
		int tabs[] = {160};
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		if (!g_safeMode)
		{
			SendMessageW(listWindow,LB_SETITEMDATA,
							  (WPARAM)SendMessageW(listWindow,LB_ADDSTRING,0,(LPARAM)getStringW(IDS_DSP_NONE,NULL,0)),
							  (LPARAM)0);
			SendMessageW(listWindow,LB_SETCURSEL,0,0);
		}

		wchar_t dirstr[MAX_PATH];	
		{
			HANDLE h;
			WIN32_FIND_DATAW d;
			PathCombineW(dirstr, DSPDIR, L"DSP_*.DLL");
			h = FindFirstFileW(dirstr,&d);
			if (h != INVALID_HANDLE_VALUE) 
			{
				int pos2=0;
				do 
				{
					wchar_t namestr[MAX_PATH+256];
					{
						if (!g_safeMode)
						{
							wchar_t b[1024];
							HINSTANCE hLib;
							PathCombineW(b, DSPDIR, d.cFileName);
							hLib = LoadLibraryW(b);
							if (hLib) 
							{
								winampDSPGetHeaderType pr;
								winampDSPHeader *header;
								pr = (winampDSPGetHeaderType) GetProcAddress(hLib,"winampDSPGetHeader2");
								if (pr)
								{
									header = pr(hMainWindow);
									if (header && header->version >= DSP_HDRVER && header->version < DSP_HDRVER+0x10)
										StringCchCopyW(namestr,MAX_PATH+256,AutoWide(header->description));
									else
										StringCchCopyW(namestr,MAX_PATH+256,L"!");
								}
								else
								{
									StringCchCopyW(namestr,MAX_PATH+256,L"!");
								}
								FreeModule(hLib);
							}

							if (wcscmp(namestr,L"!"))
							{
								int offs=lstrlenW(namestr)+1;
								pos2++;
								StringCchPrintfW(namestr+lstrlenW(namestr),MAX_PATH+256-lstrlenW(namestr), L"\t%s",d.cFileName);
								SendMessageW(listWindow,LB_SETITEMDATA,
												   (WPARAM)SendMessageW(listWindow,LB_ADDSTRING,0,(LPARAM)namestr),
												   (LPARAM)offs);

								if (!_wcsicmp(d.cFileName,config_dspplugin_name))
								{
									selset=1;
									SendMessageW(listWindow,LB_SETCURSEL,pos2,0);
									PostMessageW(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_DSPLIB,LBN_SELCHANGE),0);
									PostMessageW(listWindow,CB_SETCURSEL,config_dspplugin_num,0);                  
								}	
							}
						}
						else
						{
							StringCchPrintfW(dirstr, MAX_PATH, getStringW(IDS_NOT_LOADED, NULL, 0), d.cFileName);
							SendMessageW(listWindow, LB_ADDSTRING, 0, (LPARAM) dirstr);
						}

						tabs[0] = PluginColumnWidth(hwndDlg, listWindow, d.cFileName, tabs[0]);
					}			
				} while (FindNextFileW(h,&d));
				FindClose(h);
			}
		}

		// convert and slightly padded to character units
		RECT r;
		GetClientRect(listWindow, &r);
		tabs[0] = (((r.right - r.left) - tabs[0]) / 1.75);
		tabs[0] /= 10; tabs[0] *= 10;
		SendMessage(listWindow, LB_SETTABSTOPS, 1, (LPARAM)tabs);

		if (g_safeMode && !SendMessageW(listWindow,LB_GETCOUNT,0,0))
		{
			SendMessageW(listWindow,LB_SETITEMDATA,
							  (WPARAM)SendMessageW(listWindow,LB_ADDSTRING,0,(LPARAM)getStringW(IDS_DSP_NONE,NULL,0)),
							  (LPARAM)0);
			SendMessageW(listWindow,LB_SETCURSEL,0,0);
		}

		// indicate we can handle a selection if no dsp was selected on loading the page
		if(!selset)
			noreset=0;

		DirectMouseWheel_EnableConvertToMouseWheel(listWindow, TRUE);
	}
	else if (uMsg == WM_DESTROY)
	{
		HWND listWindow;
		listWindow = GetDlgItem(hwndDlg, IDC_DSPLIB);
		if (NULL != listWindow)
			DirectMouseWheel_EnableConvertToMouseWheel(listWindow, FALSE);
	}
	else if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
			case IDC_DSPMOD:
			case IDC_DSPLIB:
				if ((LOWORD(wParam)==IDC_DSPLIB && HIWORD(wParam) == LBN_SELCHANGE) || 
					(LOWORD(wParam)==IDC_DSPMOD && HIWORD(wParam) == CBN_SELCHANGE))
				{
					HINSTANCE hLib;
					int clicked=0;
					if (LOWORD(wParam)==IDC_DSPLIB && HIWORD(wParam) == LBN_SELCHANGE)
					{
						wchar_t *libname = 0;
						if (!g_safeMode)
						{
							wchar_t b[1024], fn[MAX_PATH*2];
							int which=SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_GETCURSEL,0,0);

							if (which == LB_ERR) return FALSE;

							// do a check to see if we've actually clicked on the item otherwise
							// it will skip the quit / init cycle which was reported as annoying
							// especially when it was happening on the dead area of the listbox
							// also this means the plug-in won't re-init when opening the page
							POINT ps;
							GetCursorPos(&ps);
							ScreenToClient(GetDlgItem(hwndDlg,IDC_DSPLIB),&ps);
							LRESULT x=SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_ITEMFROMPOINT,0,MAKELPARAM(ps.x,ps.y));

							// see if we're clicking on a valid item or using the keyboard navigation in the listbox
							if(x < SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_GETCOUNT,0,0) && GetFocus() == (HWND)lParam || pressed) {
								clicked=1;
							}
							pressed = 0;

							SendDlgItemMessageW(hwndDlg,IDC_DSPLIB,LB_GETTEXT,which,(LPARAM)fn);
							libname = fn + (int)SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_GETITEMDATA,which,0);
							if (!which) libname[0]=0;

							SendDlgItemMessage(hwndDlg,IDC_DSPMOD,CB_RESETCONTENT,0,0);
							if (lstrcmpW(config_dspplugin_name,libname))
								config_dspplugin_num=0;
							StringCchCopyW(config_dspplugin_name, MAX_PATH, libname);

							if (*libname)
							{
								PathCombineW(b, DSPDIR, libname);
								hLib = LoadLibraryW(b);
								if (hLib)
								{
									int i;
									winampDSPGetHeaderType pr;
									winampDSPModule *module;
									pr = (winampDSPGetHeaderType) GetProcAddress(hLib,"winampDSPGetHeader2");
									if (pr)
									{
										i=0;
										for (;;)
										{
											module = pr(hMainWindow)->getModule(i++);
											if (!module) break;
											SendDlgItemMessage(hwndDlg,IDC_DSPMOD,CB_ADDSTRING,0,(LPARAM)module->description);
										}
									}
									FreeModule(hLib);
								}
								SendDlgItemMessage(hwndDlg,IDC_DSPMOD,CB_SETCURSEL,config_dspplugin_num,0);
							}
						}
						EnableWindow(GetDlgItem(hwndDlg,IDC_DSPCONF),!!*libname);
						EnableWindow(GetDlgItem(hwndDlg,IDC_UNINSTDSP),!!*libname);
						EnableWindow(GetDlgItem(hwndDlg,IDC_DSPMOD),!!*libname);
						EnableWindow(GetDlgItem(hwndDlg,IDC_PLUGINLABEL),!!*libname);          
					}
					config_dspplugin_num = (unsigned char) SendDlgItemMessage(hwndDlg,IDC_DSPMOD,CB_GETCURSEL,0,0);
					if (config_dspplugin_num==CB_ERR) config_dspplugin_num=0;
					if (!noreset && clicked)
					{
						dsp_quit();
						dsp_init();
					}
					noreset=0;
				}
				return FALSE;
			case IDC_UNINSTDSP:
				{
					if (g_safeMode) return FALSE;

					int which=SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_GETCURSEL,0,0);
					if (which != CB_ERR && which > 0)
					{
						if (LPMessageBox(hwndDlg, IDS_P_PLUGIN_UNINSTALL,IDS_P_PLUGIN_UNINSTALL_CONFIRM,MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
						{
							wchar_t b[MAX_PATH];
							wchar_t fn[MAX_PATH*2];
							wchar_t *libname;
							HINSTANCE hLib;

							dsp_quit();
							SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_SETCURSEL,0,0);
							// fakes a selection and resets the focus to the listbox so it'll keep ui response working
							SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_DSPLIB,LBN_SELCHANGE),0);
							SetFocus(GetDlgItem(hwndDlg,IDC_DSPLIB));

							SendDlgItemMessageW(hwndDlg,IDC_DSPLIB,LB_GETTEXT,which,(LPARAM)fn);
							libname = fn + (int)SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_GETITEMDATA,which,0);

							PathCombineW(b, DSPDIR, libname);
							hLib = LoadLibraryW(b);
							if (hLib)
							{
								int ret = DSP_PLUGIN_UNINSTALL_NOW;
								int (*pr)(HINSTANCE hDllInst, HWND hwndDlg, int param);
								*(void**)&pr = (void*)GetProcAddress(hLib,"winampUninstallPlugin");
								if (pr)ret=pr(hLib,hwndDlg,0);
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(hLib, buf, MAX_PATH);
								FreeLibrary(hLib);
								if (ret == DSP_PLUGIN_UNINSTALL_NOW) {
									IFileTypeRegistrar *registrar=0;
									if (GetRegistrar(&registrar) == 0 && registrar)
									{
										registrar->DeleteItem(buf);
										registrar->Release();
									}
									SendDlgItemMessage(hwndDlg,IDC_DSPLIB,LB_DELETESTRING,which,0);
								}
								else if (ret == DSP_PLUGIN_UNINSTALL_REBOOT) {
									extern void _w_s(char *name, char *data);
									wchar_t buf[1024];
									GetModuleFileNameW(hLib, buf, MAX_PATH);
									_w_sW("remove_genplug",buf);
									_w_i("show_prefs",34);
									PostMessage(hMainWindow,WM_USER,0,IPC_RESTARTWINAMP);
								}
							}
						}
					}
				}
				return FALSE;
			case IDC_DSPCONF:
				{
					if (g_safeMode) return FALSE;

					wchar_t b[1024];
					HINSTANCE hLib;
					//if (!*config_dspplugin_name) return FALSE;
					PathCombineW(b, DSPDIR, config_dspplugin_name);
					hLib = LoadLibraryW(b);
					if (hLib)
					{
						winampDSPGetHeaderType pr;
						winampDSPModule *module;
						pr = (winampDSPGetHeaderType) GetProcAddress(hLib,"winampDSPGetHeader2");
						module = pr(hMainWindow)->getModule(SendDlgItemMessage(hwndDlg,IDC_DSPMOD,CB_GETCURSEL,0,0));
						if (module)
						{
							module->hDllInstance = hLib;
							module->hwndParent = hMainWindow;
							if (!(config_no_visseh&2))
							{
								try
								{
									module->Config(module);
								}
								catch(...)
								{
									LPMessageBox(hwndDlg,IDS_PLUGINERROR,IDS_ERROR,MB_OK|MB_ICONEXCLAMATION);
								}
							}
							else
							{
								module->Config(module);
							}
						}
						else
						{
							LPMessageBox(hwndDlg,IDS_ERRORLOADINGPLUGIN,IDS_ERROR,MB_OK);
						}
						FreeLibrary(hLib);
					}
					else
					{
						LPMessageBox(hwndDlg,IDS_ERRORLOADINGPLUGIN,IDS_ERROR,MB_OK);
					}
				}
				return FALSE;
			case IDC_PLUGINVERS:
				myOpenURLWithFallback(hwndDlg, L"http://www.winamp.com/plugins?loadaddons=plugins", L"http://www.winamp.com/plugins");
				return TRUE;
		}
	}

	link_handledraw(hwndDlg,uMsg,wParam,lParam);
	return FALSE;
} //dsp
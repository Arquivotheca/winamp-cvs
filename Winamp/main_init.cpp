/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "menuv5.h"
#include "resource.h"

extern "C"
{
	int g_main_created = 0;
}

static INT_PTR CALLBACK tmpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

BOOL InitApplication(HINSTANCE hInstance)  // register Winamp's window class, returns TRUE on success
{
	HICON hIcon;
	WNDCLASS wc = {0, };

	wc.style = CS_DBLCLKS;
	wc.hInstance = hInstance;
	hIcon = wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_XP));
	wc.hCursor = NULL;
	
	//wc.lpfnWndProc = Main_WndProc;
	//	wc.lpszClassName = szAppName;
	//	if (!RegisterClass(&wc))
	//		return 0;

	wc.lpfnWndProc = VIS_WndProc;
	wc.lpszClassName = "WinampVis";
	if (!RegisterClass(&wc))
		return 0;


	wc.lpfnWndProc = video_WndProc;
	wc.lpszClassName = "Winamp Video";
	if (!RegisterClass(&wc))
		return 0;

#ifdef WA2
#ifndef NETSCAPE
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = MB_WndProc;
	wc.lpszClassName = "Winamp MB";
	if (!RegisterClass(&wc))
		return 0;
#endif
#endif

	/* now register unicode windows */
	WNDCLASSW wcW = {0, };

	wcW.style = CS_DBLCLKS;
	wcW.hInstance = hInstance;
	hIcon = wcW.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_XP));
	wcW.hCursor = NULL;

	wcW.lpfnWndProc = emb_WndProc;
	wcW.lpszClassName = L"Winamp Gen";
	if (!RegisterClassW(&wcW))
		return 0;

	wcW.style |= CS_VREDRAW | CS_HREDRAW;
	wcW.lpfnWndProc = EQ_WndProc;
	wcW.lpszClassName = L"Winamp EQ";
	if (!RegisterClassW(&wcW))
		return 0;

	wcW.lpfnWndProc = PE_WndProc;
	wcW.lpszClassName = L"Winamp PE";
	if (!RegisterClassW(&wcW))
		return 0;

	// make other people's dialogs show the winamp icon
	HWND h = CreateDialog(hMainInstance, MAKEINTRESOURCE(IDD_OPENLOC), NULL, tmpProc);
	SetClassLongPtr(h, GCLP_HICON, (LONG_PTR)hIcon);
	DestroyWindow(h);

	return 1;
}

void ConvertRatingMenuStar(HMENU menu, UINT menu_id)
{
MENUITEMINFOW mi = {sizeof(mi), MIIM_DATA | MIIM_TYPE, MFT_STRING};
wchar_t rateBuf[32], *rateStr = rateBuf;
	mi.dwTypeData = rateBuf;
	mi.cch = 32;
	if(GetMenuItemInfoW(menu, menu_id, FALSE, &mi))
	{
		while(rateStr && *rateStr)
		{
			if(*rateStr == L'*') *rateStr = L'\u2605';
			rateStr=CharNextW(rateStr);
		}
		SetMenuItemInfoW(menu, menu_id, FALSE, &mi);
	}
}

void main_Create()
{
	static int t = 0;

	g_main_created = 1;
	SetWindowTextW(hMainWindow, caption);

	if (GetPrivateProfileIntW(L"Winamp", L"PromptForRegKey", 0, INI_FILE))
	{
		verify_reginfo();
		WritePrivateProfileStringW(L"Winamp", L"PromptForRegKey", L"0", INI_FILE);
		if (!g_regver)
		{
			LPDialogBoxW(IDD_EDITREG, NULL, (DLGPROC)regEntryProc);
			verify_reginfo();
		}
	}

	hVisWindow = CreateWindowExW(0, L"WinampVis", L"", WS_CHILD, 0, 0, 0, 0,
	                            hMainWindow, NULL, hMainInstance, NULL);
	ShowWindow(hVisWindow, SW_SHOWNA);

	SetWindowLong(hMainWindow, GWLP_USERDATA, (config_keeponscreen&2) ? 0x49474541 : 0);
	hTooltipWindow = CreateWindowW(TOOLTIPS_CLASSW, (LPCWSTR)NULL, TTS_ALWAYSTIP | TTS_NOPREFIX,
								   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								   hMainWindow, NULL, hMainInstance, NULL);
	SendMessage(hTooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 375);
	SendMessage(hTooltipWindow, TTM_SETDELAYTIME, TTDT_AUTOPOP, 2000);
	SendMessage(hTooltipWindow, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);

	hEQTooltipWindow = CreateWindowW(TOOLTIPS_CLASSW, (LPCWSTR)NULL, TTS_ALWAYSTIP | TTS_NOPREFIX,
									 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
									 hEQWindow, NULL, hMainInstance, NULL);
	SendMessage(hEQTooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 125);
	SendMessage(hEQTooltipWindow, TTM_SETDELAYTIME, TTDT_AUTOPOP, 2000);
	SendMessage(hEQTooltipWindow, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);


	hVideoTooltipWindow = CreateWindowW(TOOLTIPS_CLASSW, (LPCWSTR)NULL, TTS_ALWAYSTIP | TTS_NOPREFIX,
										CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
										hVideoWindow, NULL, hMainInstance, NULL);
	SendMessage(hVideoTooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 125);
	SendMessage(hVideoTooltipWindow, TTM_SETDELAYTIME, TTDT_AUTOPOP, 2000);
	SendMessage(hVideoTooltipWindow, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);

	hPLTooltipWindow = CreateWindowW(TOOLTIPS_CLASSW, (LPCWSTR)NULL, TTS_ALWAYSTIP | TTS_NOPREFIX,
									 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
									 hPLWindow, NULL, hMainInstance, NULL);
	SendMessage(hPLTooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 125);
	SendMessage(hPLTooltipWindow, TTM_SETDELAYTIME, TTDT_AUTOPOP, 2000);
	SendMessage(hPLTooltipWindow, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);

	/*hPL2TooltipWindow = CreateWindowW(TOOLTIPS_CLASSW, (LPCWSTR)NULL, TTS_ALWAYSTIP | TTS_NOPREFIX,
									  0, 0, 0, 0, hPLWindow, NULL, hMainInstance, NULL);*/
	// TODO attempt to make this  more like the listview timings
	/*SendMessage(hPL2TooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 125);
	SendMessage(hPL2TooltipWindow, TTM_SETDELAYTIME, TTDT_AUTOPOP, 2000);
	SendMessage(hPL2TooltipWindow, TTM_SETDELAYTIME, TTDT_RESHOW, 1000);*/

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

	top_menu = LPLoadMenu(WINAMP_MAIN);

#ifdef NETSCAPE
	EnableMenuItem(top_menu, WINAMP_OPTIONS_MINIBROWSER, MF_BYCOMMAND | MF_GRAYED);
#endif
	main_menu = GetSubMenu(top_menu, 0);
	{
		g_submenus_play = GetSubMenu(main_menu, 2);

		char buf[32] = {0};
		MENUITEMINFOW i = {sizeof(i), MIIM_TYPE | MIIM_DATA | MIIM_ID, MFT_STRING, };
		i.wID = ID_MAIN_PLAY_AUDIOCD;

		DWORD drives = GetLogicalDrives();
		g_audiocdletters = 0;

		in_get_extended_fileinfo("cda://", "ext_cdda", buf, sizeof(buf));
		if (atoi(buf) > 0)
		{
			int need_sep = 1;
			for (int drivemask = 0; drivemask < 32; drivemask++)
			{
				if (drives&(1 << drivemask))
				{
					wchar_t str[64] = {0}, tmp[64] = {0};
					char c = ('A' + drivemask);
					StringCchPrintfW(str, 64, L"%c:\\", c);
					if (GetDriveTypeW(str) == DRIVE_CDROM)
					{
						if (need_sep)
						{
							MENUITEMINFO i2 = {sizeof(i2), MIIM_TYPE, MFT_SEPARATOR, };
							InsertMenuItem(g_submenus_play, 4, TRUE, &i2);
							need_sep = 0;
						}

						g_audiocdletter[g_audiocdletters] = c;
						i.dwTypeData = str;
						i.cch = wcslen(str);
						InsertMenuItemW(g_submenus_play, 4 + g_audiocdletters, TRUE, &i);
						i.wID++;
						g_audiocdletters++;
						if (g_audiocdletters == 4) break;
					}
				}
			}
		}
	}
	g_submenus_bookmarks1 = GetSubMenu(main_menu, 4);
	g_submenus_bookmarks2 = GetSubMenu(GetSubMenu(main_menu, 2), 4 + (g_audiocdletters ? g_audiocdletters + 1 : 0));
	g_submenus_skins1 = GetSubMenu(main_menu, MAINMENU_OPTIONS_BASE + 3);
	g_submenus_skins2 = GetSubMenu(GetSubMenu(main_menu, MAINMENU_OPTIONS_BASE), 1);
	g_submenus_vis = GetSubMenu(main_menu, MAINMENU_OPTIONS_BASE + 2);
	g_submenus_options = GetSubMenu(main_menu, MAINMENU_OPTIONS_BASE);

	if (config_wlz_menu)
	{
		MENUITEMINFOW mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE | MIIM_ID, MFT_STRING, };
		mii.hSubMenu = g_submenus_lang = CreatePopupMenu();
		mii.dwTypeData = getStringW(IDS_LANGUAGEPACKS_MENU, NULL, 0);
		mii.cch = wcslen(mii.dwTypeData);
		g_submenus_lang_id = mii.wID = unique_loword_command++;
		InsertMenuItemW(main_menu, MAINMENU_OPTIONS_BASE+4, TRUE, &mii);
	}

	HMENU rating_menu = GetSubMenu(GetSubMenu(GetSubMenu(top_menu,2),5),9);
	ConvertRatingMenuStar(rating_menu, ID_PL_RATING5);
	ConvertRatingMenuStar(rating_menu, ID_PL_RATING4);
	ConvertRatingMenuStar(rating_menu, ID_PL_RATING3);
	ConvertRatingMenuStar(rating_menu, ID_PL_RATING2);
	ConvertRatingMenuStar(rating_menu, ID_PL_RATING1);

	rating_menu = GetSubMenu(GetSubMenu(GetSubMenu(top_menu,3),0),5);
	ConvertRatingMenuStar(rating_menu, ID_RATING5);
	ConvertRatingMenuStar(rating_menu, ID_RATING4);
	ConvertRatingMenuStar(rating_menu, ID_RATING3);
	ConvertRatingMenuStar(rating_menu, ID_RATING2);
	ConvertRatingMenuStar(rating_menu, ID_RATING1);

	Skin_Random();
	Skin_Load();

	g_skinloadedmanually = 0;
	set_aot(0);
	set_taskbar();
	CheckMenuItem(main_menu, WINAMP_OPTIONS_EASYMOVE, config_easymove ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(main_menu, WINAMP_FILE_SHUFFLE, config_shuffle ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(main_menu, WINAMP_FILE_REPEAT, config_repeat ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetSubMenu(GetSubMenu(top_menu, 3), 13), ID_VIDEOWND_VERTICALLYFLIP, (config_video_fliprgb ? MF_CHECKED: MF_UNCHECKED));
	if (!config_timeleftmode)
		CheckMenuItem(main_menu, WINAMP_OPTIONS_ELAPSED, MF_CHECKED);
	else
		CheckMenuItem(main_menu, WINAMP_OPTIONS_REMAINING, MF_CHECKED);
	draw_setnoupdate(1);
	draw_init();
	draw_shuffle(config_shuffle, 0);
	draw_eqplbut(config_eq_open, 0, config_pe_open, 0);
	draw_repeat(config_repeat, 0);
	draw_eject(0);
	draw_buttonbar( -1);
	draw_volumebar(config_volume, 0);
	draw_panbar(config_pan, 0);
	draw_songname(L"", &t, 0);
	draw_playicon(2);
	draw_clutterbar(0);
	draw_monostereo(0);
	draw_tbar(1, config_windowshade, 0);
	draw_setnoupdate(0);
	InsertMenuW(GetSystemMenu(hMainWindow, FALSE), 0, MF_POPUP | MF_BYPOSITION, (UINT_PTR) main_menu, L"Winamp");
	InsertMenu(GetSystemMenu(hMainWindow, FALSE), 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	if (config_autoscrollname) SetTimer(hMainWindow, UPDATE_DISPLAY_TIMER + 1, 200, NULL);

	if (config_shuffle) PlayList_randpos( -BIGINT);
	dsp_init();

	newversioncheck();

	SetTimer(hMainWindow, STATS_TIMER, 1000, NULL);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) // create the main Winamp window
{
	int sc = config_minimized; // config_minimized gets trashed in WM_CREATE
	int style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CAPTION;
	int exStyle = WS_EX_ACCEPTFILES;
	deferring_show = 1;

	main_Create();
	/*
	if (!CreateWindowEx(exStyle,szAppName, caption,style,
					config_wx,config_wy,0,0, // WM_CREATE will size it
					NULL, NULL,hInstance,NULL)) return FALSE;
					*/
	out_setwnd();
	
	if (!CreateWindowExW(0, L"Winamp EQ", getStringW(IDS_EQCAPTION, NULL, 0), WS_OVERLAPPED,
	                    config_eq_wx, config_eq_wy, 0, 0,  // WM_CREATE will size it
	                    hMainWindow, NULL, hInstance, NULL)) return FALSE;
	
	if (!CreateWindowExW(WS_EX_ACCEPTFILES, L"Winamp PE", getStringW(IDS_PECAPTION, NULL, 0), WS_OVERLAPPED,
	                    config_pe_wx, config_pe_wy, 0, 0,  // WM_CREATE will size it
	                    hMainWindow, NULL, hInstance, NULL)) return FALSE;

	if (!video_Create())
		return FALSE;

	hPLVisWindow = CreateWindowExW(0, L"WinampVis", L"", WS_CHILD,
	                              0, 0, 0, 0,
	                              hPLWindow, NULL, hMainInstance, NULL);
	ShowWindow(hPLVisWindow, SW_SHOWNA);

	pe_startuphack = 1;

	if (config_eq_open)
	{
		config_eq_open = 0;
		eq_dialog(hMainWindow,sc);
	}
	//	MessageBox(NULL,"p4","0",0);
	if (config_pe_open)
	{
		config_pe_open = 0;
		pleditDlg(hMainWindow,sc);
	}
	if (config_video_open)
	{
		config_video_open = 0;
		ShowVideoWindow(sc);
	}
	//	MessageBox(NULL,"p5","0",0);
	if (!config_mw_open)
	{
		MoveOffscreen(hMainWindow);
		Ipc_WindowToggle(IPC_CB_WND_MAIN, config_mw_open);
		//config_mw_open=1;
		//SendMessage(hMainWindow,WM_COMMAND,WINAMP_MAIN_WINDOW,0);
	}
	else
		CheckMenuItem(main_menu, WINAMP_MAIN_WINDOW, MF_CHECKED);
	pe_startuphack = 0;

	/*
	Browser_init();
	if (config_si_open)
		OpenBrowser();
*/
	if (sc)
		g_showcode = SW_SHOWMINIMIZED;
	else if (nCmdShow == SW_SHOWMAXIMIZED || nCmdShow == SW_MAXIMIZE)
		g_showcode = SW_SHOWNORMAL;
	else
		g_showcode = nCmdShow;

	v5_top_menu = LPLoadMenu(WINAMP5_MENUBAR);

	if (config_wlz_menu)
	{
		MENUITEMINFOW mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE | MIIM_ID, MFT_STRING, };
		mii.hSubMenu = g_submenus_lang;
		mii.dwTypeData = getStringW(IDS_LANGUAGEPACKS_MENU, NULL, 0);
		mii.cch = wcslen(mii.dwTypeData);
		mii.wID = g_submenus_lang_id;
		InsertMenuItemW(GetSubMenu(v5_top_menu, 2), 1, TRUE, &mii);
		g_mm_ffoptionsbase_adj++;
	}

	SetTimer(hMainWindow, 200, 5, NULL); // defer main window showing

	BeginFullscreenAppMonitor();

	return TRUE;
}
#include "main.h"
#include "config.h"

void hideShowRecentItemsCheckboxes(HWND hwndDlg) 
{
	int enabled = IsDlgButtonChecked(hwndDlg, IDC_CHECK1);
	int wait_secs = !!g_config->ReadInt("recent_wait_secs",0);
	int wait_percent = !!g_config->ReadInt("recent_wait_percent",0);
	int wait_end = !!g_config->ReadInt("recent_wait_end",0);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK2), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK3), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK4), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK5), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK6), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK7), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), enabled && !!g_config->ReadInt("recent_limitd",1));
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC1), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC3), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC4), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC5), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC6), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT2), enabled && wait_secs);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT3), enabled && wait_percent);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK8), enabled && !(wait_end && !wait_secs && !wait_percent));
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC7), enabled && !(wait_end && !wait_secs && !wait_percent));
}

BOOL CALLBACK PrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static int need_ref;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			CheckDlgButton(hwndDlg,IDC_CHECK2,(g_config->ReadInt("recent_track",1)&1)?BST_CHECKED:0);
			CheckDlgButton(hwndDlg,IDC_CHECK3,(g_config->ReadInt("recent_track",1)&2)?0:BST_CHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK1,!!g_config->ReadInt("showrecentitems",1));
			CheckDlgButton(hwndDlg,IDC_CHECK4,!!g_config->ReadInt("recent_limitd",1));
			CheckDlgButton(hwndDlg,IDC_CHECK5,!!g_config->ReadInt("recent_wait_secs",0));
			CheckDlgButton(hwndDlg,IDC_CHECK6,!!g_config->ReadInt("recent_wait_percent",0));
			CheckDlgButton(hwndDlg,IDC_CHECK7,!!g_config->ReadInt("recent_wait_end",0));
			CheckDlgButton(hwndDlg,IDC_CHECK8,!!g_config->ReadInt("resumeplayback",0));
			SetDlgItemInt(hwndDlg,IDC_EDIT1,g_config->ReadInt("recent_limitnd",30),FALSE);
			SetDlgItemInt(hwndDlg,IDC_EDIT2,g_config->ReadInt("recent_wait_secs_lim",5),FALSE);
			SetDlgItemInt(hwndDlg,IDC_EDIT3,g_config->ReadInt("recent_wait_percent_lim",50),FALSE);
			need_ref=0;
			hideShowRecentItemsCheckboxes(hwndDlg);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_CHECK1:
				{
					int a=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK1);
					g_config->WriteInt("showrecentitems",a);
					if (a) history_init();
					else history_quit();
					hideShowRecentItemsCheckboxes(hwndDlg);
				}
					break;

				case IDC_EDIT1:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						BOOL t;
						int v=GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
						if (t) g_config->WriteInt("recent_limitnd",v);
						need_ref++;
					}
					break;

				case IDC_CHECK4:
					g_config->WriteInt("recent_limitd",!!IsDlgButtonChecked(hwndDlg,IDC_CHECK4));
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT1),!!g_config->ReadInt("recent_limitd",1));
					need_ref++;
					break;

				case IDC_CHECK2:
				case IDC_CHECK3:
					g_config->WriteInt("recent_track",(IsDlgButtonChecked(hwndDlg,IDC_CHECK2)?1:0) | (IsDlgButtonChecked(hwndDlg,IDC_CHECK3)?0:2));
					break;

				case IDC_CHECK5:
					g_config->WriteInt("recent_wait_secs",!!IsDlgButtonChecked(hwndDlg,IDC_CHECK5));
					hideShowRecentItemsCheckboxes(hwndDlg);
					break;

				case IDC_CHECK6:
					g_config->WriteInt("recent_wait_percent",!!IsDlgButtonChecked(hwndDlg,IDC_CHECK6));
					hideShowRecentItemsCheckboxes(hwndDlg);
					break;

				case IDC_CHECK7:
					g_config->WriteInt("recent_wait_end",!!IsDlgButtonChecked(hwndDlg,IDC_CHECK7));
					hideShowRecentItemsCheckboxes(hwndDlg);
					break;

				case IDC_CHECK8:
					g_config->WriteInt("resumeplayback",!!IsDlgButtonChecked(hwndDlg,IDC_CHECK8));
					break;

				case IDC_EDIT2:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						BOOL t;
						int v=GetDlgItemInt(hwndDlg,IDC_EDIT2,&t,FALSE);
						if (t)
						{
							if(v < 0)
							{
								v = 1;
								SetDlgItemInt(hwndDlg, IDC_EDIT2, v, 0);
							}
							g_config->WriteInt("recent_wait_secs_lim",v);
						}
						need_ref++;
					}
					break;

				case IDC_EDIT3:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						BOOL t;
						int v=GetDlgItemInt(hwndDlg,IDC_EDIT3,&t,FALSE);
						if(t)
						{
							int tweaked = 0;
							if(v > 99){
								v = 99;
								tweaked = 1;
							}
							else if(v < 1)
							{
								v = 1;
								tweaked = 1;
							}
							if(tweaked)
							{
								SetDlgItemInt(hwndDlg, IDC_EDIT3, v, 0);
							}

							g_config->WriteInt("recent_wait_percent_lim",v);
						}
						need_ref++;
					}
					break;
			};
			break;

		case WM_DESTROY:
			if (need_ref)
			{
				g_config->WriteInt("recent_limitlt",0); // make sure it gets refreshed
				// TODO: only do this if the history view is open
				PostMessage(lMedia.hwndLibraryParent,WM_USER+30,0,0);	
			}
			break;
	}
	return 0;
}
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


LRESULT CALLBACK regEntryProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int is_init;
	switch (uMsg)
	{
		case WM_INITDIALOG:
			is_init = 1;
			wchar_t config_regname[512], config_regkey[128];
			readwrite_reginfo(0, config_regname, config_regkey);
			SetDlgItemTextW(hwndDlg, IDC_NAME, config_regname);

			if (g_regver)
				ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), SW_SHOWNA);
			is_init = 0;
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					if (GetFocus() == GetDlgItem(hwndDlg, IDC_NAME))
					{
						SetFocus(GetDlgItem(hwndDlg, IDC_KEY));
						return 0;
					}
					if (GetFocus() != GetDlgItem(hwndDlg, IDC_KEY)) return 0;
				case IDC_REGISTER:
				{
					wchar_t config_regname[512], config_regkey[128];
					wchar_t config_regnamebk[512], config_regkeybk[128];
					readwrite_reginfo(0, config_regnamebk, config_regkeybk);
					GetDlgItemTextW(hwndDlg, IDC_NAME, config_regname, sizeof(config_regname)/sizeof(*config_regname));
					GetDlgItemTextW(hwndDlg, IDC_KEY, config_regkey, sizeof(config_regkey)/sizeof(*config_regkey));
					readwrite_reginfo(1, config_regname, config_regkey);

					verify_reginfo();
					if (g_regver) EndDialog(hwndDlg, 1);
					else
					{
						readwrite_reginfo(1, config_regnamebk, config_regkeybk);

						verify_reginfo();
						LPMessageBox(hwndDlg, IDS_P_INVALID_PRO_KEY, IDS_P_WINAMP_PRO_KEY, MB_OK | MB_ICONEXCLAMATION);
					}
				}
				break;
				case IDC_BUTTON1:
					readwrite_reginfo(1, L"", L"");
					verify_reginfo();
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;
			}
			return 0;
	}
	return 0;
}

INT_PTR CALLBACK RegProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND)
	{
		if (LOWORD(wParam) == IDC_WEBLINK)
		{
			//myOpenURL(hwndDlg, L"https://shop.winamp.com/servlet/ControllerServlet?Action=DisplayPage&Locale=en_US&SiteID=winamp&id=QuickBuyCartPage");
			//ShellExecuteW(hwndDlg, L"open", L"https://shop.winamp.com/servlet/ControllerServlet?Action=DisplayPage&Locale=en_US&SiteID=winamp&id=QuickBuyCartPage", NULL, NULL, SW_SHOWNORMAL);
			ShellExecuteW(hwndDlg, L"open", L"http://www.winamp.com/buy", NULL, NULL, SW_SHOWNORMAL);
		}
		if (LOWORD(wParam) == IDC_WEBLINK3)
		{
			//myOpenURL(hwndDlg, L"http://www.winamp.com/help/Player_Features");
			ShellExecuteW(hwndDlg, L"open", L"http://www.winamp.com/help/Player_Features", NULL, NULL, SW_SHOWNORMAL);
		}
		if (LOWORD(wParam) == IDC_WEBLINK2 || LOWORD(wParam) == IDC_WEBLINK4)
		{
			//myOpenURL(hwndDlg, L"https://shop.winamp.com/DRHM/servlet/ControllerServlet?Action=DisplayCustomerServiceOrderSearchPage&SiteID=winamp&Locale=en_US&Env=BASE");
			//ShellExecuteW(hwndDlg, L"open", L"https://shop.winamp.com/DRHM/servlet/ControllerServlet?Action=DisplayCustomerServiceOrderSearchPage&SiteID=winamp&Locale=en_US&Env=BASE", NULL, NULL, SW_SHOWNORMAL);
			ShellExecuteW(hwndDlg, L"open", L"http://www.winamp.com/recover", NULL, NULL, SW_SHOWNORMAL);
		}
		if (LOWORD(wParam) == IDC_BUTTON1)
		{
			wchar_t config_regname[512], config_regkey[128];
			wchar_t config_regnamebk[512], config_regkeybk[128];
			readwrite_reginfo(0, config_regnamebk, config_regkeybk);
			GetDlgItemTextW(hwndDlg, IDC_REGNAME, config_regname, sizeof(config_regname)/sizeof(*config_regname));
			GetDlgItemTextW(hwndDlg, IDC_REGKEY, config_regkey, sizeof(config_regkey)/sizeof(*config_regkey));
			readwrite_reginfo(1, config_regname, config_regkey);

			verify_reginfo();
			if (!g_regver) 
			{
				readwrite_reginfo(1, config_regnamebk, config_regkeybk);

				verify_reginfo();
				LPMessageBox(hwndDlg, IDS_P_INVALID_PRO_KEY, IDS_P_WINAMP_PRO_KEY, MB_OK | MB_ICONEXCLAMATION);
			}
		}
		if (LOWORD(wParam) == IDC_BUTTON4)
		{
			if ( LPMessageBox(hwndDlg, IDS_REG_REMOVAL_MESSAGE, IDS_P_WINAMP_PRO_KEY, MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				readwrite_reginfo(1, L"", L"");
				verify_reginfo();
			}
		}
	}
	if (uMsg == WM_INITDIALOG || (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_BUTTON1) || (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_BUTTON4))
	{
		wchar_t buf[512];
		wchar_t config_regname[512], config_regkey[128];
		readwrite_reginfo(0, config_regname, config_regkey);
		SetDlgItemTextW(hwndDlg, IDC_FRAME1, g_regver ? getStringW(IDS_P_WINAMP_PRO,NULL,0) : getStringW(IDS_P_PURCHASE_WINAMP_PRO,NULL,0));
		SetDlgItemTextW(hwndDlg, IDC_ITEXT1, g_regver ? getStringW(IDS_P_WINAMP_PRO_ENABLED,NULL,0) : getStringW(IDS_P_WINAMP_PRO_YOU_CAN_DO,NULL,0) );

		ShowWindow(GetDlgItem(hwndDlg, IDC_WEBLINK3), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_WEBLINK), g_regver?SW_HIDE:SW_SHOW);
		
		ShowWindow(GetDlgItem(hwndDlg, IDC_REGGROUP2), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC4), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REGNAME), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC10), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC5), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REGKEY), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC9), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_WEBLINK2), g_regver?SW_HIDE:SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), g_regver?SW_HIDE:SW_SHOW);

		ShowWindow(GetDlgItem(hwndDlg, IDC_REGGROUP1), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC7), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_TONAME), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC8), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REGSTATUS), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_WEBLINK4), g_regver?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), g_regver?SW_SHOW:SW_HIDE);

		link_startsubclass(hwndDlg, IDC_WEBLINK2);
		link_startsubclass(hwndDlg, IDC_WEBLINK3);
		link_startsubclass(hwndDlg, IDC_WEBLINK4);

		if (!g_regver)
		{
			SetDlgItemTextW(hwndDlg, IDC_REGNAME, config_regname);
			if (config_regname[0] && config_regkey[0])
				getStringW(IDS_P_INVALID_PRO_KEY,buf,256);
			else
				getStringW(IDS_P_NOT_REGISTERED,buf,256);
			if (!config_regname[0])
			{
				SetDlgItemTextW(hwndDlg, IDC_REGNAME, L"");
				SendMessage(GetDlgItem(hwndDlg, IDC_REGNAME), EM_SETCUEBANNER, 0, (LPARAM)buf);
			}
			SetDlgItemTextW(hwndDlg, IDC_REGKEY, L"");
			SendMessage(GetDlgItem(hwndDlg, IDC_REGKEY), EM_SETCUEBANNER, 0, (LPARAM)buf);
		}
		else 
		{
			SetDlgItemTextW(hwndDlg, IDC_TONAME, config_regname);
			if (g_regver == 1) 
				getStringW(IDS_P_VALID_5_0_KEY, buf, 256);
			else 
				getStringW(IDS_P_VALID_5_0_PLUS_KEY, buf, 256);
			SetDlgItemTextW(hwndDlg, IDC_REGSTATUS, buf);
		}
	}
	link_handledraw(hwndDlg, uMsg, wParam, lParam);
	return FALSE;
}
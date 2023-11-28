#include <windows.h>
#include "config.h"
#include "../winamp/commandlink.h"
#include "../nu/listview.h"
#include "main.h"
#include "api.h"
#include "resource.h"
#include "../nu/ns_wc.h"
#include "../nu/AutoChar.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <strsafe.h>

int config_allow_mode=CONFIG_MODE_ALL;
int config_video=0;
int config_collect=0;
int config_avtrack=0;
int config_awaken_on_load=0;
int config_listening_length=0;
int config_log=0;
int config_first=0;

wchar_t config_username[512]=L"";
wchar_t config_directories[16384]=L"";

void Config_Load()
{
	char iniPath[MAX_PATH];
	PathCombineA(iniPath, inidir, "plugins");
	PathAppendA(iniPath, "gen_mud.ini");

	AGAVE_API_AUTH->GetCredentials(ORGLER_AUTH_REALM, session_key, sizeof(session_key), token_a, sizeof(token_a), config_username, sizeof(config_username), &session_expiration);

	config_allow_mode=GetPrivateProfileIntA("gen_mud", "allow_mode", CONFIG_MODE_ALL, iniPath);
	config_video=GetPrivateProfileIntA("gen_mud", "video", 0, iniPath);
	if (config_video == -1) // fix-up old beta config files
		config_video = 0;
	config_collect=GetPrivateProfileIntA("gen_mud", "collect", 0, iniPath);
	config_avtrack=GetPrivateProfileIntA("gen_mud", "avtrack", 0, iniPath);
	config_awaken_on_load=GetPrivateProfileIntA("gen_mud", "awaken", 0, iniPath);
	config_listening_length=GetPrivateProfileIntA("gen_mud", "length", 0, iniPath);
	config_log=GetPrivateProfileIntA("gen_mud", "log", 0, iniPath);
	config_first=GetPrivateProfileIntA("gen_mud", "first", 0, iniPath);

	char temp[16384];
	GetPrivateProfileStringA("gen_mud", "directories", "", temp, sizeof(temp), iniPath);
	MultiByteToWideCharSZ(CP_UTF8, 0, temp, -1, config_directories, sizeof(config_directories)/sizeof(config_directories[0]));
}

void Config_Save()
{
	char iniPath[MAX_PATH];
	PathCombineA(iniPath, inidir, "plugins");
	PathAppendA(iniPath, "gen_mud.ini");

	AGAVE_API_AUTH->SetCredentials(ORGLER_AUTH_REALM, session_key, token_a, config_username, session_expiration);
	char temp[128];
	StringCbPrintfA(temp, sizeof(temp), "%d", config_allow_mode);
	WritePrivateProfileStringA("gen_mud", "allow_mode", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_video);
	WritePrivateProfileStringA("gen_mud", "video", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_collect);
	WritePrivateProfileStringA("gen_mud", "collect", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_avtrack);
	WritePrivateProfileStringA("gen_mud", "avtrack", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_awaken_on_load);
	WritePrivateProfileStringA("gen_mud", "awaken", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_listening_length);
	WritePrivateProfileStringA("gen_mud", "length", temp, iniPath);
	StringCbPrintfA(temp, sizeof(temp), "%d", config_log);
	WritePrivateProfileStringA("gen_mud", "log", temp, iniPath);
	WritePrivateProfileStringA("gen_mud", "directories", AutoChar(config_directories, CP_UTF8), iniPath);
	WritePrivateProfileStringA("gen_mud", "first", "0", iniPath);
}


static HWND config_hwnd = 0;

void Config_SyncLogin()
{
	if (NULL == config_hwnd)
		return;

	int status=GetLoginStatus();
	wchar_t *statusStr; 
	wchar_t *loginStr = MAKEINTRESOURCEW(IDS_MENU_LOGIN);
	BOOL enableTrack = FALSE;
	wchar_t szBuffer[256];
	
	switch(status)
	{
		case LOGIN_LOGGEDIN:
			{
				wchar_t szTemplate[256];
				WASABI_API_LNGSTRINGW_BUF(IDS_STATUS_LOGGEDIN_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
				StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szTemplate, config_username);
				statusStr = szBuffer;
				loginStr = MAKEINTRESOURCEW(IDS_MENU_LOGOUT);
				enableTrack = TRUE;
				
			}
			break;
		case LOGIN_NOTLOGGEDIN:
			statusStr = MAKEINTRESOURCEW(IDS_STATUS_NOTLOGGEDIN);
			break;
		case LOGIN_EXPIRING:
			statusStr = MAKEINTRESOURCEW(IDS_STATUS_EXPIRING);
			break;
		case LOGIN_EXPIRED:
			statusStr = MAKEINTRESOURCEW(IDS_STATUS_EXPIRED);
			break;
		default:
			return;
	}


	if (IS_INTRESOURCE(statusStr))
		statusStr = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)statusStr, szBuffer, ARRAYSIZE(szBuffer));
	SetDlgItemText(config_hwnd, IDC_LOGIN_STATUS, statusStr);

	if (IS_INTRESOURCE(loginStr))
		loginStr = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)loginStr, szBuffer, ARRAYSIZE(szBuffer));
	SetDlgItemText(config_hwnd, IDC_LOGIN, loginStr);
	ModifyMenu(ctrlmenu, login_menu_id, MF_BYCOMMAND|MF_STRING, login_menu_id, loginStr);

	EnableWindow(GetDlgItem(config_hwnd, IDC_AVTRACK), enableTrack);
	
}

void Config_SyncEnabled()
{
	if (config_collect)
		CheckMenuItem(ctrlmenu, enable_menu_id, MF_CHECKED|MF_BYCOMMAND);
	else
		CheckMenuItem(ctrlmenu, enable_menu_id, MF_UNCHECKED|MF_BYCOMMAND);

	if (config_hwnd)
	{
		CheckDlgButton(config_hwnd, IDC_COLLECT, (config_collect==1)?BST_CHECKED:BST_UNCHECKED);
		EnableWindow(GetDlgItem(config_hwnd, IDC_COLLECT_ALL), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_COLLECT_MEDIA_LIBRARY), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_COLLECT_DIRECTORIES), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_DIRECTORIES), (config_collect==1 && IsDlgButtonChecked(config_hwnd, IDC_COLLECT_DIRECTORIES)==BST_CHECKED)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_ADD), (config_collect==1 && IsDlgButtonChecked(config_hwnd, IDC_COLLECT_DIRECTORIES)==BST_CHECKED)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_EDIT), (config_collect==1 && IsDlgButtonChecked(config_hwnd, IDC_COLLECT_DIRECTORIES)==BST_CHECKED)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_REMOVE), (config_collect==1 && IsDlgButtonChecked(config_hwnd, IDC_COLLECT_DIRECTORIES)==BST_CHECKED)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_VIDEO), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_CHECK_SONGLENGTH), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_SONGLENGTH), (config_collect==1 && IsDlgButtonChecked(config_hwnd, IDC_CHECK_SONGLENGTH)==BST_CHECKED)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_STATIC4), (config_collect==1)?TRUE:FALSE);
		EnableWindow(GetDlgItem(config_hwnd, IDC_STATIC5), (config_collect==1)?TRUE:FALSE);
	}
}

static void FillDirectoriesLV(HWND hwndDlg)
{
	W_ListView directories_lv(hwndDlg, IDC_DIRECTORIES);

	directories_lv.Clear();
	wchar_t temp[MAX_PATH];
	const wchar_t *directories_itr = config_directories;
	while (directories_itr && *directories_itr)
	{
		size_t directory_len = 0;
		const wchar_t *end_delimiter = wcschr(directories_itr, L'|');
		if (end_delimiter)
		{
			directory_len = end_delimiter - directories_itr;
			end_delimiter++;
		}
		else
		{
			directory_len = wcslen(directories_itr);
		}
		StringCbCopyN(temp, sizeof(temp), directories_itr, directory_len*sizeof(wchar_t));
		directories_lv.AppendItem(temp, 0);
		directories_itr = end_delimiter;
	}
}

static void ReadDirectoriesLV(HWND hwndDlg)
{
	W_ListView directories_lv(hwndDlg, IDC_DIRECTORIES);

	config_directories[0]=0;
	wchar_t temp[MAX_PATH];
	wchar_t *directories_itr = config_directories;
	size_t directories_len = sizeof(config_directories);
	int count = directories_lv.GetCount();
	for (int i=0;i<count;i++)
	{
		if (i != 0)
			StringCbCatEx(directories_itr, directories_len, L"|", &directories_itr, &directories_len, 0);

		directories_lv.GetText(i, 0, temp, MAX_PATH);
		StringCbCatEx(directories_itr, directories_len, temp, &directories_itr, &directories_len, 0);
	}
}

static int CALLBACK BrowseCB(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		if (lpData)
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

static bool BrowserForFolder(HWND hwndDlg, wchar_t path[MAX_PATH], const wchar_t *start=0)
{
	BROWSEINFOW bi;
	ITEMIDLIST *idlist;
	wchar_t name[MAX_PATH], szTitle[128];
	bi.hwndOwner = hwndDlg;
	bi.pidlRoot = 0;
	bi.pszDisplayName = name;

	WASABI_API_LNGSTRINGW_BUF(IDS_BROWSEFOLDER_TITLE, szTitle, ARRAYSIZE(szTitle));
	bi.lpszTitle = szTitle;

	bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_EDITBOX|BIF_SHAREABLE|BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCB;
	bi.lParam = (LPARAM)start;
	idlist = SHBrowseForFolderW(&bi);
	if (idlist)
	{
		SHGetPathFromIDListW( idlist, path );
		CoTaskMemFree(idlist);
		//	WASABI_API_APP->path_setWorkingPath(path);
		return true;
	}
	else
	{
		path[0]=0;
		return false;
	}		
}

INT_PTR WINAPI PreferencesDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			config_hwnd = hwndDlg;
			Config_SyncLogin();

			CheckDlgButton(hwndDlg, IDC_COLLECT, (config_collect==1)?BST_CHECKED:BST_UNCHECKED);

			switch(config_allow_mode)
			{
			case CONFIG_MODE_ALL:
				CheckDlgButton(hwndDlg, IDC_COLLECT_ALL, BST_CHECKED);
				break;
			case CONFIG_MODE_MEDIA_LIBRARY:
				CheckDlgButton(hwndDlg, IDC_COLLECT_MEDIA_LIBRARY, BST_CHECKED);
				break;
			case CONFIG_MODE_DIRECTORIES:
				CheckDlgButton(hwndDlg, IDC_COLLECT_DIRECTORIES, BST_CHECKED);
				break;
			}

			CheckDlgButton(hwndDlg, IDC_VIDEO, config_video?BST_UNCHECKED:BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_AVTRACK, (config_avtrack==1)?BST_CHECKED:BST_UNCHECKED);

			/* listening length controls */
			SendDlgItemMessage(hwndDlg,IDC_SONGLENGTH,TBM_SETRANGE,0,MAKELONG(50, 100));

			if (config_listening_length >= 50 && config_listening_length <= 90)
			{
				CheckDlgButton(hwndDlg, IDC_CHECK_SONGLENGTH, BST_CHECKED);
				SendDlgItemMessage(hwndDlg,IDC_SONGLENGTH,TBM_SETPOS,1,config_listening_length);
			}
			else
			{
				CheckDlgButton(hwndDlg, IDC_CHECK_SONGLENGTH, BST_UNCHECKED);
				SendDlgItemMessage(hwndDlg,IDC_SONGLENGTH,TBM_SETPOS,1,50);				
			}
			/* directory whitelist controls */
			FillDirectoriesLV(hwndDlg);

			EnableWindow(GetDlgItem(hwndDlg, IDC_COLLECT_ALL), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLLECT_MEDIA_LIBRARY), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLLECT_DIRECTORIES), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTORIES), (config_collect==1 && config_allow_mode==CONFIG_MODE_DIRECTORIES)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), (config_collect==1 && config_allow_mode==CONFIG_MODE_DIRECTORIES)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), (config_collect==1 && config_allow_mode==CONFIG_MODE_DIRECTORIES)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), (config_collect==1 && config_allow_mode==CONFIG_MODE_DIRECTORIES)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VIDEO), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_SONGLENGTH), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SONGLENGTH), (config_collect==1 && IsDlgButtonChecked(hwndDlg, IDC_CHECK_SONGLENGTH)==BST_CHECKED)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC4), (config_collect==1)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC5), (config_collect==1)?TRUE:FALSE);
		}
		break;
	case WM_HSCROLL:
		{
			HWND swnd = (HWND) lParam;
			if (swnd == GetDlgItem(hwndDlg, IDC_SONGLENGTH))
			{
				config_listening_length = (int)SendDlgItemMessage(hwndDlg,IDC_SONGLENGTH,TBM_GETPOS,0,0);
			}
		}
		break;
	case WM_NOTIFY:
		{
			NMHDR *pnmh = (NMHDR *)lParam;
			switch(wParam)
			{
			case IDC_CREATE_ACCOUNT:
				if (NM_CLICK == pnmh->code)
					OpenUrl(ACCOUNT_CREATION_URL, true);
				return TRUE;
			case IDC_MANAGE_PROFILE:
				if (NM_CLICK == pnmh->code)
				{
					wchar_t szBuffer[4096];
					LPCWSTR navigateUrl;
					if ( NULL != AGAVE_API_AUTH && 
						0 == AGAVE_API_AUTH->ClientToWeb(ORGLER_AUTH_REALM,USER_PROFILE_DESTURL,szBuffer,ARRAYSIZE(szBuffer)) )
						navigateUrl = szBuffer;
					else 
						navigateUrl = USER_PROFILE_URL;						
					
					OpenUrl(navigateUrl, true);
				}
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ADD:
			{
				W_ListView directories_lv(hwndDlg, IDC_DIRECTORIES);
				wchar_t path[MAX_PATH];
				if (BrowserForFolder(directories_lv.getwnd(), path))
					directories_lv.AppendItem(path, 0);
				ReadDirectoriesLV(hwndDlg);
			}
			break;
		case IDC_EDIT:
			{
				W_ListView directories_lv(hwndDlg, IDC_DIRECTORIES);
				int selected_item = directories_lv.GetNextSelected();
				if (selected_item != -1)
				{
					wchar_t start[MAX_PATH]=L"";
					directories_lv.GetText(selected_item, 0, start, MAX_PATH);
					wchar_t path[MAX_PATH];
					if (BrowserForFolder(directories_lv.getwnd(), path, *start?start:0))
						directories_lv.SetItemText(selected_item, 0, path);
				}
				ReadDirectoriesLV(hwndDlg);
			}
			break;
		case IDC_REMOVE:
			{
				W_ListView directories_lv(hwndDlg, IDC_DIRECTORIES);
				int selected_item=-1;
				while ((selected_item = directories_lv.GetNextSelected(selected_item)) != -1)
				{
					directories_lv.DeleteItem(selected_item);
				}
				ReadDirectoriesLV(hwndDlg);
			}
			break;
		case IDC_CHECK_SONGLENGTH:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SONGLENGTH)==BST_CHECKED)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_SONGLENGTH), TRUE);
					config_listening_length = (int)SendDlgItemMessage(hwndDlg,IDC_SONGLENGTH,TBM_GETPOS,0,0);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_SONGLENGTH), FALSE);
					config_listening_length = 0;					
				}
			}
			break;

		case IDC_COLLECT_ALL:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				config_allow_mode=CONFIG_MODE_ALL;
				EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTORIES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), FALSE);
			}
			break;

		case IDC_COLLECT_MEDIA_LIBRARY:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				config_allow_mode=CONFIG_MODE_MEDIA_LIBRARY;
				EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTORIES), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), FALSE);
			}
			break;

		case IDC_COLLECT_DIRECTORIES:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				config_allow_mode=CONFIG_MODE_DIRECTORIES;
				EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTORIES), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), TRUE);
			}
			break;

		case IDC_LOGIN:
			if (GetLoginStatus() == LOGIN_LOGGEDIN)
				Logout();
			else
			{
				Login(hwndDlg, session_key, 512, token_a, 512);
				EnableOrgling_AfterLogin();
			}
			Config_SyncLogin();
			break;

		case IDC_COLLECT:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				config_collect = IsDlgButtonChecked(hwndDlg, IDC_COLLECT)==BST_CHECKED;
				Config_SyncEnabled();				
			}
			break;

		case IDC_AVTRACK:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_AVTRACK)==BST_CHECKED)
				{
					config_avtrack = 1;
				}
				else
				{
					config_avtrack = 0;
				}
			}
			break;

		case IDC_VIDEO:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_VIDEO)==BST_CHECKED)
				{
					config_video = 0;
				}
				else
				{
					config_video = 1;
				}
			}
			break;

		}
		break;
	case WM_DESTROY:
		config_hwnd = 0;
		break;
	}
	return 0;
}

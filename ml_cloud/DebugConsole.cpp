#include "api.h"
#include "main.h"
#include "resource.h"
#include "BackgroundTasks.h"
#include "nswasabi/ReferenceCounted.h"
#include "../replicant/nu/PtrList.h"
#include "../ml_pmp/pmp.h"
#define REPLICANT_COMPAT
// this allows us to include this without compile issues
#include "../nu/MediaLibraryInterface.h"
#include <commdlg.h>
#include <Shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
extern Cloud_Background cloud_background;

#define WINAMP_SHOWLIBRARY 40379

HWND debug_console_window = 0, list_view = 0,
	 invalid_view = 0, tab_window = 0,
	 child_window = 0, progress_window = 0,
	 status_window = 0, status_progress_window = 0;
typedef nu::PtrList<wchar_t> log_array;
log_array log_msg, log_time;
size_t first_log = 0, num_ignored_ids = 0;
static UINT log_id, ignore_id;
static UINT_PTR timer_id;
nx_string_t current_device_name, *filenames = 0;
int logMode = 3, debug_tab_sel = 0, need_col_resize = 0,
	last_show = 0, last_position = 0, last_string_id = 0,
	*ignored_ids = 0, ignore_loaded = 0;

void DebugConsole_SetStatus(const wchar_t *status/*, bool static_text*/)
{
	if (status && *status)
	{
		SYSTEMTIME system_time;
		wchar_t time_string[MAX_PATH], item_string[MAX_PATH];
		GetLocalTime(&system_time);
		GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, time_string, MAX_PATH);
		StringCchPrintf(item_string, MAX_PATH, L"%s.%03d", time_string, system_time.wMilliseconds);
		log_time.push_back(_wcsdup(item_string));
		log_msg.push_back(/*(static_text ? (wchar_t *)status : */_wcsdup(status)/*)*/);

		if (IsWindow(debug_console_window) && log_id > 65536)
			PostMessage(debug_console_window, log_id, 0, 0);
	}
}

void DebugConsole_ShowProgess(int show)
{
	if (IsWindow(progress_window))
	{
		ShowWindow(progress_window, !!show);
		DWORD dwStyle = GetWindowLong(progress_window, GWL_STYLE);
		if (show >= 0)
		{
			if (dwStyle & PBS_MARQUEE) SetWindowLongPtrW(progress_window, GWL_STYLE, dwStyle - PBS_MARQUEE);

			SendMessage(progress_window, PBM_SETMARQUEE, 0, 100);
			SendMessage(progress_window, PBM_SETRANGE32, 0, show);
		}
		else
		{
			if (!(dwStyle & PBS_MARQUEE)) SetWindowLongPtrW(progress_window, GWL_STYLE, dwStyle | PBS_MARQUEE);

			SendMessage(progress_window, PBM_SETMARQUEE, (show == -1), 100);
			SendMessage(progress_window, PBM_SETRANGE32, 0, (show == -1));
		}
	}

	if (IsWindow(status_progress_window))
	{
		DWORD dwStyle = GetWindowLong(status_progress_window, GWL_STYLE);
		if (show >= 0)
		{
			if (dwStyle & PBS_MARQUEE) SetWindowLongPtrW(status_progress_window, GWL_STYLE, dwStyle - PBS_MARQUEE);

			SendMessage(status_progress_window, PBM_SETMARQUEE, 0, 100);
			SendMessage(status_progress_window, PBM_SETRANGE32, 0, show);
		}
		else
		{
			if (!(dwStyle & PBS_MARQUEE)) SetWindowLongPtrW(status_progress_window, GWL_STYLE, dwStyle | PBS_MARQUEE);

			SendMessage(status_progress_window, PBM_SETRANGE32, 0, (show == -1));
			SendMessage(status_progress_window, PBM_SETMARQUEE, (show == -1), 100);
		}
	}

	if (!show) StatusWindow_Message(0);

	last_show = show;
	last_position = show;
}

void DebugConsole_UpdateProgress(int position, int show)
{
	DebugConsole_ShowProgess(show);

	if (IsWindow(progress_window))
	{
		PostMessage(progress_window, PBM_SETPOS, position, 0);
	}

	if (IsWindow(status_progress_window))
	{
		PostMessage(status_progress_window, PBM_SETPOS, position, 0);
	}

	last_show = show;
	last_position = position;

	StatusWindow_DetailsMessage();
}

BOOL CALLBACK UploadProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void DebugConsole_AddLogMessage()
{
	ListView_SetItemCountEx(list_view, log_msg.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	ListView_RedrawItems(list_view, 0, 0);

	if (!need_col_resize)
	{
		ListView_SetColumnWidth(list_view, 0, (ListView_GetItemCount(list_view) > 0 ? LVSCW_AUTOSIZE : LVSCW_AUTOSIZE_USEHEADER));
		ListView_SetColumnWidth(list_view, 0, ListView_GetColumnWidth(list_view, 0) + 3);
		RECT list_rect;
		GetClientRect(list_view, &list_rect);
		ListView_SetColumnWidth(list_view, 1, (list_rect.right - list_rect.left) - ListView_GetColumnWidth(list_view, 0));
		// ideally need to get this to work out the number of items before scrollbars are needed (cannot remember if there's an api for it)
		need_col_resize = (ListView_GetItemCount(list_view) > 10);
	}

	// scrolls last item into view unless there's a selection
	if (!ListView_GetSelectedCount(list_view))
		ListView_EnsureVisible(list_view, log_msg.size() - 1, FALSE);
}

bool IsVista()
{
	static INT fVista = -1; 
	
	if (-1 == fVista) 
	{
		OSVERSIONINFO osver;
		osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
		fVista = ( ::GetVersionEx(&osver) && osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 	(osver.dwMajorVersion >= 6 )) ? 1 : 0;
	}		

	return (1 == fVista);
}

void DebugConsole_RefreshIgnoredFiles(HWND hwndDlg)
{
	ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
	if (db_connection)
	{
		if (ignored_ids)
		{
			free(ignored_ids);
			ignored_ids = 0;
		}
		if (filenames)
		{
			free(filenames);
			filenames = 0;
		}
		num_ignored_ids = 0;

		db_connection->IDMap_Get_Ignored(&ignored_ids, &filenames, &num_ignored_ids);
		ignore_loaded = 1;
		ListView_SetItemCountEx(invalid_view, num_ignored_ids, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		ListView_RedrawItems(invalid_view, 0, num_ignored_ids);

		wchar_t title[128];
		StringCchPrintf(title, 128, WASABI_API_LNGSTRINGW((num_ignored_ids > 0 ? IDS_FILES_NOT_COMPATIBLE : IDS_INCOMPATIBLE_FILES)), num_ignored_ids);
		SetDlgItemText(child_window, IDC_EXCLUDED_COUNT, title);

		RECT list_rect;
		GetClientRect(invalid_view, &list_rect);
		ListView_SetColumnWidth(invalid_view, 0, (list_rect.right - list_rect.left));
		if (!IsVista()) EnableWindow(invalid_view, (num_ignored_ids > 0));
		EnableWindow(GetDlgItem(child_window, IDC_RESET_IGNORED), num_ignored_ids > 0);
	}
}

void DebugConsole_ClearLog(HWND hwndDlg)
{
	wchar_t base_path[MAX_PATH+1];
	PathCombine(base_path, Logger::base_path->string, L"*");
	size_t len = wcslen(base_path);
	base_path[len+1] = 0;

	SHFILEOPSTRUCT shop = {0};
	shop.hwnd = hwndDlg;
	if (!hwndDlg) shop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
	shop.wFunc = FO_DELETE;
	shop.pFrom = base_path;
	int ret = SHFileOperation(&shop);
	if (!shop.fAnyOperationsAborted)
	{
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(!ret ? IDS_CLEARED_LOGS : IDS_ERROR_CLEARING_LOGS));
	}
}

static char utf8tmp2[512];
int ConvertToUTF8(const wchar_t *str) {
	memset(utf8tmp2, 0, sizeof(utf8tmp2));
	return WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8tmp2, 512, 0, 0);
}

void DebugConsole_WriteLogLine(HANDLE handle, wchar_t *buffer, int buffer_len)
{
	DWORD written = 0;
	int str_len = ConvertToUTF8(buffer) - 1;
	if (str_len > 0)
	{
		WriteFile(handle, utf8tmp2, str_len, &written, 0);
	}
}

void DebugConsole_SaveLog(HWND hwndDlg)
{
	wchar_t filepath[MAX_PATH] = {0},
			file[MAX_PATH] = {L"cloud_log.txt"},
			filter[64] = {0};

	if (log_msg.size() > 0)
	{
		OPENFILENAMEW ofn = {0};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndDlg;

		WASABI_API_LNGSTRINGW_BUF(IDS_ALL_FILES, filter, 64);
		wchar_t * ptr = filter;
		while(*ptr)
		{
			if (*ptr == L'|') *ptr=0;
			ptr++;
		}

		mediaLibrary.BuildPath(L"Cloud", filepath, MAX_PATH);

		ofn.lpstrFilter = filter;
		ofn.lpstrInitialDir = filepath;
		ofn.lpstrFile = file;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = L"txt";
		if (GetSaveFileNameW(&ofn))
		{
			HANDLE handle = CreateFileW(file, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
			if (handle != INVALID_HANDLE_VALUE) {
				SetFilePointer(handle, 0, NULL, FILE_BEGIN);

				uint8_t BOM[3] = {0xEF, 0xBB, 0xBF};
				DWORD written = 0;
				WriteFile(handle, BOM, 3, &written, NULL);

				if (WASABI2_API_APP)
				{
					wchar_t buffer[1024] = {0};
					ReferenceCountedNXString product_shortname, winamp_version;
					WASABI2_API_APP->GetProductShortName(&product_shortname);
					WASABI2_API_APP->GetVersionString(&winamp_version);

					StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"Using: %s v%s.%d [Cloud Build v%s]\r\n",
																product_shortname->string, winamp_version->string,
																WASABI2_API_APP->GetBuildNumber(), PLUGIN_VERSION);
					DebugConsole_WriteLogLine(handle, buffer, -1);
				}

				for (size_t i = 0; i < log_msg.size(); i++)
				{
					wchar_t buffer[1024] = {0};
					StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"\r\n%s - %s", log_time[i], log_msg[i]);
					DebugConsole_WriteLogLine(handle, buffer, -1);
				}

				SetEndOfFile(handle);
				CloseHandle(handle);
			}
		}
	}
}

#include <uxtheme.h>
int isthemethere = 0;
static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if (uMsg == WM_SETCURSOR) {
		if ((HWND)wParam == hwndDlg) {
			if (!link_hand_cursor) {
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id) {
	HWND ctrl = GetDlgItem(hwndDlg, id);
	if (!GetPropW(ctrl, L"link_proc")) {
		SetPropW(ctrl, L"link_proc", (HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

BOOL link_handledraw(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_DRAWITEM) {
		DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT *)lParam;
		if (di->CtlType == ODT_BUTTON) {
			wchar_t wt[123];
			int y;
			RECT r;

			// due to the fun of theming and owner drawing we have to get the background colour
			if (isthemethere){
				HTHEME hTheme = OpenThemeData(hwndDlg, L"Tab");
				if (hTheme) {
					DrawThemeParentBackground(di->hwndItem, di->hDC, &di->rcItem);
					CloseThemeData(hTheme);
				}
			}

			HPEN hPen, hOldPen;
			GetDlgItemTextW(hwndDlg, wParam, wt, ARRAYSIZE(wt));

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
			return TRUE;
		}
	}
	return FALSE;
}

static void DebugConsole_Init(HWND hwndDlg, int mode)
{
	switch (mode)
	{
		case 0:
		{
			bool logged_in = (REPLICANT_API_CLOUD && REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) == NErr_Success) && !auth_error;
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();

			if (logged_in && db_connection && db_connection->Devices_GetName(local_device_id, &current_device_name, 0) == NErr_Success && first_pull)
			{
				SetDlgItemText(hwndDlg, IDC_CURRENT_NAME, current_device_name->string);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CURRENT_NAME), FALSE);
			}

			// show a message if we're not showing as currently logged in
			ShowWindow(GetDlgItem(child_window, IDC_LOGGED_IN), logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_NOT_LOGGED_IN), !logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_MANAGE_HERE), logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_LOGIN_HERE), !logged_in);

			if (!logged_in)
			{
				SetDlgItemTextW(child_window, IDC_CLOUD_ACCOUNT, WASABI_API_LNGSTRINGW(IDS_CLOUD_ACCOUNT));
				SetDlgItemText(child_window, IDC_CURRENT_NAME, L"");
			}
			else
			{
				wchar_t user[320] = {0}, username[256] = {0};
				Config_GetFriendlyUsername(username, 256);
				StringCchPrintfW(user, 320, WASABI_API_LNGSTRINGW(IDS_CLOUD_ACCOUNT_USER), username);
				SetDlgItemTextW(child_window, IDC_CLOUD_ACCOUNT, user);
			}

			CheckDlgButton(hwndDlg, IDC_SHOW_LOCAL_DEVICE, !!Config_GetShowLocal());

			isthemethere = !SendMessage(plugin.hwndWinampParent, WM_WA_IPC, IPC_ISWINTHEMEPRESENT, IPC_USE_UXTHEME_FUNC);
			link_startsubclass(hwndDlg, IDC_LOGIN_HERE);
			link_startsubclass(hwndDlg, IDC_MANAGE_HERE);

			EnableWindow(GetDlgItem(hwndDlg, IDC_RESET), first_pull);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESCAN), first_pull);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_LOCAL_DEVICE), first_pull);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CURRENT_NAME), first_pull);
		}
		break;

		case 1:
		{
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection)
			{
				if(db_connection->Info_GetLogging(&logMode) != NErr_Success)
				{
					db_connection->Info_SetLogging((logMode = 3));
				}
			}
			if (logMode & 1) CheckDlgButton(hwndDlg, IDC_LOGGING, BST_CHECKED);
			if (logMode & 2) CheckDlgButton(hwndDlg, IDC_LOGFAILEDONLY, BST_CHECKED);
			if (logMode & 4) CheckDlgButton(hwndDlg, IDC_LOGBINARY, BST_CHECKED);
			if (logMode & 8) CheckDlgButton(hwndDlg, IDC_LOGCLEARSTARTUP, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGFAILEDONLY), (logMode & 1));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGBINARY), (logMode & 1));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGCLEARSTARTUP), (logMode & 1));

			progress_window = GetDlgItem(hwndDlg, IDC_PROGRESS);
			DebugConsole_UpdateProgress(last_position, last_show);

			need_col_resize = 0;
			list_view = GetDlgItem(hwndDlg, IDC_LIST1);

			LVCOLUMN lvc = {0};
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_TIME);
			lvc.mask = LVCF_TEXT;
			lvc.cchTextMax = lstrlen(lvc.pszText);
			ListView_InsertColumn(list_view, 0, &lvc);
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_MESSAGE);
			ListView_InsertColumn(list_view, 1, &lvc);

			ListView_SetExtendedListViewStyle(list_view, LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);

			first_log = 0;
			DebugConsole_AddLogMessage();

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(list_view, TRUE);
		}
		break;

		case 2:
		{
			int add_pl_to_cloud = GetPrivateProfileIntW(L"gen_ml_config", L"cloud_always", 1, ini_file);
			CheckDlgButton(hwndDlg, IDC_ADD_PL_TO_CLOUD, add_pl_to_cloud);

			SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_ALWAYS_PROMPT));
			SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_KEEP_PLAYLIST));
			SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRINGW(IDS_REMOVE_THE_PLAYLIST));
			SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_SETCURSEL,Config_GetPlRemoveMode(),0);
		}
		break;

		case 3:
		{
			invalid_view = GetDlgItem(hwndDlg, IDC_LIST2);
			ignore_loaded = 0;

			SetDlgItemText(child_window, IDC_EXCLUDED_COUNT, WASABI_API_LNGSTRINGW(IDS_FILES_NOT_COMPATIBLE_LOADING));

			LVCOLUMN lvc = {0};
			lvc.pszText = WASABI_API_LNGSTRINGW(IDS_FILEPATH);
			lvc.mask = LVCF_TEXT;
			lvc.cchTextMax = lstrlen(lvc.pszText);
			ListView_InsertColumn(invalid_view, 0, &lvc);

			ListView_SetExtendedListViewStyle(invalid_view, LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
			DebugConsole_UpdateIgnoredFiles();

			if (NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(invalid_view, TRUE);
		}
		break;
	}
}

static void DebugConsole_Reset(HWND hwndDlg)
{
	wchar_t title[64];
	WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_RESET, title, 64);

	if (cloud_client)
	{
		if (MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_RESET_LIBRARY_MESSAGE), title, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_RESET));
			ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
			if (db_connection)
			{
				DebugConsole_SetStatus(L"Reset");
				ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
				if (db_connection)
				{
					db_connection->Info_GetRevision((int64_t *)&revision);
					if (cloud_client->Reset() != NErr_Success)
					{
						MessageBox(hwndDlg, L"Unable to initiate reset", L"Cloud Reset", MB_OK | MB_ICONEXCLAMATION);
					}
				}
				else
				{
					MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UNABLE_DO_RESET), title, MB_OK | MB_ICONEXCLAMATION);
				}
			}
		}
	}
	else
	{
		MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UNABLE_DO_RESET), title, MB_OK | MB_ICONEXCLAMATION);
	}
}

void DebugConsole_GetRevision(int64_t revision)
{
	int64_t cur_revision = 0;
	if (revision >= 0)
	{
		wchar_t msg[128];
		ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
		if (db_connection)
		{
			db_connection->Info_GetRevision((int64_t *)&cur_revision);
			StringCchPrintf(msg, 128, L"Current revision: %lld\nStored revision: %lld", revision, cur_revision);
			MessageBox(child_window, msg, L"Cloud Revision", MB_OK);
		}
		else
		{
			MessageBox(child_window, L"Unable to query the revision from the server.", L"Cloud Revision", MB_OK);
		}
	}
}

bool SetCredentials()
{
	wchar_t auth_token[256] = {0}, username[256] = {0}, provider[256] = {0};
	Config_GetAuthToken(auth_token, 256);
	Config_GetUsername(username, 256);
	Config_GetProvider(provider, 256);

	ReferenceCountedNXString nx_username, nx_auth_token, nx_provider;
	if (NXStringCreateWithUTF16(&nx_username, username) == NErr_Success
		&& NXStringCreateWithUTF16(&nx_auth_token, auth_token) == NErr_Success
		&& NXStringCreateWithUTF16(&nx_provider, provider) == NErr_Success)
	{
		if (REPLICANT_API_CLOUD)
			REPLICANT_API_CLOUD->SetCredentials(nx_username, nx_auth_token, nx_provider);

		if (IsWindow(GetDlgItem(child_window, IDC_SHOW_LOCAL_DEVICE)))
		{
			// show a message if we're not showing as currently logged in
			bool logged_in = (REPLICANT_API_CLOUD && REPLICANT_API_CLOUD->GetCredentials(0, 0, 0) == NErr_Success) && !auth_error;
			ShowWindow(GetDlgItem(child_window, IDC_LOGGED_IN), logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_NOT_LOGGED_IN), !logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_MANAGE_HERE), logged_in);
			ShowWindow(GetDlgItem(child_window, IDC_LOGIN_HERE), !logged_in);

			if (!logged_in)
			{
				first_pull = 0;
				SetDlgItemTextW(child_window, IDC_CLOUD_ACCOUNT, WASABI_API_LNGSTRINGW(IDS_CLOUD_ACCOUNT));
				if (IsWindow(tab_window))
				{
					TabCtrl_DeleteItem(tab_window, 4);
				}
				SetDlgItemText(child_window, IDC_CURRENT_NAME, L"");
			}
			else
			{
				wchar_t user[320] = {0}, username[256] = {0};
				Config_GetFriendlyUsername(username, 256);
				StringCchPrintfW(user, 320, WASABI_API_LNGSTRINGW(IDS_CLOUD_ACCOUNT_USER), username);
				SetDlgItemTextW(child_window, IDC_CLOUD_ACCOUNT, user);
			}

			EnableWindow(GetDlgItem(child_window, IDC_RESET), first_pull);
			EnableWindow(GetDlgItem(child_window, IDC_RESCAN), first_pull);
			EnableWindow(GetDlgItem(child_window, IDC_SHOW_LOCAL_DEVICE), first_pull);
			EnableWindow(GetDlgItem(child_window, IDC_CURRENT_NAME), first_pull);

			InvalidateRect(child_window, NULL, FALSE);
			InvalidateRect(GetDlgItem(debug_console_window, IDC_DEV_MODE), NULL, FALSE);
		}
	}

	// try to kick things off or halt as needed
	if (!(auth_token[0] && username[0] && provider[0]) ||
		(auth_error && (auth_token[0] && username[0] && provider[0])))
	{
		cloud_background.CredentialsChanged(auth_error);
		return false;
	}

	// once we've made a successful login attempt
	// then we can show the status window for them
	if (!first_login)
	{
		ToggleStatusWindow(1);
		first_login = 1;
		Config_SetFirstLogin();
	}
	return true;
}

void DebugConsole_UpdateIgnoredFiles()
{
	if (IsWindow(debug_console_window) && ignore_id > 65536)
		PostMessage(debug_console_window, ignore_id, 0, 0);
}

void DebugConsole_ResetIgnoredFiles(HWND hwndDlg)
{
	wchar_t title[128];
	WASABI_API_LNGSTRINGW_BUF(IDS_CLOUD_IGNORED_FILES, title, 128);

	DebugConsole_RefreshIgnoredFiles(hwndDlg);

	int ret = MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_RESET_IGNORED_MESSAGE), title, MB_YESNOCANCEL | MB_DEFBUTTON1 | MB_ICONQUESTION);
	if (ret != IDCANCEL)
	{
		DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_RESET_IGNORED_FILES));
		ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
		if (!db_connection || db_connection->IDMap_ResetIgnored() != NErr_Success)
		{
			MessageBox(debug_console_window, WASABI_API_LNGSTRINGW(IDS_RESET_IGNORED_FAILED), title, MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			if (ret == IDYES)
			{
				DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_START_LOCAL_LIBRARY_RESCAN));
				cloud_background.Rescan(-1);
			}
		}
		DebugConsole_UpdateIgnoredFiles();
	}
}

void CALLBACK DebugConsoler_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, ignore_id);
	DebugConsole_RefreshIgnoredFiles(hwnd);
}

void DoWinXPStyle(HWND tab, int style){
typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);
ENABLETHEMEDIALOGTEXTURE pfnETDT = 0;
HINSTANCE hDll = 0;

	if((hDll = LoadLibrary(TEXT("uxtheme.dll")))){
		if((pfnETDT = (ENABLETHEMEDIALOGTEXTURE)GetProcAddress(hDll,"EnableThemeDialogTexture"))){
			pfnETDT(tab,style?style:ETDT_ENABLETAB);
		}
	    FreeLibrary(hDll);
	}
}

void DebugConsole_CreateChildTab()
{
	TCITEM tc = {TCIF_PARAM,};
	debug_tab_sel = TabCtrl_GetCurSel(tab_window);
	TabCtrl_GetItem(tab_window, debug_tab_sel, &tc);

	if (IsWindow(child_window)) DestroyWindow(child_window);
	if (debug_tab_sel == 4) {
		HWND ml_pmp_window = FindWindowW(L"ml_pmp_window", NULL);
		if (IsWindow(ml_pmp_window))
		{
			pmpDevicePrefsView view = {GetParent(tab_window), "all_sources"};
			child_window = (HWND)SendMessage(ml_pmp_window, WM_PMP_IPC, (WPARAM)&view, PMP_IPC_GET_PREFS_VIEW);
		}
	}
	else WASABI_API_CREATEDIALOGW(IDD_DEBUG_CONSOLE1+debug_tab_sel, GetParent(tab_window), PreferencesDialogProc);
	DoWinXPStyle(child_window, 0);

	RECT r;
	GetClientRect(tab_window, &r);
	TabCtrl_AdjustRect(tab_window, FALSE, &r);
	SetWindowPos(child_window, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

BOOL CALLBACK PreferencesDialogBaseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			debug_console_window = hwndDlg;
			tab_window = GetDlgItem(hwndDlg, IDC_TAB1);
			TCITEM tc = {TCIF_TEXT|TCIF_PARAM,0,0,WASABI_API_LNGSTRINGW(IDS_OPTIONS),0,0,0};
			TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
			tc.pszText = WASABI_API_LNGSTRINGW(IDS_LOGGING);
			tc.lParam = 1;
			TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
			tc.pszText = WASABI_API_LNGSTRINGW(IDS_PLAYLISTS);
			tc.lParam = 2;
			TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
			tc.pszText = WASABI_API_LNGSTRINGW(IDS_EXCLUSIONS);
			tc.lParam = 3;
			TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
			if (first_pull)
			{
				tc.pszText = WASABI_API_LNGSTRINGW(IDS_VIEW);
				tc.lParam = 4;
				TabCtrl_InsertItem(tab_window,tc.lParam,&tc);
			}

			if (log_id < 65536) log_id = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_cloud_logger", IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (ignore_id < 65536) ignore_id = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_cloud_ignored", IPC_REGISTER_WINAMP_IPCMESSAGE);

			int last_tab = Config_GetLastTab();
			if (last_tab < 0 || last_tab > (3 + first_pull)) last_tab = 0;
			TabCtrl_SetCurSel(tab_window, last_tab);

			ShowWindow(GetDlgItem(hwndDlg, IDC_DEV_MODE), Config_GetDevMode());
			SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_ADDSTRING,0,(LPARAM)L"Production");
			SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_ADDSTRING,0,(LPARAM)L"Development");
			SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_ADDSTRING,0,(LPARAM)L"QA");
			SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_ADDSTRING,0,(LPARAM)L"Staging");
			SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_SETCURSEL,Config_GetDevMode(),0);

			DebugConsole_CreateChildTab();
		}
		break;

		case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->code == TCN_SELCHANGE)
			{
				DebugConsole_CreateChildTab();
			}
			else if(((LPNMHDR)lParam)->code == NM_CLICK)
			{
				int ctrl = (GetAsyncKeyState(VK_CONTROL)&0x8000);
				int shift = (GetAsyncKeyState(VK_SHIFT)&0x8000);
				if (ctrl && shift)
				{
					ShowWindow(GetDlgItem(hwndDlg, IDC_DEV_MODE), TRUE);
				}
			}
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_DEV_MODE:
					if (HIWORD(wParam) == CBN_SELCHANGE
						&& SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_GETCURSEL,0,0) != Config_GetDevMode())
					{
						Config_SetDevMode(SendDlgItemMessage(hwndDlg,IDC_DEV_MODE,CB_GETCURSEL,0,0));
						wchar_t *ini = (wchar_t*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILEW);
						WritePrivateProfileStringW(L"winamp", L"show_prefs", L"-1", ini);
						PostMessage(plugin.hwndWinampParent, WM_USER, 0, IPC_RESTARTWINAMP);
					}
					break;
			}
		break;

		case WM_DESTROY:
		{
			if (IsWindow(child_window)) DestroyWindow(child_window);
			Config_SetLastTab();
		}
		break;
	}

	if (uMsg == log_id)
	{
		DebugConsole_AddLogMessage();
	}
	else if (uMsg == ignore_id)
	{
		// this resolves a crash when trying to run this too quickly
		// by knowing we have to do an update but smoothes them out
		if (timer_id) KillTimer(hwndDlg, ignore_id);
		timer_id = SetTimer(hwndDlg, ignore_id, 500, DebugConsoler_TimerProc);
	}

	return 0;
}

BOOL CALLBACK PreferencesDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			child_window = hwndDlg;
			DebugConsole_Init(hwndDlg, TabCtrl_GetCurSel(tab_window));
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_LOGGING:
					{
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection)
						{
							db_connection->Info_SetLogging((logMode ^= 1));
							EnableWindow(GetDlgItem(hwndDlg, IDC_LOGFAILEDONLY), (logMode & 1));
							EnableWindow(GetDlgItem(hwndDlg, IDC_LOGBINARY), (logMode & 1));
							EnableWindow(GetDlgItem(hwndDlg, IDC_LOGCLEARSTARTUP), (logMode & 1));
						}
					}
					break;

				case IDC_LOGFAILEDONLY:
					{
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection) db_connection->Info_SetLogging((logMode ^= 2));
					}
					break;

				case IDC_LOGBINARY:
					{
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection) db_connection->Info_SetLogging((logMode ^= 4));
					}
					break;

				case IDC_LOGCLEARSTARTUP:
					{
						ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
						if (db_connection) db_connection->Info_SetLogging((logMode ^= 8));
					}
					break;

				case IDC_LOGFOLDER:
					ShellExecute(NULL, L"open", Logger::base_path->string, 0, 0, SW_SHOW);			
					break;

				case IDC_CLEAR_LOGS:
					DebugConsole_ClearLog(hwndDlg);
					break;

				case IDC_SAVE_LOG:
					DebugConsole_SaveLog(hwndDlg);
					break;

				case IDC_SHOW_LOCAL_DEVICE:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						Config_SetShowLocal(IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
						DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_REFRESH_DEVICES_LIST));
						cloud_client->DeviceUpdate();
					}
					break;

				case IDC_RESET:
					DebugConsole_Reset(hwndDlg);

				case IDC_RESCAN:
				{
					int ret = MessageBox(hwndDlg, L"Do you also want to remove files from this Cloud device which cannot be found in your Local Library?\n\n"
												  L"Choose 'No' to only remove files which cannot be physically found.",
										 L"Rescan Local Library", MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1);
					if (ret != IDCANCEL)
					{
						DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_START_LOCAL_LIBRARY_RESCAN));
						cloud_background.Rescan(ret == IDNO);
					}
					break;
				}
				case IDC_RESET_IGNORED:
					DebugConsole_ResetIgnoredFiles(hwndDlg);
					break;

				case IDC_REMOVE_PL_MODE:
					if (HIWORD(wParam) == CBN_SELCHANGE
						&& SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_GETCURSEL,0,0) != Config_GetPlRemoveMode())
					{
						Config_SetPlRemoveMode(SendDlgItemMessage(hwndDlg,IDC_REMOVE_PL_MODE,CB_GETCURSEL,0,0));
					}
					break;

				case IDC_SET_NAME:
					{
						wchar_t name[256];
						GetDlgItemText(hwndDlg, IDC_CURRENT_NAME, name, 256);

						nx_string_t device_name;
						if (NXStringCreateWithUTF16(&device_name, name) == NErr_Success)
						{
							// update our 'info' value as well as the main table to avoid confusion
							ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
							if (db_connection)
							{
								db_connection->Devices_Add(local_device_token, device_name, 0, &local_device_id);
								db_connection->Info_SetDeviceName(device_name);
								DebugConsole_SetStatus(WASABI_API_LNGSTRINGW(IDS_UPDATING_LOCAL_DEVICE_NAME));
								cloud_client->DeviceUpdate();
								current_device_name = device_name;
								SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CURRENT_NAME, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_CURRENT_NAME));
							}
						}
					}
					break;

				case IDC_CURRENT_NAME:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						if (first_pull)
						{
							if (!GetWindowTextLength((HWND)lParam))
							{
								nx_string_t device_name;
								if (GetName(&device_name) == NErr_Success)
								{
									SetDlgItemText(hwndDlg, LOWORD(wParam), device_name->string);
								}
							}

							wchar_t temp[256];
							GetWindowText((HWND)lParam, temp, ARRAYSIZE(temp));
							EnableWindow(GetDlgItem(hwndDlg, IDC_SET_NAME), wcscmp(temp, current_device_name->string));
						}
					}
					break;

				case IDC_LOGIN_HERE:
				{
					HNAVITEM item = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, (signin_treeItem ? signin_treeItem : cloud_treeItem));
					MLNavItem_Select(plugin.hwndLibraryParent, item);
					SendMessage(plugin.hwndWinampParent, WM_COMMAND, MAKEWPARAM(WINAMP_SHOWLIBRARY, 0), 0L);
					break;
				}

				case IDC_MANAGE_HERE:
				{
					HNAVITEM item = MLNavCtrl_FindItemById(plugin.hwndLibraryParent, cloud_treeItem);
					MLNavItem_Select(plugin.hwndLibraryParent, item);
					SendMessage(plugin.hwndWinampParent, WM_COMMAND, MAKEWPARAM(WINAMP_SHOWLIBRARY, 0), 0L);
					break;
				}

				case IDC_ADD_PL_TO_CLOUD:
				{
					wchar_t temp[16] = {0};
					StringCchPrintfW(temp, 16, L"%d", IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
					WritePrivateProfileStringW(L"gen_ml_config", L"cloud_always", temp, ini_file);
					WritePrivateProfileStringW(L"gen_ml_config", L"cloud_prompt", L"1", ini_file);
					break;
				}
			}
			break;

		case WM_NOTIFY:
			{
				if(((LPNMHDR)lParam)->code == LVN_KEYDOWN)
				{
					if (((LPNMHDR)lParam)->hwndFrom == list_view)
					{
						if (((LPNMLVKEYDOWN)lParam)->wVKey == VK_SPACE)
						{
							// basically use this as a way to use space to toggle following the
							// newest item in the listview or to keep on the last selected item
							int pos = ListView_GetNextItem(list_view, -1, LVNI_ALL | LVNI_SELECTED);
							if (pos != -1) {
								ListView_SetItemState(list_view, pos, 0, LVIS_SELECTED);
								SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)1);
								return 1;
							}
						}
					}
					else if (((LPNMHDR)lParam)->hwndFrom == invalid_view)
					{
						if (((LPNMLVKEYDOWN)lParam)->wVKey == VK_DELETE)
						{
							ifc_clouddb *db_connection = cloud_background.Get_DB_Connection();
							if (db_connection)
							{
								db_connection->BeginTransaction();
								int pos = ListView_GetNextItem(invalid_view, -1, LVNI_SELECTED);	
								while (pos >= 0)
								{
									db_connection->IDMap_ResetIgnored(ignored_ids[pos]);
									pos = ListView_GetNextItem(invalid_view, pos, LVNI_SELECTED);
								}
								db_connection->Commit();
								DebugConsole_UpdateIgnoredFiles();
							}
						}
					}
				}
				else if(((LPNMHDR)lParam)->code == LVN_GETEMPTYMARKUP && ((LPNMHDR)lParam)->hwndFrom == invalid_view)
				{
					NMLVEMPTYMARKUP *pnmMarkup = (NMLVEMPTYMARKUP*)lParam;
					wcsncpy(pnmMarkup->szMarkup, 
							WASABI_API_LNGSTRINGW(IDS_NO_FILES_INCOMPATIBLE),
							L_MAX_URL_LENGTH);
					SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)(num_ignored_ids<=0));
					return (num_ignored_ids<=0);
				}
				else if((((LPNMHDR)lParam)->code == LVN_GETDISPINFOW || ((LPNMHDR)lParam)->code == LVN_GETDISPINFOA))
				{
					NMLVDISPINFO *pdi = (NMLVDISPINFO *)lParam;
					LVITEMW *pItem = &pdi->item;
					if ((LVIF_TEXT & pItem->mask))
					{
						if (((LPNMHDR)lParam)->hwndFrom == list_view && log_msg.size() > 0)
						{
							if (0 == pItem->iSubItem)
							{
								pdi->item.pszText = log_time[pItem->iItem];
							}
							else
							{
								pdi->item.pszText = log_msg[pItem->iItem];
							}
						}
						else if (((LPNMHDR)lParam)->hwndFrom == invalid_view && num_ignored_ids > 0)
						{
							pdi->item.pszText = filenames[pItem->iItem]->string;
						}
					}
				}
			}
			break;

		case WM_DESTROY:
		{
			if (IsWindow(list_view) && NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(list_view, FALSE);

			if (IsWindow(invalid_view) && NULL != WASABI_API_APP)
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(invalid_view, FALSE);

			if (ignored_ids)
			{
				free(ignored_ids);
				ignored_ids = 0;
			}
			if (filenames)
			{
				free(filenames);
				filenames = 0;
			}
			num_ignored_ids = 0;
		}
		break;
	}

	link_handledraw(hwndDlg, uMsg, wParam, lParam);
	return 0;
}

void StatusWindow_SetDialogIcon(HWND hwndDlg)
{
	static HICON wa_icy;
	if (wa_icy)
	{
		wa_icy = (HICON)LoadImage(GetModuleHandle(L"winamp.exe"),
								  MAKEINTRESOURCE(102), IMAGE_ICON,
								  GetSystemMetrics(SM_CXSMICON),
								  GetSystemMetrics(SM_CYSMICON),
								  LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
	}
	SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)wa_icy);
}

void StatusWindow_DetailsMessage()
{
	HWND status = GetDlgItem(status_window, IDC_STATUS_X_OF_X);
	BOOL visible = (last_string_id && last_position > 0 && last_show > 0);
	if (IsWindow(status) && visible)
	{
		wchar_t buf[256];
		StringCchPrintfW(buf, 256, WASABI_API_LNGSTRINGW(IDS_STATUS_PROCESSING_X_OF_X), last_position, last_show, (last_position * 100 / last_show));
		SetWindowText(status, buf);
	}
	ShowWindow(status, (visible ? SW_SHOW : SW_HIDE));
}

void StatusWindow_Message(int string_id)
{
	last_string_id = string_id;
	if (IsWindow(status_window))
	{
		SetDlgItemText(status_window, IDC_STATUS_TEXT, WASABI_API_LNGSTRINGW(!last_string_id ? IDS_STATUS_NORMAL : last_string_id));
	}
	StatusWindow_DetailsMessage();
}

void StatusWindow_SavePos(int do_destroy)
{
	if (IsWindow(status_window))
	{
		RECT r = {0};
		GetWindowRect(status_window, &r);
		Config_SaveShowPos(r.left, r.top);
		DestroyWindow(status_window);
	}
}

void StatusWindow_Center(HWND hwndDlg, LONG *left, LONG *top) {
    RECT rect, rectP;
    int width, height;
    int screenwidth, screenheight;
    int x, y;

	GetWindowRect(hwndDlg, &rect);
	GetWindowRect(GetDesktopWindow(), &rectP);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	x = ((rectP.right-rectP.left) - width) / 2 + rectP.left;
	y = ((rectP.bottom-rectP.top) - height) / 2 + rectP.top;

	screenwidth = GetSystemMetrics(SM_CXSCREEN);
	screenheight = GetSystemMetrics(SM_CYSCREEN);

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x + width > screenwidth) x = screenwidth - width;
	if (y + height > screenheight) y = screenheight - height;

	*left = x;
	*top = y;
}

VOID CALLBACK StatusDialog_ShowTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	SetForegroundWindow(status_progress_window);
}

void getViewport(RECT *r, HWND wnd, int full, RECT *sr)
{
	POINT *p = NULL;
	if (p || sr || wnd)
	{
		HMONITOR hm = NULL;
		if (sr) hm = MonitorFromRect(sr, MONITOR_DEFAULTTONEAREST);
		else if (wnd) hm = MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST);
		else if (p) hm = MonitorFromPoint(*p, MONITOR_DEFAULTTONEAREST);
		if (hm)
		{
			MONITORINFOEX mi;
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);

			if (GetMonitorInfoA(hm, &mi))
			{
				if (!full) *r = mi.rcWork;
				else *r = mi.rcMonitor;
				return ;
			}
		}
	}
	if (full)
	{ // this might be borked =)
		r->top = r->left = 0;
		r->right = GetSystemMetrics(SM_CXSCREEN);
		r->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, r, 0);
	}
}

BOOL windowOffScreen(HWND hwnd, POINT pt)
{
	RECT r = {0}, wnd = {0}, sr = {0};
	GetWindowRect(hwnd, &wnd);
	sr.left = pt.x;
	sr.top = pt.y;
	sr.right = sr.left + (wnd.right - wnd.left);
	sr.bottom = sr.top + (wnd.bottom - wnd.top);
	getViewport(&r, hwnd, 0, &sr);
	return !PtInRect(&r, pt);
}

BOOL CALLBACK StatusDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		status_window = hwndDlg;
		status_progress_window = GetDlgItem(hwndDlg, IDC_PROGRESS);
		StatusWindow_SetDialogIcon(hwndDlg);

		StatusWindow_Message(last_string_id);
		DebugConsole_UpdateProgress(last_position, last_show);

		POINT pt = {-1, -1};
		Config_GetShowPos(&pt.x, &pt.y);
		if (!windowOffScreen(hwndDlg, pt))
			SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		else
		{
			StatusWindow_Center(hwndDlg, &pt.x, &pt.y);
			SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
	}
	else if (uMsg == WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
			case IDCANCEL:
				if (Config_GetShowStatus() == 2)
				{
					wchar_t title[128];
					GetWindowTextW(hwndDlg, title, 128);
					MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_RESTORE_STATUS_WINDOW), title, MB_ICONINFORMATION);
				}
				Config_SetShowStatus(0);
				StatusWindow_SavePos();
			break;
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		StatusWindow_SavePos(0);
	}
	return 0;
}

void CALLBACK StatusTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD elapsed)
{
	if (eventId == 777)
	{
		KillTimer(hwnd, eventId);
		HWND h = WASABI_API_CREATEDIALOGW(IDD_STATUS_DIALOG, plugin.hwndWinampParent, StatusDialogProc);
		SetTimer(hwnd, 998, 1, StatusDialog_ShowTimerProc);
	}
}

void ToggleStatusWindow(bool first)
{
	if (!IsWindow(status_window))
	{
		SetTimer(plugin.hwndWinampParent, 777, 1, StatusTimer);
		if (!first_login && !first) first = 1;
		Config_SetShowStatus(1+first);
		Config_SetFirstLogin();
		first_login = 1;
	}
	else
	{
		Config_SetShowStatus(0);
		StatusWindow_SavePos();
	}
}
#include "main.h"
#include "api.h"
#include "config.h"
#include "../auth/ifc_authcallback.h"
#include "resource.h"
#include "./authCallback.h"
#include "jsapi2_callbackmanager.h"
#include <api/service/waservicefactory.h>
#include <strsafe.h>


static HWND authwnd = 0;
enum
{
	MODE_USER_PASS = 0,
	MODE_USER_SECURID = 1,
};

class AuthParams : public ifc_authcallback
{
public:
	AuthParams() 
	{
		mode = MODE_USER_PASS;
		username=0;
		password=0;
		thread = 0;
		context=0;
		securid=0;
		abort = CreateEvent(NULL, FALSE, FALSE, NULL);
		ZeroMemory(&results, sizeof(api_auth::AuthResults));
	}
	~AuthParams()
	{
		CloseHandle(abort);
		free(username);
		free(password);
		free(securid);
		free(context);
	}

	int mode;
	void Reset();
	HWND hwnd;

	wchar_t *username;
	wchar_t *password;
	wchar_t *securid;
	char *context;

	api_auth::AuthResults	results;

	int Run();
	HANDLE thread;
	HANDLE abort;
private:
	int OnConnecting();
	int OnSending();
	int OnReceiving();
	int OnIdle();

protected:
	RECVS_DISPATCH;
};

void AuthParams::Reset()
{
	free(username);
	username=0;
	free(password);
	password=0;
	free(securid);
	securid=0;
}

int AuthParams::OnConnecting()
{
	if (WaitForSingleObject(abort, 0) != WAIT_TIMEOUT)
		return 1;
	return 0;
}

int AuthParams::OnSending()
{
	if (WaitForSingleObject(abort, 0) != WAIT_TIMEOUT)
		return 1;
	return 0;
}

int AuthParams::OnReceiving()
{
	if (WaitForSingleObject(abort, 0) != WAIT_TIMEOUT)
		return 1;
	return 0;
}

int AuthParams::OnIdle()
{
	if (WaitForSingleObject(abort, 50) == WAIT_TIMEOUT)
		return 0;
	else
		return 1;
}

#define CBCLASS AuthParams
START_DISPATCH;
CB(ONCONNECTING, OnConnecting)
CB(ONSENDING, OnSending)
CB(ONRECEIVING, OnReceiving)
CB(ONIDLE, OnIdle)
END_DISPATCH;
#undef CBCLASS

int AuthParams::Run()
{
	if (mode == MODE_USER_SECURID)
		return AGAVE_API_AUTH->LoginSecurID(username, password, context, securid, &results, this);
	else
		return AGAVE_API_AUTH->Login(username, password, &results, this);
}

static DWORD WINAPI AuthThread(void *param)
{
	AuthParams *params = (AuthParams *)param;
	int ret = params->Run();
	if (ret != AUTH_ABORT)
		PostMessage(params->hwnd, WM_APP, ret, 0);
	return ret;
}

static void StartLogin(HWND hwndDlg, AuthParams *params)
{
	params->Reset();

	wchar_t username[512];
	wchar_t password[512];
	wchar_t securid[7];
	GetDlgItemText(hwndDlg, IDC_USERNAME, username, 512);
	GetDlgItemText(hwndDlg, IDC_PASSWORD, password, 512);
	GetDlgItemText(hwndDlg, IDC_SECURID, securid, 7);

	params->username = _wcsdup(username);
	params->password = _wcsdup(password);
	params->securid = _wcsdup(securid);
	params->thread = CreateThread(0, 0, AuthThread, params, 0, 0);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
}

static INT_PTR CALLBACK AuthDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			authwnd = hwndDlg;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			AuthParams *params = (AuthParams *)lParam;
			params->hwnd = hwndDlg;
			SetDlgItemText(hwndDlg, IDC_USERNAME, config_username);
			if (config_username[0])
				SetFocus(GetDlgItem(hwndDlg, IDC_PASSWORD));
			else
				SetFocus(GetDlgItem(hwndDlg, IDC_USERNAME));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
		}
		return 0;
	case WM_NOTIFY:
		{
			NMHDR *pnmh = (NMHDR *)lParam;
			if (NM_CLICK == pnmh->code)
			{
				switch(wParam)
				{
				case IDC_CREATE_ACCOUNT:
					OpenUrl(ACCOUNT_CREATION_URL, true);
					EndDialog(hwndDlg, AUTH_ABORT);
					return TRUE;
				case IDC_FORGOT_PASSWORD:
					OpenUrl(FORGOT_PASSWORD_URL, true);
					EndDialog(hwndDlg, AUTH_ABORT);
					return TRUE;
				case IDC_TOS:
					OpenUrl(TOS_URL, true);
					EndDialog(hwndDlg, AUTH_ABORT);
					return TRUE;
				}
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_AGREE:
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_AGREE)==BST_CHECKED)
					EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), FALSE);
			}
			break;
		case IDC_LOGIN:
			{
				AuthParams *params = (AuthParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				StartLogin(hwndDlg, params);
			}
			break;
		case IDCANCEL:
			{
				AuthParams *params = (AuthParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				SetEvent(params->abort);
				if (params->mode == MODE_USER_PASS)
				{
					EndDialog(hwndDlg, AUTH_ABORT);
				}
				else if (params->mode == MODE_USER_SECURID)
				{
#if 1 // this code section will close the dialog when cancel is selected on securid prompt
					EndDialog(hwndDlg, AUTH_ABORT);
#else // and this will return to name/password 
					if (params->thread)
					{
						WaitForSingleObject(params->thread, INFINITE);
						CloseHandle(params->thread);
						params->thread = 0;
					}
					SetDlgItemText(hwndDlg, IDC_STATUS, L"");
					SetDlgItemText(hwndDlg, IDC_SECURID, L"");
					ShowWindow(GetDlgItem(hwndDlg, IDC_SECURID), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_SECURID), SW_HIDE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_USERNAME), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), TRUE);
					params->mode = MODE_USER_PASS;
#endif
				}
			}
			break;
		}
		break;
	case WM_APP:
		{
			AuthParams *params = (AuthParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			switch (wParam)
			{
			case AUTH_SUCCESS:
				GetDlgItemText(hwndDlg, IDC_USERNAME, config_username, sizeof(config_username)/sizeof(*config_username));
				EndDialog(hwndDlg, wParam);
				break;
			case AUTH_SECURID:
				SetDlgItemText(hwndDlg, IDC_STATUS, L"SecurID required");
				EnableWindow(GetDlgItem(hwndDlg, IDC_USERNAME), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SECURID), SW_SHOWNA);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_SECURID), SW_SHOWNA);
				SetFocus(GetDlgItem(hwndDlg, IDC_SECURID));
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				free(params->context);
				params->context = _strdup(params->results.context);
				if (params->thread)
				{
					WaitForSingleObject(params->thread, INFINITE);
					CloseHandle(params->thread);
					params->thread = 0;
				}
				params->mode = MODE_USER_SECURID;
				break;
			default:
				if (wParam == AUTH_UNCONFIRMED)
					SetDlgItemText(hwndDlg, IDC_STATUS, L"Unconfirmed Account");
				else
					SetDlgItemText(hwndDlg, IDC_STATUS, L"Login Failed");
				EnableWindow(GetDlgItem(hwndDlg, IDC_LOGIN), TRUE);
				WaitForSingleObject(params->thread, INFINITE);
				if (params->thread)
				{
					WaitForSingleObject(params->thread, INFINITE);
					CloseHandle(params->thread);
					params->thread = 0;
				}
				break;
			}

		}	
		break;
	case WM_DESTROY:
		{
			AuthParams *params = (AuthParams *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetEvent(params->abort);
			if (params->thread)
			{
				WaitForSingleObject(params->thread, INFINITE);
				CloseHandle(params->thread);
				params->thread = 0;
			}
			authwnd = 0;
		}
		break;

	}
	return 0;
}

void EnableOrgling_AfterLogin()
{

	if (GetLoginStatus() == LOGIN_LOGGEDIN && config_collect == false)
	{
		wchar_t szTitle[128], szMessage[4096];
		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGE_ENABLEORGLING_TITLE, szTitle, ARRAYSIZE(szTitle));
		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGE_ENABLEORGLING, szMessage, ARRAYSIZE(szMessage));
		if (IDYES == MessageBox(0, szMessage, szTitle, MB_YESNO | MB_ICONWARNING))
		{
			config_collect = true;
			Config_SyncEnabled();
		}
	}
}


int Login(HWND hOwner, char *session_key, size_t session_key_len, char *token, size_t token_len)
{
	INT_PTR r = AGAVE_API_AUTH->LoginBox(ORGLER_AUTH_REALM, hOwner, 0);
	if (-1 != r) return (INT)r;
	
	if (authwnd)
	{
		SetFocus(authwnd);
		return AUTH_ABORT;
	}
	AuthParams params;

	INT_PTR err = DialogBoxParamW(plugin.hDllInstance, MAKEINTRESOURCEW(IDD_AUTH), 0, AuthDlgProc, (LPARAM)&params);
	AGAVE_API_AUTH->SetCredentials(ORGLER_AUTH_REALM, params.results.session_key, params.results.token, config_username, params.results.expire);
	SecureZeroMemory(&params.results, sizeof(params.results));

	return (INT)err;
}

void Logout()
{
	AGAVE_API_AUTH->SetCredentials(ORGLER_AUTH_REALM, 0, 0, config_username, 0);
}

int GetLoginStatus()
{
	if (session_key[0] && token_a[0])
	{
		__time64_t now = _time64(0);
		if (now > session_expiration)
			return LOGIN_EXPIRED;
		else if ((now - 604800 /* one week */) > session_expiration)
			return LOGIN_EXPIRING;
		else
			return LOGIN_LOGGEDIN;
	}
	else
	{
		return LOGIN_NOTLOGGEDIN;
	}
}
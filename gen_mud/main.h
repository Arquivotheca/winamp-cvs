#pragma once
#include "../Winamp/gen.h"
#include "../Winamp/wa_ipc.h"
#include <time.h>

#define ACCOUNT_CREATION_URL L"https://new.aol.com/productsweb/?promocode=825431"
#define FORGOT_PASSWORD_URL L"https://account.login.aol.com/opr/_cqr/opr/opr.psp?sitedomain=winamp.com&authLev=1&lang=en&locale=us"
#define TOS_URL L"http://www.winamp.com/legal/plugineula"
#define USER_PROFILE_DESTURL L"http://www.winamp.com/_cqr/login?siteState=OrigUrl%3Dhttp%253A%252F%252Fwww.winamp.com%252Fuser%252Forgler"
#define USER_PROFILE_URL L"http://www.winamp.com/user/orgler"

#define VERSION_MAJOR 1
#define VERSION_MINOR 1 /* can be two digits or one */

#define ORGLER_AUTH_REALM		GUID_NULL

/* main.cpp */
extern winampGeneralPurposePlugin plugin;
extern const char *inidir;
extern int winampVersion;
extern char session_key[512];
extern char token_a[512];
extern __time64_t session_expiration;
extern WORD enable_menu_id;
extern WORD options_menu_id;
extern WORD login_menu_id;
extern HMENU ctrlmenu;
extern prefsDlgRecW g_prefsItem;

bool OpenUrl(const wchar_t *url, bool forceExternalBrowser);

/* allowed.cpp */
bool AllowedFilename(const wchar_t *filename);

/* listen.cpp */
void HookWinampProc(HWND hwnd);
void ListenReset();
void OnStop(bool quick=false);

/* db.cpp */
void CloseDatabase();
void CompactDatabase();

/* queue.cpp */
void CloseQueue();
void AwakenAvQueue();
void AwakenQueue();
void FlushAvQueue();

/* auth.cpp */
enum
{
	MUD_ERROR=10,
};
int Login(HWND hOwner, char *session_key, size_t session_key_len, char *token, size_t token_len);
void EnableOrgling_AfterLogin();
void Logout();

enum
{
	LOGIN_LOGGEDIN=0,
	LOGIN_NOTLOGGEDIN=1,
	LOGIN_EXPIRING=2,
	LOGIN_EXPIRED=3,
};
int GetLoginStatus();

/* config.cpp */
INT_PTR WINAPI PreferencesDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void Config_Load();
void Config_Save();
void Config_SyncLogin();
void Config_SyncEnabled();

/* log.cpp */
void Log(const wchar_t *format, ...);
void CloseLog();
const wchar_t *MakeDateString(__time64_t convertTime);


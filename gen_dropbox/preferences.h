#ifndef NULLOSFT_DROPBOX_PLUGIN_PREFERENCES_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_PREFERENCES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


#define PREFPAGE_BADINDEX		((UINT)-1)

BOOL Plugin_RegisterPreferences();
BOOL Plugin_UnregisterPreferences();
BOOL Plugin_ShowPreferences();
const prefsDlgRecW *Plugin_GetPreferences(); 
UINT Preferences_GetPageCount();

UINT Preferences_InsertPage(	UINT index, 
							INT pageId,
							HINSTANCE hInstance, 
							LPCTSTR pszTitle, 
							INT imageId,
							LPCTSTR pszDialog,
							DLGPROC dialogProc);

BOOL Preferences_RemovePage(INT pageId);


INT Preferences_ShowWarning(HWND hDialog, BOOL fShow);
BOOL Preferences_EnableWarning(HWND hDialog, BOOL fEnable); // call this to allow warning to be shown


// known pages id
#define PREFPAGE_INVALID	0x0000
#define PREFPAGE_GENERAL	0x0001
#define PREFPAGE_VIEW		0x0002
#define PREFPAGE_FILTER		0x0003

BOOL PreferencesFrame_SelectProfile(HWND hwnd,  Profile *profile);
Profile *PreferencesFrame_GetProfile(HWND hwnd);

BOOL WINAPI PreferencesGeneral_RegisterPage();
BOOL WINAPI PreferencesView_RegisterPage();
BOOL WINAPI PreferencesFilter_RegisterPage();

#define PREF_MSG_FIRST				(WM_USER + 0x250)
#define PREF_MSG_PROFILECHANGED		(PREF_MSG_FIRST + 0)

#endif //NULLOSFT_DROPBOX_PLUGIN_PREFERENCES_HEADER

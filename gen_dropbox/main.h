#ifndef NULLOSFT_WINAMP_GENERAL_PLUGIN_DROPBOX_HEADER
#define NULLOSFT_WINAMP_GENERAL_PLUGIN_DROPBOX_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define OEMRESOURCE

#include <wtypes.h>
#include "../winamp/gen.h"
#include "../winamp/wa_ipc.h"
#include "./configInterface.h"
#include "../nu/trace.h"
#include "./lfHeap.h"


#define PLUGIN_VERSION_MAJOR		0
#define PLUGIN_VERSION_MINOR		5

#define PLUGIN_VERSION		(((DWORD)PLUGIN_VERSION_MAJOR << 16) | (0x0000FFFF & (DWORD)PLUGIN_VERSION_MINOR))

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)


#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus


#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))
#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#define SENDCMD(__hwnd, __ctrlId, __eventId, __hctrl) (SENDMSG((__hwnd), WM_COMMAND, MAKEWPARAM(__ctrlId, __eventId), (LPARAM)(__hctrl)))

#ifndef GetWindowStyle
#define GetWindowStyle(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_STYLE))
#endif //GetWindowStyle

#ifndef GetWindowStyleEx
#define GetWindowStyleEx(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_EXSTYLE))
#endif //GetWindowStyleEx

interface IConfiguration;
HRESULT Plugin_QueryConfiguration(REFGUID configId, Profile *profile, IConfiguration **ppConfig);

class ConfigurationManager;
extern ConfigurationManager *pluginConfigManager;
#define PLUGIN_CFGMNGR pluginConfigManager

class ProfileManager;
extern ProfileManager *pluginProfileManager;
#define PLUGIN_PROFILEMNGR pluginProfileManager

class TypeCollection;
extern TypeCollection *pluginRegisteredTypes;
#define PLUGIN_REGTYPES pluginRegisteredTypes


EXTERN_C const GUID winampSettingsGuid;
#define CFG_TITLEFORMAT MAKEINTRESOURCEA(1)

EXTERN_C const GUID mediaLibrarySettingsGuid;
#define CFG_ACTIONTYPE	MAKEINTRESOURCEA(1)
#define ACTIONTYPE_PLAY			0
#define ACTIONTYPE_ENQUEUE		1

#ifdef __cplusplus
extern "C" {
#endif

	INT_PTR GetPluginUID(void);
	extern winampGeneralPurposePlugin plugin;

#ifdef __cplusplus
}
#endif


#endif //NULLOSFT_WINAMP_GENERAL_PLUGIN_DROPBOX_HEADER

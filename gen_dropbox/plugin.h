#ifndef NULLOSFT_DROPBOX_PLUGIN_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

typedef void (CALLBACK *PLUGINUNLOADCALLBACK)(void);

#include "./typeCollection.h"
#include "./profileManager.h"
#include "./dropboxClass.h"
#include "./skinWindow.h"

#ifdef __cplusplus
extern "C" {
#endif 

int Plugin_OnInit();
void Plugin_OnConfig();
void Plugin_OnQuit();

HRESULT Plugin_SetClipboard(IDataObject *pObject);

void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK);

BOOL Plugin_EnsurePathExist(LPCTSTR pszDirectory);

HRESULT Plugin_GetWinampIniFile(LPTSTR pszBuffer, INT cchBufferMax);
HRESULT Plugin_GetWinampIniDirectory(LPTSTR pszBuffer, INT cchBufferMax);
HRESULT Plugin_GetDropboxPath(LPTSTR pszBuffer, INT cchBufferMax);

HRESULT Plugin_RegisterClass(const DROPBOXCLASSINFO *pdbci);
HRESULT Plugin_UnregisterClass(const UUID &classUid);
DROPBOXCLASSINFO *Plugin_FindRegisteredClass(const UUID &classUid);

EXTERN_C const UUID defaultClassUid;

#ifdef __cplusplus
}
#endif 


#endif // NULLOSFT_DROPBOX_PLUGIN_HEADER



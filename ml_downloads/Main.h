#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include "Downloaded.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		24

void DestroyLoader(HWND);
void BuildLoader(HWND);
extern int winampVersion;

#define NAVITEM_UNIQUESTR	L"download_svc"
BOOL Navigation_Update(void);

bool AddDownloadData(const DownloadedFile &data);
void CloseDatabase();

#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/ChildSizer.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoLock.h"
#include "./menu.h"
#include <windows.h>
#include <shlwapi.h>

extern ATOM VIEWPROP;
extern winampMediaLibraryPlugin plugin;
extern DWORD DOWNLOADS_VIEW_LOADED, DOWNLOADS_UPDATE;
extern int downloads_treeItem;

#include "../dlmgr/api_downloadmanager.h"
extern api_downloadManager *downloadManagerApi;
#define AGAVE_API_DOWNLOADMANAGER downloadManagerApi

#define ML_ENQDEF_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"enqueuedef", 0, ml_cfg))
extern wchar_t ml_cfg[1024 + 32];

#include "DownloadViewCallback.h"
extern DownloadViewCallback *downloadViewCallback;
#endif
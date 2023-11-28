#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include "Wire.h"
#include "Downloaded.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		34

#define SERVICE_PODCAST			720
#define SERVICE_SUBSCRIPTION	721
#define SERVICE_DOWNLOADS		722

#define BAD_CHANNEL		((size_t)-1)
#define BAD_ITEM			((size_t)-1)

void SaveChannels(ChannelList &channels);
void SaveAll(bool rss_only=false);
void HookTerminate();

void DestroyLoader(HWND);
void BuildLoader(HWND);
extern int winampVersion;
void addToLibrary(const DownloadedFile& d); // call in winamp main thread only
void addToLibrary_thread(const DownloadedFile& d); // call from any thread

bool AddPodcastData(const DownloadedFile &data);
bool IsPodcastDownloaded(const wchar_t *url);
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
extern int downloadsViewLoaded;
extern UINT DOWNLOADS_VIEW_LOADED;

#include "../dlmgr/api_downloadmanager.h"
extern api_downloadManager *downloadManagerApi;
#define AGAVE_API_DOWNLOADMANAGER downloadManagerApi

#define ML_ENQDEF_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"enqueuedef", 0, ml_cfg))
extern wchar_t ml_cfg[1024 + 32];
wchar_t *urlencode(wchar_t *p);

#endif
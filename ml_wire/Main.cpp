#include "main.h"
#include "Cloud.h"
#include "DownloadThread.h"
#include "OPMLParse.h"
#include "FeedsDialog.h"
#include "DownloadsParse.h"
#include "Loader.h"
#include "XMLWriter.h"
#include "FeedParse.h"
#include "DownloadsDialog.h"
#include "Preferences.h"
#include "../gen_ml/ml.h"
#include "Defaults.h"
#include "Wire.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "RSSCOM.h"

#include "api.h"
#include "Downloaded.h"
#include "DownloadStatus.h"
#include "Factory.h"
#include "JSAPI2_Creator.h"
#include "./navigation.h"

#include <strsafe.h>

#include "PCastFactory.h"

Cloud cloud;
WireManager channelMgr;

int treeId = 0, allId = 0
#if 0
	, discoverId = 0
#endif	
	;
MLTREEITEMW downloadsTree;
wchar_t downloadsStr[64] = {0};
wchar_t ml_cfg[1024 + 32] = {0};
wchar_t feedsXmlFileName[1024] = {0},
		feedsXmlFileNameBackup[1024] = {0},
		rssXmlFileName[1024] = {0},
		rssXmlFileNameBackup[1024] = {0};

ATOM VIEWPROP = 0;

api_downloadManager *AGAVE_API_DOWNLOADMANAGER = 0;

int Init();
void Quit();
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);
DWORD threadStorage=TLS_OUT_OF_INDEXES;
winampMediaLibraryPlugin plugin =
    {
        MLHDR_VER,
        "nullsoft(ml_wire.dll)",
        Init,
        Quit,
        MessageProc,
        0,
        0,
        0,
    };

static prefsDlgRecW preferences;

static wchar_t preferencesName[64] = {0};
void SaveChannels(ChannelList &channels)
{
	// generate a backup of the feeds.xml to cope with it being wiped randomly for some
	// people or it not being able to be saved in time e.g. during a forced OS shutdown
	CopyFile(feedsXmlFileName, feedsXmlFileNameBackup, FALSE);
	SaveChannels(feedsXmlFileName, channels);
}

void SaveAll(bool rss_only)
{
	if (AGAVE_API_STATS)
		AGAVE_API_STATS->SetStat(api_stats::PODCAST_COUNT, channels.size());
	if(!rss_only) SaveChannels(channels);

	// generate a backup of the rss.xml to cope with it being wiped randomly for some
	// people or it not being able to be saved in time e.g. during a forced OS shutdown
	CopyFile(rssXmlFileName, rssXmlFileNameBackup, FALSE);
	SaveSettings(rssXmlFileName, downloadedFiles);
}

static PodcastsFactory podcastsFactory;

HANDLE hMainThread;

HCURSOR hDragNDropCursor;
int winampVersion = 0;

JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = 0;
JSAPI2Factory jsapi2Creator;

api_service *WASABI_API_SVC = 0;
obj_ombrowser *browserManager = NULL;
api_application *applicationApi = NULL;
api_stats *AGAVE_API_STATS = 0;
api_threadpool *WASABI_API_THREADPOOL = 0;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static PCastFactory pcastFactory;

UINT DOWNLOADS_VIEW_LOADED = 0;
int downloadsViewLoaded = 0;

int Init()
{
	hMainThread = GetCurrentThread();
	hDragNDropCursor = LoadCursor(GetModuleHandle(L"gen_ml.dll"), MAKEINTRESOURCE(ML_IDC_DRAGDROP));
	threadStorage = TlsAlloc();
	
	if (0 == VIEWPROP)
	{
		 VIEWPROP = GlobalAddAtom(L"Nullsoft_PodcastView");
		 if (0 == VIEWPROP) return 1;
	}

	InitCommonControls();

	winampVersion = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION);

	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WASABI_API_SVC || WASABI_API_SVC == (api_service *)1)
		return 1;

	WASABI_API_SVC->service_register(&podcastsFactory);
	WASABI_API_SVC->service_register(&jsapi2Creator);
	WASABI_API_SVC->service_register(&pcastFactory);

	// loader so that we can get the localisation service api for use
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(JSAPI2::api_securityGUID);
	if (sf) AGAVE_API_JSAPI2_SECURITY = reinterpret_cast<JSAPI2::api_security*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(OBJ_OmBrowser);
	if (sf) OMBROWSERMNGR = reinterpret_cast<obj_ombrowser*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(AnonymousStatsGUID);
	if (sf) AGAVE_API_STATS = reinterpret_cast<api_stats*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(ThreadPoolGUID);
	if (sf) WASABI_API_THREADPOOL = reinterpret_cast<api_threadpool*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(DownloadManagerGUID);
	if (sf) AGAVE_API_DOWNLOADMANAGER = reinterpret_cast<api_downloadManager*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(ExplorerFindFileApiGUID);
	if (sf) WASABI_API_EXPLORERFINDFILE = reinterpret_cast<api_explorerfindfile*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,MlWireLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME), PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
	plugin.description = (char*)szDescription;

	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	RssCOM *rss;
	if (SUCCEEDED(RssCOM::CreateInstance(&rss)))
	{
		DispatchInfo dispatchInfo;
		dispatchInfo.name = (LPWSTR)rss->GetName();
		dispatchInfo.dispatch = rss;

		SENDWAIPC(plugin.hwndWinampParent, IPC_ADD_DISPATCH_OBJECT, (WPARAM)&dispatchInfo);
		rss->Release();
	}

	BuildDefaultDownloadPath(plugin.hwndWinampParent);

	preferences.hInst = WASABI_API_LNG_HINST;
	preferences.dlgID = IDD_PREFERENCES;
	preferences.proc = (void *)PreferencesDialogProc;
	preferences.name = WASABI_API_LNGSTRINGW_BUF(IDS_PODCAST_DIRECTORY,preferencesName,64);
	preferences.where = 0;
	mediaLibrary.AddPreferences(preferences);

	wchar_t g_path[MAX_PATH];
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds", g_path, MAX_PATH);
	CreateDirectoryW(g_path, NULL);

	wchar_t oldxmlFileName[1024], oldxmlFileNameBackup[1024];
	mediaLibrary.BuildPath(L"Plugins\\ml\\rss.xml", oldxmlFileName, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds\\rss.xml", rssXmlFileName, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\rss.xml.backup", oldxmlFileNameBackup, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds\\rss.xml.backup", rssXmlFileNameBackup, 1024);

	if (PathFileExists(oldxmlFileName) && !PathFileExists(rssXmlFileName))
	{
		MoveFile(oldxmlFileName, rssXmlFileName);
		MoveFile(oldxmlFileNameBackup, rssXmlFileNameBackup);
	}

	{
		DownloadsParse downloader;
		downloader.DownloadFile(rssXmlFileName);
	}

	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds.xml", oldxmlFileName, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds\\feeds.xml", feedsXmlFileName, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds.xml.backup", oldxmlFileNameBackup, 1024);
	mediaLibrary.BuildPath(L"Plugins\\ml\\feeds\\feeds.xml.backup", feedsXmlFileNameBackup, 1024);

	if (PathFileExists(oldxmlFileName) && !PathFileExists(feedsXmlFileName))
	{
		MoveFile(oldxmlFileName, feedsXmlFileName);
		MoveFile(oldxmlFileNameBackup, feedsXmlFileNameBackup);
	}

	{
		Nullsoft::Utility::AutoLock lock (channels LOCKNAME("feeds.xml load!"));

		FeedParse downloader(&channelMgr, true);
		downloader.DownloadFile(feedsXmlFileName);
		if (AGAVE_API_STATS)
			AGAVE_API_STATS->SetStat(api_stats::PODCAST_COUNT, channels.size());
	}

	if (updateOnLaunch)
	{
		cloud.RefreshAll();
	}

	cloud.Init();
	cloud.Pulse();

	Navigation_Initialize();

	wchar_t *dir = (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW);
	PathCombine(ml_cfg, dir, L"Plugins\\gen_ml.ini");

	return 0;
}

void Quit()
{
	// If there are still files downloading, cancel download to remove incomplete downloaded files
	while ( downloadStatus.CurrentlyDownloading() ) 
    { 
           Nullsoft::Utility::AutoLock lock(downloadStatus.statusLock); 
           DownloadToken dltoken = downloadStatus.downloads.begin()->first; 
           AGAVE_API_DOWNLOADMANAGER->CancelDownload(dltoken); 
    }

	cloud.Quit();

	SaveAll();


	CloseDatabase();

	WASABI_API_SVC->service_deregister(&podcastsFactory);
	WASABI_API_SVC->service_deregister(&jsapi2Creator);
	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(OBJ_OmBrowser);
	if (NULL != sf) sf->releaseInterface(OMBROWSERMNGR);

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (NULL != sf) sf->releaseInterface(WASABI_API_APP);

	sf = WASABI_API_SVC->service_getServiceByGuid(AnonymousStatsGUID);
	if (NULL != sf) sf->releaseInterface(AGAVE_API_STATS);

	sf = WASABI_API_SVC->service_getServiceByGuid(ThreadPoolGUID);
	if (NULL != sf) sf->releaseInterface(WASABI_API_THREADPOOL);

	sf = WASABI_API_SVC->service_getServiceByGuid(ExplorerFindFileApiGUID);
	if (NULL != sf) sf->releaseInterface(WASABI_API_EXPLORERFINDFILE);

	sf = WASABI_API_SVC->service_getServiceByGuid(DownloadManagerGUID);
	if (NULL != sf) sf->releaseInterface(AGAVE_API_DOWNLOADMANAGER);

	if (0 != VIEWPROP)
	{
		GlobalDeleteAtom(VIEWPROP);
		VIEWPROP = 0;
	}
}

static INT_PTR Podcast_OnContextMenu(INT_PTR param1, HWND hHost, POINTS pts)
{
	HNAVITEM hItem = (HNAVITEM)param1;
	HNAVITEM myItem = Navigation_FindService(SERVICE_PODCAST, NULL, NULL);

	HNAVITEM podcastItem = MLNavItem_GetChild(plugin.hwndLibraryParent, myItem); 
	HNAVITEM subscriptionItem = Navigation_FindService(SERVICE_SUBSCRIPTION, podcastItem, NULL);

	if (hItem != myItem && hItem != subscriptionItem) 
		return FALSE;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(plugin.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}


	HMENU hMenu = WASABI_API_LOADMENU(IDR_MENU1);
	int subMenuId = (hItem == subscriptionItem)? 4 : 3;
	HMENU subMenu = (NULL != hMenu) ? GetSubMenu(hMenu, subMenuId) : NULL;
	if (NULL != subMenu)
	{

		INT r = Menu_TrackPopup(subMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | 
				TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, 
				pt.x, pt.y, hHost, NULL);

		switch(r)
		{
			case ID_NAVIGATION_DIRECTORY:
				MLNavItem_Select(plugin.hwndLibraryParent, myItem);
				break;
			case ID_NAVIGATION_PREFERENCES:
				SENDWAIPC(plugin.hwndWinampParent, IPC_OPENPREFSTOPAGE, &preferences);
				break;
			case ID_NAVIGATION_HELP:
				SENDWAIPC(plugin.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/help/Main_Page#Podcast_Directory");
				break;
		}
	}

	if (NULL != hMenu)
		DestroyMenu(hMenu);
	
	return TRUE;
}

INT_PTR MessageProc(int msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	INT_PTR result = 0;
	if (FALSE != Navigation_ProcessMessage(msg, param1, param2, param3, &result))
		return result;

	switch (msg)
	{
		case ML_MSG_CONFIG:
			mediaLibrary.GoToPreferences(preferences._id);
			return TRUE;
		case ML_MSG_NAVIGATION_CONTEXTMENU:
			return Podcast_OnContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
		case ML_MSG_MLVISIBLE:
			DOWNLOADS_VIEW_LOADED = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)"DownloadsViewLoaded", IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (DOWNLOADS_VIEW_LOADED != 0)
				downloadsViewLoaded = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, DOWNLOADS_VIEW_LOADED);
			return TRUE;
	}

	return FALSE;
}


#define TREE_IMAGE_LOCAL_PODCASTS			108

void addToLibrary(const DownloadedFile& d) 
{
	itemRecordW item={0};
	item.year = -1;
	item.track = -1;
	item.tracks = -1;
	item.length = -1;
	item.rating = -1;
	item.lastplay = -1;
	item.lastupd = -1;
	item.filetime = -1;
	item.filesize = -1;
	item.bitrate = -1;
	item.type = -1;
	item.disc = -1;
	item.discs = -1;
	item.bpm = -1;
	item.playcount = -1;
	item.filename = _wcsdup(d.path);
	setRecordExtendedItem(&item,L"ispodcast",L"1");
	setRecordExtendedItem(&item,L"podcastchannel",d.channel);
	wchar_t buf[40];
	_i64tow(d.publishDate,buf,10);
	if(d.publishDate) 
		setRecordExtendedItem(&item,L"podcastpubdate",buf);
	LMDB_FILE_ADD_INFOW fai = {item.filename,-1,-1};
	SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&fai, ML_IPC_DB_ADDORUPDATEFILEW);
	SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&item, ML_IPC_DB_UPDATEITEMW);
	PostMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
	freeRecord(&item);

	if(needToMakePodcastsView) {
		mlSmartViewInfo m = {sizeof(mlSmartViewInfo),2,L"Podcasts",
							 L"ispodcast = 1",461315,TREE_IMAGE_LOCAL_PODCASTS,0};
		WASABI_API_LNGSTRINGW_BUF(IDS_PODCASTS,m.smartViewName,128);
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&m, ML_IPC_SMARTVIEW_ADD);
		needToMakePodcastsView=false;
	}
}

typedef struct {
	const DownloadedFile *d;
	volatile UINT done;
} apc_addtolib_waiter;

static VOID CALLBACK apc_addtolib(ULONG_PTR dwParam) {
	apc_addtolib_waiter *w = (apc_addtolib_waiter *)dwParam;
	addToLibrary(*w->d);
	w->done=1;
}

void addToLibrary_thread(const DownloadedFile& d) {
	apc_addtolib_waiter w={&d,0};
	if(!QueueUserAPC(apc_addtolib,hMainThread,(ULONG_PTR)&w)) return;
	while(!w.done) SleepEx(5,true);
}


extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}
#include "../nu/Vector.h"
#include "../ml_pmp/pmp.h"
#include "../Winamp/wa_ipc.h"
#include "api.h"
#include "resource.h"

#include "IpodDevice2.h"

#include <devguid.h>

#define PLUGIN_VERSION "v1.0"

// plugin skeleton
static int Init();
static void Quit();
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);
static bool doRegisterForDevNotification(void);

// the plugin definition
extern PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};

// our main ini file
static const char *winampini;

api_config *AGAVE_API_CONFIG=0;
api_albumart *AGAVE_API_ALBUMART=0;
api_memmgr *WASABI_API_MEMMGR=0;

// our device notification handle
static HDEVNOTIFY hDevNotify;

HANDLE autoDetectThread=NULL;
Vector<IpodDevice2*> iPods;
bool g_detectAll=false;

static int fileSize(wchar_t * filename) {
  FILE * fh = _wfopen(filename,L"rb");
  if(!fh) return -1;
  fseek(fh,0,2);
  int l = ftell(fh);
  fclose(fh);
  return l;
}

static DWORD WINAPI ThreadFunc_DeviceChange(LPVOID lpParam) 
{
	int p = (int)lpParam;
	bool connect = p > 0x10000;
	if(connect) p -= 0x10000;
	char drive = tolower((char)p);
	if(drive == 0) return 0;
	wchar_t buf[] = L"x: connected";
	buf[0] = drive;
	OutputDebugString(buf);

	if(connect) 
	{ 
		// something plugged in
		for(int j=0; j<iPods.size(); j++) if(((IpodDevice2*)iPods.at(j))->drive == drive) return 0;
		IpodDevice2 * d = new IpodDevice2(drive);
	} 
	else 
	{ 
		//something removed
		for(int i=0; i<iPods.size(); i++) if(((IpodDevice2*)iPods.at(i))->drive == drive) 
		{
			IpodDevice2* d = (IpodDevice2*)iPods.at(i);
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)d,PMP_IPC_DEVICEDISCONNECTED);
			delete d;
			iPods.eraseAt(i);
			return 0;
		} 
		SendMessage(plugin.hwndPortablesParent, WM_PMP_IPC, NULL, PMP_IPC_DEVICEDISCONNECTED);
	}
	return 0;
}



static void autoDetectCallback(wchar_t driveW,UINT type) 
{
	char drive = 'A' + (driveW - L'A');
	wchar_t ipodtest[]={drive,L":\\iPod_Control\\iTunes\\iTunesDB"};
	wchar_t ipodtest2[]={drive,L":\\iPod_Control\\iTunes\\firsttime"};
	
	if(type == DRIVE_REMOVABLE || g_detectAll) 
	{
		if (GetFileAttributes(ipodtest) == INVALID_FILE_ATTRIBUTES)
		{
			if (GetFileAttributes(ipodtest2) == INVALID_FILE_ATTRIBUTES)
			{
				return;
			}
		}
		
		bool taken=false;
		for(int j=0; j<iPods.size(); j++) 
		{
			IpodDevice2 * d = (IpodDevice2*)iPods.at(j);
			if(d->drive == drive || d->drive == tolower(drive)) taken=true;
		}
    
		if(taken) return;

		//found!
		if(!drive) return;
		IpodDevice2 * d = new IpodDevice2(drive);
	}
}

static DWORD WINAPI ThreadFunc_AutoDetect(LPVOID lpParam) 
{
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	return NULL;
}

static int Init() 
{
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	WasabiInit(WASABI_API_SVC);

	// the the winamp.ini handle
	winampini = (const char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILE);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpIPODLangGUID);

	static char szDescription[256];
	wsprintfA(szDescription,"%s "PLUGIN_VERSION,WASABI_API_LNGSTRING(IDS_NULLSOFT_IPOD_PLUGIN));
	plugin.description = szDescription;

	/** we need to register for notification on device arrivals and removals */
	doRegisterForDevNotification();

	//detect iPods
	DWORD dwThreadId; 
	autoDetectThread = CreateThread(NULL, 0, ThreadFunc_AutoDetect, NULL, 0, &dwThreadId);
	return 0;
}

static bool doRegisterForDevNotification(void)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid  = GUID_DEVCLASS_USB;

	hDevNotify = RegisterDeviceNotification( plugin.hwndWinampParent, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	return(NULL != hDevNotify);
}

static void Quit() 
{
	WasabiQuit();
	UnregisterDeviceNotification(hDevNotify);
}

static char FirstDriveFromMask(ULONG unitmask) 
{
  char i;
  for(i=0; i<26; ++i) {
    if(unitmask & 0x1) break;
    unitmask = unitmask >> 1;
  }
  return (i+'A');
}

int wmDeviceChange(WPARAM wParam, LPARAM lParam) 
{
	wchar_t buf[100]; buf[0] = L'\0';
	wsprintf(buf,L"dev ch %x",wParam);
	OutputDebugString(buf);

	if(wParam==DBT_DEVICEARRIVAL || wParam==DBT_DEVICEREMOVECOMPLETE)
	{ 
		// something has been inserted or removed
		OutputDebugString(L"dev ch1");
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

		if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME) 
		{ 
			// its a volume
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			if((!(lpdbv->dbcv_flags & DBTF_MEDIA) && !(lpdbv->dbcv_flags & DBTF_NET)) || g_detectAll) 
			{ 
				// its not a network drive or a CD/floppy, game on!
				char drive = FirstDriveFromMask(lpdbv->dbcv_unitmask);
				if(drive==0) return 0;
				int p = ((int)drive) + (wParam==DBT_DEVICEARRIVAL?0x10000:0);
				ThreadFunc_DeviceChange((LPVOID)p);
			}
		}
	}
	return 0;
}

static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	switch(msg) {
		case PMP_DEVICECHANGE:
			return wmDeviceChange(param1,param2);
		case PMP_CONFIG:
			// TODO: Implement
			return 1;
	}
	return 0;
}

extern "C" 	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
{
	return &plugin;
}
//#define PLUGIN_NAME "Nullsoft USB Device Plug-in"
#define PLUGIN_VERSION "v0.9"

#include "UsbDevice.h"
#include <winioctl.h>
#include "../nu/AutoChar.h"
#include <strsafe.h>

int init();
void quit();
intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);
extern PMPDevicePlugin plugin = {PMPHDR_VER,0,init,quit,MessageProc};

// Start-NDE
// Delay load library control << begin >>
#include <delayimp.h>
#pragma comment(lib, "delayimp")

FARPROC WINAPI FailHook(unsigned dliNotify, PDelayLoadInfo  pdli) 
{
	char error[1024];
	StringCchPrintfA(error,1024, WASABI_API_LNGSTRING(IDS_DELAYLOAD_FAILURE), pdli->szDll);
	MessageBoxA(plugin.hwndLibraryParent, error, WASABI_API_LNGSTRING(IDS_ERROR), MB_OK | MB_ICONERROR);
	return 0;
}

extern "C"
{
	PfnDliHook __pfnDliFailureHook2 = FailHook;
}
// Delay load library control << end >>
// End-NDE

api_service *serviceManager;
int driveNumInArray(HWND hwndDlg, int cbNum = -1);
C_ItemList devices;
HWND config;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

// Metadata service
api_metadata *AGAVE_API_METADATA=0;

static C_ItemList blacklist;
static const char *winampini;

//set true if we want all devices to be detected
static bool g_detectAll=false;

C_ItemList loadingThreads;
static DWORD WINAPI ThreadFunc_Load(LPVOID lpParam);


template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
}

static void blacklistLoad() {
	char keyname[64];
	int l = GetPrivateProfileIntA("pmp_usb","blacklistnum",0,winampini);
	for(int i=l>100?l-100:0; i<l; i++) {
		char buf[100]="";
		wsprintfA(keyname,"blacklist-%d",i);
		GetPrivateProfileStringA("pmp_usb",keyname, "", buf, 100, winampini);
		if(buf[0]) 
		blacklist.Add(AutoWideDup(buf, CP_UTF8));
	}
}

static void blacklistSave() {
	char buf[64];
	wsprintfA(buf,"%d",blacklist.GetSize());
	WritePrivateProfileStringA("pmp_usb","blacklistnum",buf,winampini);
	for(int i=0; i<blacklist.GetSize(); i++) {
		wsprintfA(buf,"blacklist-%d",i);
		WritePrivateProfileStringA("pmp_usb",buf,AutoChar((const wchar_t*)blacklist.Get(i), CP_UTF8),winampini);
	}
}

static wchar_t *makeBlacklistString(wchar_t drive) {
	wchar_t path[4]={drive,L":\\"};
	wchar_t name[100]=L"";
	wchar_t buf[1024]=L"";
	DWORD serial=0;
	UINT olderrmode=SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	GetVolumeInformation(path,name,100,&serial,NULL,NULL,NULL,0);
	if(serial) {
		wsprintf(buf,L"s:%d",serial);
		if(!olderrmode) SetErrorMode(olderrmode);
		return _wcsdup(buf);
	}

	ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
	GetDiskFreeSpaceEx(path,  &tfree, &total, &freeb);
	wsprintf(buf,L"n:%s,%d,%d",name,total.HighPart,total.LowPart);
	if(!olderrmode) SetErrorMode(olderrmode);
	return _wcsdup(buf);
}

static bool blacklistCheck(wchar_t drive) {
	wchar_t *s = makeBlacklistString(drive);
	for(int i=0; i<blacklist.GetSize(); i++) if(!wcscmp(s,(wchar_t*)blacklist.Get(i))) { free(s); return true; }
	free(s);
	return false;
}

static void connectDrive(wchar_t drive, bool checkSize=true, bool checkBlacklist=true) {
	if(checkBlacklist && blacklistCheck(drive)) return;

	for(int i=0; i < devices.GetSize(); i++) 
		if(((UsbDevice*)devices.Get(i))->drive == drive) {
			return;
		}

	wchar_t path[4]=L"x:\\";
	path[0]=drive;
	if(checkSize) {
		ULARGE_INTEGER tfree={0,}, total={0,}, freeb={0,};
		GetDiskFreeSpaceExW(path,  &tfree, &total, &freeb);
		if(total.HighPart == 0 && total.LowPart == 0) return;
	}

	// I suppose we should check its not an iPod...
	{
		wchar_t iPodDb[] = {drive,L":\\iPod_Control"};
		WIN32_FIND_DATA ffd={0};
		HANDLE h = FindFirstFile(iPodDb,&ffd);
		if(h != INVALID_HANDLE_VALUE) {
			if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {FindClose(h); return;}
			FindClose(h);
		}
	}
	/*
	{
		wchar_t iPodDb[] = L"x:\\iPod_Control\\iTunes\\iTunesDB";
		iPodDb[0] = drive;
		FILE * f = _wfopen(iPodDb,L"rb");
		if(f) { fclose(f); return; }//if it is an ipod, close file
	}
	*/
	//not an ipod
	FILE *f;
	//do we know about the device already?
	wchar_t checkFile[100];
	wsprintf(checkFile,L"%c:\\" TAG_CACHE ,drive);
	f = _wfopen(checkFile,L"r");
	if(f) { fclose(f); }
	else { // new
		wchar_t drvname[100]=L"", titleStr[128];
		DWORD serial=0;
		GetVolumeInformation(path,drvname,100,&serial,NULL,NULL,NULL,0);
		wchar_t buf[1024];
		wsprintf(buf,WASABI_API_LNGSTRINGW(IDS_REMOVEABLE_DRIVE_DETECTED),drvname,towupper(drive));
		wchar_t * bstr = makeBlacklistString(drive);
		blacklist.Add(bstr);
		if(MessageBox(plugin.hwndLibraryParent,buf,
					  WASABI_API_LNGSTRINGW_BUF(IDS_WINAMP_PMP_SUPPORT,titleStr,128),
					  MB_YESNO|MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_TOPMOST) == IDNO)
			return;
		else
		{
			for(int i=0; i < blacklist.GetSize(); i++) if((wchar_t*)blacklist.Get(i) == bstr) { blacklist.Del(i); break; }
			free(bstr);
		}
	}

	DWORD dwThreadId; 
	loadingThreads.Add((void*)CreateThread(NULL, 0, ThreadFunc_Load, (LPVOID)(intptr_t)drive, 0, &dwThreadId));	
}

static DWORD WINAPI ThreadFunc_Load(LPVOID lpParam)
{
	wchar_t drive = (wchar_t)(intptr_t)lpParam;
	pmpDeviceLoading load;
	Device * d = new UsbDevice(drive,&load);
	return 0;
}

static void autoDetectCallback(wchar_t drive,UINT type) {
	if(type == DRIVE_REMOVABLE || g_detectAll)
	{
		connectDrive(drive,true,true);
	}

	if(type == DRIVE_FIXED)
	{
		if(blacklistCheck(drive)) return;
		wchar_t checkFile[100];
		wsprintf(checkFile,L"%c:\\" TAG_CACHE,drive);
		FILE *f = _wfopen(checkFile,L"r");
		if(f) {
			fclose(f);
			connectDrive(drive);
		}
	}
}

int init() {
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	winampini = (const char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILE);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);

	blacklistLoad();
	int l = GetPrivateProfileIntA("pmp_usb","blacklistnum",0,winampini);

	// loader so that we can get the localisation service api for use
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpUSBLangGUID);

	static char szDescription[256];
	wsprintfA(szDescription,"%s "PLUGIN_VERSION,WASABI_API_LNGSTRING(IDS_NULLSOFT_USB_DEVICE_PLUGIN));
	plugin.description = szDescription;

	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	return 0;
}

void quit() {
	while(loadingThreads.GetSize())
	{
		DWORD d = WaitForMultipleObjectsEx(loadingThreads.GetSize(),(HANDLE*)loadingThreads.GetAll(),TRUE,INFINITE,TRUE);
		if(d == WAIT_OBJECT_0) 
		{
			for(int i=0; i<loadingThreads.GetSize(); i++)
				CloseHandle((HANDLE)loadingThreads.Get(i));
			break;
		}
		else if(d == WAIT_FAILED) 
			break;
	}
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	blacklistSave();
	for(int i=0; i<blacklist.GetSize(); i++) free(blacklist.Get(i));
}

static wchar_t FirstDriveFromMask(ULONG *unitmask) {
	char i;
	ULONG adj = 0x1, mask = *unitmask;
	for(i=0; i<26; ++i) {
		if(mask & 0x1) {
			*unitmask -= adj;
			break;
		}
		adj = adj << 1;
		mask = mask >> 1;
	}
	return (i+L'A');
}

static int GetNumberOfDrivesFromMask(ULONG unitmask) {
	int count = 0;
	for(int i=0; i<26; ++i)
	{
		if(unitmask & 0x1)
			count++;

		unitmask = unitmask >> 1;
	}
	return count;
}

int wmDeviceChange(WPARAM wParam, LPARAM lParam) {
	UINT olderrmode=SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	if(wParam==DBT_DEVICEARRIVAL || wParam==DBT_DEVICEREMOVECOMPLETE)
	{ // something has been inserted or removed
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{ // its a volume
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			if((!(lpdbv->dbcv_flags & DBTF_MEDIA) && !(lpdbv->dbcv_flags & DBTF_NET)) || g_detectAll) 
			{ // its not a network drive or a CD/floppy, game on!
				ULONG dbcv_unitmask = lpdbv->dbcv_unitmask;

				// see just how many drives have been flagged on the action
				// eg one usb drive could have multiple partitions that we handle
				int count = GetNumberOfDrivesFromMask(dbcv_unitmask);
				for(int j = 0; j < count; j++)
				{
					wchar_t drive = FirstDriveFromMask(&dbcv_unitmask);
					if(wParam == DBT_DEVICEARRIVAL && !blacklistCheck(drive))
					{ // connected
						connectDrive(drive);
						//send a message as if the user just selected a drive from the combo box, this way the fields are refreshed to the correct device's settings
						SendMessage(config, WM_COMMAND,MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); 
					}
					else
					{ // removal
						for(int i=0; i < devices.GetSize(); i++) {
							UsbDevice * d = (UsbDevice*)devices.Get(i);
							if(d->drive == drive)
							{
								devices.Del(i);
								if(config) SendMessage(config,WM_USER,0,0); //refresh fields 
								if(config) SendMessage(config,WM_COMMAND, MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); //update to correct device change as if the user had clicked on the combo box themself
								SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)d,PMP_IPC_DEVICEDISCONNECTED);
								delete d;
							}
						}
					}
				}
			}
		}
	}
	SetErrorMode(olderrmode);
	return 0;
}

static int IsDriveConnectedToPMP(wchar_t drive) {
	int connected = 0;
	for(int i = 0; i < devices.GetSize(); i++) 
	{
		if(((UsbDevice*)devices.Get(i))->drive == drive) {
			connected = 1;
			break;
		}
	}
	return connected;
}

static INT_PTR CALLBACK config_dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			{
				for(wchar_t d=L'A'; d<='Z'; d++) {
					wchar_t drive[3] = {d,L':',0}, drv[4] = {d,L':','\\',0};
					{
						UINT uDriveType = GetDriveType(drv);
						if(uDriveType == DRIVE_REMOVABLE || uDriveType == DRIVE_CDROM || uDriveType == DRIVE_FIXED) {
							int position = SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_ADDSTRING,0,(LPARAM)drive);
							SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_SETITEMDATA,position,d);
						}
					}
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALCONNECT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALDISCONNECT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALBLACKLIST), FALSE);
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg,0);
			break;
        case WM_COMMAND:
			switch(LOWORD(wParam)) {
			    case IDC_COMBO_MANUALCONNECT: 
				{
					if(HIWORD(wParam)==CBN_SELCHANGE) {
						int indx = SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
						wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
						if(indx >= 0)
						{
							int connected = IsDriveConnectedToPMP(drive), isblacklisted = blacklistCheck(drive);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALCONNECT), !connected && !isblacklisted);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALDISCONNECT), connected);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALBLACKLIST), TRUE);
							SetDlgItemText(hwndDlg, IDC_MANUALBLACKLIST, WASABI_API_LNGSTRINGW(isblacklisted ? IDS_UNBLACKLIST_DRIVE : IDS_BLACKLIST_DRIVE));
						}
					}
				}
					break;
				case IDC_MANUALCONNECT:
				{
					wchar_t titleStr[32];
					if(MessageBox(hwndDlg, WASABI_API_LNGSTRINGW(IDS_MANUAL_CONNECT_PROMPT),
								  WASABI_API_LNGSTRINGW_BUF(IDS_WARNING,titleStr,32), MB_YESNO) == IDYES)
					{
						int indx = SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
						wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
						if(drive >= L'A' && drive <= L'Z') {
							wchar_t *bl = makeBlacklistString(drive);
							for(int i=0; i<blacklist.GetSize(); i++)
							{
								if(!wcscmp(bl,(wchar_t*)blacklist.Get(i)))
								{
									free(blacklist.Get(i));
									blacklist.Del(i);
									break;
								}
							}
							free(bl);
							connectDrive(drive,false);
							// should do a better check here incase of failure, etc
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALCONNECT), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_MANUALDISCONNECT), TRUE);
						}
					}
				}
					break;
				case IDC_MANUALDISCONNECT:
				{
					int indx = SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
					wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
					if(drive >= L'A' && drive <= L'Z') {
						for(int i=0; i < devices.GetSize(); i++) {
							UsbDevice * d = (UsbDevice*)devices.Get(i);
							if(d->drive == drive)
							{
								devices.Del(i);
								if(config) SendMessage(config,WM_USER,0,0); //refresh fields 
								if(config) SendMessage(config,WM_COMMAND, MAKEWPARAM(IDC_DRIVESELECT,CBN_SELCHANGE),0); //update to correct device change as if the user had clicked on the combo box themself
								SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)d,PMP_IPC_DEVICEDISCONNECTED);
								SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_MANUALCONNECT,CBN_SELCHANGE),0);
								delete d;
							}
						}
					}
				}
					break;
				case IDC_MANUALBLACKLIST:
				{
					int indx = SendDlgItemMessageW(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETCURSEL,0,0);
					wchar_t drive = (wchar_t)SendDlgItemMessage(hwndDlg,IDC_COMBO_MANUALCONNECT,CB_GETITEMDATA,indx,0);
					if(drive >= L'A' && drive <= L'Z') {
						wchar_t *bl = makeBlacklistString(drive);
						if(!blacklistCheck(drive)) {
							blacklist.Add(bl);
							// see if we've got a connected drive and prompt to remove it or wait till restart
							if(IsDriveConnectedToPMP(drive)) {
								wchar_t title[96] = {0};
								GetWindowText(hwndDlg, title, 96);
								if(MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_DRIVE_CONNECTED_DISCONNECT_Q),title,MB_YESNO)==IDYES){
									SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_MANUALDISCONNECT,0),0);
								}
							}
						}
						else {
							for(int i=0; i < blacklist.GetSize(); i++)
							{
								if(!wcscmp(bl,(wchar_t*)blacklist.Get(i)))
								{
									free(blacklist.Get(i));
									blacklist.Del(i);
									break;
								}
							}
							free(bl);
						}
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_COMBO_MANUALCONNECT,CBN_SELCHANGE),0);
					}
				}
					break;
				case IDOK:
					EndDialog(hwndDlg,0);
					break;
			}
			break;
	}
	return 0;
}

intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) {
	switch(msg) {
		case PMP_DEVICECHANGE:
			return wmDeviceChange(param1,param2);
		case PMP_CONFIG:
			WASABI_API_DIALOGBOXW(IDD_CONFIG_GLOBAL,(HWND)param1,config_dialogProc);
			return 1;
	}
	return 0;
}

extern "C" {
	__declspec( dllexport ) PMPDevicePlugin * winampGetPMPDevicePlugin(){return &plugin;}
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		int i = devices.GetSize();
		while(i-- > 0) ((UsbDevice*)devices.Get(i))->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};
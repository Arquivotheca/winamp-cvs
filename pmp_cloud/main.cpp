#define PLUGIN_VERSION L"0.85 Beta"
#include "api.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "../replicant/cloud/cb_cloudevents.h"
#include "../ml_cloud/shared.h"
#include <shlwapi.h>
#include <strsafe.h>
#include "resource.h"
#include "nswasabi/ReferenceCounted.h"

static wchar_t ini_file[MAX_PATH];
class DeviceCloudCallback : public cb_cloudevents
{
	void WASABICALL CloudEvents_OnDeviceAdded(ifc_cloudclient *client, nx_string_t device_token, int device_id, DeviceInfoStruct *deviceInfo);
	void WASABICALL CloudEvents_OnDeviceChanged(ifc_cloudclient *client, nx_string_t device_token, int device_id, nx_string_t device_name, DeviceInfoStruct *deviceInfo);
	void WASABICALL CloudEvents_OnDeviceRemoved(ifc_cloudclient *client, int device_id);
	void WASABICALL CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced);
	void WASABICALL CloudEvents_OnError(ifc_cloudclient *client, nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field);
	int WASABICALL CloudEvents_GetRunDeviceEnum() {
		int dev_mode = GetPrivateProfileInt(L"ml_cloud", L"dev_mode", 0, ini_file);
		// just look for a username and if so assume we're ok to show devices on load
		wchar_t username[16] = {0}, provider[16] = {0}, auth_token[16] = {0};
		GetPrivateProfileString(L"ml_cloud", L"username", L"", username, 16, ini_file);
		GetPrivateProfileString(L"ml_cloud", L"provider", L"", provider, 16, ini_file);
		switch(dev_mode)
		{
			case 0: // production
				GetPrivateProfileString(L"ml_cloud", L"auth_token", L"", auth_token, 16, ini_file);
				break;
			case 1: // dev
				GetPrivateProfileString(L"ml_cloud", L"dev_auth_token", L"", auth_token, 16, ini_file);
				break;
			case 2: // qa
				GetPrivateProfileString(L"ml_cloud", L"qa_auth_token", L"", auth_token, 16, ini_file);
				break;
			case 3: // stage
				GetPrivateProfileString(L"ml_cloud", L"stage_auth_token", L"", auth_token, 16, ini_file);
				break;
		}
		
		return (username[0] && provider[0] && auth_token[0]);
	}
};

static DeviceCloudCallback device_cloud_callback;
int winampVersion, firstpull = 0, network_fail = 0, IPC_GET_CLOUD_HINST = -1;
static int Init();
static void Quit();
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0, cloud_hinst = 0;
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);
PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};
ifc_cloudclient *cloud_client=0;
nx_string_t local_device_token=0;
int local_device_id=0;
nu::PtrList<CloudDevice> cloudDevices;

void DeviceCloudCallback::CloudEvents_OnDeviceAdded(ifc_cloudclient *client, nx_string_t device_token, int device_id, DeviceInfoStruct *deviceInfo)
{
	CloudDevice::SpecialDevice device_type = CloudDevice::DEVICE_CLIENT;
	CloudDevice::DevicePlatform platform_type = CloudDevice::PLATFORM_NULL;

	if (!NXStringKeywordCompareWithCString(device_token, HSS_CLIENT))
	{
		device_type = CloudDevice::DEVICE_HSS;
	}
	else if (!NXStringKeywordCompareWithCString(device_token, DROPBOX_CLIENT))
	{
		device_type = CloudDevice::DEVICE_DROPBOX;
	}
	else if (!NXStringKeywordCompare(device_token, local_device_token))
	{
		local_device_id = device_id;
		device_type = CloudDevice::DEVICE_LOCAL_LIBRARY;
	}
	else if (!NXStringKeywordCompareWithCString(device_token, OLE_WEB_CLIENT))
	{
		device_type = CloudDevice::DEVICE_WEB_CLIENT;
	}

	// for the time being do not show specific 'clients' as it otherwise will
	// confuse the experience for users who will just use anything randomly!
	// this specifically will prevent the 'local library' being seen

	// just in-case we're getting device updates, re-check the show_local
	// state so that we can add or remove the 'local library' device item

	if (device_type == CloudDevice::DEVICE_WEB_CLIENT)
		return;

	if (device_type == CloudDevice::DEVICE_LOCAL_LIBRARY
		&& !GetPrivateProfileInt(L"ml_cloud", L"show_local", 0, ini_file))
	{
		return;
	}

	// check for the device info so we can show an appropriate device
	// icon - if there is no match then we just default to a pc icon
	if (deviceInfo && deviceInfo->platform)
	{
		if (!NXStringKeywordCompareWithCString(deviceInfo->platform, "android"))
		{
			platform_type = CloudDevice::PLATFORM_ANDROID;
		}
		else if (!NXStringKeywordCompareWithCString(deviceInfo->platform, "apple"))
		{
			platform_type = CloudDevice::PLATFORM_ANDROID;
		}
		else if (!NXStringKeywordCompareWithCString(deviceInfo->platform, "windows"))
		{
			platform_type = CloudDevice::PLATFORM_WINDOWS;

			// refine the platform type if possible
			if (deviceInfo->type && device_type == CloudDevice::DEVICE_CLIENT)
			{
				if (!NXStringKeywordCompareWithCString(deviceInfo->type, "laptop"))
				{
					platform_type = CloudDevice::PLATFORM_WINDOWS_LAPTOP;
				}
			}
		}
	}

	CloudDevice *device = new CloudDevice(device_token, device_id, device_type, platform_type);
	// TODO: benski> deadlock alert!
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICECONNECTED);
	cloudDevices.push_back(device);
}

void DeviceCloudCallback::CloudEvents_OnDeviceChanged(ifc_cloudclient *client, nx_string_t device_token, int device_id, nx_string_t device_name, DeviceInfoStruct *deviceInfo)
{
	int new_find = 0;
	size_t index = cloudDevices.size();
	while(index--)
	{
		CloudDevice *device = cloudDevices[index];
		if (device->device_id == device_id/* && NXStringKeywordCompare(device->device_name, device_name)*/)
		{
			new_find = 1;
			device->device_name = device_name;
			// TODO: benski> deadlock alert!
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICENAMECHANGED);
			break;
		}
	}
	if (!new_find)
	{
		this->CloudEvents_OnDeviceAdded(client, device_token, device_id, deviceInfo);
	}

	// just in-case we're getting device updates, re-check the show_local
	// state so that we can add or remove the 'local library' device item
	if (!NXStringKeywordCompare(device_token, local_device_token))
	{
		if (!GetPrivateProfileInt(L"ml_cloud", L"show_local", 0, ini_file))
			this->CloudEvents_OnDeviceRemoved(client, device_id);
		else
		{
			int found = 0;
			index = cloudDevices.size();
			while(index--)
			{
				CloudDevice *device = cloudDevices[index];
				if (device->device_id == device_id)
				{
					found = 1;
					break;
				}
			}
			if (!found)
			{
				this->CloudEvents_OnDeviceAdded(client, device_token, device_id, deviceInfo);
			}
		}
	}
}

void DeviceCloudCallback::CloudEvents_OnDeviceRemoved(ifc_cloudclient *client, int device_id)
{
	size_t index = cloudDevices.size();
	while(index--)
	{
		CloudDevice *device = cloudDevices[index];
		if (device->device_id == device_id)
		{
			// TODO: benski> deadlock alert!
			SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICEDISCONNECTED);
			cloudDevices.eraseindex(index);
			delete device;
			break;
		}
	}
}

void DeviceCloudCallback::CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced)
{
	// use this to block anything we don't want to do before we've pulled
	firstpull = 1;

	HWND ml_pmp_window = FindWindow(L"ml_pmp_window", NULL);
	if (IsWindow(ml_pmp_window))
	{
		PostMessage(ml_pmp_window, WM_USER+4, 0, 0);
	}
}

void DeviceCloudCallback::CloudEvents_OnError(ifc_cloudclient *client, nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field)
{
	if (action)
	{
		if (!NXStringKeywordCompareWithCString(action, "pull") ||
			!NXStringKeywordCompareWithCString(action, "user-devices"))
		{
			nx_string_t con_fail;
			NXStringCreateWithFormatting(&con_fail, "%d", NErr_ConnectionFailed);
			if (!NXStringKeywordCompare(code, con_fail))
			{
				network_fail = 1;
			}
		}
	}
}

class CloudDeviceConnection : public ifc_deviceconnection
{
public:
	CloudDeviceConnection()
	{
	}
	const char *GetName()
	{
		return "cloud";
	}

	HRESULT GetIcon(wchar_t *buffer, size_t bufferMax, int width, int height)
	{
		if(FALSE == FormatResProtocol(MAKEINTRESOURCE(IDB_CLOUD), L"PNG", buffer, bufferMax))
			return E_FAIL;

		return S_OK;
	}

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax)
	{
		if (NULL == buffer)
			return E_POINTER;

		swprintf(buffer, bufferMax, L"%s", L"Internet");
		return S_OK;
	}
protected:

#define CBCLASS CloudDeviceConnection
	START_DISPATCH_INLINE;
	CB(API_GETNAME, GetName);
	CB(API_GETICON, GetIcon);
	CB(API_GETDISPLAYNAME, GetDisplayName);
	END_DISPATCH;
#undef CBCLASS
};

static CloudDeviceConnection cloud_connection;
static int Init()
{
	// check for ml_cloud and if not then don't load
	// as it otherwise cripples parts of the ui, etc
	IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);
	if (!cloud_hinst || cloud_hinst == (HINSTANCE)1)
		return PMP_INIT_FAILURE;

	winampVersion = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION);
	if (winampVersion < 0x5070)
		return PMP_INIT_FAILURE;

	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!WasabiInit(WASABI_API_SVC))
		return PMP_INIT_FAILURE;

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance, PmpCloudLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_CLOUD_DEVICE_PLUGIN), PLUGIN_VERSION);
	plugin.description = szDescription;

	PathCombineW(ini_file, (const wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORYW), L"Plugins\\gen_ml.ini");

	if (AGAVE_API_DEVICEMANAGER)
	{
		ifc_deviceconnection *connection = &cloud_connection;
		AGAVE_API_DEVICEMANAGER->ConnectionRegister(&connection, 1);
	}

	if (REPLICANT_API_METADATA)
	{
		ReferenceCountedNXString metadata;
		if (NXStringCreateWithUTF8(&metadata, "cloud/mediahash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MediaHashMetadata::MetadataKey_CloudMediaHash);

		if (NXStringCreateWithUTF8(&metadata, "cloud/art/album/hash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &ArtHashMetadata::MetadataKey_CloudArtHashAlbum);
	}

	if (REPLICANT_API_CLOUD)
	{
		REPLICANT_API_CLOUD->SetDevMode(GetPrivateProfileInt(L"ml_cloud", L"dev_mode", 0, ini_file));
		char winamp_id_str[40];
		GUID winamp_id;
		WASABI_API_APP->GetUserID(&winamp_id);
		sprintf(winamp_id_str, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", (int)winamp_id.Data1, (int)winamp_id.Data2, (int)winamp_id.Data3, (int)winamp_id.Data4[0], (int)winamp_id.Data4[1], (int)winamp_id.Data4[2], (int)winamp_id.Data4[3], (int)winamp_id.Data4[4], (int)winamp_id.Data4[5], (int)winamp_id.Data4[6], (int)winamp_id.Data4[7] );
		NXStringCreateWithUTF8(&local_device_token, winamp_id_str);

		if (REPLICANT_API_CLOUD->CreateCloudClient(local_device_token, &cloud_client) == NErr_Success && cloud_client)
		{
			cloud_client->RegisterCallback(&device_cloud_callback);
		}

		CloudDevice *device = new CloudDevice(NXStringCreateFromUTF8(ALL_SOURCES_CLIENT)/*device_token*/, 0/*device_id*/, CloudDevice::DEVICE_ALL_SOURCES/*device_type*/, CloudDevice::PLATFORM_NULL/*platform_type*/);
		// TODO: benski> deadlock alert!
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICECONNECTED);
		cloudDevices.push_back(device);
	}

	return PMP_INIT_SUCCESS;
}

static void Quit()
{
	// clean up the one we manually added for the 'all sources' view
	if (cloudDevices.size() == 1)
	{
		if (cloudDevices[0]->special_device == CloudDevice::DEVICE_ALL_SOURCES)
		{
			CloudDevice *device = cloudDevices[0];
			SendMessage(plugin.hwndPortablesParent, WM_PMP_IPC, (intptr_t)device, PMP_IPC_DEVICEDISCONNECTED);
			cloudDevices.erase(device);
			delete device;
		}
	}

	if (cloud_client) cloud_client->Release();
	WasabiQuit();
}

static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	if (msg == PMP_NO_CONFIG)
		return TRUE;

	return FALSE;
}

extern "C" {
	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
	{
		return &plugin;
	}

	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		size_t index = cloudDevices.size();
		while(index--) cloudDevices[index]->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};
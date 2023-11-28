#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./dropBoxFactory.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./mouseHook.h"
#include "./skinWindow.h"
#include "../nu/ptrlist.h"
#include "./guiObjects.h"
#include "./preferences.h"
#include "./preferencesShortcut.h"
#include "./typeCollection.h"
#include "./pluginShortcut.h"

#include <shlwapi.h>
#include <strsafe.h>

#define DROPBOX_PATH	TEXT("\\Plugins\\dropBox")


// {E2E4AD32-D87B-4b9e-BF58-46A1336DDC8F}
EXTERN_C const GUID defaultClassUid  = 
{ 0xe2e4ad32, 0xd87b, 0x4b9e, { 0xbf, 0x58, 0x46, 0xa1, 0x33, 0x6d, 0xdc, 0x8f } };

static const DROPBOXCLASSINFO defaultClassInfo = 
{
	defaultClassUid,
	{ FVIRTKEY | FCONTROL | FSHIFT, (WORD)'D', 0}, 
	DBCS_ONEINSTANCE | DBCS_SKINWINDOW | DBCS_WINAMPGROUP | DBCS_REMEMBERPROFILE | DBCS_SHOWHEADER,
	MAKEINTRESOURCE(IDS_DROPBOX),
	GUID_NULL,
	-32000,
	-32000,
};

#define CONFIGITEM_STR(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_STRING, ((LPCTSTR)(__defaultValue))}
#define CONFIGITEM_INT(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_INT, MAKEINTRESOURCE(__defaultValue)}

ConfigurationManager *pluginConfigManager = NULL;
ProfileManager *pluginProfileManager = NULL;
TypeCollection *pluginRegisteredTypes = NULL;


HRESULT Plugin_QueryConfiguration(REFGUID configId, Profile *profile, IConfiguration **ppConfig)
{
	return PLUGIN_CFGMNGR->QueryConfiguration(configId, profile, ppConfig);
}

static ConfigIniSection::CONFIGITEM  winampSettings[] = 
{
	CONFIGITEM_STR(CFG_TITLEFORMAT, TEXT("titlefmt"), TEXT("[%artist% - ]$if2(%title%,$filepart(%filename%))")), 
};

static ConfigIniSection::CONFIGITEM  mediaLibSettings[] = 
{
	CONFIGITEM_INT(CFG_ACTIONTYPE, TEXT("enqueuedef"), ACTIONTYPE_PLAY), 
};

void STDMETHODCALLTYPE DropboxWindow_RegisterConfig(ConfigurationManager *pcm);
void STDMETHODCALLTYPE SimpleView_RegisterConfig(ConfigurationManager *pcm);
void STDMETHODCALLTYPE DetailsView_RegisterConfig(ConfigurationManager *pcm);
void STDMETHODCALLTYPE Meterbar_RegisterConfig(ConfigurationManager *pcm);

#ifdef __cplusplus
extern "C" {
#endif 

static DropBoxFactory dropBoxFactory;
static IDataObject *clipboadObject = NULL;
static nu::HandleList<PLUGINUNLOADCALLBACK> *unloadCallbacks = NULL;
static nu::PtrList<DROPBOXCLASSINFO> registeredClasses;

static LPSTR pszWinampIniFile = NULL;
static LPSTR pszWinampIniDirectory = NULL;


static void Plugin_RegisterConfig()
{
	HRESULT hr;
	ConfigIniSection *pConfig;
	
	hr = ConfigIniSection::CreateConfig(winampSettingsGuid, FILEPATH_WINAMPINI, TEXT("winamp"), winampSettings, ARRAYSIZE(winampSettings), &pConfig);
	if (SUCCEEDED(hr)){ PLUGIN_CFGMNGR->Add(pConfig); pConfig->Release(); }
	hr = ConfigIniSection::CreateConfig(mediaLibrarySettingsGuid, FILEPATH_MEDIALIBINI, TEXT("gen_ml_config"), mediaLibSettings, ARRAYSIZE(mediaLibSettings), &pConfig);
	if (SUCCEEDED(hr)){ PLUGIN_CFGMNGR->Add(pConfig); pConfig->Release(); }

	DropboxWindow_RegisterConfig(PLUGIN_CFGMNGR);
	SimpleView_RegisterConfig(PLUGIN_CFGMNGR);
	DetailsView_RegisterConfig(PLUGIN_CFGMNGR);
	Meterbar_RegisterConfig(PLUGIN_CFGMNGR);
}


#define CREATE_OBJECT(__objectInstance, __objectCreator)\
	{if(NULL == ##__objectInstance) { ##__objectInstance = ##__objectCreator;\
			if (NULL == ##__objectInstance) return 1;}}

#define DELETE_OBJECT(__objectInstance)\
	{if(NULL != ##__objectInstance) { delete(##__objectInstance);  ##__objectInstance = NULL; }}

int Plugin_OnInit()
{	
	CREATE_OBJECT(pluginConfigManager, new ConfigurationManager());
	CREATE_OBJECT(pluginProfileManager, new ProfileManager());
	CREATE_OBJECT(pluginRegisteredTypes, new TypeCollection());

	if (!InitializeWasabiApi())
		return 1;

	Plugin_RegisterConfig();
	PLUGIN_REGTYPES->RegisterKnownTypes();

	WASABI_API_SVC->service_register(&dropBoxFactory);

	InitializeMouseHook();
	InitializeSkinSupport();

	DROPBOXCLASSINFO defaultClass;
	const DROPBOXCLASSINFO* pClass;
	if (SUCCEEDED(DropboxClass_Load(defaultClassUid, &defaultClass)))
		pClass =  &defaultClass;
	else
	{
		pClass = &defaultClassInfo;
		DropboxClass_Save(pClass);

		if (0 == PLUGIN_PROFILEMNGR->LoadProfiles(NULL, 0))
			PLUGIN_PROFILEMNGR->RegisterDefault();
	}
	Plugin_RegisterClass(pClass);
	

	Plugin_RegisterPreferences();
	PreferencesGeneral_RegisterPage();
	PreferencesView_RegisterPage();
	PreferencesFilter_RegisterPage();

	Plugin_RegisterShortcutPreferences();

	return 0;
}

void Plugin_OnConfig()
{
	Plugin_ShowPreferences();
}

void Plugin_OnQuit()
{
	Plugin_UnregisterShortcutPreferences();
	Plugin_UnregisterPreferences();
	

	if (NULL != clipboadObject && 
		S_OK == OleIsCurrentClipboard(clipboadObject))
	{
		OleFlushClipboard();
	}
	WASABI_API_SVC->service_deregister(&dropBoxFactory);

	if (NULL != unloadCallbacks)
	{
		size_t index = unloadCallbacks->size();
		while(index--)
			unloadCallbacks->at(index)();
		delete(unloadCallbacks);
	}
		
	RelasePluginFonts();

	UninitializeWasabiApi();
	PLUGIN_CFGMNGR->Clear();
	

	if (NULL != pszWinampIniFile)
	{
		free(pszWinampIniFile);
		pszWinampIniFile = NULL;
	}

	if (NULL != pszWinampIniDirectory)
	{
		free(pszWinampIniDirectory);
		pszWinampIniDirectory = NULL;
	}

	DELETE_OBJECT(pluginConfigManager);
	DELETE_OBJECT(pluginProfileManager);
	DELETE_OBJECT(pluginRegisteredTypes);
	
}

HRESULT Plugin_SetClipboard(IDataObject *pObject)
{
	clipboadObject = NULL;
	HRESULT hr = OleSetClipboard(pObject);
	if (NULL != pObject && SUCCEEDED(hr))
		clipboadObject = pObject;
	return hr;
}

void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback)
{
	if (NULL == unloadCallbacks)
	{
		unloadCallbacks = new nu::HandleList<PLUGINUNLOADCALLBACK>();
		if (NULL == unloadCallbacks)
			return;
	}
	unloadCallbacks->push_back(callback);
}

BOOL Plugin_EnsurePathExist(LPCTSTR pszDirectory)
{
	DWORD ec = ERROR_SUCCESS;
	UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	if (!CreateDirectory(pszDirectory, NULL))
	{
		ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec)
		{
			LPCTSTR pszBlock = pszDirectory;
			TCHAR szBuffer[MAX_PATH];
			
			LPCTSTR pszCursor = PathFindNextComponent(pszBlock);
			ec = (pszCursor == pszBlock || S_OK != StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock))) ?
					ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			pszBlock = pszCursor;
			
			while (ERROR_SUCCESS == ec && NULL != (pszCursor = PathFindNextComponent(pszBlock)))
			{
				if (pszCursor == pszBlock || S_OK != StringCchCatN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock)))
					ec = ERROR_INVALID_NAME;

				if (ERROR_SUCCESS == ec && !CreateDirectory(szBuffer, NULL))
				{
					ec = GetLastError();
					if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
				}
				pszBlock = pszCursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
	}
	SetErrorMode(errorMode);
	SetLastError(ec);
	return (ERROR_SUCCESS == ec);
}


HRESULT Plugin_GetWinampIniFile(LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	*pszBuffer = TEXT('\0');

	if (NULL == pszWinampIniFile)
	{
		LPCSTR pathA = (LPCSTR) SENDWAIPC(plugin.hwndParent, IPC_GETINIFILE, 0);
		if (NULL != pathA) 
			pszWinampIniFile = _strdup(pathA);

		if (NULL == pszWinampIniFile)
			return E_UNEXPECTED;
	}

	INT cchRequired = MultiByteToWideChar(CP_ACP, 0, pszWinampIniFile, -1, NULL, 0);
	if (cchRequired == 0)
		return HRESULT_FROM_WIN32(GetLastError());
	
	if (cchBufferMax < cchRequired)
		return STRSAFE_E_INSUFFICIENT_BUFFER;

	if (0 == MultiByteToWideChar(CP_ACP, 0, pszWinampIniFile, -1, pszBuffer, cchBufferMax))
		return HRESULT_FROM_WIN32(GetLastError());
	
	return S_OK;
}

HRESULT Plugin_GetWinampIniDirectory(LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	*pszBuffer = TEXT('\0');

	if (NULL == pszWinampIniDirectory)
	{
		LPCSTR pathA = (LPCSTR) SENDWAIPC(plugin.hwndParent, IPC_GETINIDIRECTORY, 0);
		if (NULL != pathA) 
			pszWinampIniDirectory = _strdup(pathA);

		if (NULL == pszWinampIniDirectory)
			return E_UNEXPECTED;
	}

	INT cchRequired = MultiByteToWideChar(CP_ACP, 0, pszWinampIniDirectory, -1, NULL, 0);
	if (cchRequired == 0)
		return HRESULT_FROM_WIN32(GetLastError());
	
	if (cchBufferMax < cchRequired)
		return STRSAFE_E_INSUFFICIENT_BUFFER;

	INT cchCopied = MultiByteToWideChar(CP_ACP, 0, pszWinampIniDirectory, -1, pszBuffer, cchBufferMax);
	if (0 == cchCopied)
		return HRESULT_FROM_WIN32(GetLastError());

	if (TEXT('\0') != pszBuffer[cchCopied - 1])
		pszBuffer[cchCopied] = TEXT('\0');

	return S_OK;

}

HRESULT Plugin_GetDropboxPath(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = Plugin_GetWinampIniDirectory(pszBuffer, cchBufferMax);
	if (FAILED(hr))
		return hr;

	INT pos = lstrlen(pszBuffer);
	hr = (cchBufferMax > pos) ? 
			StringCchCopy(pszBuffer + pos, cchBufferMax - pos, DROPBOX_PATH) : 
			STRSAFE_E_INSUFFICIENT_BUFFER;

	return hr;
}


HRESULT Plugin_RegisterClass(const DROPBOXCLASSINFO *pdbci)
{
	if (NULL == pdbci)
		return E_POINTER;

	DROPBOXCLASSINFO *pTarget = NULL;

	size_t index = registeredClasses.size();

	while(index--)
	{
		if (IsEqualGUID(pdbci->classUid, registeredClasses[index]->classUid))
		{
			pTarget = registeredClasses[index];
			DropboxClass_FreeString(pTarget->pszTitle);
			PluginShortcut_Unregister(pTarget->classUid);
			break;
		}
	}
	
	if (NULL == pTarget)
	{
		pTarget = (DROPBOXCLASSINFO*)malloc(sizeof(DROPBOXCLASSINFO));
		if (NULL == pTarget)
			return E_OUTOFMEMORY;
		registeredClasses.push_back(pTarget);
	}

	CopyMemory(pTarget, pdbci, sizeof(DROPBOXCLASSINFO));
	if (!IS_INTRESOURCE(pdbci->pszTitle))
		pTarget->pszTitle = StrDup(pdbci->pszTitle);

	if (0 != pTarget->shortcut.key)
		PluginShortcut_Register(pTarget->classUid, &pTarget->shortcut);

	return S_OK;
}

HRESULT Plugin_UnregisterClass(const UUID &classUid)
{
	size_t index = registeredClasses.size();
	while(index--)
	{
		if (IsEqualGUID(classUid, registeredClasses[index]->classUid))
		{
			DROPBOXCLASSINFO *pTarget = registeredClasses[index];
			registeredClasses.eraseindex(index);
			DropboxClass_FreeString(pTarget->pszTitle);
			PluginShortcut_Unregister(pTarget->classUid);
			free(pTarget);
			return S_OK;
		}
	}

	return S_FALSE;
}
DROPBOXCLASSINFO *Plugin_FindRegisteredClass(const UUID &classUid)
{
	size_t index = registeredClasses.size();
	while(index--)
	{
		if (IsEqualGUID(classUid, registeredClasses[index]->classUid))
			return  registeredClasses[index];
	}
	return NULL;
}

HRESULT Plugin_RegisterDropboxClass(DROPBOXCLASSINFO *pdbci)
{

	UUID defaultClassUid = { 0xac5c8eef, 0x47e9, 0x40d3, { 0x97, 0x73, 0x7c, 0xf8, 0x8, 0xcf, 0xc3, 0xfe } };
	DROPBOXCLASSINFO classInfo;
	ZeroMemory(&classInfo, sizeof(DROPBOXCLASSINFO));

	classInfo.classUid = defaultClassUid;
	classInfo.pszTitle = TEXT("My dropbox");
	classInfo.profileUid = GUID_NULL;
	classInfo.shortcut.fVirt = FVIRTKEY | FCONTROL | FSHIFT;
	classInfo.shortcut.key = VK_SPACE;
	classInfo.style = DBCS_ONEINSTANCE | DBCS_SKINWINDOW | DBCS_WINAMPGROUP;
	classInfo.x = 16;
	classInfo.y = 32;
	
	if (SUCCEEDED(DropboxClass_Save(&classInfo)))
	{		
		DROPBOXCLASSINFO readClass;
		if (SUCCEEDED(DropboxClass_Load(defaultClassUid, &readClass)))
		{
			if (0 != memcmp(&classInfo, &readClass, sizeof(DROPBOXCLASSINFO)))
				aTRACE_LINE("saved and readed class info are different");
		}
		else aTRACE_LINE("read class info failed");
	}
	else aTRACE_LINE("save class info failed");


	return S_OK;
}
#ifdef __cplusplus
}
#endif 


// {C8A2F60A-7690-4c24-B3FE-A4579456EC51}
EXTERN_C const GUID winampSettingsGuid = 
{ 0xc8a2f60a, 0x7690, 0x4c24, { 0xb3, 0xfe, 0xa4, 0x57, 0x94, 0x56, 0xec, 0x51 } };

// {A61A54DA-7AF0-4d81-9F33-EB8D7E2CA223}
EXTERN_C const GUID pluginSettingsGuid = 
{ 0xa61a54da, 0x7af0, 0x4d81, { 0x9f, 0x33, 0xeb, 0x8d, 0x7e, 0x2c, 0xa2, 0x23 } };


// {A20E266B-A075-4161-88D6-8C3F5F1F62B2}
EXTERN_C const GUID mediaLibrarySettingsGuid = 
{ 0xa20e266b, 0xa075, 0x4161, { 0x88, 0xd6, 0x8c, 0x3f, 0x5f, 0x1f, 0x62, 0xb2 } };


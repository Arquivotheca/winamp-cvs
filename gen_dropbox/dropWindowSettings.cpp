#include "./main.h"
#include "./plugin.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./profile.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./wasabiApi.h"
#include "./formatData.h"
#include "./itemView.h"
#include "./itemViewManager.h"
#include "./itemViewMeta.h"
#include <shlwapi.h>
#include <strsafe.h>

// {91D1BD23-B286-44ae-BD66-9E7FC8D080A7}
EXTERN_C const GUID windowSettingsGuid = 
{ 0x91d1bd23, 0xb286, 0x44ae, { 0xbd, 0x66, 0x9e, 0x7f, 0xc8, 0xd0, 0x80, 0xa7 } };


#define CONFIGITEM_STR(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_STRING, ((LPCTSTR)(__defaultValue))}
#define CONFIGITEM_INT(__key, __keyString, __defaultValue) { (__key), (__keyString), ConfigIniSection::CONFIGITEM_TYPE_INT, MAKEINTRESOURCE(__defaultValue)}

static ConfigIniSection::CONFIGITEM  windowSettings[] = 
{
	CONFIGITEM_STR(CFG_WINDOWSIZE, TEXT("windowSize"), TEXT("")), 
	CONFIGITEM_STR(CFG_ACTIVEVIEW, TEXT("activeView"), TEXT("")), 
};

void STDMETHODCALLTYPE DropboxWindow_RegisterConfig(ConfigurationManager *pcm)
{
	static BOOL configRegistered = FALSE;
	if (configRegistered) return;

	HRESULT hr;
	ConfigIniSection *pConfig;
	
	hr = ConfigIniSection::CreateConfig(windowSettingsGuid, FILEPATH_PROFILEINI, TEXT("Window"), windowSettings, ARRAYSIZE(windowSettings), &pConfig);
	if (SUCCEEDED(hr)){ pcm->Add(pConfig); pConfig->Release(); }

	configRegistered = TRUE;
}

void DropboxWindow_LoadProfileView(HWND hwnd, Profile *profile)
{
	IConfiguration *pConfig;
	
	if (NULL == profile || FAILED(profile->QueryConfiguration(windowSettingsGuid, &pConfig)))
		return;

	TCHAR szBuffer[1024];
	DropboxViewMeta *viewMeta = NULL;

	if (S_OK == pConfig->ReadString(CFG_ACTIVEVIEW, szBuffer, ARRAYSIZE(szBuffer)))
		viewMeta = PLUGIN_VIEWMNGR->FindByName(szBuffer);
  
	if (NULL == viewMeta)
		viewMeta = PLUGIN_VIEWMNGR->First();

	if (NULL != viewMeta)
		DropboxWindow_CreateView(hwnd, viewMeta->GetId());	

	if (SUCCEEDED(pConfig->ReadString(CFG_WINDOWSIZE, szBuffer, ARRAYSIZE(szBuffer))))
	{
		POINT pt;
		if (ParsePoint(szBuffer, &pt, NULL))
		{			
			DropboxWindow_ResizeFrame(hwnd, pt.x, pt.y, TRUE);
		}
	}
	
	pConfig->Release();
}

void DropboxWindow_SaveViewSize(HWND hwnd, Profile *profile)
{
	RECT rect;
	
	TCHAR szBuffer[512];
	IConfiguration *pConfig;

	if (NULL != profile && 
		GetClientRect(hwnd, &rect) &&
		SUCCEEDED(profile->QueryConfiguration(windowSettingsGuid, &pConfig)))
	{
		SIZE sz = {rect.right - rect.left, rect.bottom - rect.top};
		pConfig->WriteString(CFG_WINDOWSIZE, FormatSize(sz, szBuffer, ARRAYSIZE(szBuffer)));
		pConfig->Release();
	}
}

void DropboxWindow_SaveClassInfo(HWND hwnd, Profile *profile)
{		
	RECT rect;
	
	UUID classUid;
	if (DropboxWindow_GetClassUid(hwnd, &classUid)) 
	{
		const DROPBOXCLASSINFO *classInfo;
		classInfo = (DropboxWindow_GetClassUid(hwnd, &classUid)) ?
						Plugin_FindRegisteredClass(classUid) : NULL;
		
		if (NULL != classInfo && 0 == (DBCS_DONOTSAVE & classInfo->style))
		{
			if (0 != (DBCS_REMEMBERPROFILE & classInfo->style))
			{
				UUID profileUid;
				if (NULL == profile || FAILED(profile->GetUID(&profileUid)))
					profileUid = GUID_NULL;
				DropboxClass_SaveProfile(classUid, profileUid);
			}
			if (GetWindowRect(hwnd, &rect))
				DropboxClass_SavePosition(classUid, *((POINT*)&rect));
		}
	}
}

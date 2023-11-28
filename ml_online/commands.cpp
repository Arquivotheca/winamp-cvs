#include "main.h"
#include "./commands.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./navigation.h"
#include "./preferences.h"
#include "./messagebox.h"
#include "./serviceHelper.h"

#include "../winamp/wa_ipc.h"

#include <ifc_omservice.h>
#include <browserView.h>

#include <wininet.h>
#include <shlwapi.h>
#include <strsafe.h>

#define BEGIN_COMMAND_SELECT(__commandId)  switch(commandId) {
#define END_COMMAND_SELECT		}

#define OMCOMMAND(__commandId, __commandCode, __resultOut) case (__commandId):\
	{ BOOL result = ##__commandCode; \
		if (NULL != (__resultOut)) { *(__resultOut) = result;}\
		return TRUE;}

BOOL Command_SetServiceRating(ifc_omservice *service, INT rating)
{
	return SUCCEEDED(ServiceHelper_SetRating(service, rating, SHF_NOTIFY | SHF_VERBAL | SHF_SAVE));
}

BOOL Command_OpenServiceView(ifc_omservice *service)
{
	BOOL resultOk = FALSE;;
	Navigation *navigation;
	if (NULL != service && SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		HNAVITEM hItem = navigation->FindService(service->GetId(), NULL);
		if (NULL != hItem)
		{
			HRESULT hr = navigation->SelectItem(hItem, NULL);

			if (SUCCEEDED(hr))
				resultOk = TRUE;
		}
		navigation->Release();
	}

	return resultOk;
}

BOOL Command_OpenServicePopup(ifc_omservice *service)
{
	BOOL resultOk = FALSE;;
	Navigation *navigation;
	if (NULL != service && SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		HNAVITEM hItem = navigation->FindService(service->GetId(), NULL);
		if (NULL != hItem)
		{
			HWND hPopup;
			HRESULT hr = navigation->CreatePopup(hItem, &hPopup);
			if (SUCCEEDED(hr))
			{
				ShowWindow(hPopup, SW_SHOWNORMAL);
				resultOk = TRUE;
			}
		}
		navigation->Release();
	}

	return resultOk;
}
BOOL Command_ReportService(ifc_omservice *service)
{
	HWND hWinamp = Plugin_GetWinamp();
	if (NULL == hWinamp || !IsWindow(hWinamp))
		return FALSE;

	if (NULL == service) 
		return FALSE;
		
	WCHAR szUrl[256];
	WCHAR szClient[128];
	
	OMBROWSERMNGR->GetClientId(szClient, ARRAYSIZE(szClient));

	HRESULT hr = StringCchPrintf(szUrl, ARRAYSIZE(szUrl), L"http://www.winamp.com/legal/abuse?svc_id=%u&unique=%s", 
					service->GetId(), szClient);

	SENDWAIPC(hWinamp, IPC_OPEN_URL, szUrl);
	return TRUE;
}

BOOL Command_UnsubscribeService(ifc_omservice *service)
{
	return (SUCCEEDED(ServiceHelper_Subscribe(service, FALSE, SHF_NOTIFY | SHF_VERBAL | SHF_SAVE)));
}

BOOL Command_ShowServiceInfo(ifc_omservice *service)
{
	if (NULL == service)
		return FALSE;

	BOOL resultOk = FALSE;

	HRESULT hr;
	WCHAR szUrl[INTERNET_MAX_URL_LENGTH], szName[INTERNET_MAX_URL_LENGTH];
	
	DWORD cchName = ARRAYSIZE(szName);
	if (FAILED(service->GetName(szUrl, ARRAYSIZE(szUrl))) ||
		FAILED(UrlEscape(szUrl, szName, &cchName, URL_ESCAPE_SEGMENT_ONLY | URL_ESCAPE_PERCENT)))
	{
		StringCchCopy(szName, ARRAYSIZE(szName), L"Info");
	}

	hr = StringCchPrintf(szUrl, ARRAYSIZE(szUrl), L"http://client.winamp.com/service/detail/%s/%d#", szName, service->GetId());
	if (FAILED(hr)) return hr;

	Navigation *navigation;
	hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr))
	{
		HNAVITEM hRoot = navigation->FindService(ROOTSERVICE_ID, NULL);
		if (NULL != hRoot)
		{
			HNAVITEM hActive = navigation->GetActive(NULL);
			if (hActive == hRoot)
			{
				HWND hView = navigation->GetActiveView(NULL);
				if (NULL != hView && FALSE != BrowserView_Navigate(hView, szUrl, TRUE))
					resultOk = TRUE;
			}
			
			if (FALSE == resultOk && SUCCEEDED(navigation->SelectItem(hRoot, szUrl)))
				resultOk = TRUE;
		}
		navigation->Release();
	}
	return resultOk;
}

BOOL Command_ResetServicePolicy(ifc_omservice *service)
{
	return (SUCCEEDED(ServiceHelper_ResetPermissions(service, SHF_NOTIFY | SHF_VERBAL)));
}

BOOL Command_ResetSubscription()
{	
	HRESULT hr = ServiceHelper_ResetSubscription(SHF_VERBAL);
	return SUCCEEDED(hr);
}

static BOOL Command_OpenPreferences()
{	
	return Preferences_Show();
}

static BOOL Command_OpenHelp()
{	
	return (BOOL)SENDWAIPC(Plugin_GetWinamp(), IPC_OPEN_URL, L"http://www.winamp.com/help/Main_Page#Online_Services");
}

static BOOL Command_NavigateView(HWND hView, LPCWSTR navigateUrl)
{
	Navigation *navigation;
	if (FAILED(Plugin_GetNavigation(&navigation))) 
		return E_UNEXPECTED;

	HWND hActive = navigation->GetActiveView(NULL);
	if (hActive != hView) hView = NULL;
	
	BOOL resultOk = (NULL != hView  && 
					FALSE != BrowserView_Navigate(hView, navigateUrl, TRUE));
	
	navigation->Release();

	return resultOk;
}

BOOL Command_ProcessService(ifc_omservice *service, INT commandId, BOOL *fSuccess)
{	 
	BEGIN_COMMAND_SELECT(commandId)
		OMCOMMAND(ID_RATING_VALUE_5,		Command_SetServiceRating(service, 5), fSuccess);
		OMCOMMAND(ID_RATING_VALUE_4,		Command_SetServiceRating(service, 4), fSuccess);
		OMCOMMAND(ID_RATING_VALUE_3,		Command_SetServiceRating(service, 3), fSuccess);
		OMCOMMAND(ID_RATING_VALUE_2,		Command_SetServiceRating(service, 2), fSuccess);
		OMCOMMAND(ID_RATING_VALUE_1,		Command_SetServiceRating(service, 1), fSuccess);
		OMCOMMAND(ID_VIEW_OPEN,				Command_OpenServiceView(service), fSuccess);
		OMCOMMAND(ID_VIEW_OPENPOPUP,		Command_OpenServicePopup(service), fSuccess);
		OMCOMMAND(ID_SERVICE_REPORT,		Command_ReportService(service), fSuccess);
		OMCOMMAND(ID_SERVICE_UNSUBSCRIBE,	Command_UnsubscribeService(service), fSuccess);
		OMCOMMAND(ID_SERVICE_GETINFO,		Command_ShowServiceInfo(service), fSuccess);
		OMCOMMAND(ID_SERVICE_RESETPOLICY,	Command_ResetServicePolicy(service), fSuccess);
	END_COMMAND_SELECT

	return FALSE;
}

BOOL Command_ProcessGeneral(INT commandId, BOOL *fSuccess)
{
	BEGIN_COMMAND_SELECT(commandId)
		OMCOMMAND(ID_SERVICEMANAGER_RESET,	Command_ResetSubscription(), fSuccess);
		OMCOMMAND(ID_PLUGIN_PREFERENCES,		Command_OpenPreferences(), fSuccess);
		OMCOMMAND(ID_PLUGIN_HELP,	Command_OpenHelp(), fSuccess);
	END_COMMAND_SELECT

	return FALSE;
}

BOOL Command_ProcessView(HWND hView, INT commandId, BOOL *fSuccess)
{
	BEGIN_COMMAND_SELECT(commandId)
		OMCOMMAND(ID_NAVIGATION_HOME,	Command_NavigateView(hView, NAVIGATE_HOME), fSuccess);
		OMCOMMAND(ID_NAVIGATION_BACK,	Command_NavigateView(hView, NAVIGATE_BACK), fSuccess);
		OMCOMMAND(ID_NAVIGATION_FORWARD,	Command_NavigateView(hView, NAVIGATE_FORWARD), fSuccess);
		OMCOMMAND(ID_NAVIGATION_REFRESH,	Command_NavigateView(hView, NAVIGATE_REFRESH), fSuccess);
		OMCOMMAND(ID_NAVIGATION_STOP,	Command_NavigateView(hView, NAVIGATE_STOP), fSuccess);
	END_COMMAND_SELECT
	return FALSE;
}
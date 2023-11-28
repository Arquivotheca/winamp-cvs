#define GUID_DEFINE
#include "../../winamp/setup/svc_setup.h"
#undef GUID_DEFINE

#include "./setupPage.h"

#include "../wasabi.h"


static HRESULT Setup_RegisterPage()
{	
	HRESULT hr;
	svc_setup *setupSvc;
	SetupPage *page;


	if (FAILED(WasabiApi_LoadDefaults()) ||
		NULL == OMBROWSERMNGR ||
		NULL == OMSERVICEMNGR ||
		NULL == OMUTILITY) 
	{
		return E_UNEXPECTED;
	}
	
	setupSvc = QueryWasabiInterface(svc_setup, UID_SVC_SETUP);
	if (NULL == setupSvc) 
		return E_POINTER;

	page = SetupPage::CreateInstance();
	if (NULL == page) 
		hr = E_OUTOFMEMORY;
	else
	{
		size_t index;
		if (FAILED(setupSvc->GetPageCount(&index)))
			index = 0xFFFFF;
		else if (index > 0) 
			index--;

		hr = setupSvc->InsertPage(page, &index);
		if (SUCCEEDED(hr))
			setupSvc->AddJob((ifc_setupjob*)page);
		
		page->Release();
	}
	
	ReleaseWasabiInterface(UID_SVC_SETUP, setupSvc);
	
	return hr;
}

EXTERN_C _declspec(dllexport) BOOL RegisterSetup(HINSTANCE hInstance, api_service *waServices)
{
	BOOL result;

	if (FAILED(WasabiApi_Initialize(hInstance, waServices)))
		return FALSE;

	result = SUCCEEDED(Setup_RegisterPage());
	
	WasabiApi_Release();
	return result;
}

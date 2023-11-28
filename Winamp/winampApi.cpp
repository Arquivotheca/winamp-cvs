#include "main.h"
#include "./winampApi.h"


WinampApi::WinampApi() : ref(1)
{
}
WinampApi::~WinampApi()
{
}

HRESULT WinampApi::CreateInstance(WinampApi **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new WinampApi();
	if (NULL == instance) return E_OUTOFMEMORY;
	return S_OK;
}

const char *WinampApi::getServiceName() 
{ 
	return "Winamp API"; 
}

const GUID WinampApi::getServiceGuid()
{ 
	return winampApiGuid; 
}

size_t WinampApi::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t WinampApi::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int WinampApi::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	*object = NULL;
	return E_NOINTERFACE;
}

HWND WinampApi::GetMainWindow(void)
{
	return hMainWindow;
}

HWND WinampApi::GetDlgParent(void)
{
	return (g_dialog_box_parent) ? g_dialog_box_parent : hMainWindow;
}

HRESULT WinampApi::OpenUrl(HWND hwnd, const wchar_t *url)
{
	myOpenURL(hwnd, (wchar_t*)url);
	return S_OK;
}

int WinampApi::GetRegVer()
{
	return g_regver;
}

#define CBCLASS WinampApi
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETMAINWINDOW, GetMainWindow)
CB(API_GETDLGPARENT, GetDlgParent)
CB(API_OPENURL, OpenUrl)
CB(API_GETREGVER, GetRegVer)
END_DISPATCH;
#undef CBCLASS

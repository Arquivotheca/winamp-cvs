#include "main.h"
#include "./service.h"
#include "./api.h"
#include "external.h"

#include "../winamp/wa_ipc.h"
#include <strsafe.h>

#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

OmService::OmService()
	: ref(1)
{
}

OmService::~OmService()
{
}

HRESULT OmService::CreateInstance(OmService **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;
	
	OmService *service = new OmService();
	if (NULL == service) return E_OUTOFMEMORY;

	*instance = service;
	return S_OK;
}

size_t OmService::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmService::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmService::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmService))
		*object = static_cast<ifc_omservice*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

unsigned int OmService::GetId()
{
	return SERVICE_ID;
}

HRESULT OmService::GetName(wchar_t *pszBuffer, int cchBufferMax)
{
	return StringCchCopy(pszBuffer, cchBufferMax, L"WebAuth Service");
}

HRESULT OmService::GetUrl(wchar_t *pszBuffer, int cchBufferMax)
{
	return StringCchCopy(pszBuffer, cchBufferMax, SERVICE_HOMEURL);
}

HRESULT OmService::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;
	
	return WebAuthDispatch::CreateInstance((WebAuthDispatch **)ppDispatch);
}

#define CBCLASS OmService
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETID, GetId)
CB(API_GETNAME, GetName)
CB(API_GETURL, GetUrl)
CB(API_GETEXTERNAL, GetExternal)
END_DISPATCH;
#undef CBCLASS

	
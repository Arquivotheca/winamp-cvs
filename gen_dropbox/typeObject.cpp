#include "main.h"
#include "./typeObject.h"
#include "./wasabiApi.h"

#include <strsafe.h>

TypeObject::TypeObject(UINT nId, LPCSTR pszName, UINT nIconId, LPCTSTR pszDisplayName, LPCTSTR pszDescription, UINT nCapabilities, CreatorProc fnCreator) : 
	ref(1), id(nId), name(NULL), displayName(NULL), description(NULL), capabilities(nCapabilities),
	iconId(nIconId), createCallback(fnCreator)
{
	name = 	lfh_strdupA(pszName);
	
	displayName = (IS_INTRESOURCE(pszDisplayName)) ? 
			(LPTSTR)pszDisplayName : lfh_strdup(pszDisplayName);
	
	description = (IS_INTRESOURCE(pszDescription)) ? 
			(LPTSTR)pszDescription : lfh_strdup(pszDescription);
	
}

TypeObject::~TypeObject()
{
	if (NULL != name)
		lfh_free(name);

	if (NULL != displayName && !IS_INTRESOURCE(displayName))
		lfh_free(displayName);

	if (NULL != description && !IS_INTRESOURCE(description))
		lfh_free(description);
}

STDMETHODIMP_(ULONG) TypeObject::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) TypeObject::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP TypeObject::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_ITypeInfo))
		*ppvObject = (IFileInfo*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((ITypeInfo*)*ppvObject)->AddRef();
    return S_OK;
}
	
STDMETHODIMP_(BYTE) TypeObject::GetId()
{
	return id;
}

STDMETHODIMP TypeObject::GetName(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;

	if (NULL == name) 
	{
		*pszBuffer = '\0';
		hr = E_NOTIMPL;
	}
	else
	{
		if (0 ==MultiByteToWideChar(CP_ACP, 0, name, -1, pszBuffer, cchBufferMax))
			hr = HRESULT_FROM_WIN32(GetLastError());
	}
	
	return hr;
}



STDMETHODIMP TypeObject::GetDisplayName(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;

	if (NULL == displayName) 
		*pszBuffer = TEXT('\0');
	else if (IS_INTRESOURCE(displayName))
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)displayName, pszBuffer, cchBufferMax);	
	else
		hr = StringCchCopy(pszBuffer, cchBufferMax, displayName);
	
	return hr;
}

STDMETHODIMP TypeObject::GetDescription(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;

	if (NULL == description) 
		*pszBuffer = TEXT('\0');
	else if (IS_INTRESOURCE(description))
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)description, pszBuffer, cchBufferMax);	
	else
		hr = StringCchCopy(pszBuffer, cchBufferMax, description);
	
	return hr;
}

STDMETHODIMP_(UINT) TypeObject::GetCapabilities()
{
	return capabilities;
}

STDMETHODIMP_(UINT) TypeObject::GetIconId()
{
	return iconId;
}
STDMETHODIMP TypeObject::CreateItem(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData, IFileInfo **ppItem)
{
	if (NULL == createCallback)
		return E_POINTER;
	return createCallback(pszFilePath, pAttributeData, ppItem);
}


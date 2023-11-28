#include "./main.h"
#include "./plugin.h"
#include "./feLink.h"
#include "./fiUnknown.h"
#include <shlwapi.h>
#include <shlguid.h>
#include <strsafe.h>

LinkFileEnumerator::LinkFileEnumerator(LPCTSTR pszLinkFile) 
	: ref(1), pszLink(NULL), pLink(NULL)
{
	pszLink = lfh_strdup(pszLinkFile);
}

LinkFileEnumerator::~LinkFileEnumerator()
{
	if (NULL != pszLink)
		lfh_free(pszLink);

	if (NULL != pLink)
		pLink->Release();
}


STDMETHODIMP_(ULONG) LinkFileEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) LinkFileEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP LinkFileEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileEnumerator))
		*ppvObject = (IFileEnumerator*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP LinkFileEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *puFetched)
{
	if(celt <= 0 || pfiBuffer == NULL)
		return S_FALSE;
	
	if(puFetched == NULL && celt != 1)
		return S_FALSE;

	if(puFetched != NULL)
		*puFetched = 0;
	
	pfiBuffer[0] = NULL;

	if (NULL != pLink)
		return S_FALSE;

	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
	if(SUCCEEDED(hr))
	{
		IPersistFile *ppf;
		hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if(SUCCEEDED(hr))
		{
			hr = ppf->Load(pszLink, STGM_READ);
			if (SUCCEEDED(hr))
			{
				hr = pLink->Resolve(NULL, SLR_NO_UI | SLR_NOUPDATE | SLR_NOSEARCH);
				if (SUCCEEDED(hr))
				{
					TCHAR szBuffer[MAX_PATH*2];
					hr = pLink->GetPath(szBuffer, ARRAYSIZE(szBuffer), NULL, 0);
					if (SUCCEEDED(hr))
					{
						hr = PLUGIN_REGTYPES->CreateItem(szBuffer, NULL, &pfiBuffer[0]);
						if (FAILED(hr))
						{
							hr = E_FILEENUM_CREATEINFO_FAILED;
						}
					}
				}
			}
			ppf->Release();
		}
	}

	if (S_OK == hr && NULL != puFetched)
		*puFetched = 1;

	
	return hr;

}

STDMETHODIMP LinkFileEnumerator::Skip(ULONG celt)
{		
	return E_NOTIMPL;
}

STDMETHODIMP LinkFileEnumerator::Reset(void)
{	
	if (NULL != pLink)
	{
		pLink->Release();
		pLink = NULL;
	}
	return S_OK;
}
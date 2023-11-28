#include "./main.h"
#include "./plugin.h"
#include "./feFileNames.h"
#include "./fileInfoInterface.h"
#include "../nu/AutoWideFn.h"



FileNamesEnumeratorA::FileNamesEnumeratorA(LPCSTR pszFileNames) 
	: ref(1), pszFiles(NULL), pszCursor(NULL)
{	
	INT cchLen = 0;
	if (NULL != pszFileNames)
	{
		LPCSTR p = pszFileNames;
		while ('\0' != *p)
		{
			INT partLen = lstrlenA(p);
			if (0 != partLen)
			{
				partLen++;
				cchLen += partLen;
				p += partLen;
			}
		}
	}
	
	pszFiles = (LPSTR)malloc(sizeof(CHAR) * (cchLen + 1));
	if (NULL != pszFiles)
	{
		if (cchLen > 0)
			CopyMemory(pszFiles, pszFileNames, sizeof(CHAR) * cchLen);
		pszFiles[cchLen] = '\0';
	}
	pszCursor = pszFiles;
}

FileNamesEnumeratorA::~FileNamesEnumeratorA()
{
	if (NULL != pszFiles)
		free(pszFiles);

}

STDMETHODIMP_(ULONG) FileNamesEnumeratorA::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) FileNamesEnumeratorA::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP FileNamesEnumeratorA::QueryInterface(REFIID riid, PVOID *ppvObject)
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


STDMETHODIMP FileNamesEnumeratorA::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{
	ULONG cReturn = 0L;
	if(celt <= 0 || pfiBuffer == NULL || NULL == pszCursor || '\0' == *pszCursor)
		return S_FALSE;

	if(pceltFetched == NULL && celt != 1)
		return S_FALSE;

	if(pceltFetched != NULL)
		*pceltFetched = 0;
	
	HRESULT hr = S_OK;
	
	while(celt > 0)
	{
		LPCSTR pBlock = pszCursor;
		while('\0' != *pszCursor) pszCursor++;
		pszCursor++;

		if (pszCursor == pBlock)
			break;

		
		hr = PLUGIN_REGTYPES->CreateItem(AutoWideFn(pBlock), NULL, &pfiBuffer[cReturn]);
		if (S_OK != hr) break;

		cReturn++;
		celt--;
	}
	
	if (pceltFetched) 
		*pceltFetched = (cReturn - celt);

	return (cReturn > 0 && S_OK == hr) ? S_OK : S_FALSE;
}

STDMETHODIMP FileNamesEnumeratorA::Skip(ULONG celt)
{
	if (NULL == pszCursor)
		return E_POINTER;

	while(celt > 0 && '\0' != *pszCursor)
	{
		while('\0' != *pszCursor) pszCursor++;
		pszCursor++;
		celt--;
	}
	return S_OK;
}

STDMETHODIMP FileNamesEnumeratorA::Reset(void)
{
	pszCursor = pszFiles;
	return S_OK;
}

FileNamesEnumeratorW::FileNamesEnumeratorW(LPCWSTR pszFileNames) 
	: ref(1), pszFiles(NULL), pszCursor(NULL)
{
	INT cchLen = 0;
	if (NULL != pszFileNames)
	{
		LPCWSTR p = pszFileNames;
		while (L'\0' != *p)
		{
			INT partLen = lstrlenW(p);
			if (0 != partLen)
			{
				partLen++;
				cchLen += partLen;
				p += partLen;
			}
		}
	}
	
	pszFiles = (LPWSTR)malloc(sizeof(WCHAR) * (cchLen + 1));
	if (NULL != pszFiles)
	{
		if (cchLen > 0)
			CopyMemory(pszFiles, pszFileNames, sizeof(WCHAR) * cchLen);
		pszFiles[cchLen] = L'\0';
	}
	pszCursor = pszFiles;
}

FileNamesEnumeratorW::~FileNamesEnumeratorW()
{
	if (NULL != pszFiles)
		free(pszFiles);
}


STDMETHODIMP_(ULONG) FileNamesEnumeratorW::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) FileNamesEnumeratorW::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP FileNamesEnumeratorW::QueryInterface(REFIID riid, PVOID *ppvObject)
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


STDMETHODIMP FileNamesEnumeratorW::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{
	ULONG cReturn = 0L;
	if(celt <= 0 || pfiBuffer == NULL || NULL == pszCursor || L'\0' == *pszCursor)
		return S_FALSE;

	if(pceltFetched == NULL && celt != 1)
		return S_FALSE;

	if(pceltFetched != NULL)
		*pceltFetched = 0;


	HRESULT hr = S_OK;
	while(celt > 0)
	{
		LPCWSTR pBlock = pszCursor;
		while(L'\0' != *pszCursor) pszCursor++;
		pszCursor++;

		if (pszCursor == pBlock)
			break;

		hr = PLUGIN_REGTYPES->CreateItem(pBlock, NULL, &pfiBuffer[cReturn]);

		if (S_OK != hr) 
		{
			hr = E_FILEENUM_CREATEINFO_FAILED;
			pfiBuffer[cReturn] = NULL;
			break;
		}

		cReturn++;
		celt--;
	}
	
	if (pceltFetched) 
		*pceltFetched = (cReturn - celt);

	if (S_OK != hr)
		return hr;
	return (cReturn > 0) ? S_OK : S_FALSE;

}

STDMETHODIMP FileNamesEnumeratorW::Skip(ULONG celt)
{
	if (NULL == pszCursor)
		return E_POINTER;

	while(celt > 0 && '\0' != *pszCursor)
	{
		while(L'\0' != *pszCursor) pszCursor++;
		pszCursor++;
		celt--;
	}
	return S_OK;
}

STDMETHODIMP FileNamesEnumeratorW::Reset(void)
{
	pszCursor = pszFiles;
	return S_OK;
}
#include "./main.h"
#include "./plugin.h"
#include "./feFolder.h"
#include "./fiUnknown.h"

#include <shlwapi.h>
#include <strsafe.h>

FolderFileEnumerator::FolderFileEnumerator(LPCTSTR  pszFolder) : ref(1), hFind(NULL)
{
	if (FAILED(StringCchCopyEx(szPath, ARRAYSIZE(szPath), pszFolder, NULL, NULL, STRSAFE_IGNORE_NULLS)))
		szPath[0] = TEXT('\0');
}

FolderFileEnumerator::~FolderFileEnumerator()
{
	if (NULL != hFind)
	{
		FindClose(hFind);
		hFind = NULL;
	}
}
STDMETHODIMP_(ULONG) FolderFileEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) FolderFileEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP FolderFileEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP FolderFileEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{	
	if (0 == celt || NULL == pfiBuffer) 
		return S_FALSE;

	ULONG counter = 0;
	BOOL bFoundNext;
	HRESULT hr = S_OK;
	TCHAR szBuffer[2*MAX_PATH];

	if (NULL == hFind)
	{
		
		PathCombine(szBuffer, szPath, TEXT("*"));
		hFind = FindFirstFile(szBuffer, &fData);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			DWORD error = GetLastError();
			return HRESULT_FROM_WIN32(error);
		}
		bFoundNext = TRUE;
	}
	else
	{
		bFoundNext = FindNextFile(hFind, &fData);
	}
	
	if (bFoundNext)
	{		
		do 
		{	
			LPCTSTR p = fData.cFileName;
			while(TEXT('.') == *p && TEXT('\0') != *p) p++;
			if (TEXT('\0') != *p)
			{				
				if (!PathCombine(szBuffer, szPath, fData.cFileName))
					PathCombine(szBuffer, szPath, fData.cAlternateFileName);
				
				WIN32_FILE_ATTRIBUTE_DATA attributeData;
				UnknownFileInfo::FindDataToFileAttribute(&fData, &attributeData);
				
				hr = PLUGIN_REGTYPES->CreateItem(szBuffer, &attributeData, &pfiBuffer[counter]);

				if (S_OK != hr) 
				{
					hr = E_FILEENUM_CREATEINFO_FAILED;
					break;
				}
				celt--;
				counter++;
				if (0 == celt) break;
			}
			bFoundNext = FindNextFile(hFind, &fData);	
		}while(bFoundNext);
	}

	if (!bFoundNext)
	{
		FindClose(hFind);
		hFind = NULL;
	}

	if (pceltFetched) *pceltFetched = counter;
	
	if (S_OK != hr)
		return hr;
	return (counter > 0) ? S_OK : S_FALSE;
}

STDMETHODIMP FolderFileEnumerator::Skip(ULONG celt)
{
	
	return E_NOTIMPL;
}

STDMETHODIMP FolderFileEnumerator::Reset(void)
{
	if (NULL != hFind)
	{
		FindClose(hFind);
		hFind = NULL;
	}
	return S_OK;
}

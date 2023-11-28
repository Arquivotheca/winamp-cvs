#include "./main.h"
#include "./fiStream.h"
#include "./itemTypeInterface.h"
#include <shlwapi.h>
#include <strsafe.h>


StreamFileInfo::StreamFileInfo(LPCTSTR pszStreamUrl) 
	: ref(1), pszUrl(NULL), extraData(0)
{	

	if (NULL != pszStreamUrl)
	{
		int cchLen = lstrlen(pszStreamUrl);
		if (cchLen > 0)
		{
			cchLen++;
			pszUrl = (LPTSTR)lfh_malloc(sizeof(TCHAR) * cchLen);
			CopyMemory(pszUrl, pszStreamUrl, sizeof(TCHAR) * cchLen);
		}
	}
}

StreamFileInfo::~StreamFileInfo()
{	
	if (NULL != pszUrl)
		lfh_free(pszUrl);
}

STDMETHODIMP StreamFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new StreamFileInfo(filePath);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP_(ULONG) StreamFileInfo::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) StreamFileInfo::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP StreamFileInfo::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileInfo))
		*ppvObject = (IFileInfo*)this;
	else if (IsEqualIID(riid, IID_IFileMeta))
		*ppvObject = (IFileMeta*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)(IFileInfo*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP StreamFileInfo::GetPath(LPCTSTR *pszBuffer)
{
	if (!pszUrl)
		return E_POINTER;
	*pszBuffer = pszUrl;
	return S_OK;
}

STDMETHODIMP StreamFileInfo::GetFileName(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = pszUrl;
	return S_OK;
}


STDMETHODIMP StreamFileInfo::GetExtension(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = NULL;
	return E_NOTIMPL;
}


STDMETHODIMP StreamFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeHttpStream;
	
	return S_OK;
}
STDMETHODIMP StreamFileInfo::GetSize(ULONGLONG *pSize)
{
	if (NULL != pSize)
		*pSize = 0;
	return E_NOTIMPL;
}
STDMETHODIMP StreamFileInfo::GetAttributes(DWORD *pAttributes)
{
	if (NULL != pAttributes)
		*pAttributes = 0;
	return E_NOTIMPL;
}

STDMETHODIMP StreamFileInfo::GetCreationTime(FILETIME *pTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP StreamFileInfo::GetModifiedTime(FILETIME *pTime)
{
	return E_NOTIMPL;
}


STDMETHODIMP StreamFileInfo::ResetCache(void)
{
	return S_OK;
}

STDMETHODIMP StreamFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL != ppEnumerator)
		*ppEnumerator = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP StreamFileInfo::CanCopy(void)
{
	return S_FALSE;
}

STDMETHODIMP StreamFileInfo::CanPlay(void)
{
	return S_OK;
}

STDMETHODIMP StreamFileInfo::SetExtraInfo(ULONG_PTR data)
{
	extraData = data;
	return S_OK;
}
STDMETHODIMP StreamFileInfo::GetExtraInfo(ULONG_PTR *pData)
{
	if (NULL == pData)
		return E_POINTER;
	*pData = extraData;
	return S_OK;
}
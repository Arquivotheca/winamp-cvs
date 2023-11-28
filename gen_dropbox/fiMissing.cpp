#include "./main.h"
#include "./fiMissing.h"
#include <shlwapi.h>
#include <strsafe.h>
#include "./itemTypeInterface.h"


MissingFileInfo::MissingFileInfo(LPCTSTR pszFilePath) 
	: ref(1), pszPath(NULL), extraData(0)
{	
	if (NULL != pszFilePath)
	{
		int cchLen = lstrlen(pszFilePath);
		if (cchLen > 0)
		{
			cchLen++;
			pszPath = (LPTSTR)lfh_malloc(sizeof(TCHAR) * cchLen);
			CopyMemory(pszPath, pszFilePath, sizeof(TCHAR) * cchLen);
		}
	}
}

MissingFileInfo::~MissingFileInfo()
{	
	if (NULL != pszPath)
		lfh_free(pszPath);
}

STDMETHODIMP MissingFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new MissingFileInfo(filePath);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}


STDMETHODIMP_(ULONG) MissingFileInfo::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) MissingFileInfo::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP MissingFileInfo::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP MissingFileInfo::GetPath(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = pszPath;
	return S_OK;
}

STDMETHODIMP MissingFileInfo::GetFileName(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = PathFindFileName(pszPath);
	return S_OK;
}


STDMETHODIMP MissingFileInfo::GetExtension(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;

	LPCTSTR pszExtension = PathFindExtension(pszPath);
	if (TEXT('.') == *pszExtension)
		pszExtension++;
	else
		pszExtension = TEXT("");

	*pszBuffer = pszExtension;
	return S_OK;
}


STDMETHODIMP MissingFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeMissingFile;
	
	return S_OK;
}
STDMETHODIMP MissingFileInfo::GetSize(ULONGLONG *pSize)
{
	if (NULL != pSize)
		*pSize = 0;
	return E_NOTIMPL;
}
STDMETHODIMP MissingFileInfo::GetAttributes(DWORD *pAttributes)
{
	if (NULL != pAttributes)
		*pAttributes = 0;
	return E_NOTIMPL;
}

STDMETHODIMP MissingFileInfo::GetCreationTime(FILETIME *pTime)
{
	return E_NOTIMPL;
}

STDMETHODIMP MissingFileInfo::GetModifiedTime(FILETIME *pTime)
{
	return E_NOTIMPL;
}


STDMETHODIMP MissingFileInfo::ResetCache(void)
{
	return S_OK;
}


STDMETHODIMP MissingFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL != ppEnumerator)
		*ppEnumerator = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP MissingFileInfo::CanCopy(void)
{
	return S_FALSE;
}

STDMETHODIMP MissingFileInfo::CanPlay(void)
{
	return S_FALSE;
}

STDMETHODIMP MissingFileInfo::SetExtraInfo(ULONG_PTR data)
{
	extraData = data;
	return S_OK;
}
STDMETHODIMP MissingFileInfo::GetExtraInfo(ULONG_PTR *pData)
{
	if (NULL == pData)
		return E_POINTER;
	*pData = extraData;
	return S_OK;
}
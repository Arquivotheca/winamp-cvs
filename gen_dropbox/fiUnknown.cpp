#include "./main.h"
#include "./fiUnknown.h"
#include "./itemTypeInterface.h"
#include <shlwapi.h>
#include <strsafe.h>

void UnknownFileInfo::FindDataToFileAttribute(WIN32_FIND_DATA *pFindData, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData)
{
	pAttributeData->dwFileAttributes = pFindData->dwFileAttributes;
	pAttributeData->ftCreationTime = pFindData->ftCreationTime;
	pAttributeData->ftLastWriteTime = pFindData->ftLastWriteTime;
	pAttributeData->ftLastAccessTime = pFindData->ftLastAccessTime;
	pAttributeData->nFileSizeHigh = pFindData->nFileSizeHigh;
	pAttributeData->nFileSizeLow = pFindData->nFileSizeLow;
}
HRESULT UnknownFileInfo::ReadFileAttributes(LPCTSTR pszPath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData)
{
	if (NULL == pAttributeData)
		return E_INVALIDARG;

	ZeroMemory(pAttributeData, sizeof(WIN32_FILE_ATTRIBUTE_DATA));

	if (!GetFileAttributesEx(pszPath, GetFileExInfoStandard, pAttributeData))
	{
		DWORD error = GetLastError();
		if (ERROR_SHARING_VIOLATION == error)
		{
			SetLastError(ERROR_SUCCESS);
			WIN32_FIND_DATA fd;
			HANDLE hFind = FindFirstFile(pszPath, &fd);
			if (INVALID_HANDLE_VALUE != hFind)
			{
				FindDataToFileAttribute(&fd, pAttributeData);
				FindClose(hFind);
				return S_OK;
			}
		}
		return HRESULT_FROM_WIN32(error);
	}
	return S_OK;
}

UnknownFileInfo::UnknownFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) 
	: ref(1), pszPath(NULL), bInfoRead(FALSE), extraData(0)
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
	if (NULL != pAttributeData)
	{
		CopyMemory(&info, pAttributeData, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
		bInfoRead = TRUE;
	}
	else
		ZeroMemory(&info, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
}

UnknownFileInfo::~UnknownFileInfo()
{	
	if (NULL != pszPath)
		lfh_free(pszPath);
}

STDMETHODIMP UnknownFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new UnknownFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP_(ULONG) UnknownFileInfo::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) UnknownFileInfo::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP UnknownFileInfo::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP UnknownFileInfo::GetPath(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = pszPath;
	return S_OK;
}

STDMETHODIMP UnknownFileInfo::GetFileName(LPCTSTR *pszBuffer)
{
	if (!pszBuffer)
		return E_POINTER;
	*pszBuffer = PathFindFileName(pszPath);
	return S_OK;
}


STDMETHODIMP UnknownFileInfo::GetExtension(LPCTSTR *pszBuffer)
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


STDMETHODIMP UnknownFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeUnknown;
	
	return S_OK;
}
STDMETHODIMP UnknownFileInfo::GetSize(ULONGLONG *pSize)
{
	if (NULL == pSize)
		return E_POINTER;

	if (FALSE == bInfoRead)
	{
		ReadFileAttributes(pszPath, &info);
		bInfoRead = TRUE;
	}
	*pSize  = ((ULONGLONG)(((ULONGLONG)info.nFileSizeHigh << 32) | info.nFileSizeLow));
	return S_OK;
}
STDMETHODIMP UnknownFileInfo::GetAttributes(DWORD *pAttributes)
{
	if (NULL == pAttributes)
		return E_POINTER;
	if (FALSE == bInfoRead)
	{
		ReadFileAttributes(pszPath, &info);
		bInfoRead = TRUE;
	}

	*pAttributes = info.dwFileAttributes;
	return S_OK;
}

STDMETHODIMP UnknownFileInfo::GetCreationTime(FILETIME *pTime)
{
	if (NULL == pTime)
		return E_POINTER;

	if (FALSE == bInfoRead)
	{
		ReadFileAttributes(pszPath, &info);
		bInfoRead = TRUE;
	}

	*pTime = info.ftCreationTime;
	
	return S_OK;
}

STDMETHODIMP UnknownFileInfo::GetModifiedTime(FILETIME *pTime)
{
	if (NULL == pTime)
		return E_POINTER;

	if (FALSE == bInfoRead)
	{
		ReadFileAttributes(pszPath, &info);
		bInfoRead = TRUE;
	}

	*pTime = info.ftLastWriteTime;
	
	return S_OK;
}


STDMETHODIMP UnknownFileInfo::ResetCache(void)
{
	bInfoRead = FALSE;
	ZeroMemory(&info, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
	return S_OK;
}


STDMETHODIMP UnknownFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL != ppEnumerator)
		*ppEnumerator = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP UnknownFileInfo::CanCopy(void)
{
	return S_OK;
}

STDMETHODIMP UnknownFileInfo::CanPlay(void)
{
	return S_FALSE;
}


STDMETHODIMP UnknownFileInfo::SetExtraInfo(ULONG_PTR data)
{
	extraData = data;
	return S_OK;
}
STDMETHODIMP UnknownFileInfo::GetExtraInfo(ULONG_PTR *pData)
{
	if (NULL == pData)
		return E_POINTER;
	*pData = extraData;
	return S_OK;
}

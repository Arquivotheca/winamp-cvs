#include "./main.h"
#include "./fiFolder.h"
#include "./feFolder.h"
#include "./itemTypeInterface.h"
#include <shlwapi.h>

FolderFileInfo::FolderFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) : 
	UnknownFileInfo(pszFilePath, pAttributeData)
{	

}

FolderFileInfo::~FolderFileInfo()
{	
}

STDMETHODIMP FolderFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new FolderFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP FolderFileInfo::GetExtension(LPCTSTR *pszBuffer)
{
	return E_NOTIMPL;
}

STDMETHODIMP FolderFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeFolder;
	
	return S_OK;
}


STDMETHODIMP FolderFileInfo::GetSize(ULONGLONG *pSize)
{
	return E_NOTIMPL;
}


STDMETHODIMP FolderFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL == ppEnumerator)
		return E_INVALIDARG;
	
	*ppEnumerator = new FolderFileEnumerator(pszPath);
	if (NULL == *ppEnumerator)
		return E_OUTOFMEMORY;

	return S_OK;
}
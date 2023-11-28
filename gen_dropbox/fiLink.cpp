#include "./main.h"
#include "./wasabiApi.h"
#include "./fiLink.h"
#include "./feLink.h"
#include "./itemTypeInterface.h"


LinkFileInfo::LinkFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) : 
	UnknownFileInfo(pszFilePath, pAttributeData)
{	

}

LinkFileInfo::~LinkFileInfo()
{	
	
}

STDMETHODIMP LinkFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new LinkFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP LinkFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeLinkFile;
	
	return S_OK;
}

STDMETHODIMP LinkFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL == ppEnumerator)
		return E_INVALIDARG;
	
	*ppEnumerator = new LinkFileEnumerator(pszPath);
	if (NULL == *ppEnumerator)
		return E_OUTOFMEMORY;

	return S_OK;
}


STDMETHODIMP LinkFileInfo::CanPlay(void)
{
	return S_OK;
}
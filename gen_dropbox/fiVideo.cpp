#include "./main.h"
#include "./fiVideo.h"
#include "./itemTypeInterface.h"

VideoFileInfo::VideoFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) : 
	UnknownFileInfo(pszFilePath, pAttributeData)
{	

}

VideoFileInfo::~VideoFileInfo()
{	

}

STDMETHODIMP VideoFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new VideoFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP VideoFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeVideoFile;
	
	return S_OK;
}

STDMETHODIMP VideoFileInfo::CanPlay(void)
{
	return S_OK;
}
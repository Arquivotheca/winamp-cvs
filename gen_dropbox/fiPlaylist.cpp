#include "./main.h"
#include "./wasabiApi.h"
#include "./fiPlaylist.h"
#include "./fePlaylist.h"
#include "./itemTypeInterface.h"
#include "./playlistMetaReader.h"


PlaylistFileInfo::PlaylistFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) : 
	UnknownFileInfo(pszFilePath, pAttributeData)
{	

}

PlaylistFileInfo::~PlaylistFileInfo()
{	
	
}

STDMETHODIMP PlaylistFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new PlaylistFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP PlaylistFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypePlaylistFile;
	
	return S_OK;
}

STDMETHODIMP PlaylistFileInfo::EnumItems(IFileEnumerator **ppEnumerator)
{
	if (NULL == ppEnumerator)
		return E_INVALIDARG;
	
	*ppEnumerator = new PlaylistFileEnumerator(pszPath);
	if (NULL == *ppEnumerator)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP PlaylistFileInfo::GetReader(METAKEY *metaKey, INT keyCount, INT readMode, IFileMetaReader **ppMetaReader)
{
	if (NULL == ppMetaReader)
		return E_POINTER;

	*ppMetaReader = NULL;
	
	UINT flags = 0;
	for (INT i = 0; i < keyCount; i++)
	{
		switch(metaKey[i])
		{
			case METAKEY_TRACKTITLE:
			case METAKEY_FORMATTEDTITLE:	flags |= PlaylistMetaReader::READ_TITLE; break;
			case METAKEY_TRACKLENGTH:	flags |= PlaylistMetaReader::READ_LENGTH; break;
			case METAKEY_TRACKCOUNT:		flags |= PlaylistMetaReader::READ_COUNT; break;
		}

	}
	
	if (0 == flags)
		return E_NOTIMPL;

	if (METAREADMODE_NORMAL == readMode && !NeedRead(metaKey, keyCount))
		return E_META_NOTHINGTOREAD; 

	*ppMetaReader = new PlaylistMetaReader(this, readMode, flags);
    if (NULL == *ppMetaReader)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP PlaylistFileInfo::CanPlay(void)
{
	return S_OK;
}

STDMETHODIMP PlaylistFileInfo::CanRead(METAKEY metaKey)
{
	switch(metaKey)
	{
		case METAKEY_TRACKTITLE:
		case METAKEY_FORMATTEDTITLE:	
		case METAKEY_TRACKLENGTH:	
		case METAKEY_TRACKCOUNT:		
			return S_OK;
	}
	return E_METAKEY_UNKNOWN;
}
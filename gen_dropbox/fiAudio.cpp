#include "./main.h"
#include "./fiAudio.h"
#include "./itemTypeInterface.h"
#include <shlwapi.h>


#include "./genericMetaReader.h"
#include <strsafe.h>


AudioFileInfo::AudioFileInfo(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData) : 
	UnknownFileInfo(pszFilePath, pAttributeData)
{	
	
}

AudioFileInfo::~AudioFileInfo()
{	

}

STDMETHODIMP AudioFileInfo::CreateInstance(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *attributes, IFileInfo **itemOut)
{
	*itemOut = new AudioFileInfo(filePath, attributes);
	return  (NULL != *itemOut) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP AudioFileInfo::GetType(DWORD *pType)
{
	if (NULL == pType)
		return E_POINTER;
	*pType = IItemType::itemTypeAudioFile;
	
	return S_OK;
}


static METAKEY szAudioKeys[] = 
{
	METAKEY_TRACKARTIST,
	METAKEY_TRACKTITLE,
	METAKEY_TRACKALBUM,
	METAKEY_TRACKGENRE,
	METAKEY_TRACKCOMMENT,
	METAKEY_TRACKPUBLISHER,
	METAKEY_TRACKCOMPOSER,
	METAKEY_TRACKLENGTH,
	METAKEY_TRACKBITRATE,
	METAKEY_TRACKYEAR,
	METAKEY_TRACKNUMBER,
	METAKEY_TRACKCOUNT,
	METAKEY_DISCNUMBER,
	METAKEY_DISCCOUNT,
	METAKEY_ALBUMARTIST,
	METAKEY_FORMATTEDTITLE,
	METAKEY_TRACKBPM,
};

#define MAX_KEYREAD		256

STDMETHODIMP AudioFileInfo::GetReader(METAKEY *metaKey, INT keyCount, INT readMode, IFileMetaReader **ppMetaReader)
{
	if (NULL == ppMetaReader)
		return E_POINTER;

	*ppMetaReader = NULL;


	if (METAREADMODE_NORMAL == readMode && !NeedRead(metaKey, keyCount))
		return E_META_NOTHINGTOREAD; 
	
    
	*ppMetaReader = new GenericMetaReader(this, szAudioKeys, ARRAYSIZE(szAudioKeys), readMode);
	if (NULL == *ppMetaReader)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP AudioFileInfo::CanPlay(void)
{
	return S_OK;
}

STDMETHODIMP AudioFileInfo::CanRead(METAKEY metaKey)
{
	for (INT  i = 0; i < ARRAYSIZE(szAudioKeys); i++)
	{
		if (szAudioKeys[i] == metaKey)
			return S_OK;
	}
	return E_METAKEY_UNKNOWN;
}
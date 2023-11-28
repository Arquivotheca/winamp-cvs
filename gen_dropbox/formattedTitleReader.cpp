#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./formattedTitleReader.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./formatters.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>


static LPWSTR pszAtfTemplateCache = NULL;
static BOOL unloadCallbackRegistered = FALSE;


#define PROCESS_TAG(__knownTag, __testTag, __formatterProc, __fileInfo, __bufferOut, __bufferSize)\
	{if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, (__knownTag), -1, (__testTag), -1)){\
	return  ((LPTSTR)(##__formatterProc(__fileInfo, __bufferOut, __bufferSize)));}}

#define REPLACE_TAG(__tagVariable, __tagReplaceFrom, __tagReplaceTo)\
	{if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, (__tagVariable), -1, (__tagReplaceFrom), -1)){(##__tagVariable) = (__tagReplaceTo);}}

#define PROCESSTAG(__knownTag, __formatterProc)\
		PROCESS_TAG(__knownTag, name, __formatterProc, pInfo, szChunk, ARRAYSIZE(szChunk))


static void CALLBACK UninitializeCache(void)
{
	if (NULL != pszAtfTemplateCache) 
	{
		lfh_free(pszAtfTemplateCache);
		pszAtfTemplateCache = NULL;
	}
}

static void CacheTitleFormatTemplate(void)
{
	TCHAR szBuffer[4096];
	
	if (!unloadCallbackRegistered)
	{
		Plugin_RegisterUnloadCallback(UninitializeCache);
		unloadCallbackRegistered = TRUE;
	}

	IConfiguration *pConfig;
	Plugin_QueryConfiguration(winampSettingsGuid, NULL, &pConfig);
	if (NULL != pConfig)
	{
		pConfig->ReadString(CFG_TITLEFORMAT, szBuffer, ARRAYSIZE(szBuffer));
		pConfig->Release();
	}
	else szBuffer[0] = TEXT('\0');

	if (NULL != pszAtfTemplateCache) 
		lfh_free(pszAtfTemplateCache);

	INT cchLen = lstrlen(szBuffer);
	if (cchLen > 0)
	{
		cchLen++;
		pszAtfTemplateCache = (LPTSTR)lfh_malloc(sizeof(TCHAR)*cchLen);
		CopyMemory(pszAtfTemplateCache, szBuffer, sizeof(TCHAR)*cchLen);
	}
}


static LPCTSTR CopyFileName(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	LPCTSTR pszFileName;
	if (NULL == pFileInfo || FAILED(pFileInfo->GetFileName(&pszFileName)) ||
		FAILED(StringCchCopy(pszBuffer, cchBufferMax, pszFileName)))
	{
		pszBuffer[0] = TEXT('\0');
	}
	return pszBuffer;
}

static void ReplaceFormattingSymbols(LPTSTR pszTitle)
{
	INT len = -1;
	
	for (LPTSTR p = pszTitle; TEXT('\0') != *p; p++)
	{
		switch (*p)
		{
			case TEXT('\r'):
			case TEXT('\n'):
			case TEXT('\t'):
				*p = TEXT(' ');
				break;
		}
	}
}

template<int TITLESIZE, int CHUNKSIZE>
FormattedTitleReader<TITLESIZE, CHUNKSIZE>::FormattedTitleReader(IFileInfo *pFileInfo) : pInfo(pFileInfo)
{
	szTitle[0] = TEXT('\0');
	if (NULL == pInfo) return;

	if (NULL == AGAVE_API_TAGZ && 
		NULL == (AGAVE_API_TAGZ = QueryWasabiInterface(api_tagz, tagzGUID)))
	{
		CopyFileName(pInfo, szTitle, ARRAYSIZE(szTitle));
		return;
	}

	if (NULL == AGAVE_API_METADATA) 
		AGAVE_API_METADATA = QueryWasabiInterface(api_metadata, api_metadataGUID);

	if (NULL == pszAtfTemplateCache)
		CacheTitleFormatTemplate();
	
	AGAVE_API_TAGZ->format(pszAtfTemplateCache, szTitle, ARRAYSIZE(szTitle), this, 0 /*&parameters*/);

	ReplaceFormattingSymbols(szTitle);
}

template<int TITLESIZE, int CHUNKSIZE>
wchar_t *FormattedTitleReader<TITLESIZE, CHUNKSIZE>::GetTag(const wchar_t *name, ifc_tagparams *parameters)
{
	
	REPLACE_TAG(name, L"tracknum", L"track");
	REPLACE_TAG(name, L"trackcount", L"track");
	REPLACE_TAG(name, L"discnum", L"disc");
	REPLACE_TAG(name, L"disccount", L"disc");


	PROCESSTAG(L"filename", Formatter_FileName)
	PROCESSTAG(L"folder", Formatter_FileFolder);
	PROCESSTAG(L"path", Formatter_FilePath);
	PROCESSTAG(L"extension", Formatter_FileExtension);
	PROCESSTAG(L"title", Formatter_TrackTitle);
	PROCESSTAG(L"artist", Formatter_TrackArtist);
	PROCESSTAG(L"albumartist", Formatter_AlbumArtist);
	PROCESSTAG(L"album", Formatter_TrackAlbum);
	PROCESSTAG(L"genre", Formatter_TrackGenre);
	PROCESSTAG(L"year", Formatter_TrackYear);
	PROCESSTAG(L"disc", Formatter_DiscNumber);
	PROCESSTAG(L"publisher", Formatter_TrackPublisher);
	PROCESSTAG(L"comment", Formatter_TrackComment);
	PROCESSTAG(L"track", Formatter_TrackNumber);
	PROCESSTAG(L"composer", Formatter_TrackComposer);
	PROCESSTAG(L"bitrate", Formatter_TrackBitrate);
	PROCESSTAG(L"length", Formatter_TrackLength);
	PROCESSTAG(L"type", Formatter_FileType);				// thats wrong but i have no time to redo it
	PROCESSTAG(L"family", Formatter_ExtensionFamily);
	PROCESSTAG(L"size", Formatter_FileSize);
	PROCESSTAG(L"bpm", Formatter_TrackBpm);
	

	if (NULL != AGAVE_API_METADATA) 
	{
		LPCTSTR pszFile;
		szChunk[0] = TEXT('\0');
		if (SUCCEEDED(pInfo->GetPath(&pszFile)) && 
			AGAVE_API_METADATA->GetExtendedFileInfo(pszFile, name, szChunk, ARRAYSIZE(szChunk)))
		{
			return szChunk;
		}
	}
	return NULL;
}

template<int TITLESIZE, int CHUNKSIZE>
void FormattedTitleReader<TITLESIZE, CHUNKSIZE>::FreeTag(wchar_t *tag)
{
}
template<int TITLESIZE, int CHUNKSIZE>
void FormattedTitleReader<TITLESIZE, CHUNKSIZE>::ResetCachedTitleFormatTemplate(void)
{
	if (NULL != pszAtfTemplateCache)
	{		
		lfh_free(pszAtfTemplateCache);
		pszAtfTemplateCache = NULL;
	}
}



#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS FormattedTitleReader<TITLESIZE, CHUNKSIZE>
template<INT TITLESIZE, INT CHUNKSIZE>
START_DISPATCH;
CB(IFC_TAGPROVIDER_GET_TAG, GetTag);
VCB(IFC_TAGPROVIDER_FREE_TAG, FreeTag);
END_DISPATCH;
#undef CBCLASS




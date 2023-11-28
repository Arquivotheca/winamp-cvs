#include "./main.h"
#include "./documentSaver.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./wasabiApi.h"
#include "../playlist/api_playlists.h"

#include <strsafe.h>


DocumentSaver::DocumentSaver(LPCTSTR pszDocPath, LPCTSTR pszDocName, IFileInfo **ppFileList, size_t fileCount, BOOL bRegister)
	: pszPath(NULL), pszTitle(NULL), ppItems(NULL), count(0), registerPl(bRegister)
{
	size_t cbLen;
	if (NULL != pszDocPath)
	{
		cbLen = (lstrlen(pszDocPath) + 1) * sizeof(TCHAR);
		pszPath = (LPTSTR)lfh_malloc(cbLen);
		if (NULL != pszPath)
			CopyMemory(pszPath, pszDocPath, cbLen);
	}
	if (NULL != pszDocName)
	{
		cbLen = (lstrlen(pszDocName) + 1) * sizeof(TCHAR);
		pszTitle = (LPTSTR)lfh_malloc(cbLen);
		if (NULL != pszTitle)
			CopyMemory(pszTitle, pszDocName, cbLen);
	}
	
	if (NULL != ppFileList && fileCount > 0)
	{
		cbLen = fileCount * sizeof(IFileInfo*);
		ppItems = (IFileInfo**)malloc(cbLen);
		if (NULL != ppItems)
		{
			count = fileCount;
			CopyMemory(ppItems, ppFileList, cbLen);
			for (size_t i =0; i < count; i++)
			{
				if (NULL != ppItems[i]) ppItems[i]->AddRef();
			}
		}
	}
}

DocumentSaver::~DocumentSaver()
{
	if (NULL != pszPath)
		lfh_free(pszPath);
	if (NULL != pszTitle)
		lfh_free(pszTitle);
	if (NULL != ppItems)
	{
		for(size_t i = 0; i < count; i ++)
		{
			if (NULL != ppItems[i]) ppItems[i]->Release();
		}
		free(ppItems);
	}
}

HRESULT DocumentSaver::Save()
{
	if (NULL == WASABI_API_PLAYLISTMNGR)
	{
		WASABI_API_PLAYLISTMNGR = QueryWasabiInterface(api_playlistmanager, api_playlistmanagerGUID);
		if (NULL == WASABI_API_PLAYLISTMNGR)
			return E_UNEXPECTED;
	}
	
	INT result = WASABI_API_PLAYLISTMNGR->Save(pszPath, this);
	
	if (PLAYLISTMANAGER_SUCCESS == result)
	{
		api_playlists *playlistsApi;
		playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
		if (NULL != playlistsApi)
		{
			BOOL bFound = FALSE;
			playlistsApi->Lock();
			size_t count = playlistsApi->GetCount();
			while (count--)
			{
				LPCWSTR fileName = playlistsApi->GetFilename(count);
				if (NULL != fileName || L'\0' != fileName)
				{
					if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, fileName, -1, pszPath, -1))
					{	
						playlistsApi->RenamePlaylist(count, pszTitle);
						if (NULL != WASABI_API_SYSCB)
						{
							WASABI_API_SYSCB->syscb_issueCallback(api_playlists::SYSCALLBACK, 
								api_playlists::PLAYLIST_SAVED, count, GetPluginUID());
						}
						bFound = TRUE;
						break;
					}
				}
			}
            playlistsApi->Unlock();
			
			if (!bFound && registerPl)
				playlistsApi->AddPlaylist(pszPath, pszTitle, INVALID_GUID);
			
			ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);
		}
		
	}
	
	
	return (PLAYLISTMANAGER_SUCCESS == result) ? S_OK : E_FAIL;
}

size_t DocumentSaver::GetNumItems()
{
	return count;
}

size_t DocumentSaver::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	LPCTSTR itemPath;
	if (item >= count || FAILED(ppItems[item]->GetPath(&itemPath)) ||
		NULL == itemPath || TEXT('\0') == itemPath)
	{
		if (NULL != filename)
			*filename = L'\0';
		return 0;
	}
		
	if (NULL == filename)
		return lstrlen(itemPath);

	HRESULT hr = StringCchCopyEx(filename, filenameCch, itemPath, NULL, NULL, STRSAFE_IGNORE_NULLS);
	if (FAILED(hr))
		*filename = L'\0';

	return SUCCEEDED(hr);
}

size_t DocumentSaver::GetItemTitle(size_t item, wchar_t *title, size_t titleCch)
{
	IFileMeta *pMeta;
	if (item >= count || 
		FAILED(ppItems[item]->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{
		if (NULL != title)
			*title = L'\0';
		return 0;
	}
	HRESULT hr = S_OK;
	METAVALUE val;
	val.type = METATYPE_WSTR;

	if (NULL == title)
	{
		TCHAR szTemp[1024];
		hr = pMeta->QueryValueHere(METAKEY_FORMATTEDTITLE, &val, szTemp, sizeof(szTemp));
		pMeta->Release();
		if (SUCCEEDED(hr))
			return lstrlen(szTemp);
		return 0;
	}
	
	hr = pMeta->QueryValueHere(METAKEY_FORMATTEDTITLE, &val, title, sizeof(WCHAR) * titleCch);
	pMeta->Release();

	if (FAILED(hr))
		*title = L'\0';
	
	return SUCCEEDED(hr);
}

int DocumentSaver::GetItemLengthMs(size_t item)
{
	IFileMeta *pMeta;
	if (item >= count || 
		FAILED(ppItems[item]->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	return -1;
	
	HRESULT hr = S_OK;
	METAVALUE val;
	val.type = METATYPE_INT32;
	INT lenMs;
	hr = pMeta->QueryValue(METAKEY_TRACKLENGTH, &val);
	pMeta->Release();

	lenMs = val.iVal;
	if (lenMs > 0) 
		lenMs *= 1000;
	ReleaseMetaValue(&val);

	return SUCCEEDED(hr) ? lenMs : -1;
}

size_t DocumentSaver::GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	return 0;
}

#define CBCLASS DocumentSaver
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMTITLE, GetItemTitle)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMs)
CB(IFC_PLAYLIST_GETITEMEXTENDEDINFO, GetItemExtendedInfo)
END_DISPATCH;
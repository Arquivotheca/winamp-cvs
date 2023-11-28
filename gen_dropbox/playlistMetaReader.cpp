#include "./main.h"
#include "./playlistMetaReader.h"
#include "./wasabiAPi.h"
#include "./fileInfoInterface.h"
#include "../playlist/api_playlists.h"
#include "../nu/trace.h"
#include <shlwapi.h>
#include <strsafe.h>

static METAKEY szSupportedKeys[] = {METAKEY_TRACKTITLE, METAKEY_TRACKLENGTH, METAKEY_TRACKCOUNT, METAKEY_FORMATTEDTITLE };
PlaylistMetaReader::PlaylistMetaReader(GenericItemMeta *pMetaObject, INT readMode, UINT readFlags) 
	: ref(1), pObject(pMetaObject), mode(readMode), pszTitle(NULL), counter(0), length(0), flags(readFlags)
{
	if (NULL != pObject)
	{		
		METAKEY key[4];
		INT count = 0;
		if (READ_TITLE & flags)
		{
			key[count++] = METAKEY_TRACKTITLE;
			key[count++] = METAKEY_FORMATTEDTITLE;
		}
		if (READ_LENGTH & flags) key[count++] = METAKEY_TRACKLENGTH;
		if (READ_COUNT & flags) key[count++] = METAKEY_TRACKCOUNT;
	
		if (count > 0)
			pObject->SetState(key, count, METARECORD_READING, TRUE, TRUE);

		pObject->AddRef();

	}
}

PlaylistMetaReader::~PlaylistMetaReader()
{
	if (NULL != pObject)
	{
		METAKEY key[4];
		INT count = 0;
		if (READ_TITLE & flags)
		{
			key[count++] = METAKEY_TRACKTITLE;
			key[count++] = METAKEY_FORMATTEDTITLE;
		}
		if (READ_LENGTH & flags) key[count++] = METAKEY_TRACKLENGTH;
		if (READ_COUNT & flags) key[count++] = METAKEY_TRACKCOUNT;
	
		if (count > 0)
			pObject->SetState(szSupportedKeys, ARRAYSIZE(szSupportedKeys), METARECORD_READING, FALSE, FALSE);
		pObject->Release();
	}
	SetTitle(NULL);
}

STDMETHODIMP_(ULONG) PlaylistMetaReader::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) PlaylistMetaReader::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP PlaylistMetaReader::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileMetaReader))
	{
		*ppvObject = (IFileMetaReader*)this;
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}
	return pObject->QueryInterface(riid, ppvObject);
}

BOOL PlaylistMetaReader::CanRead(METAKEY metaKey)
{
	for(int i = 0; i < ARRAYSIZE(szSupportedKeys); i++)
	{
		if (szSupportedKeys[i] == metaKey)
			return TRUE;
	}
	return FALSE; 
}

void PlaylistMetaReader::SetTitle(LPCTSTR pszName)
{
	if (NULL != pszTitle)
	{
		lfh_free(pszTitle);
		pszTitle = NULL;
	}
	if (NULL != pszName)
	{
		INT cbLen = (lstrlenW(pszName) + 1) * sizeof(TCHAR);
        pszTitle = (LPTSTR)lfh_malloc(cbLen);
		if (NULL != pszTitle)
			CopyMemory(pszTitle, pszName, cbLen);
	}

}

BOOL PlaylistMetaReader::TitleFromPlaylists(LPCTSTR pszPlaylist)
{
	api_playlists *playlistsApi;
	playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
	if (NULL == playlistsApi)
		return FALSE;

	BOOL bFound = FALSE;

	playlistsApi->Lock();
	size_t count = playlistsApi->GetCount();
	while (count--)
	{
		LPCWSTR fileName = playlistsApi->GetFilename(count);
		if (NULL != fileName || L'\0' != fileName)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, fileName, -1, pszPlaylist, -1))
			{
				SetTitle(playlistsApi->GetName(count));
				bFound = TRUE;
				break;
			}
		}
	}
	playlistsApi->Unlock();
	ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);
	return bFound;
}

BOOL PlaylistMetaReader::TitleFromFileName(LPCTSTR pszPlaylist)
{
	if (NULL == pszPlaylist || TEXT('\0') == pszPlaylist)
		return FALSE;
	SetTitle(pszPlaylist);
	PathRemoveExtension(pszTitle);
	PathStripPath(pszTitle);
	return TRUE;
}

STDMETHODIMP PlaylistMetaReader::Read(void)
{
	if (NULL == pObject) 
		return E_POINTER;

	if (NULL == WASABI_API_PLAYLISTMNGR)
	{
		WASABI_API_PLAYLISTMNGR = QueryWasabiInterface(api_playlistmanager, api_playlistmanagerGUID);
		if (NULL == WASABI_API_PLAYLISTMNGR)
		return E_FAIL;
	}

	IFileInfo *pfi;
	if (FAILED(pObject->QueryInterface(IID_IFileInfo, (void**)&pfi)))
		return E_FAIL;

	LPCTSTR pszPath;
	if (FAILED(pfi->GetPath(&pszPath)))
		return E_FAIL;

	SetTitle(NULL);
	counter = 0;
	length = 0;

	WASABI_API_PLAYLISTMNGR->Load(pszPath, this);
		
	if (NULL == pszTitle || TEXT('\0') == pszTitle)
		TitleFromPlaylists(pszPath);
	if (NULL == pszTitle || TEXT('\0') == pszTitle)
		TitleFromFileName(pszPath);

	pfi->Release();

	FILEMETARECORD records[ARRAYSIZE(szSupportedKeys)];
	INT rCount = 0;
	for(int i = 0; i < ARRAYSIZE(records); i++)
	{
		switch(szSupportedKeys[i])
		{
			case METAKEY_FORMATTEDTITLE:
			case METAKEY_TRACKTITLE:
				if ((READ_TITLE & flags) && NULL != pszTitle)
				{
					records[rCount].key = szSupportedKeys[i];
					MetaValueWStrW(&records[rCount].value, pszTitle);
					rCount++;
				}
				break;
			case METAKEY_TRACKLENGTH:
				if (READ_LENGTH & flags)
				{
					records[rCount].key = szSupportedKeys[i];
					MetaValueInt32(&records[rCount].value, length);
					rCount++;
				}
				break;
			case METAKEY_TRACKCOUNT:
				if (READ_COUNT & flags)
				{
					records[rCount].key = szSupportedKeys[i];
					MetaValueInt32(&records[rCount].value, (INT)counter);
					rCount++;
				}
				break;
		}
	}

	HRESULT hr = S_OK;
	if (rCount > 0)
	{
		INT addFlags = GenericItemMeta::ADDMETAFLAGS::ADDMETAFLAG_MARKREAD;
		HRESULT hr = pObject->AddMetaRecords(records, rCount, mode, addFlags);
		if (FAILED(hr))
		{	
			for(int i = 0; i < rCount; i++)
				ReleaseMetaValue(&records[i].value);	
		}
	}
	return hr;
}

STDMETHODIMP PlaylistMetaReader::SetCookie(DWORD cookie)
{
	this->cookie = cookie;
	return S_OK;
}
STDMETHODIMP PlaylistMetaReader::GetCookie(DWORD *pCookie)
{
	if (NULL == pCookie)
		return E_POINTER;
	*pCookie = cookie;
	return S_OK;
}

int PlaylistMetaReader::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	counter++;
	if (lengthInMS>0)
		length += lengthInMS;
	return (0 == ((READ_LENGTH | READ_COUNT) & flags));
}


int PlaylistMetaReader::OnPlaylistInfo(const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info)
{	
	if (0 != (READ_TITLE & flags))
	{
		if (NULL != playlistName && L'\0' != playlistName)
			SetTitle(playlistName);
	}

	return (0 == ((READ_LENGTH | READ_COUNT) & flags));
}

const wchar_t *PlaylistMetaReader::GetBasePath()
{
	return NULL;
}

#define CBCLASS PlaylistMetaReader
START_DISPATCH;
CB(IFC_PLAYLISTLOADERCALLBACK_ONFILE_RET, OnFile)
CB(IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO_RET, OnPlaylistInfo)
CB(IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, GetBasePath)
END_DISPATCH;
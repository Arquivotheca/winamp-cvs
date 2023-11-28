#include "./main.h"
#include "./plugin.h"
#include "./fePlaylist.h"
#include "./wasabiApi.h"
#include "./fileMetaInterface.h"

#include <shlwapi.h>
#include <strsafe.h>

PlaylistFileEnumerator::PlaylistFileEnumerator(LPCTSTR pszPlaylist) 
	: ref(1), bLoaded(FALSE), cursor(NULL), pszPath(NULL), pszFile(NULL)
{
	if (NULL != pszPlaylist)
	{
		pszPath = lfh_strdup(pszPlaylist);
		if (NULL != pszPath)
		{			
			LPTSTR pFileSpec = PathFindFileName(pszPath);
			pszFile = lfh_strdup(pFileSpec);
			PathRemoveFileSpec(pszPath);
		}
	}
}

void PlaylistFileEnumerator::ReleaseList()
{
	size_t index = list.size();
	while (index--)
	{
		if (NULL != list[index].pszPath)
			lfh_free(list[index].pszPath);
		if (NULL != list[index].pMetaRec)
		{
			for (int i = 0; i < list[index].metaCount; i++)
				ReleaseMetaValue(&list[index].pMetaRec[i].value);
			free(list[index].pMetaRec);
			list[index].pMetaRec = NULL;
			list[index].metaCount = 0;
		}
	}
	list.clear();
}

PlaylistFileEnumerator::~PlaylistFileEnumerator()
{
	ReleaseList();

	if (NULL != pszPath)
		lfh_free(pszPath);
	if (NULL != pszFile)
		lfh_free(pszFile);
}


STDMETHODIMP_(ULONG) PlaylistFileEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) PlaylistFileEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP PlaylistFileEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileEnumerator))
		*ppvObject = (IFileEnumerator*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP PlaylistFileEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *puFetched)
{
	HRESULT hr;
	ULONG cReturn = 0L;
	TCHAR szBuffer[MAX_PATH];

	if(puFetched == NULL && celt != 1)
		return S_FALSE;

	if(puFetched != NULL)
		*puFetched = 0;

	hr = S_OK;
	if (FALSE == bLoaded)
	{
		if (NULL == WASABI_API_PLAYLISTMNGR)
		{
			WASABI_API_PLAYLISTMNGR = QueryWasabiInterface(api_playlistmanager, api_playlistmanagerGUID);
			if (NULL == WASABI_API_PLAYLISTMNGR)
				hr = E_UNEXPECTED;
		}

		if (NULL == pszPath || NULL == pszFile)
			hr = E_INVALIDARG;

		if (SUCCEEDED(hr))
		{
			if (PathCombine(szBuffer, pszPath, pszFile))
			{
				WASABI_API_PLAYLISTMNGR->Load(szBuffer, this);
				cursor = list.begin();
			}
			else
				cursor = NULL;

			if (NULL == cursor)
				hr = E_UNEXPECTED;
			else 
				bLoaded = TRUE;
		}
	}

	if (FAILED(hr))
	{
		return hr;
	}

	if(cursor >= list.end())
		return S_FALSE;
	
	IFileMeta *pMeta;

	while(cursor < list.end() && celt > 0)
	{
		hr = PLUGIN_REGTYPES->CreateItem(cursor->pszPath, NULL, &pfiBuffer[cReturn]);
        if (S_OK != hr)
		{
			pfiBuffer[cReturn] = NULL;
			hr = E_FILEENUM_CREATEINFO_FAILED;
			break;
		}
		
		if (NULL != cursor->pMetaRec &&
			SUCCEEDED(pfiBuffer[cReturn]->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
		{
			if (FAILED(pMeta->SetRecords(cursor->pMetaRec, cursor->metaCount, METAREADMODE_NORMAL, FALSE, FALSE)))
			{
				for (int i = 0; i < cursor->metaCount; i++)
					ReleaseMetaValue(&cursor->pMetaRec[i].value);
			}
			free(cursor->pMetaRec);
			cursor->pMetaRec = NULL;
			cursor->metaCount = 0;
			pMeta->Release();

		}
		cursor++;
		cReturn++;
		celt--;
	}

	if(NULL != puFetched)
		*puFetched = (cReturn - celt);

	if (S_OK != hr)
		return hr;

	return (cReturn > 0) ? S_OK : S_FALSE;
}

STDMETHODIMP PlaylistFileEnumerator::Skip(ULONG celt)
{	
	if (NULL == cursor)
		return E_POINTER;

	if((cursor + celt) >= list.end())
		return S_FALSE;
	cursor += celt;
	return S_OK;
}

STDMETHODIMP PlaylistFileEnumerator::Reset(void)
{
	ReleaseList();
	
	cursor = NULL;
	bLoaded = FALSE;
	return S_OK;
}

void PlaylistFileEnumerator::OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
{
	LPTSTR name = NULL;

	if (NULL != filename)
	{
		INT cchLen = lstrlen(filename);
		if (0 != cchLen)
		{
			cchLen++;
			name = (LPTSTR)lfh_malloc(sizeof(WCHAR) * cchLen);
			if (NULL != name)
				CopyMemory(name, filename, sizeof(WCHAR)*cchLen);
		}
	}
	if (NULL != name)
	{
		size_t capacity = list.capacity();
		if (capacity == list.size())
			list.reserve(capacity*2);
		
		PLITEM item;
		item.pszPath = name;
		item.metaCount = 0;
		item.pMetaRec = NULL;

		if (NULL != title) item.metaCount += 2;
		if (-1 != lengthInMS) item.metaCount++;
		
		if (item.metaCount > 0)
		{
			item.pMetaRec = (FILEMETARECORD*)malloc(sizeof(FILEMETARECORD) * item.metaCount);
			if (NULL != item.pMetaRec)
			{
				INT index = 0;
				if (NULL != title)
				{
					item.pMetaRec[index].key = METAKEY_TRACKTITLE;
					MetaValueWStrW(&item.pMetaRec[index].value, title);
					index++;
					item.pMetaRec[index].key = METAKEY_FORMATTEDTITLE;
					MetaValueWStrW(&item.pMetaRec[index].value, title);
					index++;
				}
				if (-1 != lengthInMS && index < item.metaCount)
				{
					item.pMetaRec[index].key = METAKEY_TRACKLENGTH;
					MetaValueInt32(&item.pMetaRec[index].value, lengthInMS/1000);
					index++;
				}
			}
		}
		else
			item.pMetaRec = NULL;
		
		list.push_back(item);
	}
}

void PlaylistFileEnumerator::OnPlaylistInfo(const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info)
{
	list.reserve(numEntries);
}

const wchar_t *PlaylistFileEnumerator::GetBasePath()
{
	return pszPath;
}

#define CBCLASS PlaylistFileEnumerator
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
VCB(IFC_PLAYLISTLOADERCALLBACK_ONPLAYLISTINFO, OnPlaylistInfo)
CB(IFC_PLAYLISTLOADERCALLBACK_GETBASEPATH, GetBasePath)
END_DISPATCH;
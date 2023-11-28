#include "./main.h"
#include "./fileInfoInterface.h"
#include "./itemTypeInterface.h"
#include "./extensionFilterList.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./supportedExtensions.h"
#include "../playlist/api_playlisthandler.h"
#include <api/service/waservicefactorybase.h>
#include <api/service/services.h>

#include <shlwapi.h>
#include <strsafe.h>

HRESULT AddFilterListEntry(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pName, LPCTSTR pFilter, LPTSTR *ppBufferOut, size_t *pRemaining, BOOL bShowFilter)
{
	HRESULT hr;
	
	LPTSTR pCursor = pszBuffer;

	if (NULL != ppBufferOut)
		*ppBufferOut = pszBuffer;

	if (NULL != pRemaining)
		*pRemaining = cchBufferMax;

	if (NULL == pszBuffer || NULL == pName || NULL == pFilter)
		return E_INVALIDARG;

	pszBuffer[0] = TEXT('\0');

	hr = StringCchCopyEx(pCursor, cchBufferMax, pName, &pCursor, &cchBufferMax, 
			STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
	if (bShowFilter && SUCCEEDED(hr))
	{		
		LPTSTR p = pCursor;
		hr = StringCchPrintfEx(pCursor, cchBufferMax, &pCursor, &cchBufferMax, 
				STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE, TEXT(" (%s)"), pFilter);
		if (SUCCEEDED(hr) && p != pCursor)
			CharLowerBuff(p, (INT)(INT_PTR)(pCursor - p)); 
			
	}
	if (SUCCEEDED(hr))
	{
		pCursor++;
		cchBufferMax--;
		hr = StringCchCopyEx(pCursor, cchBufferMax, pFilter, &pCursor, &cchBufferMax, 
			STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
	}

	if (cchBufferMax < 1)
		hr = STRSAFE_E_INSUFFICIENT_BUFFER;
	
	pCursor++;
	cchBufferMax--;

	if (SUCCEEDED(hr))
	{
		pCursor[0] = TEXT('\0');
		if (NULL != ppBufferOut)
			*ppBufferOut = pCursor;
		if (NULL != pRemaining)
			*pRemaining = cchBufferMax;
	}
	else 
	{
		pszBuffer[0] = TEXT('\0');
		pszBuffer[1] = TEXT('\0');
	}

	return hr;
}

static HRESULT Playlist_GetHandlerExtensionList(api_playlisthandler *pPlh, LPTSTR pszBuffer, size_t cchBufferMax, LPTSTR *ppBufferOut, size_t *pRemaining)
{
	if (NULL != ppBufferOut)
		*ppBufferOut = pszBuffer;

	if (NULL != pRemaining)
		*pRemaining = cchBufferMax;

	if (NULL == pszBuffer)
		return E_POINTER;
	
	pszBuffer[0] = TEXT('\0');

	if (NULL == pPlh)
		return E_INVALIDARG;

	LPCTSTR pExtension;
	LPTSTR pCursor = pszBuffer;
	BOOL first = TRUE;
	HRESULT hr = S_OK;
	for (int k = 0; NULL != (pExtension = pPlh->EnumerateExtensions(k)) && SUCCEEDED(hr); k++)
	{
		if (NULL != pExtension && TEXT('\0') != *pExtension)
		{			
			hr = StringCchPrintfEx(pCursor, cchBufferMax, &pCursor, &cchBufferMax, STRSAFE_NULL_ON_FAILURE, 
								((first) ? TEXT("*.%s") : TEXT("; *.%s")), pExtension);
			first = FALSE;
		}
	}

	if (SUCCEEDED(hr))
	{		
		if (NULL != ppBufferOut)
			*ppBufferOut = pCursor;
		if (NULL != pRemaining)
			*pRemaining = cchBufferMax;
	}
	else 
		pszBuffer[0] = TEXT('\0');
	return hr;
}

static LPCTSTR szKnownWriters[] = { TEXT("m3u"), TEXT("m3u8"), TEXT("pls"), TEXT("b4s") };

BOOL CanSavePlaylistByExtension(LPCTSTR pszExtension)
{
	if (NULL == pszExtension || TEXT('\0') == *pszExtension)
		return FALSE;

	for (int i = 0; i < ARRAYSIZE(szKnownWriters); i++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, 
				NORM_IGNORECASE, pszExtension, -1, szKnownWriters[i], -1))
		{
			return TRUE;
		}
	}
	return FALSE;
}

HRESULT GetPlaylistSaveFilters(LPTSTR pszFilter, size_t cchFilterMax, DWORD *pIndex)
{
	if (NULL != pIndex)
		 *pIndex = 0;

	if (NULL == pszFilter)
		return E_INVALIDARG;
		
	if(cchFilterMax < 2)
		return E_OUTOFMEMORY;
	
	pszFilter[0] = TEXT('\0');
	pszFilter[1] = TEXT('\0');

	HRESULT hr = S_OK;
	
	HANDLE hse = GetDefaultSupportedExtensionsHandle();
	LPCTSTR pFamily;
	DWORD count = 0;
	TCHAR szBuffer[128];

	if (NULL == hse)
		return E_UNEXPECTED;

	for (int i = 0; i < ARRAYSIZE(szKnownWriters) && SUCCEEDED(hr); i++)
	{
		IItemType *type = GetTypeByExtension(hse, szKnownWriters[i], &pFamily);
		if (NULL != type && IItemType::itemTypePlaylistFile == type->GetId())
		{
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("*.%s"), szKnownWriters[i]);
			hr = AddFilterListEntry(pszFilter, cchFilterMax, pFamily, szBuffer, &pszFilter, &cchFilterMax, TRUE); 
			if(SUCCEEDED(hr))
			{
				if (NULL != pIndex &&
					CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, TEXT("m3u8"), -1, szKnownWriters[i], -1))
					*pIndex = (count + 1);
				
				count++;
			}
		}
	}
	return hr;
}

HRESULT GetPlaylistOpenFilters(LPTSTR pszFilter, size_t cchFilterMax, DWORD *pIndex)
{
	if (NULL != pIndex)
		 *pIndex = 0;

	if (NULL == pszFilter)
		return E_INVALIDARG;
		
	if(cchFilterMax < 2)
		return E_OUTOFMEMORY;
	
	
	pszFilter[0] = TEXT('\0');
	pszFilter[1] = TEXT('\0');

	if (NULL == WASABI_API_PLAYLISTMNGR)
	{
		WASABI_API_PLAYLISTMNGR = QueryWasabiInterface(api_playlistmanager, api_playlistmanagerGUID);
		if (NULL == WASABI_API_PLAYLISTMNGR)
			return E_UNEXPECTED;
	}
	
    waServiceFactory *sf = 0;
	api_playlisthandler *handler;
	TCHAR szBuffer[1024], szLang[128];
	HRESULT hr = S_OK;
	DWORD count = 0;

	WASABI_API_LNGSTRINGW_BUF(IDS_FILEFILTER_ALLFILES, szLang, ARRAYSIZE(szLang));
	hr = AddFilterListEntry(pszFilter, cchFilterMax, szLang, TEXT("*.*"), &pszFilter, &cchFilterMax, TRUE); 
	if (FAILED(hr)) return hr;
	else count++;

	size_t remaining = ARRAYSIZE(szBuffer);
	LPTSTR cursor = szBuffer;
	BOOL first = TRUE;
	for(int n = 0; NULL != (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n)) && SUCCEEDED(hr); n++)
	{			
		if (NULL != (handler = static_cast<api_playlisthandler *>(sf->getInterface())))
		{
			if (!first)
				hr = StringCchCopyEx(cursor, remaining, TEXT("; "), &cursor, &remaining, 
						STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
			else 
				first  = FALSE;

			if (SUCCEEDED(hr))
				hr = Playlist_GetHandlerExtensionList(handler, cursor, remaining, &cursor, &remaining);
			sf->releaseInterface(handler);
			
		}
	}

	if (SUCCEEDED(hr))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_FILEFILTER_ALLPLAYLIST, szLang, ARRAYSIZE(szLang));
		hr = AddFilterListEntry(pszFilter, cchFilterMax, szLang, szBuffer, &pszFilter, &cchFilterMax, TRUE); 
		if (NULL != pIndex && SUCCEEDED(hr))
			*pIndex = (count + 1);
		count++;

	}
	if (FAILED(hr)) return hr;


	for(int n = 0; NULL != (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n)) && SUCCEEDED(hr); n++)
	{			
		if (NULL != (handler = static_cast<api_playlisthandler *>(sf->getInterface())))
		{
			LPCTSTR name = handler->GetName();
			if (NULL == name || TEXT('\0') == *name)
			{
				WASABI_API_LNGSTRINGW_BUF(IDS_PLAYLIST, szLang, ARRAYSIZE(szLang));
				name = szLang;
			}

			hr = Playlist_GetHandlerExtensionList(handler, szBuffer, ARRAYSIZE(szBuffer), NULL, NULL);
			sf->releaseInterface(handler);
			if (SUCCEEDED(hr))
				hr = AddFilterListEntry(pszFilter, cchFilterMax, name, szBuffer, &pszFilter, &cchFilterMax, TRUE); 
			if (SUCCEEDED(hr))
				count++;
		}
	}

	return hr;
}
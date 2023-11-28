#ifndef NULLSOFT_DROPBOX_PLUGIN_EXTENSIONFILTERLIST_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_EXTENSIONFILTERLIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


HRESULT GetPlaylistOpenFilters(LPTSTR pszFilter, size_t cchFilterMax, DWORD *pIndex);
HRESULT GetPlaylistSaveFilters(LPTSTR pszFilter, size_t cchFilterMax, DWORD *pIndex);
BOOL CanSavePlaylistByExtension(LPCTSTR pszExtension);

HRESULT AddFilterListEntry(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pName, LPCTSTR pFilter, LPTSTR *ppBufferOut, size_t *pRemaining, BOOL bShowFilter);


#endif // NULLSOFT_DROPBOX_PLUGIN_EXTENSIONFILTERLIST_HEADER



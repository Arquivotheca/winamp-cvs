#ifndef NULLOSFT_DROPBOX_PLUGIN_EXPORE_FOLDERHEADER
#define NULLOSFT_DROPBOX_PLUGIN_EXPORE_FOLDERHEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

HRESULT ExploreFile(HWND hwnd, LPCTSTR pszFilePath, BOOL bAsync);

#endif // NULLOSFT_DROPBOX_PLUGIN_EXPORE_FOLDERHEADER
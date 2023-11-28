#ifndef NULLSOFT_WEBDEV_PLUGIN_EXPORE_FOLDER_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_EXPORE_FOLDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

HRESULT ExploreFile(HWND hwnd, LPCTSTR pszFilePath, BOOL bAsync);

#endif // NULLSOFT_WEBDEV_PLUGIN_EXPORE_FOLDER_HEADER
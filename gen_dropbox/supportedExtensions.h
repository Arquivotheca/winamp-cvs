#ifndef NULLSOFT_WINAMP_SUPPORTEDEXTENSIONS_HEADER
#define NULLSOFT_WINAMP_SUPPORTEDEXTENSIONS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IItemType;

HANDLE InitializeSupportedExtensions();
void ReleaseSupportedExtensions(HANDLE hData);
HANDLE GetDefaultSupportedExtensionsHandle(void);
IItemType *GetTypeByExtension(HANDLE hSupportedExt, LPCTSTR pszExtension, LPCTSTR *ppszFamily);


#endif // NULLSOFT_WINAMP_SUPPORTEDEXTENSIONS_HEADER

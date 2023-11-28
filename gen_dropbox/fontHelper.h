#ifndef NULLOSFT_WINAMP_DROPBOX_PLUGIN_FONTHELPER_HEADER
#define NULLOSFT_WINAMP_DROPBOX_PLUGIN_FONTHELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL FontHelper_PickFontName(HDC hdc, LOGFONT *pLogFont, LPCTSTR *ppszFamilyNames, INT cchFamilyNames); // hdc can be NULL
INT FontHelper_GetSysFontHeight();

#endif // NULLOSFT_WINAMP_DROPBOX_PLUGIN_FONTHELPER_HEADER
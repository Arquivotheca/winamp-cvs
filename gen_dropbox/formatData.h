#ifndef NULLSOFT_DROPBOX_PLUGIN_FORMATDATA_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FORMATDATA_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct __ACTIVECOLUMN ACTIVECOLUMN;

LPCTSTR FormatFileSize(ULONGLONG *pFileSize, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatFileTime(FILETIME *pFileTime, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatFileTimeLocalize(FILETIME *pFileTime, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatFileAttributes(DWORD fileAttributes, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatFileAttributes(DWORD fileAttributes, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatItemType(UINT typeId, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatLength(INT trackLength, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatBitrate(INT trackBitrate, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatPositiveInt(INT integerValue, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatIntSlashInt(DWORD packedValue, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatExtensionFamily(LPCTSTR fileExtension, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatRect(const RECT *pRect, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatPoint(POINT pt, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR FormatSize(SIZE sz, LPTSTR pszBuffer, INT cchBufferMax);
LPTSTR FormatActiveColumns(LPTSTR pszBufferOut, size_t cchBufferMax, ACTIVECOLUMN *pColumns, INT columnsCount);
LPTSTR FormatLangResource(INT_PTR resourceId, LPTSTR pszBuffer, INT cchBufferMax);
LPTSTR FormatShortcut(BYTE fVirt, WORD key, LPTSTR pszBuffer, INT cchBufferMax);
LPTSTR FormatColumnName(INT columnId, LPTSTR pszBuffer, INT cchBufferMax);

// parsers
BOOL ParseRect(LPCTSTR pszStringIn, RECT *pRectOut, LPCTSTR *ppszCursor);
BOOL ParsePoint(LPCTSTR pszStringIn, POINT *pPointOut, LPCTSTR *ppszCursor);
INT ParseActiveColumns(ACTIVECOLUMN *pColumns, INT nMax, LPCTSTR pszString);
BOOL ParseLangResource(LPCTSTR pszInput, INT_PTR *resourceId);
BOOL ParseShortcut(LPCTSTR pszInput, ACCEL *accelOut);
INT ParseColumnName(LPCTSTR pszColumnName);

#define KWPARSER_ABORT		((UINT)0x00000000)
#define KWPARSER_CONTINUE	((UINT)0x00000001)
#define KWPARSER_FOUND		((UINT)0x80000000)

typedef UINT (CALLBACK *KWPARSERPROC)(LPCTSTR /*pszKeyword*/, INT /*cchKeyword*/, LPVOID /*user*/);
INT ParseKeywords(LPCTSTR input, INT cchInput, LPCTSTR separators, BOOL eatSpace, KWPARSERPROC callback, void *user);

#endif //NULLSOFT_DROPBOX_PLUGIN_FORMATDATA_HEADER
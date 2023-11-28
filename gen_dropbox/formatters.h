#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FORMATTERS_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FORMATTERS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IFileInfo;

LPCTSTR __cdecl Formatter_FilePath(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileName(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileExtension(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileFolder(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileType(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileAttributes(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileSize(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileCreationTime(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FileModifiedTime(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackTitle(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackAlbum(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackArtist(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackGenre(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackPublisher(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackComposer(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_AlbumArtist(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackLength(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackBitrate(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackYear(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackNumber(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_DiscNumber(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_FormattedName(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackComment(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_ExtensionFamily(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);
LPCTSTR __cdecl Formatter_TrackBpm(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax);


#endif // NULLSOFT_DROPBOX_PLUGIN_FILEINFO_FORMATTERS_HEADER
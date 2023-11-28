#include "./main.h"
#include "./formatters.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./formatData.h"
#include <shlwapi.h>
#include <strsafe.h>

#define COPY_STRING(__pszTextToCopy, __pszBuffer, __cchBufferMax)\
	(StringCchCopyEx((__pszBuffer), (__cchBufferMax), (__pszTextToCopy), NULL, NULL,\
				STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE))
		
#define FORMATTER_FORMATVALUE(__pFileInfo, __stdmethodCaller, __valueType, __formatRoutine, __pszBuffer, __cchBufferMax)\
	{__valueType __val;\
	(__pFileInfo)->AddRef();\
	if (S_OK == (__pFileInfo)->##__stdmethodCaller(&__val)) {##__formatRoutine(__val, (__pszBuffer), (__cchBufferMax));}\
	else *(__pszBuffer) = TEXT('\0');\
	(__pFileInfo)->Release();}

#define FORMATTER_FORMATVALUE_POINTER(__pFileInfo, __stdmethodCaller, __valueType, __formatRoutine, __pszBuffer, __cchBufferMax)\
	{ __valueType __val;\
	(__pFileInfo)->AddRef();\
	if (S_OK == (__pFileInfo)->##__stdmethodCaller(&__val))	{##__formatRoutine(&__val, (__pszBuffer), (__cchBufferMax));}\
	else *(__pszBuffer) = TEXT('\0');\
	(__pFileInfo)->Release();}

#define FORMATTER_METASTRING(__pFileInfo, __metaKey, __pszBuffer, __cchBufferMax)\
	{IFileMeta *_meta;\
	if (SUCCEEDED((__pFileInfo)->QueryInterface(IID_IFileMeta, (void**)&_meta))){\
		METAVALUE metaVal; metaVal.type = METATYPE_WSTR;\
		HRESULT hr = _meta->QueryValueHere((METAKEY_##__metaKey##), &metaVal, (__pszBuffer), ((__cchBufferMax)*sizeof(TCHAR)));\
		if (FAILED(hr)) *(__pszBuffer) = TEXT('\0');\
		_meta->Release();}\
	else *(__pszBuffer) = TEXT('\0');}

#define FORMATTER_FORMATMETA(__pFileInfo, __metaKey, __valueType, __formatRoutine, __pszBuffer, __cchBufferMax)\
	{IFileMeta *_meta;\
	if (SUCCEEDED((__pFileInfo)->QueryInterface(IID_IFileMeta, (void**)&_meta)))	{\
		METAVALUE metaVal; metaVal.type = METATYPE_INT32;\
		HRESULT hr = _meta->QueryValue((METAKEY_##__metaKey##), (&metaVal));\
		if (FAILED(hr)) *(__pszBuffer) = TEXT('\0');\
		else {##__formatRoutine((__valueType)metaVal.iVal, (__pszBuffer), (__cchBufferMax));}\
		_meta->Release();}\
	else *(__pszBuffer) = TEXT('\0');}

#define FORMATTER_METAINTPAIR(__pFileInfo, __metaKey1, __metaKey2, __formatRoutine, __pszBuffer, __cchBufferMax)\
	{IFileMeta *_meta;\
	if (SUCCEEDED((__pFileInfo)->QueryInterface(IID_IFileMeta, (void**)&_meta)))	{\
		METAVALUE metaVal1; metaVal1.type = METATYPE_INT32;\
		METAVALUE metaVal2; metaVal2.type = METATYPE_INT32;\
		_meta->QueryValue((METAKEY_##__metaKey1##), (&metaVal1));\
		_meta->QueryValue((METAKEY_##__metaKey2##), (&metaVal2));\
		##__formatRoutine(MAKELONG(metaVal1.iVal, metaVal2.iVal), (__pszBuffer), (__cchBufferMax));\
		_meta->Release();}\
	else *(__pszBuffer) = TEXT('\0');}


LPCTSTR __cdecl Formatter_FilePath(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetPath, LPCTSTR, COPY_STRING, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_FileName(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetFileName, LPCTSTR, COPY_STRING, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_FileExtension(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetExtension, LPCTSTR, COPY_STRING, pszBuffer, cchBufferMax);
	if (TEXT('\0') != *pszBuffer) 
		CharUpperBuff(pszBuffer, lstrlen(pszBuffer));
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_FileFolder(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetPath, LPCTSTR, COPY_STRING, pszBuffer, cchBufferMax);
	if (TEXT('\0') != *pszBuffer) 
	{
		PathRemoveFileSpec(pszBuffer);
		PathRemoveBackslash(pszBuffer);
		PathStripPath(pszBuffer);
	}
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_FileType(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetType, DWORD, FormatItemType, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_FileAttributes(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE(pFileInfo, GetAttributes, DWORD, FormatFileAttributes, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_FileSize(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE_POINTER(pFileInfo, GetSize, ULONGLONG, FormatFileSize, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_FileCreationTime(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE_POINTER(pFileInfo, GetCreationTime, FILETIME, FormatFileTimeLocalize, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_FileModifiedTime(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATVALUE_POINTER(pFileInfo, GetModifiedTime, FILETIME, FormatFileTimeLocalize, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackTitle(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKTITLE, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackAlbum(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKALBUM, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackArtist(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKARTIST, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackGenre(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKGENRE, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackPublisher(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKPUBLISHER, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackComposer(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKCOMPOSER, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_AlbumArtist(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, ALBUMARTIST, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackLength(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATMETA(pFileInfo, TRACKLENGTH, DWORD, FormatLength, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackBitrate(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATMETA(pFileInfo, TRACKBITRATE, DWORD, FormatBitrate, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackYear(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATMETA(pFileInfo, TRACKYEAR, DWORD, FormatPositiveInt, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackNumber(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METAINTPAIR(pFileInfo, TRACKNUMBER, TRACKCOUNT, FormatIntSlashInt, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_DiscNumber(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METAINTPAIR(pFileInfo, DISCNUMBER, DISCCOUNT, FormatIntSlashInt, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_FormattedName(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, FORMATTEDTITLE, pszBuffer, cchBufferMax);
	if (TEXT('\0') == *pszBuffer) 
		FORMATTER_FORMATVALUE(pFileInfo, GetFileName, LPCTSTR, COPY_STRING, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackComment(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_METASTRING(pFileInfo, TRACKCOMMENT, pszBuffer, cchBufferMax);
	return pszBuffer;
}

LPCTSTR __cdecl Formatter_TrackBpm(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	FORMATTER_FORMATMETA(pFileInfo, TRACKBPM, DWORD, FormatPositiveInt, pszBuffer, cchBufferMax);
	return pszBuffer;
}
LPCTSTR __cdecl Formatter_TrackType(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{
	return NULL;
}
LPCTSTR __cdecl Formatter_ExtensionFamily(IFileInfo *pFileInfo, LPTSTR pszBuffer, INT cchBufferMax)
{	
	FORMATTER_FORMATVALUE(pFileInfo, GetExtension, LPCTSTR, FormatExtensionFamily, pszBuffer, cchBufferMax);
	return pszBuffer;
}


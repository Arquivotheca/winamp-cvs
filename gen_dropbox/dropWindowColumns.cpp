#include "./main.h"
#include "./wasabiApi.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./fileMetaScheduler.h"
#include "./formatters.h"
#include "./formatData.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

#define COLUMN_WIDTH_MIN			16
#define COLUMN_WIDTH_MAX			400
#define COLUMN_WIDTH_MAX_LONG	1000


#define REGISTER_COLUMN( __name, __stringId, __width, __format, __minWidth, __maxWidth, __fnFormatter, __fnComparer, __pszMetaKey)\
	{	COLUMN_##__name,\
		__stringId,\
		MAKEINTRESOURCE(IDS_COLUMN_##__name),\
		MAKEINTRESOURCE(IDS_COLUMN_##__name##_LONG),\
		__width,\
		__format,\
		__minWidth,\
		__maxWidth,\
		__fnFormatter,\
		__fnComparer,\
		__pszMetaKey}

#define REGISTER_COLUMN_S(__name, __stringId, __width, __fnFormatter, __fnComparer, __idMetaKey)\
	REGISTER_COLUMN(__name, __stringId, __width, LVCFMT_LEFT, COLUMN_WIDTH_MIN, COLUMN_WIDTH_MAX,\
					__fnFormatter, __fnComparer, MAKEINTRESOURCEA(METAKEY_##__idMetaKey))

#define REGISTER_COLUMN_L(__name, __stringId, __width, __fnFormatter, __fnComparer, __idMetaKey)\
	REGISTER_COLUMN(__name, __stringId, __width, LVCFMT_LEFT, COLUMN_WIDTH_MIN, COLUMN_WIDTH_MAX_LONG,\
					__fnFormatter, __fnComparer, MAKEINTRESOURCEA(METAKEY_##__idMetaKey))

#define REGISTER_COLUMN_RS(__name, __stringId, __width, __fnFormatter, __fnComparer, __idMetaKey)\
	REGISTER_COLUMN(__name, __stringId, __width, LVCFMT_RIGHT, COLUMN_WIDTH_MIN, COLUMN_WIDTH_MAX,\
					__fnFormatter, __fnComparer, MAKEINTRESOURCEA(METAKEY_##__idMetaKey))

#define COMPARE_STR_I(__str1, __str2)\
	((NULL == (__str1) || NULL == (__str2)) ? ((INT)(ULONG_PTR)((__str1) - (__str2))) :\
	(CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (__str1), -1, (__str2), -1) - 2))


#define COMPARE_STR_LOGICAL(__str1, __str2)\
	((NULL == (__str1) || NULL == (__str2)) ? ((INT)(ULONG_PTR)((__str1) - (__str2))) :\
	(CompareStringLogical((__str1), (__str2))))

#define COMPARE_INTPTR(__val1, __val2) ((*(INT*)(__val1)) - (*(INT*)(__val2)))

#define GETFILEINFO_VALUES(__type, __stdmethodCaller)\
	__type val1, val2;\
	IFileInfo *pfi;\
	BOOL f1, f2;\
	f1 = (NULL == (pfi = *(IFileInfo**)pElem1) || S_OK != pfi->##__stdmethodCaller(&val1));\
	f2 = (NULL == (pfi = *(IFileInfo**)pElem2) || S_OK != pfi->##__stdmethodCaller(&val2));\
	if (f1 || f2) return (f2 - f1);\

#define COMPARE_METATAG(__metaTagKey, __metaType, __bufferSizeBytes, __compareFunction, __outResultValue)\
	{IFileMeta *pMeta1 = NULL, *pMeta2 = NULL;\
	INT f1 = (SUCCEEDED((*(IFileInfo**)pElem1)->QueryInterface(IID_IFileMeta, (void**)&pMeta1)));\
	INT f2 = (SUCCEEDED((*(IFileInfo**)pElem2)->QueryInterface(IID_IFileMeta, (void**)&pMeta2)));\
	if (f1 & f2 && f1 == f2)	{\
		BYTE szBuffer1[__bufferSizeBytes], szBuffer2[__bufferSizeBytes];\
		METAVALUE metaVal1; metaVal1.type = (METATYPE_##__metaType);\
		METAVALUE metaVal2; metaVal2.type = (METATYPE_##__metaType);\
		f1 = SUCCEEDED(pMeta1->QueryValueHere(METAKEY_##__metaTagKey##, &metaVal1, szBuffer1, sizeof(szBuffer1)));\
		f2 = SUCCEEDED(pMeta2->QueryValueHere(METAKEY_##__metaTagKey##, &metaVal2, szBuffer2, sizeof(szBuffer2)));\
		if (f1 & f2 && f1 == f2)	{\
			f2 = 0;\
			f1 = (##__compareFunction(szBuffer1, szBuffer2));}}\
	if (NULL != pMeta1) pMeta1->Release();\
	if (NULL != pMeta2) pMeta2->Release();\
	##__outResultValue = (f1 - f2);}

#define RETURN_COMPARE_METATAG(__metaTagKey, __metaType, __bufferSizeBytes, __compareFunction)\
	{ INT r; COMPARE_METATAG(__metaTagKey, __metaType, __bufferSizeBytes, __compareFunction, r); return r;}


typedef INT (CALLBACK *SHLWAPI_STRCMPLOGICALW)(LPCWSTR, LPCWSTR);



static int __cdecl CompareStringLogical(const void *pElem1, const void *pElem2)
{
	static BOOL loadFailed = FALSE;
	static SHLWAPI_STRCMPLOGICALW fnStrCmpLogicalW = NULL;
	
	if (NULL == fnStrCmpLogicalW && !loadFailed)
	{
		UINT prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
		HMODULE  hModule = LoadLibraryW(L"Shlwapi.dll");
		SetErrorMode(prevErrorMode);

		if (hModule) 
		{
			fnStrCmpLogicalW = (SHLWAPI_STRCMPLOGICALW)GetProcAddress(hModule, "StrCmpLogicalW");
			FreeLibrary(hModule);
		}
		loadFailed = (NULL == fnStrCmpLogicalW);
	}

	return (NULL != fnStrCmpLogicalW) ? fnStrCmpLogicalW((LPCTSTR)pElem1, (LPCTSTR)pElem2) :
			(CompareString(LOCALE_USER_DEFAULT, 0, (LPCTSTR)pElem1, -1, (LPCTSTR)pElem2, -1) - 2);
}

static int __cdecl Comparer_FilePath(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(LPCTSTR, GetPath);
	return COMPARE_STR_LOGICAL(val1, val2);
}

static int __cdecl Comparer_FileName(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(LPCTSTR, GetFileName);
	return COMPARE_STR_LOGICAL(val1, val2);
}
static int __cdecl Comparer_FileExtension(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(LPCTSTR, GetExtension);
	return COMPARE_STR_I(val1, val2);
}

static int __cdecl Comparer_FileSize(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(ULONGLONG, GetSize);
	return (INT)(val1 - val2);
}

static int __cdecl Comparer_FileCreationTime(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(FILETIME, GetCreationTime);
	return CompareFileTime(&val1, &val2);
}

static int __cdecl Comparer_FileModifiedTime(const void *pElem1, const void *pElem2)
{
	GETFILEINFO_VALUES(FILETIME, GetModifiedTime);
	return CompareFileTime(&val1, &val2);
}

static int __cdecl Comparer_FileAttributes(const void *pElem1, const void *pElem2)
{
	TCHAR szTest1[32], szTest2[32];
	GETFILEINFO_VALUES(DWORD, GetAttributes);
	return COMPARE_STR_I(FormatFileAttributes(val1, szTest1, ARRAYSIZE(szTest1)), 
						FormatFileAttributes(val2, szTest2, ARRAYSIZE(szTest2)));

}

static int __cdecl Comparer_FileType(const void *pElem1, const void *pElem2)
{
	TCHAR szTest1[32], szTest2[32];
	GETFILEINFO_VALUES(DWORD, GetType);
	return COMPARE_STR_I(FormatItemType(val1, szTest1, ARRAYSIZE(szTest1)), 
						FormatItemType(val2, szTest2, ARRAYSIZE(szTest2)));
}

static int __cdecl Comparer_ExtensionFamily(const void *pElem1, const void *pElem2)
{
	TCHAR szTest1[256], szTest2[256];
	GETFILEINFO_VALUES(LPCTSTR, GetExtension);
	return COMPARE_STR_I(FormatExtensionFamily(val1, szTest1, ARRAYSIZE(szTest1)), 
						FormatExtensionFamily(val2, szTest2, ARRAYSIZE(szTest2)));

}

static int __cdecl Comparer_TrackTitle(const void *pElem1, const void *pElem2)
{		
	RETURN_COMPARE_METATAG(TRACKTITLE, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_TrackArtist(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKARTIST, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_TrackAlbum(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKALBUM, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_TrackGenre(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKGENRE, WSTR, 128, CompareStringLogical);
}
static int __cdecl Comparer_TrackPublisher(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKPUBLISHER, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_TrackComposer(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKCOMPOSER, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_AlbumArtist(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(ALBUMARTIST, WSTR, 512, CompareStringLogical);
} 
static int __cdecl Comparer_TrackComment(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKCOMMENT, WSTR, 512, CompareStringLogical);
}
static int __cdecl Comparer_TrackLength(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKLENGTH, INT32, sizeof(INT), COMPARE_INTPTR);
}
static int __cdecl Comparer_TrackBitrate(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKBITRATE, INT32, sizeof(INT), COMPARE_INTPTR);
}
static int __cdecl Comparer_TrackYear(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKYEAR, INT32, sizeof(INT), COMPARE_INTPTR);
}
static int __cdecl Comparer_TrackBpm(const void *pElem1, const void *pElem2)
{	
	RETURN_COMPARE_METATAG(TRACKBPM, INT32, sizeof(INT), COMPARE_INTPTR);
}
static int __cdecl Comparer_TrackNumber(const void *pElem1, const void *pElem2)
{	
	INT result;
	COMPARE_METATAG(TRACKNUMBER, INT32, sizeof(INT), COMPARE_INTPTR, result);
	if (0 == result) COMPARE_METATAG(TRACKCOUNT, INT32, sizeof(INT), COMPARE_INTPTR, result);
	return result;
}

static int __cdecl Comparer_DiscNumber(const void *pElem1, const void *pElem2)
{	
	INT result;
	COMPARE_METATAG(DISCNUMBER, INT32, sizeof(INT), COMPARE_INTPTR, result);
	if (0 == result) COMPARE_METATAG(DISCCOUNT, INT32, sizeof(INT), COMPARE_INTPTR, result);
	return result;
}

static int __cdecl Comparer_FormattedName(const void *pElem1, const void *pElem2)
{	
	TCHAR szVal1[512], szVal2[512];
	return CompareStringLogical(Formatter_FormattedName(*(IFileInfo**)pElem1, szVal1, ARRAYSIZE(szVal1)),
		Formatter_FormattedName(*(IFileInfo**)pElem2, szVal2, ARRAYSIZE(szVal2)));
}

INT ColumnIdToMetaKey(INT columnId, METAKEY *pMetaKey, INT metaKeyMax)
{
	INT count = 0;
	*pMetaKey = METAKEY_INVALID;
	INT columnIndex;
	for (columnIndex = 0; columnIndex < ARRAYSIZE(szRegisteredColumns); columnIndex++)
	{
		if (szRegisteredColumns[columnIndex].id == columnId)
			break;
	}

	if (ARRAYSIZE(szRegisteredColumns) == columnIndex)
		return 0;

	const LISTCOLUMN *column = &szRegisteredColumns[columnIndex];
	
	if (IS_INTRESOURCE(column->pszMetaKey))
	{
		*pMetaKey = (METAKEY)(INT_PTR)column->pszMetaKey;
		return (METAKEY_INVALID != *pMetaKey) ? 1 : 0;
	}
	LPCSTR p = column->pszMetaKey;

	while ('\0' != *p && count < metaKeyMax)
	{
		if (SUCCEEDED(GetMetaKeyByName(p, &pMetaKey[count])) &&
			METAKEY_INVALID != pMetaKey[count])
		{
			count++;
		}
		p += (lstrlenA(p) + 1);
	}
	
	return count;
}

const static LISTCOLUMN szRegisteredColumns[COLUMN_LAST] = 
{ 
	REGISTER_COLUMN_L(FORMATTEDTITLE, TEXT("formattedTitle"), 120, Formatter_FormattedName, Comparer_FormattedName, FORMATTEDTITLE),
	REGISTER_COLUMN_S(FILEPATH, TEXT("filePath"), 72, Formatter_FilePath, Comparer_FilePath, INVALID),
	REGISTER_COLUMN_S(FILENAME, TEXT("fileName"), 80, Formatter_FileName, Comparer_FileName, INVALID),
	REGISTER_COLUMN_RS(FILESIZE, TEXT("fileSize"), 60, Formatter_FileSize, Comparer_FileSize, INVALID),
	REGISTER_COLUMN_S(FILETYPE, TEXT("fileType"), 96, Formatter_FileType, Comparer_FileType, INVALID),
	REGISTER_COLUMN_S(FILEEXTENSION, TEXT("fileExtension"), 60, Formatter_FileExtension, Comparer_FileExtension, INVALID),
	REGISTER_COLUMN_S(FILEMODIFIED, TEXT("fileModified"), 132, Formatter_FileModifiedTime, Comparer_FileModifiedTime, INVALID),
	REGISTER_COLUMN_S(FILECREATED, TEXT("fileCreated"), 132, Formatter_FileCreationTime, Comparer_FileCreationTime, INVALID),
	REGISTER_COLUMN_S(FILEATTRIBUTES, TEXT("fileAttributes"), 80, Formatter_FileAttributes, Comparer_FileAttributes, INVALID),
	REGISTER_COLUMN_S(TRACKARTIST, TEXT("trackArtist"), 120, Formatter_TrackArtist, Comparer_TrackArtist, TRACKARTIST),
	REGISTER_COLUMN_S(TRACKALBUM, TEXT("trackAlbum"), 120, Formatter_TrackAlbum, Comparer_TrackAlbum, TRACKALBUM),
	REGISTER_COLUMN_S(TRACKTITLE, TEXT("trackTitle"), 120, Formatter_TrackTitle, Comparer_TrackTitle, TRACKTITLE),
	REGISTER_COLUMN_S(TRACKGENRE, TEXT("trackGenre"), 100, Formatter_TrackGenre, Comparer_TrackGenre, TRACKGENRE),
	REGISTER_COLUMN_S(TRACKLENGTH, TEXT("trackLength"), 80, Formatter_TrackLength, Comparer_TrackLength, TRACKLENGTH),
	REGISTER_COLUMN_S(TRACKBITRATE, TEXT("trackBitrate"), 60, Formatter_TrackBitrate, Comparer_TrackBitrate, TRACKBITRATE),
	
	REGISTER_COLUMN(TRACKNUMBER, TEXT("trackNumber"), 60, LVCFMT_LEFT, COLUMN_WIDTH_MIN, COLUMN_WIDTH_MAX,
					Formatter_TrackNumber, Comparer_TrackNumber, METAKEY_TRACKNUMBER_STRING "\0" METAKEY_TRACKCOUNT_STRING "\0\0"),

	REGISTER_COLUMN(DISCNUMBER, TEXT("discNumber"), 60, LVCFMT_LEFT, COLUMN_WIDTH_MIN, COLUMN_WIDTH_MAX,
					Formatter_DiscNumber, Comparer_DiscNumber, METAKEY_DISCNUMBER_STRING "\0" METAKEY_DISCCOUNT_STRING "\0\0"),
	
	REGISTER_COLUMN_S(TRACKYEAR, TEXT("trackYear"), 60, Formatter_TrackYear, Comparer_TrackYear, TRACKYEAR),
	REGISTER_COLUMN_S(TRACKPUBLISHER, TEXT("trackPublisher"), 120, Formatter_TrackPublisher, Comparer_TrackPublisher, TRACKPUBLISHER),
	REGISTER_COLUMN_S(TRACKCOMPOSER, TEXT("trackComposer"), 120, Formatter_TrackComposer, Comparer_TrackComposer, TRACKCOMPOSER),
	REGISTER_COLUMN_S(ALBUMARTIST, TEXT("albumArtist"), 120, Formatter_AlbumArtist, Comparer_AlbumArtist, ALBUMARTIST),
	REGISTER_COLUMN_S(TRACKCOMMENT, TEXT("trackComment"), 120, Formatter_TrackComment, Comparer_TrackComment, TRACKCOMMENT),
	REGISTER_COLUMN_S(TRACKBPM, TEXT("trackBpm"), 60, Formatter_TrackBpm, Comparer_TrackBpm, TRACKBPM),
	REGISTER_COLUMN_S(EXTENSIONFAMILY, TEXT("extensionFamily"), 120, Formatter_ExtensionFamily, Comparer_ExtensionFamily, INVALID),
	
};

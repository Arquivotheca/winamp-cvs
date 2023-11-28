#ifndef NULLOSFT_DROPBOX_PLUGIN_FILEIMAGELIST_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_FILEIMAGELIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <commctrl.h>

class FileImageList
{
public:
	FileImageList(HINSTANCE hResourseInstance, LPCTSTR pszResourceName, INT nCacheSize);
	~FileImageList();

public:
	ULONG AddRef();
	ULONG Release();
	HRESULT Load();
	
	BOOL IsLoaded() { return (NULL != hList); }
	INT GetImageCount() { return imageCount;}
	HIMAGELIST GetList() { return hList;}

	BOOL GetImageSize(INT *cx, INT *cy);

	INT GetOriginal(INT imageIndex);
	INT Get(INT imageIndex, COLORREF rgbBk, COLORREF rgbFg);

	BOOL Draw(HDC hdc, INT imageIndex, INT x, INT y, INT cx, INT cy, COLORREF rgbBk, COLORREF rgbFg);

protected:
	INT AddImage(COLORREF rgbBk, COLORREF rgbFg);
	BOOL PerformFileteredCopy(HBITMAP hbmp, RECT *prcSource, RECT *prcDest, COLORREF rgbBk, COLORREF rgbFg);

protected:
	ULONG ref;
	HINSTANCE hInstance;
	LPTSTR pszResource;
	HIMAGELIST hList;
	INT cacheSize;
	INT imageCount;

	__int64 *colorKey;
	int keyCount;
	int keyCursor;
	int keyInsert;
};

#endif //NULLOSFT_DROPBOX_PLUGIN_FILEIMAGELIST_HEADER

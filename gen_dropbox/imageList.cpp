#include "./imageList.h"
#include "./imageLoader.h"
#include "../nu/trace.h"

#define MIN_CACHESIZE	2
#define MAX_CACHESIZE	8

#define MAKECOLORKEY(__rgb1, __rgb2) ((((__int64)(0x00FFFFFF & (__rgb1))) << 24) | ((__int64)(0x00FFFFFF & (__rgb2))))

FileImageList::FileImageList(HINSTANCE hResourseInstance, LPCTSTR pszResourceName, INT nCacheSize)
	: ref(1), hList(NULL), hInstance(hResourseInstance), pszResource(NULL), cacheSize(nCacheSize), imageCount(0)
{
	if (!IS_INTRESOURCE(pszResourceName))
	{
		INT cchLen = lstrlen(pszResourceName);
		if (0 != cchLen)
		{
			INT cbLen = ((cchLen + 1) * sizeof(TCHAR));
			pszResource = (LPTSTR)malloc(cbLen);
			if (NULL != pszResource)
				CopyMemory(pszResource, pszResourceName, cbLen);
		}
	}
	else 
		pszResource = (LPTSTR)pszResourceName;

	if (cacheSize < MIN_CACHESIZE)
		cacheSize = MIN_CACHESIZE;
	else if (cacheSize > MAX_CACHESIZE)
		cacheSize = MAX_CACHESIZE;

	colorKey = (__int64*)malloc(sizeof(__int64) * cacheSize);
	keyCount = 0;
	keyCursor = 0;
	keyInsert = 0;
}

FileImageList::~FileImageList()
{
	if (!IS_INTRESOURCE(pszResource))
		free(pszResource);
	
	if (NULL != hList)
		ImageList_Destroy(hList);

	if (NULL != colorKey)
		free(colorKey);
}

ULONG FileImageList::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG FileImageList::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT FileImageList::Load()
{
	if (NULL != hList)
	{
		ImageList_Destroy(hList);
		hList = NULL;
	}

	imageCount = 0;
	keyCount = 0;
	keyCursor = 0;
	keyInsert = 0;

	BITMAPINFOHEADER bitmapHeader;
	BYTE *pixelData;
	HBITMAP hBitmap = ImageLoader_LoadPngEx(hInstance, pszResource, &bitmapHeader, (void**)&pixelData);
	if (NULL == hBitmap)
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	
	HRESULT hr = S_OK;
	INT elementSize = abs(bitmapHeader.biHeight);
	imageCount = bitmapHeader.biWidth / elementSize;

	if (imageCount > 0)
	{		
		hList = ImageList_Create(elementSize, elementSize, ILC_COLOR32, imageCount * (cacheSize + 1), imageCount);
		if (NULL == hList)
		{
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}
	}
	else 
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	if (NULL != hList)
	{
		for (int i = 0; i < cacheSize; i++)
		{
			INT k = ImageList_Add(hList, hBitmap, NULL);
			if (-1 == k) 
			{
				aTRACE_LINE("d'oh");

			}

		}
	}
	
	if (NULL != hBitmap)
		DeleteObject(hBitmap);

	if (FAILED(hr))
	{
		if (NULL != hList)
		{
			ImageList_Destroy(hList);
			hList = NULL;
		}
		imageCount = NULL;
	}
	return hr;
}

INT FileImageList::GetOriginal(INT imageIndex)
{
	if (NULL == hList || imageIndex < 0 || imageIndex >= imageCount)
		return -1;
	return imageIndex;
}



INT FileImageList::Get(INT imageIndex, COLORREF rgbBk, COLORREF rgbFg)
{
	if (NULL == hList || imageIndex < 0 || imageIndex >= imageCount)
		return -1;

	__int64 key = MAKECOLORKEY(rgbFg, rgbBk);

	INT keyIndex = -1;
	
	for (INT i = keyCursor ; i < keyCount; i++)
	{ if (colorKey[i] == key) { keyIndex = i; break;} }
	if (-1 == keyIndex && keyCursor > 0)
	{
		for (INT i = 0 ; i < keyCursor; i++)
		{ if (colorKey[i] == key) { keyIndex = i; break;} }
	}
	if (-1 == keyIndex)
		keyIndex = AddImage(rgbBk, rgbFg);
	
	INT realIndex;
	if (-1 != keyIndex)
	{
		realIndex = (keyIndex + 1) * imageCount + imageIndex;
		keyCursor = keyIndex;
	}
	else realIndex = -1;
	
	return realIndex;
}

INT FileImageList::AddImage(COLORREF rgbBk, COLORREF rgbFg)
{
	INT keyIndex = -1;
	
	if ((keyInsert + 1) >= keyCount || (keyInsert + 1)  == cacheSize)
		keyInsert = 0;
	INT offset = (imageCount * (keyInsert + 1));
	IMAGEINFO info;
	RECT rSrc;
	for (int i = 0; i < imageCount; i++)
	{
		if (0 != ImageList_GetImageInfo(hList, i, &info))
		{
			CopyRect(&rSrc, &info.rcImage);
			if (0 != ImageList_GetImageInfo(hList, i + offset, &info))
			{
				PerformFileteredCopy(info.hbmImage, &rSrc, &info.rcImage, rgbBk, rgbFg);
			}
		}
	}

	__int64 key = MAKECOLORKEY(rgbFg, rgbBk);
	
	keyIndex = keyInsert;
	colorKey[keyInsert] = key;
	if (keyCount < cacheSize)
		keyCount++;
	keyInsert++;

	return keyIndex;
}

BOOL FileImageList::PerformFileteredCopy(HBITMAP hbmp, RECT *prcSource, RECT *prcDest, COLORREF rgbBk, COLORREF rgbFg)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel < 24) 
		return FALSE;
	if (NULL == prcSource || NULL == prcDest)
		return FALSE;

	LONG pitch, x;
	INT step = (dibsec.dsBm.bmBitsPixel>>3);
	BYTE rBk, gBk, bBk, rFg, gFg, bFg;
	LPBYTE sc, dc, sl, dl;
	pitch = dibsec.dsBm.bmWidth * step;
	while (pitch%4) pitch++;

	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);

	
	INT ofs, ofd;
	if (dibsec.dsBm.bmHeight > 0)
	{ 
		ofs = dibsec.dsBm.bmHeight - prcSource->bottom;
		ofd = dibsec.dsBm.bmHeight - prcDest->bottom;
	}
	else
	{
		ofs = prcSource->top;
		ofd = prcDest->top;
	} 

	sl = ((LPBYTE)dibsec.dsBm.bmBits) + pitch * ofs + prcSource->left*step;
	dl = ((LPBYTE)dibsec.dsBm.bmBits) + pitch * ofd + prcDest->left*step;
	LONG cy = prcSource->bottom  - prcSource->top;
	LONG cx = prcSource->right - prcSource->left;

	if (3 == step) //24bpp
	{
		for (; cy-- != 0; dl += pitch, sl += pitch )
		{	
			for (x = cx, sc = sl, dc = dl; x-- != 0; dc += 3, sc += 3) 
			{
				dc[0] = bFg - ((bFg - bBk)*(255 - sc[0])>>8);
				dc[1] = gFg - ((gFg - gBk)*(255 - sc[1])>>8);
				dc[2] = rFg - ((rFg - rBk)*(255 - sc[2])>>8);
			}
		}
	}
	else if (4 == step) //32 bpp
	{
		for (; cy-- != 0; dl += pitch, sl += pitch )
		{	
			for (x = cx, sc = sl, dc = dl; x-- != 0; dc += 4, sc += 4) 
			{
				if (0x00 == sc[3]) 
				{
					dc[0] = bBk;
					dc[1] = gBk;
					dc[2] = rBk;
					dc[3] = 0xFF;
				}
				else if (0xFF == sc[3])
				{
					dc[0] = bFg - ((bFg - bBk)*(255 - sc[0])>>8);
					dc[1] = gFg - ((gFg - gBk)*(255 - sc[1])>>8);
					dc[2] = rFg - ((rFg - rBk)*(255 - sc[2])>>8);
				}
				else
				{
					dc[0] = ((bFg - ((bFg - bBk)*(255 - sc[0])>>8))*sc[3] + (((255 - sc[3])*255 + 127)/255)*bBk + 127)/255;
					dc[1] = ((gFg - ((gFg - gBk)*(255 - sc[1])>>8))*sc[3] + (((255 - sc[3])*255 + 127)/255)*gBk + 127)/255;
					dc[2] = ((rFg - ((rFg - rBk)*(255 - sc[2])>>8))*sc[3] + (((255 - sc[3])*255 + 127)/255)*rBk + 127)/255;
					dc[3] = 0xFF;
				}
			}
		}
	}
	return TRUE;
}

BOOL FileImageList::GetImageSize(INT *cx, INT *cy)
{
	if (NULL == hList || !ImageList_GetIconSize(hList, cx, cy))
	{
		*cx = 0;
		*cy = 0;
		return FALSE;
	}
	return TRUE;
}

BOOL FileImageList::Draw(HDC hdc, INT imageIndex, INT x, INT y, INT cx, INT cy, COLORREF rgbBk, COLORREF rgbFg)
{
	INT realIndex = Get(imageIndex, rgbBk, rgbFg);
	if (-1 == realIndex)
		return FALSE;
	return ImageList_DrawEx(hList, realIndex, hdc, x, y, cx, cy, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL);
}

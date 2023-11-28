#include "./main.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./imageLoader.h"
#include <shlobj.h>
#include <strsafe.h>



HBITMAP CreateDragImage(SHDRAGIMAGE *pDragImage, INT iCount)
{
	if (NULL == pDragImage)
		return NULL;
		
	HFONT hf = NULL, hfo = NULL;
	TCHAR szText[64];
	size_t cchText = 0;
	SIZE textSize = {0, 0};


	LOGFONT dragNumberLogFont = 
	{
		-12,
		0, // lfWidth 
		0, // lfEscapement 
		0, // lfOrientation
		FW_ULTRABOLD, // lfWeight 
		FALSE, // lfItalic
		FALSE, // lfUnderline 
		FALSE, // lfStrikeOut 
		DEFAULT_CHARSET, //lfCharSet
		OUT_TT_PRECIS, //lfOutPrecision
		CLIP_DEFAULT_PRECIS, //lfClipPrecision 
		PROOF_QUALITY, //lfQuality 
		DEFAULT_PITCH | FF_DONTCARE, //lfPitchAndFamily 
		TEXT("Verdana"), //lfFaceName
	};

	HDC hdc = CreateCompatibleDC(0);
	if (NULL == hdc)
		return NULL;
	
	if (iCount < 1 || 
		FAILED(StringCchPrintfEx(szText, ARRAYSIZE(szText), NULL, &cchText, 0, TEXT("%d"), iCount)))
	{
		szText[0] = TEXT('\0');
		cchText = 0;
	}
	else cchText = ARRAYSIZE(szText) - cchText;

	
	HBITMAP bmpComposition = NULL, bmpIcon = NULL, bmpNumber = NULL;
	BITMAPINFOHEADER hdrComposition;
	BYTE *pxComposition;
	INT iconCX, iconCY, numberCX, numberCY;
	
	bmpIcon = ImageLoader_LoadPng(plugin.hDllInstance, MAKEINTRESOURCE(IDR_DRAGIMAGE), &iconCX, &iconCY);
	if (NULL != bmpIcon)
	{
		ZeroMemory(&hdrComposition, sizeof(BITMAPINFOHEADER));
		hdrComposition.biSize = sizeof(BITMAPINFOHEADER);
		hdrComposition.biBitCount = 32;
		hdrComposition.biPlanes = 1;
		hdrComposition.biCompression = BI_RGB;
		hdrComposition.biWidth = iconCX;
		hdrComposition.biHeight = -iconCY;
		
		if (cchText > 0)
		{
			bmpNumber = ImageLoader_LoadPng(plugin.hDllInstance, MAKEINTRESOURCE(IDR_NUMHOLDERIMAGE), &numberCX, &numberCY);
			if (NULL != bmpNumber)
			{
				dragNumberLogFont.lfHeight = numberCY - 4;
				hf = CreateFontIndirect(&dragNumberLogFont);
				if (NULL != hf) 
				{
					hfo = (HFONT)SelectObject(hdc, hf);
					if (!GetTextExtentPoint32(hdc, szText, (INT)cchText, &textSize))
						ZeroMemory(&textSize, sizeof(SIZE));
				}
	
				INT numberWidth = numberCX - 1 + textSize.cx - 2;
				if (numberWidth > 10)
					hdrComposition.biWidth += (numberWidth - 10);
			}
		}

		bmpComposition = CreateDIBSection(hdc, (LPBITMAPINFO)&hdrComposition, DIB_RGB_COLORS, (void**)&pxComposition, NULL, 0);
		if (NULL != bmpComposition)
		{
			HBITMAP bmpOld = (HBITMAP)SelectObject(hdc, bmpComposition);
			SetBkColor(hdc, 0x00FF00FF);
			RECT rc;
			SetRect(&rc, 0, 0, hdrComposition.biWidth, abs(hdrComposition.biHeight));
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			
			HDC hdcSrc = CreateCompatibleDC(0);
			if (NULL != hdcSrc)
			{
				INT x, y, cx, cy;

				BLENDFUNCTION blendFunction;
				blendFunction.AlphaFormat = AC_SRC_ALPHA;
				blendFunction.BlendOp = AC_SRC_OVER;
				blendFunction.BlendFlags = 0;
				blendFunction.SourceConstantAlpha = 255;

				HBITMAP bmpSrc = (HBITMAP)SelectObject(hdcSrc, bmpIcon);
				
				cx = iconCX;
				cy = iconCY;
				AlphaBlend(hdc, 0, 0, cx, cy, hdcSrc, 0, 0, cx, cy, blendFunction);
				
				if (NULL != bmpNumber)
				{				
					x = iconCX - 10;
					y = iconCY - 20;

					if (x < 4) x = 4;
					if (y < 4) y = 4;
				
					cy = numberCY;

					SelectObject(hdcSrc, bmpNumber);
					
					cx = (numberCX - 1) / 2;
					AlphaBlend(hdc, x, y, cx, cy, hdcSrc, 0, 0, cx, cy, blendFunction);
					x += cx;

					RECT rcText;
					rcText.left = x - 1;
					rcText.right = rcText.left + textSize.cx;
					rcText.top = y + (cy - textSize.cy) / 2;
					if (rcText.top < (y + 1)) rcText.top = y + 1; 
					rcText.bottom = rcText.top + textSize.cy;
					if (rcText.bottom > (y + cy - 1)) rcText.bottom = (y + cy - 1);
					
					cx = textSize.cx - 2 - 1;
					AlphaBlend(hdc, x, y, cx, cy, hdcSrc, (numberCX - 1) / 2, 0, 1, cy, blendFunction);
					x += cx;

					cx = (numberCX - 1) / 2;
					AlphaBlend(hdc, x, y, cx, cy, hdcSrc, (numberCX - 1) / 2 + 1, 0, cx, cy, blendFunction);

					SetTextColor(hdc, 0x00FFFFFF);
					SetBkMode(hdc, TRANSPARENT);
					ExtTextOut(hdc, rcText.left,  rcText.top, ETO_CLIPPED, &rcText, szText, (INT)cchText, NULL); 
	

				}
				SelectObject(hdcSrc, bmpSrc);
				DeleteDC(hdcSrc);
			}

			SelectObject(hdc, bmpOld);
		}
	}
	
	ZeroMemory(pDragImage, sizeof(SHDRAGIMAGE));
	if (NULL != bmpComposition)
	{				
		pDragImage->hbmpDragImage = bmpComposition;
		pDragImage->crColorKey = 0x00FF00FF;
		pDragImage->sizeDragImage.cx = hdrComposition.biWidth;
		pDragImage->sizeDragImage.cy = abs(hdrComposition.biHeight);
		pDragImage->ptOffset.x = 10;
		pDragImage->ptOffset.y = 20;
	}

	if (NULL != bmpIcon)
		DeleteObject(bmpIcon);
	if (NULL != bmpNumber)
		DeleteObject(bmpNumber);
	
	if (NULL != hf)
	{
		SelectObject(hdc, hfo);
		DeleteObject(hf);
	}
	DeleteDC(hdc);

	return bmpComposition;
}

BOOL CreateDragImageList(INT iCount)
{	
	SHDRAGIMAGE di;
	BOOL bCreated = FALSE;
	HBITMAP hbmp = CreateDragImage(&di, iCount);
	HIMAGELIST imageList = NULL;
	if (NULL != hbmp)
	{
		imageList = ImageList_Create(di.sizeDragImage.cx, di.sizeDragImage.cy, ILC_COLOR32 | ILC_MASK, 1, 0);
		if (NULL != imageList && -1 == ImageList_AddMasked(imageList, hbmp, di.crColorKey))
		{
			ImageList_Destroy(imageList);
			imageList = NULL;
		}
		DeleteObject(hbmp);
	}
	
	if (NULL != imageList)
	{
		bCreated  = ImageList_BeginDrag(imageList, 0, di.ptOffset.x, di.ptOffset.y);
		ImageList_Destroy(imageList);
		imageList = NULL;
	}

	return bCreated;
}
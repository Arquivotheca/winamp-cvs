#include "main.h"
#include "./lengthConverter.h"

static float
LengthConverter_GetAverageCharWidth(HDC hdc)
{
	SIZE textSize;
	float dataCount;
	const char data[] = 
	{ 
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P', 'Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l', 'm','n','o','p','q','r','s','t','u','v','w','x','y','z'
	};

	if (FALSE == GetTextExtentPointA(hdc, data, ARRAYSIZE(data) -1, &textSize))
		return 0.0f;

	dataCount = ARRAYSIZE(data);
	
	return ((float)textSize.cx + dataCount/2.0f)/dataCount;
}

BOOL
LengthConverter_Init(LengthConverter *converter, HDC hdc, float blockWidth, float blockHeight)
{
	HDC privateDC;
	TEXTMETRICW tm;
	
	if (NULL == converter)
		return FALSE;

	if (NULL == hdc)
	{		
		privateDC = GetDCEx(NULL, NULL, 0);
		if (NULL == privateDC)
			return FALSE;

		hdc = privateDC;
	}
	else
		privateDC = NULL;
		
	converter->dpiX = (float)GetDeviceCaps(hdc, LOGPIXELSX);
	converter->dpiY = (float)GetDeviceCaps(hdc, LOGPIXELSY);

	converter->blockHeight = blockHeight;
	converter->blockWidth = blockWidth;
	converter->fontWidth = LengthConverter_GetAverageCharWidth(hdc);
	converter->fontHeight = (FALSE != GetTextMetricsW(hdc, &tm)) ? (float)tm.tmHeight : 0.0f;
	
	if (NULL != privateDC)
		ReleaseDC(NULL, privateDC);

	return TRUE;
}

BOOL
LengthConverter_InitFromWindow(LengthConverter *converter, HWND hwnd)
{
	HDC hdc;
	HFONT font, fontPrev; 
	RECT rect;
	BOOL result;

	if (NULL == hwnd)
		return FALSE;

	if (FALSE == GetClientRect(hwnd, &rect))
		return FALSE;

	hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc)
		return FALSE;

	font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	fontPrev = SelectFont(hdc, font);

	result = LengthConverter_Init(converter, hdc, (float)RECTWIDTH(rect), (float)RECTHEIGHT(rect));

	SelectFont(hdc, fontPrev);
	ReleaseDC(hwnd, hdc);

	return result;
}


BOOL
LengthConverter_SetBlock(LengthConverter *converter, float blockWidth, float blockHeight)
{
	if (NULL == converter)
		return FALSE;

	converter->blockWidth = blockWidth;
	converter->blockHeight = blockHeight;
	return TRUE;
}

float
LengthConverter_ScaleX(const LengthConverter *converter, float x)
{
	if (NULL == converter)
		return 0.0f;

	return (x * converter->dpiX) / 96.0f;
}

float
LengthConverter_ScaleY(const LengthConverter *converter, float y)
{
	if (NULL == converter)
		return 0.0f;

	return (y * converter->dpiY) / 96.0f;
}

float
LengthConverter_UnscaleX(const LengthConverter *converter, float x)
{
	if (NULL == converter || 0.0f == converter->dpiX)
		return 0.0f;

	return (x * 96.0f) / converter->dpiX;
}

float
LengthConverter_UnscaleY(const LengthConverter *converter, float y)
{
	if (NULL == converter || 0.0f == converter->dpiY)
		return 0.0f;

	return (y * 96.0f) / converter->dpiY;

}

float
LengthConverter_PointsToPixelsX(const LengthConverter *converter, float pt)
{
	if (NULL == converter)
		return 0.0f;

	return (pt * converter->dpiX) / 72.0f;
}

float
LengthConverter_PointsToPixelsY(const LengthConverter *converter, float pt)
{
	if (NULL == converter)
		return 0.0f;

	return (pt * converter->dpiY) / 72.0f;
}

float
LengthConverter_PixelsToPointsX(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->dpiX)
		return 0.0f;

	return (px * 72.0f) / converter->dpiX;
}

float
LengthConverter_PixelsToPointsY(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->dpiY)
		return 0.0f;

	return (px * 72.0f) / converter->dpiY;
}

float
LengthConverter_DluToPixelsX(const LengthConverter *converter, float dlu)
{
	if (NULL == converter)
		return 0.0f;

	return (dlu * converter->fontWidth) / 4.0f;
}

float
LengthConverter_DluToPixelsY(const LengthConverter *converter, float dlu)
{
	if (NULL == converter)
		return 0.0f;

	return (dlu * converter->fontHeight) / 8.0f;
}

float
LengthConverter_PixelsToDluX(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->fontWidth)
		return 0.0f;

	return (px * 4.0f) / converter->fontWidth;
}

float
LengthConverter_PixelsToDluY(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->fontHeight)
		return 0.0f;

	return (px * 8.0f) / converter->fontHeight;
}

float
LengthConverter_EmToPixelsX(const LengthConverter *converter, float em)
{
	if (NULL == converter)
		return 0.0f;

	return (converter->fontWidth * em);
}

float
LengthConverter_EmToPixelsY(const LengthConverter *converter, float em)
{
	if (NULL == converter)
		return 0.0f;

	return (converter->fontHeight * em);
}

float
LengthConverter_PixelsToEmX(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->fontWidth)
		return 0.0f;

	return (px / converter->fontWidth);
}

float
LengthConverter_PixelsToEmY(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->fontHeight)
		return 0.0f;

	return (px / converter->fontHeight);
}

float
LengthConverter_PercentToPixelsX(const LengthConverter *converter, float percent)
{
	if (NULL == converter)
		return 0.0f;

	return (converter->blockWidth * percent) / 100.0f;
}

float
LengthConverter_PercentToPixelsY(const LengthConverter *converter, float percent)
{
	if (NULL == converter)
		return 0.0f;

	return (converter->blockHeight * percent) / 100.0f;
}

float
LengthConverter_PixelsToPercentX(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->blockWidth)
		return 0.0f;

	return (px * 100.0f) / converter->blockWidth;
}

float
LengthConverter_PixelsToPercentY(const LengthConverter *converter, float px)
{
	if (NULL == converter || 0.0f == converter->blockHeight)
		return 0.0f;

	return (px * 100.0f) / converter->blockHeight;
}


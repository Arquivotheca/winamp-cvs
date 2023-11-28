#ifndef _NULLSOFT_WINAMP_DATAVIEW_LENGTH_CONVERTER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_LENGTH_CONVERTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct LengthConverter
{
	float fontWidth;
	float fontHeight;
	float blockWidth;
	float blockHeight;
	float dpiX;
	float dpiY;
} LengthConverter;

BOOL
LengthConverter_Init(LengthConverter *converter, HDC hdc, float blockWidth, float blockHeight);

BOOL
LengthConverter_InitFromWindow(LengthConverter *converter, HWND hwnd);

BOOL
LengthConverter_SetBlock(LengthConverter *converter, float blockWidth, float blockHeight);

// Scale
float
LengthConverter_ScaleX(const LengthConverter *converter, float x);
float
LengthConverter_ScaleY(const LengthConverter *converter, float y);

// Unscale
float
LengthConverter_UnscaleX(const LengthConverter *converter, float x);
float
LengthConverter_UnscaleY(const LengthConverter *converter, float y);

// Points -> Pixels
float
LengthConverter_PointsToPixelsX(const LengthConverter *converter, float pt);
float
LengthConverter_PointsToPixelsY(const LengthConverter *converter, float pt);

// Pixels -> Points
float
LengthConverter_PixelsToPointsX(const LengthConverter *converter, float px);
float
LengthConverter_PixelsToPointsY(const LengthConverter *converter, float px);

// DLU -> Pixels
float
LengthConverter_DluToPixelsX(const LengthConverter *converter, float dlu);
float
LengthConverter_DluToPixelsY(const LengthConverter *converter, float dlu);

// Pixels -> DLU
float
LengthConverter_PixelsToDluX(const LengthConverter *converter, long px);
float
LengthConverter_PixelsToDluY(const LengthConverter *converter, long px);

// Em -> Pixels
float
LengthConverter_EmToPixelsX(const LengthConverter *converter, float em);
float
LengthConverter_EmToPixelsY(const LengthConverter *converter, float em);

// Pixels -> Em
float
LengthConverter_PixelsToEmX(const LengthConverter *converter, float px);
float
LengthConverter_PixelsToEmY(const LengthConverter *converter, float px);

// Percent -> Pixels
float
LengthConverter_PercentToPixelsX(const LengthConverter *converter, float percent);
float
LengthConverter_PercentToPixelsY(const LengthConverter *converter, float percent);

// Pixels -> Percent
float
LengthConverter_PixelsToPercentX(const LengthConverter *converter, float px);
float
LengthConverter_PixelsToPercentY(const LengthConverter *converter, float px);


#endif //_NULLSOFT_WINAMP_DATAVIEW_LENGTH_CONVERTER_HEADER
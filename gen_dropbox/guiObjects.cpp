#include "./main.h"
#include "./guiObjects.h"

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY       5
#define CLEARTYPE_NATURAL_QUALITY       6
#endif

static HCURSOR szOleCursors[OLECURSOR_LASTENTRY] = 
{
	NULL,		// OLECURSOR_NODROP
	NULL,		// OLECURSOR_DROPMOVE
	NULL,		// OLECURSOR_DROPCOPY
	NULL,		// OLECURSOR_DROPLINK
	NULL,		// OLECURSOR_DROPMOVE_INVERT
	NULL,		// OLECURSOR_DROPCOPY_INVERT
	NULL,		// OLECURSOR_DROPLINK_INVERT
};


HCURSOR GetOleCursorFromDropEffect(INT dropEffect)
{
	INT oleCode = -1;

	if (0 != (DROPEFFECT_SCROLL & dropEffect))
	{
		return (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0,
					LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE);
	}

	switch(dropEffect)
	{		
		case DROPEFFECT_COPY: oleCode = OLECURSOR_DROPCOPY; break;
		case DROPEFFECT_MOVE: oleCode = OLECURSOR_DROPMOVE; break;
		case DROPEFFECT_LINK: oleCode = OLECURSOR_DROPLINK; break;
		default: oleCode = OLECURSOR_NODROP; break;
	}
	return GetOleCursor(oleCode);
}

HCURSOR GetOleCursor(INT oleCursorId)
{
	if (oleCursorId < 0 || oleCursorId >= OLECURSOR_LASTENTRY) 
		return NULL;
			
	if(NULL == szOleCursors[oleCursorId])
	{
		HMODULE hModule = LoadLibrary(TEXT("ole32.dll"));
		if (NULL != hModule)
		{
			szOleCursors[oleCursorId] = (HCURSOR)LoadImage(hModule, MAKEINTRESOURCE(oleCursorId + 1), IMAGE_CURSOR, 
														0, 0, LR_DEFAULTSIZE | LR_SHARED);
			FreeLibrary(hModule);
		}
	}

	return szOleCursors[oleCursorId];
}

typedef enum
{
	FONCREATE_SOURCEMASK = 0x000000FF,
	FONCREATE_GDISTOCK = 0x00000001,
	FONCREATE_LOGFONT = 0x00000002,
	FONCREATE_WINAMP = 0x00000003,
} FONTCREATEFLAGS;

typedef struct 
{
	HFONT handle;
	UINT flags;
	union
	{
		LOGFONT logFont;
		INT stockObjectId;
		
	};
}PLUGINFONT;

static PLUGINFONT szPluginFonts[PLUGINFONT_LASTENTRY] =
{
	{ NULL, FONCREATE_GDISTOCK, DEFAULT_GUI_FONT},		/*PLUGINFONT_WINDOWS*/
	{ NULL, FONCREATE_LOGFONT, {						/*PLUGINFONT_HEADERTITLE*/
			9, /* lfHeight */
			0, /* lfWidth */
			0, /* lfEscapement */
			0, /* lfOrientation */
			FW_NORMAL, /* lfWeight */
			FALSE, /* lfItalic */
			FALSE, /* lfUnderline */
			FALSE, /* lfStrikeOut */
			DEFAULT_CHARSET, /* lfCharSet */
			OUT_DEFAULT_PRECIS, /* lfOutPrecision */
			CLIP_DEFAULT_PRECIS, /* lfClipPrecision */
			CLEARTYPE_QUALITY, /* lfQuality */
			DEFAULT_PITCH | FF_DONTCARE, /* lfPitchAndFamily */
			TEXT("Arial Unicode"),/*Trebuchet MS Bold*/ /* lfFaceName */
	}},
	{ NULL, FONCREATE_LOGFONT, {						/*PLUGINFONT_METERBARTEXT*/
			7, /* lfHeight */
			0, /* lfWidth */
			0, /* lfEscapement */
			0, /* lfOrientation */
			FW_DONTCARE, /* lfWeight */
			FALSE, /* lfItalic */
			FALSE, /* lfUnderline */
			FALSE, /* lfStrikeOut */
			DEFAULT_CHARSET, /* lfCharSet */
			OUT_DEFAULT_PRECIS, /* lfOutPrecision */
			CLIP_DEFAULT_PRECIS, /* lfClipPrecision */
			CLEARTYPE_QUALITY, /* lfQuality */
			DEFAULT_PITCH | FF_DONTCARE, /* lfPitchAndFamily */
			TEXT("Tahoma"), /* lfFaceName */
	}},
	
			
};

static BYTE GetHighestFontQuality(void)
{
	BOOL bSmoothing;
	if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &bSmoothing, 0) && bSmoothing)
	{
		OSVERSIONINFO vi = { sizeof(OSVERSIONINFO), };
		return (GetVersionEx(&vi) && (vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0))) ?
						CLEARTYPE_QUALITY : ANTIALIASED_QUALITY;
	}
	return DEFAULT_QUALITY;

}


HFONT GetPluginFont(INT pluginFontId)
{
	if (pluginFontId < 0 || pluginFontId >= PLUGINFONT_LASTENTRY) 
		return NULL;

	if(NULL == szPluginFonts[pluginFontId].handle)
	{
		PLUGINFONT *ppf = &szPluginFonts[pluginFontId];
		switch(FONCREATE_SOURCEMASK & ppf->flags)
		{
			case FONCREATE_GDISTOCK:
				ppf->handle = (HFONT)GetStockObject(ppf->stockObjectId);
				break;
			case FONCREATE_LOGFONT:
				{
					LOGFONT lf;
					CopyMemory(&lf, &ppf->logFont, sizeof(LOGFONT));

					HDC hdc = GetDCEx(HWND_DESKTOP, NULL, DCX_WINDOW | DCX_CACHE);
					if (NULL != hdc)
					{
						lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72),
						ReleaseDC(HWND_DESKTOP, hdc);
					}

					if (CLEARTYPE_QUALITY == lf.lfQuality) 
							lf.lfQuality = GetHighestFontQuality();

					ppf->handle = CreateFontIndirect(&lf);
				}
				break;

			case FONCREATE_WINAMP:
				{
					LOGFONT lf;
					CopyMemory(&lf, &ppf->logFont, sizeof(LOGFONT));
					
					lf.lfHeight = -(INT)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 3);
					lf.lfCharSet = (BYTE)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 2);
					
					LPCSTR faceNameA = (LPCSTR)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 1);
					if (NULL != faceNameA && '\0' != faceNameA)
					{
						INT count = MultiByteToWideChar(CP_ACP, 0, faceNameA, -1, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
						lf.lfFaceName[count] = TEXT('\0');
					}
					
					if (CLEARTYPE_QUALITY == lf.lfQuality) 
							lf.lfQuality = GetHighestFontQuality();

					ppf->handle = (HFONT)GetStockObject(ppf->stockObjectId);
				}
				break;
		}
	}
	return szPluginFonts[pluginFontId].handle;
}

void ResetPluginFontCache(INT pluginFontId)
{
	if (pluginFontId < 0 || pluginFontId >= PLUGINFONT_LASTENTRY) 
		return;

	if(NULL == szPluginFonts[pluginFontId].handle)
		return;
	PLUGINFONT *ppf = &szPluginFonts[pluginFontId];
	switch(FONCREATE_SOURCEMASK & ppf->flags)
	{
		case FONCREATE_LOGFONT:
		case FONCREATE_WINAMP:
			DeleteObject(ppf->handle);
			break;
	}

	ppf->handle = NULL;
}

void RelasePluginFonts(void)
{
	PLUGINFONT *ppf = szPluginFonts;
	for (INT i =0; i < ARRAYSIZE(szPluginFonts); i++, ppf++)
	{
		if (NULL != ppf->handle)
		{
			switch(FONCREATE_SOURCEMASK & ppf->flags)
			{
				case FONCREATE_LOGFONT:
				case FONCREATE_WINAMP:
					DeleteObject(ppf->handle);
					break;
			}
			ppf->handle = NULL;
		}

	}

}

COLORREF BlendColorsF(COLORREF rgbTop, COLORREF rgbBottom, float alphaF)
{	
	if (alphaF >= 1.0f) return rgbTop;
	if (alphaF <= 0.0f) return rgbBottom;

	float ir = 1.0f - alphaF;
	
	return RGB(((float)GetRValue(rgbTop)) * alphaF + ((float)GetRValue(rgbBottom)) * ir,
					((float)GetGValue(rgbTop)) * alphaF + ((float)GetGValue(rgbBottom)) * ir,
					((float)GetBValue(rgbTop)) * alphaF + ((float)GetBValue(rgbBottom)) * ir);
	
}

COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	WORD k = (((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}

COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2)
{
	INT g1 = (GetRValue(rgb1)*299 + GetGValue(rgb1)*587 + GetBValue(rgb1)*114);
	INT g2 = (GetRValue(rgb2)*299 + GetGValue(rgb2)*587 + GetBValue(rgb2)*114);
	return (g1 < g2) ? rgb1 : rgb2;
}

INT GetColorDistance(COLORREF rgb1, COLORREF rgb2)
{
	return (GetRValue(rgb1) - GetRValue(rgb2)) + 
			(GetGValue(rgb1) - GetGValue(rgb2)) +
			(GetBValue(rgb1) - GetBValue(rgb2));
}

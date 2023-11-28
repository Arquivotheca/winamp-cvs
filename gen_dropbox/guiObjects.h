#ifndef NULLSOFT_DROPBOX_PLUGIN_GUIOBJECTS_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_GUIOBJECTS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef enum
{
	OLECURSOR_NODROP = 0,
	OLECURSOR_DROPMOVE = 1,
	OLECURSOR_DROPCOPY = 2,
	OLECURSOR_DROPLINK = 3,
	OLECURSOR_DROPMOVE_INVERT = 4,
	OLECURSOR_DROPCOPY_INVERT = 5,
	OLECURSOR_DROPLINK_INVERT = 6,
	OLECURSOR_LASTENTRY,
} OLECURSORS;

HCURSOR GetOleCursor(INT oleCursorId);
HCURSOR GetOleCursorFromDropEffect(INT dropEffect);


typedef enum
{
	PLUGINFONT_WINDOWS = 0,
	PLUGINFONT_HEADERTITLE,
	PLUGINFONT_METERBARTEXT,
	PLUGINFONT_LASTENTRY,
} PLUGINFONTS;

HFONT GetPluginFont(INT pluginFontId);
void ResetPluginFontCache(INT pluginFontId);
void RelasePluginFonts(void);


COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha);
COLORREF BlendColorsF(COLORREF rgbTop, COLORREF rgbBottom, float alphaF);

COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2);
INT GetColorDistance(COLORREF rgb1, COLORREF rgb2);

#endif //NULLSOFT_DROPBOX_PLUGIN_GUIOBJECTS_HEADER
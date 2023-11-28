/******************************************************************************
Plush Version 1.1
text.c
Text code and data (8xX bitmapped)
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/

#include "plush.h"
#include <stdarg.h>
#include <tataki/canvas/bltcanvas.h>
#include <tataki/region/region.h>

extern "C"
{
static pl_uChar font_height = 16;
static HFONT font=0;
static BltCanvas *canvas=0, *shadow=0;
static HGDIOBJ oldFont = 0;
static HGDIOBJ oldShadowFont =0;

static void restoreCanvas(Canvas *canvas, api_canvas *dest, int w, int h, COLORREF color, int antialiased,int alpha)
{
	ARGB32 *buf = static_cast<ARGB32 *>(canvas->getBits());

	for (int y = 0; y < h; y++)
	{
		int linewidth = y * w;

		for (int x = 0; x < w; x++)
		{
			BYTE alpha = 255;

			ARGB32* prgb = &buf[linewidth + x];
			unsigned char *pixel = (unsigned char *)prgb;
			if (*prgb == 0)
			{
				// Do nothing
			}
			else
			{
				if (*prgb != 0xFFFFFF)
				{

					UINT value = pixel[0] + pixel[1] + pixel[2];
					value = value / 3;
					alpha = value;
				}

				pixel[3] = (BYTE)alpha;
				pixel[2] = ((GetRValue(color) * alpha) + 128) / 255;
				pixel[1] = ((GetGValue(color) * alpha) + 128) / 255;
				pixel[0] = ((GetBValue(color) * alpha) + 128) / 255;
			}
		}
	}

	canvas->blitAlpha(dest, 0, 0, alpha);
}

void plTextPutStr(pl_Cam *cam, pl_sInt x, pl_sInt y, const wchar_t *string, int alpha) {
	alpha*=16;
	if (alpha>255)
		alpha=255;

	HDC hdc=canvas->getHDC(), shadowHDC=shadow->getHDC();

	canvas->fillBits(RGB(0,0,0));
	shadow->fillBits(RGB(0,0,0));

	int sdc = SaveDC(cam->hdc);

	DCCanvas dest(cam->hdc);

	RECT clipRect={cam->ClipLeft, cam->ClipTop, cam->ClipRight, cam->ClipBottom};
	RegionI rgn(&clipRect);	
	dest.selectClipRgn(&rgn);
	
	RECT r = {x, y, cam->ScreenWidth-1, cam->ScreenHeight-1};
	DrawTextW(hdc, string, -1, &r, DT_LEFT|DT_NOPREFIX/*|DT_TABSTOP|(5<<8)*/);
	r.left++;
	r.top++;
	DrawTextW(shadowHDC, string, -1, &r, DT_LEFT|DT_NOPREFIX/*|DT_TABSTOP|(5<<8)*/);	
	restoreCanvas(shadow, &dest, cam->ScreenWidth, cam->ScreenHeight, RGB(0,0,0), 1, alpha); // drop shadow effect
	restoreCanvas(canvas, &dest, cam->ScreenWidth, cam->ScreenHeight, RGB(255,255,255), 1, alpha);

	RestoreDC(cam->hdc, sdc);
}

void plTextInit(int w, int h)
{
	if (!font)
		font= CreateFontW(font_height, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, L"Verdana");

		if (oldShadowFont)
			SelectObject(shadow->getHDC(), oldShadowFont);
		delete shadow;
		shadow = new BltCanvas(w,h);
		oldShadowFont = SelectObject(shadow->getHDC(), font);

		SetBkColor(shadow->getHDC(), RGB(0, 0, 0));
		SetTextColor(shadow->getHDC(), RGB(255, 255, 255));

		if (oldFont)
			SelectObject(canvas->getHDC(), oldFont);
		delete canvas;
		canvas = new BltCanvas(w,h);
		oldFont = SelectObject(canvas->getHDC(), font);
		SetBkColor(canvas->getHDC(), RGB(0, 0, 0));
		SetTextColor(canvas->getHDC(), RGB(255, 255, 255));
	}
}
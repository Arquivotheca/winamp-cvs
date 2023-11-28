#ifndef NULLOSFT_DROPBOX_PLUGIN_IMAGELOADER_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_IMAGELOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

void* ImageLoader_LoadPngData(HINSTANCE hInstance, LPCTSTR pszResourceName, INT *pWidthOut, INT *pHeightOut);
BOOL ImageLoader_FreeData(void *data);

HBITMAP ImageLoader_LoadPng(HINSTANCE hInstance, LPCTSTR pszResourceName, INT *pWidthOut, INT *pHeightOut);
HBITMAP ImageLoader_LoadPngEx(HINSTANCE hInstance, LPCTSTR pszResourceName, BITMAPINFOHEADER *pHeader, void **ppData);



BOOL BlendOnColorEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb);
BOOL BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);

BOOL ColorOverImageEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb);
BOOL ColorOverImage(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);
BOOL ColorizeImage(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg);
BOOL ColorizeImageEx(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha);

BOOL PremultiplyImage(BYTE *pPixels, LONG cx, LONG cy);

BOOL BoxBlur(BYTE *pixels, LONG cx, LONG cy, WORD bpp, INT blurRadius, BOOL ignoreAlpha, HANDLE hKill);
BOOL BoxBlurEx(BYTE *pixels, LONG left, LONG top, LONG right, LONG bottom, LONG cx, LONG cy, WORD bpp, INT blurRadius, BOOL ignoreAlpha, HANDLE hKill);

#endif //NULLOSFT_DROPBOX_PLUGIN_IMAGELOADER_HEADER
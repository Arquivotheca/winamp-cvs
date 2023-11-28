#ifndef _NULLSOFT_WINAMP_DATAVIEW_IMAGE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_IMAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


ARGB32 *
Image_Load(const void *data, 
		   size_t length, 
		   int *width, 
		   int *height, 
		   BOOL premultiply);

ARGB32* 
Image_LoadFromResource(const wchar_t *type, 
					   const wchar_t *name, 
					   int *width, 
					   int *height, 
					   BOOL premultiply);

void 
Image_Colorize(ARGB32 *data, 
			   size_t length, 
			   COLORREF frontColor);

#endif //_NULLSOFT_WINAMP_DATAVIEW_IMAGE_HEADER
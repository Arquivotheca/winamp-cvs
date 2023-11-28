#ifndef _NULLSOFT_WINAMP_DATAVIEW_NUMBER_FORMAT_EX_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_NUMBER_FORMAT_EX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

typedef struct NumberFormatEx
{
	NUMBERFMT format;
	wchar_t decimalSep[4];
	wchar_t thousandSep[4];
} NumberFormatEx;

HRESULT NumberFormatEx_Init(NumberFormatEx *self, LCID localeId);
HRESULT NumberFormatEx_Copy(const NumberFormatEx *self, NumberFormatEx *dest);


#endif //_NULLSOFT_WINAMP_DATAVIEW_NUMBER_FORMAT_EX_HEADER
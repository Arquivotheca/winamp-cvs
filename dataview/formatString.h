#ifndef _NULLSOFT_WINAMP_DATAVIEW_FORMAT_STRING_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FORMAT_STRING_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

HRESULT 
Format_Rating(wchar_t *buffer, size_t bufferSize, LCID localeId, 
			  int rating);

HRESULT 
Format_DateTime(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				__time64_t time);

HRESULT 
Format_TimeSpan(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				unsigned int totalSec);

HRESULT
Format_FileSize(wchar_t *buffer, size_t bufferSize, LCID localeId,
				unsigned __int64 value,  
				unsigned int numberLengthMax,	// max number of characters in number (XXX.XX = 6chars)
				unsigned int fractionLengthMax); // max number of characters in fraction (2 - max)

HRESULT
Format_Year(wchar_t *buffer, size_t bufferSize, LCID localeId,
			const SYSTEMTIME* time, 
			BOOL shortFormat);
HRESULT 
Format_YearRange(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				 const SYSTEMTIME* rangeBegin, 
				 const SYSTEMTIME* rangeEnd);

HRESULT 
Format_TrackLength(wchar_t *buffer, size_t bufferSize, LCID localeId,
				   unsigned int lengthMs);

HRESULT 
Format_TrackLength64(wchar_t *buffer, size_t bufferSize, LCID localeId,
					 unsigned __int64 lengthMs);

HRESULT 
Format_NumberInt(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				 int value,  
				 BOOL unsignedMode);

HRESULT 
Format_NumberInt64(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				   __int64 value, 
				   BOOL unsignedMode);

HRESULT 
Format_NumberFloat(wchar_t *buffer, size_t bufferSize, LCID localeId, 
				   float value, 
				   unsigned int fractonsCount);

HRESULT 
Format_TrackType(wchar_t *buffer, size_t bufferSize, LCID localeId,
				 int type);

HRESULT 
Format_Bitrate(wchar_t *buffer, size_t bufferSize, LCID localeId,
			   unsigned int bitrate);

#endif  // _NULLSOFT_WINAMP_DATAVIEW_FORMAT_STRING_HEADER
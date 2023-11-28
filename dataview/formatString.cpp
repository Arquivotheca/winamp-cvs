#include "main.h"
#include "./formatString.h"
#include "./numberFormatEx.h"
#include "./resource.h"

#include <time.h>
#include <strsafe.h>

#define KB_FACTOR	1024

HRESULT 
Format_Rating(wchar_t *buffer, size_t bufferSize, LCID localeId, int rating)
{
	const wchar_t *ratingString = L"*****";

	if (NULL == buffer)
		return E_POINTER;

	if (rating < 1)
	{
		buffer[0] = L'\0';
		return S_OK;
	}
	
	if (rating > 5)
		rating = 5;

	return StringCchCopyN(buffer, bufferSize, ratingString, rating);
}

HRESULT
Format_DateTime(wchar_t *buffer, size_t bufferSize, LCID localeId, __time64_t time)
{
	tm localTime;
	SYSTEMTIME systemTime;
	int length;
	wchar_t *cursor;
	size_t remaining;

	if (NULL == buffer)
		return E_POINTER;

	buffer[0] = L'\0';

	 if(0 == (__int64)time)
	 	 return S_OK;
	 
	if (0 != _localtime64_s(&localTime, &time))
		return E_FAIL;

	systemTime.wYear	= (unsigned short)(localTime.tm_year + 1900);
	systemTime.wMonth	= (unsigned short)(localTime.tm_mon + 1);
	systemTime.wDayOfWeek = (unsigned short)localTime.tm_wday;
	systemTime.wDay		= (unsigned short)localTime.tm_mday;
	systemTime.wHour	= (unsigned short)localTime.tm_hour;
	systemTime.wMinute	= (unsigned short)localTime.tm_min;
	systemTime.wSecond	= (unsigned short)localTime.tm_sec;
	systemTime.wMilliseconds = 0;

	cursor = buffer;
	remaining = bufferSize;

	length = GetDateFormatW(localeId, DATE_SHORTDATE, &systemTime, NULL, cursor, remaining);
	if (0 == length)
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
			return HRESULT_FROM_WIN32(errorCode);
	}

	if (length > 1)
	{
		length--;
		cursor += length;
		remaining -= length;
		if (remaining > 0)
		{
			*cursor++ = L' ';
			remaining--;
		}
	}

	length = GetTimeFormatW(localeId, 0, &systemTime, NULL, cursor, remaining);
	if (0 == length)
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
			return HRESULT_FROM_WIN32(errorCode);
	}
	
	return S_OK;
}

HRESULT
Format_TimeSpan(wchar_t *buffer, size_t bufferSize, LCID localeId, unsigned int totalSec)
{
	unsigned int secs, mins, hours, days;
	wchar_t formatTemplate[128];

	secs=totalSec%60;
	mins=(totalSec/60)%60;
	hours=(totalSec/3600)%24;
	days=(totalSec/86400);
  
	if(0 != days) 
	{
		wchar_t dayName[32];
		WASABI_API_LNGSTRINGW_BUF((1 == days) ? IDS_DAY : IDS_DAY_PLURAL, dayName, ARRAYSIZE(dayName));

		WASABI_API_LNGSTRINGW_BUF(IDS_FORMAT_TIMESPAN_LONG, formatTemplate, ARRAYSIZE(formatTemplate));
		
		return StringCchPrintf(buffer, bufferSize, formatTemplate, days, dayName, hours, mins, secs);
	} 
	
	if (0 == hours)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_FORMAT_TIMESPAN_SHORT, formatTemplate, ARRAYSIZE(formatTemplate));
		return StringCchPrintf(buffer, bufferSize, formatTemplate, mins, secs);
	}
		
	WASABI_API_LNGSTRINGW_BUF(IDS_FORMAT_TIMESPAN, formatTemplate, ARRAYSIZE(formatTemplate));
	return StringCchPrintf(buffer, bufferSize, formatTemplate, hours, mins, secs);
}

static inline unsigned int
FormatFileSize_GetNumberLength(size_t number)
{
	unsigned int length;
	size_t t;

	for(length = 0, t = number; t > 0; t = t /10, length++);

	return length;
}

static HRESULT
FormatFileSize_WriteToBuffer(wchar_t *buffer, size_t bufferSize, LCID localeId, 
							 const wchar_t *suffix, unsigned int number, unsigned int fraction, 
							 size_t numberLengthMax, BOOL writeZeroFraction)
{
	HRESULT hr;
	wchar_t buffer2[8], invariantNum[32];
	wchar_t *cursor;
	size_t remaining, length;
	NumberFormatEx formatEx;
	
	cursor = invariantNum;
	remaining = ARRAYSIZE(invariantNum);

	hr = Plugin_GetNumberFormat(localeId, &formatEx);
	if (FAILED(hr))
		return hr;

	hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, L"%u", number);
	if (FAILED(hr))
		return hr;

	if (FALSE != writeZeroFraction ||
		0 != fraction)
	{
		length = bufferSize - remaining;
		if (length > 0)
			length++;

		if (length > numberLengthMax)
		{
			if (0 != fraction)
			{
				length = FormatFileSize_GetNumberLength(fraction);

				if (length > numberLengthMax)
				{
					unsigned int t;
					t = (length - numberLengthMax) * 10;
					fraction = fraction/t + (fraction%t)/(t/2);
					if (0 == fraction)
						fraction = 1;
					length -= numberLengthMax;
				}
			}
			else
				length = 1;

			hr = StringCchCopyEx(cursor, remaining, L".", &cursor, &remaining, 0);
			if (FAILED(hr))
				return hr;

			hr = StringCchPrintf(buffer2, ARRAYSIZE(buffer2), L"%%0%uu", length);
			if (FAILED(hr))
				return hr;

			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, buffer2, fraction);
			if (FAILED(hr))
				return hr;
		}

		formatEx.format.NumDigits = length;
	}
	else
	{
		formatEx.format.NumDigits = 0;
	}

	if (0 == GetNumberFormat(localeId, 0, invariantNum, &formatEx.format, buffer, bufferSize))
		RETURN_HRESULT_FROM_LAST_ERROR();

	length = lstrlen(buffer);
	cursor = buffer + length;
	remaining = bufferSize - length;

	if (FALSE != IS_INTRESOURCE(suffix) && 
		NULL != suffix)
	{
		WASABI_API_LNGSTRINGW_BUF((int)(INT_PTR)suffix, buffer2, ARRAYSIZE(buffer2));
		suffix = buffer2;
	}
	
	if (FALSE == IS_STRING_EMPTY(suffix))
	{
		hr = StringCchCopyEx(cursor, remaining, L" ", &cursor, &remaining, 0);
		if (FAILED(hr))
			return hr;

		hr = StringCchCopyEx(cursor, remaining, suffix, &cursor, &remaining, 0);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT
Format_FileSize(wchar_t *buffer, size_t bufferSize, LCID localeId, unsigned __int64 value,
				unsigned int numberLengthMax, unsigned int fractionLengthMax)
{
	HRESULT hr;
	unsigned int number, fraction;
	BOOL writeZeroFraction;
	int suffixId;
	
	writeZeroFraction = FALSE;
	
	if (fractionLengthMax > 2)
		fractionLengthMax = 2;

	if (value < 1024)
	{
		suffixId = IDS_SIZE_BYTE;
		number = (unsigned int)value;
		fraction = 0;
		//writeZeroFraction = FALSE;
	}
	else if (value < 1024*1024) 
	{
		suffixId = IDS_SIZE_KB;
		number = (unsigned int)(value >> 10);
		fraction = (unsigned int)value;
	}
	else if (value < 1024*1024*1024)
	{	
		suffixId = IDS_SIZE_MB;
		number = (unsigned int)(value >> 20);
		fraction = (unsigned int)(value >> 10);
	}
	else if (value < 1024*1024*1024*1024ULL)
	{
		suffixId = IDS_SIZE_GB;
		number = (unsigned int)(value >> 30);
		fraction = (unsigned int)(value >> 20);
	}
	else
	{
		suffixId = IDS_SIZE_TB;
		number = (unsigned int)(value >> 40);
		fraction = (unsigned int)(value >> 30);
	}

	if (0 != fraction)
	{
		size_t numberLength, fractionLength;

		numberLength = FormatFileSize_GetNumberLength(number);
		if ((numberLength + 1) < numberLengthMax)
		{
			fractionLength = numberLengthMax - (numberLength + 1);
			if (fractionLength > fractionLengthMax)
				fractionLength = fractionLengthMax;

			if (fractionLength > 0)
			{
				fraction = (fraction & 1023) * (unsigned int)pow(10.0f, (int)fractionLength);
				fraction = (fraction >> 10) + (fraction & 1023) / 512;

				while(fraction > 0 && 0 == (fraction % 10))
					fraction = fraction/10;
			}
			else
			{
				if (fraction >= 512)
					number += 1;
				fraction = 0;
			}
		}
		else
		{
			fraction = 0;
		}
		
		
	}

	hr = FormatFileSize_WriteToBuffer(buffer, bufferSize, localeId, 
							MAKEINTRESOURCE(suffixId), number, fraction, 
							numberLengthMax, writeZeroFraction);

	return hr;
}
										
HRESULT
Format_Year(wchar_t *buffer, size_t bufferSize, LCID localeId,
			const SYSTEMTIME* time, BOOL shortFormat)
{
	if (NULL == time)
		return E_INVALIDARG;

	if (0 == GetDateFormat(localeId, 0, time, 
						   (FALSE == shortFormat) ? L"yyyy" : L"yy", 
						   buffer, bufferSize))
	{
		RETURN_HRESULT_FROM_LAST_ERROR();
	}

	return S_OK;
}

HRESULT
Format_YearRange(wchar_t  *buffer, size_t bufferSize, LCID localeId, 
				 const SYSTEMTIME* rangeBegin, const SYSTEMTIME* rangeEnd)
{	
	int result;

	if(NULL == rangeBegin) 
		return E_INVALIDARG;

	result = GetDateFormat(localeId, 0, rangeBegin, L"yyyy", buffer, bufferSize);
	if (0 == result)
		RETURN_HRESULT_FROM_LAST_ERROR();
	
	if(NULL != rangeEnd && rangeBegin->wYear != rangeEnd->wYear) 
	{
		HRESULT hr;
		size_t remaining;
		wchar_t *cursor;
		const wchar_t *filling, *dateFormat;

		remaining = bufferSize - result;
		cursor = buffer + result;

		if (rangeBegin->wYear/100 == rangeEnd->wYear/100)
		{
			filling = L" - '";
			dateFormat = L"yy";
		}
		else
		{
			filling = L" - ";
			dateFormat = L"yyyy";
		}
		
		hr = StringCchCopyEx(cursor, remaining, filling, &cursor, &remaining, 0);
		if (FAILED(hr))
			return hr;

		result = GetDateFormat(localeId, 0, rangeEnd, dateFormat, buffer, bufferSize);
		if (0 == result)
			RETURN_HRESULT_FROM_LAST_ERROR();
	}

	return S_OK;
}

HRESULT 
Format_TrackLength(wchar_t *buffer, size_t bufferSize, LCID localeId, unsigned int lengthMs)
{
	unsigned int lengthSec, lengthMin, lengthHour;
		
	lengthMs = lengthMs/1000;

	lengthSec = lengthMs%60;
	lengthMin = (lengthMs/60)%60;
	lengthHour = lengthMs/3600;

	if (0 == lengthHour)
	{
		return StringCchPrintf(buffer, bufferSize, L"%u:%02u", 
								lengthMin, lengthSec); 
	}
	
	return StringCchPrintf(buffer, bufferSize, L"%u:%02u:%02u", 
								lengthHour, lengthMin, lengthSec); 
}

HRESULT 
Format_TrackLength64(wchar_t *buffer, size_t bufferSize, LCID localeId, unsigned __int64 lengthMs)
{
	unsigned __int64 lengthSec, lengthMin, lengthHour;
	
	lengthMs = lengthMs/1000LL;

	lengthSec = lengthMs%60LL;
	lengthMin = (lengthMs/60LL)%60LL;
	lengthHour = lengthMs/3600LL;

	if (0 == lengthHour)
	{
		return StringCchPrintf(buffer, bufferSize, L"%02I64u:%02I64u", lengthMin, lengthSec); 
	}
	
	return StringCchPrintf(buffer, bufferSize, L"%I64u:%02I64u:%02I64u", lengthHour, lengthMin, lengthSec); 
}

HRESULT 
Format_NumberInt(wchar_t *buffer, size_t bufferSize, LCID localeId, int value, BOOL unsignedMode)
{
	HRESULT hr;
	_locale_t localeC;
	const wchar_t *formatString;
	int result;
	NumberFormatEx formatEx;
	wchar_t bufferInvariant[16];

	localeC = Plugin_GetCLocale();
	if (NULL == localeC)
		return E_FAIL;

	formatString = (FALSE == unsignedMode) ? L"%d" : L"%u";

	result  = _swprintf_s_l(bufferInvariant, ARRAYSIZE(bufferInvariant), formatString, localeC, value); 
	if (-1 == result)
		return E_FAIL;

	hr = Plugin_GetNumberFormat(localeId, &formatEx);
	if (FAILED(hr))
		return hr;

	formatEx.format.NumDigits = 0;

	result = GetNumberFormat(localeId, 0, bufferInvariant, &formatEx.format, buffer, bufferSize);
	if (0 == result)
	{
		DWORD errorCode;
		errorCode = GetLastError();
		if (ERROR_INVALID_FLAGS == errorCode ||
			ERROR_INVALID_PARAMETER == errorCode)
		{
			hr = StringCchCopy(buffer, bufferSize, bufferInvariant);
		}
		else
			hr = HRESULT_FROM_WIN32(errorCode);
	}

	return hr;
}

HRESULT 
Format_NumberInt64(wchar_t *buffer, size_t bufferSize, LCID localeId, __int64 value, BOOL unsignedMode)
{
	HRESULT hr;
	_locale_t localeC;
	const wchar_t *formatString;
	int result;
	NumberFormatEx formatEx;
	wchar_t bufferInvariant[16];

	localeC = Plugin_GetCLocale();
	if (NULL == localeC)
		return E_FAIL;

	formatString = (FALSE == unsignedMode) ? L"%I64d" : L"%I64u";

	result  = _swprintf_s_l(bufferInvariant, ARRAYSIZE(bufferInvariant), formatString, localeC, value); 
	if (-1 == result)
		return E_FAIL;

	hr = Plugin_GetNumberFormat(localeId, &formatEx);
	if (FAILED(hr))
		return hr;

	formatEx.format.NumDigits = 0;

	result = GetNumberFormat(localeId, 0, bufferInvariant, &formatEx.format, buffer, bufferSize);
	if (0 == result)
	{
		DWORD errorCode;
		errorCode = GetLastError();
		if (ERROR_INVALID_FLAGS == errorCode ||
			ERROR_INVALID_PARAMETER == errorCode)
		{
			hr = StringCchCopy(buffer, bufferSize, bufferInvariant);
		}
		else
			hr = HRESULT_FROM_WIN32(errorCode);
	}

	return hr;
}

HRESULT 
Format_NumberFloat(wchar_t *buffer, size_t bufferSize, LCID localeId, float value, unsigned int fractionsCount)
{
	HRESULT hr;
	_locale_t localeC;
	const wchar_t *formatString;
	int result;
	NumberFormatEx formatEx;
	wchar_t bufferInvariant[16];

	localeC = Plugin_GetCLocale();
	if (NULL == localeC)
		return E_FAIL;

	hr = Plugin_GetNumberFormat(localeId, &formatEx);
	if (FAILED(hr))
		return hr;

	if ((unsigned int)-1 != fractionsCount)
		formatEx.format.NumDigits = fractionsCount;

	formatString = L"%f";

	result  = _swprintf_s_l(bufferInvariant, ARRAYSIZE(bufferInvariant), formatString, localeC, value); 
	if (-1 == result)
		return E_FAIL;

	result = GetNumberFormat(localeId, 0, bufferInvariant, &formatEx.format, buffer, bufferSize);
	if (0 == result)
	{
		DWORD errorCode;
		errorCode = GetLastError();
		if (ERROR_INVALID_FLAGS == errorCode ||
			ERROR_INVALID_PARAMETER == errorCode)
		{
			hr = StringCchCopy(buffer, bufferSize, bufferInvariant);
		}
		else
			hr = HRESULT_FROM_WIN32(errorCode);
	}

	return hr;
}

HRESULT 
Format_TrackType(wchar_t *buffer, size_t bufferSize, LCID localeId, int type)
{
	int stringId;
	if (NULL == buffer)
		return E_POINTER;

	switch(type)
	{
		case 0: stringId = IDS_TRACK_AUDIO; break;
		case 1: stringId = IDS_TRACK_VIDEO; break;
		default: stringId = IDS_TRACK_UNKNOWN; break;
	}
	
	WASABI_API_LNGSTRINGW_BUF(stringId, buffer, bufferSize);
	return S_OK;
}

HRESULT 
Format_Bitrate(wchar_t *buffer, size_t bufferSize, LCID localeId, unsigned int bitrate)
{
	HRESULT hr;
	size_t remaining;
	wchar_t *cursor;

	if (0 == bitrate)
	{
		if (NULL == buffer)
			return E_POINTER;
		
		*buffer = L'\0';
		return S_OK;
	}

	hr = StringCchPrintfEx(buffer, bufferSize, &cursor, &remaining, 0, L"%u ", bitrate);
	if (FAILED(hr))
		return hr;
	
	if (NULL == WASABI_API_LNGSTRINGW_BUF(IDS_KBPS, cursor, remaining))
		return E_FAIL;
	
	return S_OK;
}
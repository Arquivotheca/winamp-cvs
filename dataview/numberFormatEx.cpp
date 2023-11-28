#include "main.h"
#include "./numberFormatEx.h"

HRESULT NumberFormatEx_Init(NumberFormatEx *self, LCID localeId)
{
	if (NULL == self)
		return E_INVALIDARG;

	unsigned long result, errorCode;
		wchar_t	grouping[16];
		
		errorCode = ERROR_SUCCESS;

		result = GetLocaleInfo(localeId, LOCALE_IDIGITS | LOCALE_RETURN_NUMBER, (wchar_t*)&self->format.NumDigits, 
								sizeof(self->format.NumDigits)/sizeof(wchar_t));

		if (0 == result)
			errorCode = GetLastError();

		result = GetLocaleInfo(localeId, LOCALE_ILZERO | LOCALE_RETURN_NUMBER, (wchar_t*)&self->format.LeadingZero, 
								sizeof(self->format.LeadingZero)/sizeof(wchar_t));
		if (0 == result)
			errorCode = GetLastError();

		result = GetLocaleInfo(localeId, LOCALE_INEGNUMBER | LOCALE_RETURN_NUMBER, (wchar_t*)&self->format.NegativeOrder, 
								sizeof(self->format.NegativeOrder)/sizeof(wchar_t));
		if (0 == result)
			errorCode = GetLastError();


		self->format.lpDecimalSep = self->decimalSep;
		result = GetLocaleInfo(localeId, LOCALE_SDECIMAL, self->decimalSep, ARRAYSIZE(self->decimalSep));
		if (0 == result)
		{
			errorCode = GetLastError();
			self->decimalSep[0] = L'\0';
		}
			
		self->format.lpThousandSep = self->thousandSep;
		result = GetLocaleInfo(localeId, LOCALE_STHOUSAND, self->thousandSep, ARRAYSIZE(self->thousandSep));
		if (0 == result)
		{
			errorCode = GetLastError();
			self->thousandSep[0] = L'\0';
		}
			
		result = GetLocaleInfo(localeId, LOCALE_SGROUPING, grouping, ARRAYSIZE(grouping));
		if (0 != result)
		{
			unsigned int iRead, iWrite;

			iRead = 0;
			iWrite = 0;
			result--;

			for (; iRead < result; iRead++)
			{
				if (L';' == grouping[iRead])
					continue;
				grouping[iWrite] = grouping[iRead];
				iWrite++;
			}

			grouping[iWrite] = L'\0';
			result = StrToIntEx(grouping, STIF_DEFAULT, (int*)&self->format.Grouping);
			if (0 != result)
			{
				if (iWrite > 0 && L'0' != grouping[iWrite - 1])
					self->format.Grouping = self->format.Grouping * 10;
			}
		}

		if (0 == result)
			errorCode = GetLastError();

		return HRESULT_FROM_WIN32(errorCode);
}

HRESULT NumberFormatEx_Copy(const NumberFormatEx *self, NumberFormatEx *dest)
{
	if (NULL == self || NULL == dest)
		return E_INVALIDARG;

	memcpy(dest, self, sizeof(NumberFormatEx));
	return S_OK;
}

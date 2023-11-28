#include "./fontHelper.h"
#include <strsafe.h>

static INT CALLBACK PickFont_EnumProc(const LOGFONT *logFont, const TEXTMETRIC *textMetric, DWORD fontType, LPARAM param)
{
	BOOL *foundOk = (BOOL*)param;
	if (NULL != foundOk)
		*foundOk = TRUE;
	
	return 0;
}

INT FontHelper_GetSysFontHeight()
{
	LOGFONT lf;
	if (!SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT), &lf, 0))
	{
    	HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (NULL == hf || sizeof(LOGFONT) != GetObject(hf, sizeof(LOGFONT), &lf))
			lf.lfHeight = -8;
	}
	return lf.lfHeight;
}

BOOL FontHelper_PickFontName(HDC hdc, LOGFONT *pLogFont, LPCTSTR *ppszFamilyNames, INT cchFamilyNames)
{
	BOOL foundOk;

	HDC hdcOwned = NULL;
	if (NULL == hdc)
	{
		hdcOwned = GetDCEx(NULL, NULL, DCX_CACHE | DCX_NORESETATTRS | DCX_WINDOW);
		hdc = hdcOwned;
	}

	for (INT i = 0; i < cchFamilyNames; i++)
	{
		foundOk = FALSE;

		if (NULL == ppszFamilyNames[i] ||
			FAILED(StringCchCopy(pLogFont->lfFaceName, ARRAYSIZE(pLogFont->lfFaceName), ppszFamilyNames[i])))
		{
			break;
		}

		EnumFontFamiliesEx(hdc, pLogFont, PickFont_EnumProc, (LPARAM)&foundOk, 0);
		
		if (FALSE != foundOk)
			break;
	}

	if (!foundOk)
		pLogFont->lfFaceName[0] = TEXT('\0');
	if (NULL != hdcOwned)
	{
		ReleaseDC(NULL, hdcOwned);
	}
	return foundOk;
}
#include "./main.h"
#include "./plugin.h"
#include "./formatData.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./fileInfoInterface.h"
#include "./itemTypeInterface.h"
#include "./supportedExtensions.h"
#include "./dropWindowInternal.h"
#include <shlwapi.h>
#include <strsafe.h>

#define NOCOLUMN		TEXT("none")

LPCTSTR FormatFileSize(ULONGLONG *pFileSize, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;

	size_t remaining = cchBufferMax;
	pszBuffer[0] = TEXT('\0');

	if (NULL == pFileSize)
		return pszBuffer;
	
	HRESULT hr = S_OK;
	DWORD part;
	if (*pFileSize < 1024*1024) 
	{
		hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, &remaining, STRSAFE_IGNORE_NULLS,
			TEXT("%u KB"), (DWORD)(*pFileSize >> 10) + ((((DWORD)(*pFileSize))&1023) ? 1: 0));
	}
	else if (*pFileSize < 1024*1024*1024)
	{
		part = ((((DWORD)(*pFileSize >> 10))&1023)*100) >> 10;
		if (part > 0) 
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, &remaining, STRSAFE_IGNORE_NULLS,
					TEXT("%u.%02u MB"), (DWORD)(*pFileSize >> 20), part);
		}
		else 
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, &remaining, STRSAFE_IGNORE_NULLS,
					TEXT("%u MB"), (DWORD)(*pFileSize >> 20));
		}
	}
	else
	{
		part = ((((DWORD)(*pFileSize >> 20))&1023)*100) >> 10;
		if (part > 0) 
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, &remaining, STRSAFE_IGNORE_NULLS,
					TEXT("%u.%02u GB"), (DWORD)(*pFileSize >> 30), part);
		}
		else 
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, &remaining, STRSAFE_IGNORE_NULLS,
				TEXT("%u GB"), (DWORD)(*pFileSize >> 30));
		}
	}

	return (SUCCEEDED(hr)) ? pszBuffer : NULL;
}



LPCTSTR FormatFileTime(FILETIME *pFileTime, LPTSTR pszBuffer, INT cchBufferMax)
{	
	if (NULL == pszBuffer) return NULL;

	pszBuffer[0] = TEXT('\0');
	if (0 == pFileTime->dwHighDateTime && 0 == pFileTime->dwLowDateTime) 
		return pszBuffer;
	
	SYSTEMTIME st;
	if (FileTimeToSystemTime(pFileTime, &st))
	{	
		INT len;
		LPTSTR  pszCursor = pszBuffer;
		len = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszCursor, cchBufferMax);
		if (0 == len) return NULL;
		cchBufferMax -= len;
		if (cchBufferMax > 0)
		{
 			pszCursor += len;
			*(pszCursor - 1) = TEXT(' ');
				
			INT len2;
			len2 = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszCursor, cchBufferMax);
			if (0 == len2) 
				return NULL;
			len += len2;
		}
		else 
			return NULL;
	}
	return pszBuffer;
}
LPCTSTR FormatFileTimeLocalize(FILETIME *pFileTime, LPTSTR pszBuffer, INT cchBufferMax)
{
	FILETIME ft;
	if (NULL == pszBuffer) return NULL;
	pszBuffer[0] = TEXT('\0');

	if (0 == FileTimeToLocalFileTime(pFileTime, &ft))
		return pszBuffer;
	return FormatFileTime(&ft, pszBuffer, cchBufferMax);
}
LPCTSTR FormatFileAttributes(DWORD fileAttributes, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	TCHAR szAttrib[32];
	INT len = 0;

	if(NULL != WASABI_API_LNGSTRINGW_BUF(IDS_FILE_ATTRIBUTES, szAttrib, ARRAYSIZE(szAttrib)))
	{
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_READONLY & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[0]; len++; }
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_HIDDEN & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[1]; len++; }
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_SYSTEM & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[2]; len++; }
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_ARCHIVE & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[3]; len++; }
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_COMPRESSED & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[4]; len++; }
		if (len < cchBufferMax && 0 != (FILE_ATTRIBUTE_ENCRYPTED & fileAttributes)) 
			{ pszBuffer[len] = szAttrib[5]; len++; }
	}
	if (len < cchBufferMax) pszBuffer[len] = TEXT('\0');	
	return pszBuffer;
}

LPCTSTR FormatItemType(UINT typeId, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;

	IItemType *type = PLUGIN_REGTYPES->FindById((BYTE)typeId);
	if (NULL == type || FAILED(type->GetDisplayName(pszBuffer, cchBufferMax)))
		pszBuffer[0] = TEXT('\0');
	return pszBuffer;
}

LPCTSTR FormatLength(INT length, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	
	if (length > 0)
	{
		HRESULT hr;
		if (length >= 60*60)
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, TEXT("%d:%02d:%02d"), length / (60 * 60), (length % (60 * 60)) / 60, length % 60);
		}
		else
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, TEXT("%2d:%02d"), (length % (60 * 60)) / 60, length % 60);
		}
		if (FAILED(hr)) pszBuffer[0] = TEXT('\0');
	}
	else
		pszBuffer[0] = TEXT('\0');
	
	return pszBuffer;
}

LPCTSTR FormatBitrate(INT bitrate, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	
	if (bitrate > 0)
	{
		HRESULT hr;
		hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, TEXT("%dkbps"), bitrate);
		if (FAILED(hr)) pszBuffer[0] = TEXT('\0');
	}
	else
		pszBuffer[0] = TEXT('\0');
	return pszBuffer;
}

LPCTSTR FormatPositiveInt(INT integerValue, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	
	if (integerValue > 0)
		StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, TEXT("%d"), integerValue);
	else 
		pszBuffer[0] = TEXT('\0');
	
	return pszBuffer;
}

LPCTSTR FormatIntSlashInt(DWORD packedValue, LPTSTR pszBuffer, INT cchBufferMax)
{	
	INT part1 = ((INT)(short)LOWORD(packedValue));
	if (NULL == pszBuffer) return NULL;
	
	if (part1 > 0)
	{
		HRESULT hr;
		INT part2 = ((INT)(short)HIWORD(packedValue));

		if (part2 > 0) 
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS,  TEXT("%d/%d"), part1, part2);
		}
		else
		{
			hr = StringCchPrintfEx(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS,  TEXT("%d"), part1);
		}
			
		if (FAILED(hr)) 
			pszBuffer[0] = TEXT('\0');
	}
	else 
		pszBuffer[0] = TEXT('\0');
	
	return pszBuffer;
}

LPCTSTR FormatExtensionFamily(LPCTSTR fileExtension, LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr;
	HANDLE supportedExt = GetDefaultSupportedExtensionsHandle();
	if (NULL != supportedExt)
	{
		LPCTSTR family;
		GetTypeByExtension(supportedExt, fileExtension, &family);
		hr = StringCchCopyEx(pszBuffer, cchBufferMax, family, NULL, NULL, STRSAFE_IGNORE_NULLS);
	}
	else hr = E_FAIL;

	if (FAILED(hr)) 
		pszBuffer[0] = TEXT('\0');
	return pszBuffer;
}

LPCTSTR FormatRect(const RECT *pRect, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	HRESULT hr = StringCchPrintf(pszBuffer, cchBufferMax, TEXT("{%d, %d, %d, %d}"), pRect->left, pRect->top, pRect->right, pRect->bottom);
	if (FAILED(hr)) 
		pszBuffer[0] = TEXT('\0');
	return pszBuffer;
}

LPCTSTR FormatPoint(POINT pt, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return NULL;
	HRESULT hr = StringCchPrintf(pszBuffer, cchBufferMax, TEXT("{%d, %d}"), pt.x, pt.y);
	if (FAILED(hr))
		pszBuffer[0] = TEXT('\0');
	return pszBuffer;
}

LPCTSTR FormatSize(SIZE sz, LPTSTR pszBuffer, INT cchBufferMax)
{	
	return FormatPoint(*((POINT*)&sz), pszBuffer, cchBufferMax);
}

LPCTSTR FormatTimeSpan(ULONGLONG seconds, LPTSTR pszBuffer, INT cchBufferMax)
{

	TCHAR szFormat[] =TEXT("year\0month\0week\0day\0hour\0minute\0second\0");
	LPCTSTR labels[6], cursor, limit;
	INT count;
	
	ZeroMemory(labels, sizeof(labels));
	limit = szFormat + ARRAYSIZE(szFormat);


	
	for(count = 0, cursor = szFormat; count < ARRAYSIZE(labels) && TEXT('\0') != *cursor;)
	{
		labels[count] = cursor;	
		while(TEXT('\0') != *cursor && cursor < limit) cursor++;
		if (cursor != labels[count])
		{			
			count++;
		}
		if (cursor == limit)
			break;
		cursor++;
	}
	
	HRESULT hr;
	INT level = 6;
	ULONGLONG p1, p2;
	p1 = seconds;
	
	do
	{	
		level--;
		p2 = p1;

		switch(level)
		{
			case 5: p1 = p2/60; break;
			case 4: p1 = p2/60; break;
			case 3: p1 = p2/24; break;
			case 2: p1 = p2/7; break;
			case 1: p1 = p2/7; break;
		}
		if (0 == p1) break;
		
	} while(level > 0 && p1);

	if (0 == p1)
		hr = StringCchPrintf(pszBuffer, cchBufferMax, TEXT("%d%s"), p2, labels[level]);
	else
		hr = StringCchPrintf(pszBuffer, cchBufferMax, TEXT("%d%s %d%s"), p1, labels[level], p2, labels[level + 1]);
	return (S_OK == hr) ? pszBuffer : NULL;
}
BOOL ParseRect(LPCTSTR pszStringIn, RECT *pRectOut, LPCTSTR *ppszCursor)
{
	INT vert = 0;
	
	while (TEXT('{') != *pszStringIn && TEXT('\0') != *pszStringIn) pszStringIn++;
	if (TEXT('{') != *pszStringIn)
	{
		if (NULL != ppszCursor)
			*ppszCursor = pszStringIn;
		return FALSE;
	}
	pszStringIn++;
	
	LPCTSTR pBlock = pszStringIn;
	
	for (; TEXT('\0') != *pszStringIn && vert < 4; pszStringIn++)
	{
		if (TEXT(',') == *pszStringIn || TEXT('}') == *pszStringIn)
		{
			while (TEXT(' ') == *pBlock && pBlock != pszStringIn) pBlock++;
			if (pBlock != pszStringIn)
			{					
				LONG v =  StrToInt(pBlock);
				switch(vert)
				{
					case 0: pRectOut->left = v; break;
					case 1: pRectOut->top = v; break;
					case 2: pRectOut->right = v; break;
					case 3: pRectOut->bottom = v; break;
				}
				vert++;
			}
			pBlock = (pszStringIn + 1);
		}
	}

	if (NULL != ppszCursor)
			*ppszCursor = pszStringIn;
	return (4 == vert);
}

BOOL ParsePoint(LPCTSTR pszStringIn, POINT *pPointOut, LPCTSTR *ppszCursor)
{
	INT vert = 0;
	
	while (TEXT('{') != *pszStringIn && TEXT('\0') != *pszStringIn) pszStringIn++;
	if (TEXT('{') != *pszStringIn)
	{
		if (NULL != ppszCursor)
			*ppszCursor = pszStringIn;
		return FALSE;
	}
	pszStringIn++;
	
	LPCTSTR pBlock = pszStringIn;
	
	for (; TEXT('\0') != *pszStringIn && vert < 4; pszStringIn++)
	{
		if (TEXT(',') == *pszStringIn || TEXT('}') == *pszStringIn)
		{
			while (TEXT(' ') == *pBlock && pBlock != pszStringIn) pBlock++;
			if (pBlock != pszStringIn)
			{					
				LONG v =  StrToInt(pBlock);
				switch(vert)
				{
					case 0: pPointOut->x = v; break;
					case 1: pPointOut->y = v; break;
				}
				vert++;
			}
			pBlock = (pszStringIn + 1);
		}
	}

	if (NULL != ppszCursor)
			*ppszCursor = pszStringIn;
	return (2 == vert);
}

LPTSTR FormatActiveColumns(LPTSTR pszBufferOut, size_t cchBufferMax, ACTIVECOLUMN *pColumns, INT columnsCount)
{
	if (NULL == pszBufferOut || cchBufferMax < 1) 
		return NULL;

	pszBufferOut[0] = TEXT('\0');
	if (!pColumns) 
		return pszBufferOut;

	HRESULT hr;
	LPTSTR pc = pszBufferOut;
	for(int i = 0; i < columnsCount; i++)
	{
		hr = StringCchPrintfEx(pc, cchBufferMax, &pc, &cchBufferMax, STRSAFE_IGNORE_NULLS,
				TEXT("%c(%s, %d)"), ((0 == i) ? TEXT(' ') : TEXT(',')), szRegisteredColumns[pColumns[i].id].pszName, pColumns[i].width);
		if (S_OK != hr) return NULL;
	}
	return pszBufferOut;
}

LPTSTR FormatLangResource(INT_PTR resourceId, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 1) 
		return NULL;

	HRESULT hr = StringCchPrintf(pszBuffer, cchBufferMax, TEXT("langDll@%d"), resourceId);
	
	if (FAILED(hr))
		pszBuffer[0] = TEXT('\0');

	return (SUCCEEDED(hr)) ? pszBuffer : NULL;
}

static HRESULT FormatVirtKey(WORD key, BOOL virtualKey, BOOL firstKey, LPTSTR pszDest, size_t cchDest, LPTSTR *ppszDestEnd, size_t *pcchRemaining)
{
	TCHAR szKey[64];
	if (virtualKey)
	{
		UINT sCode = MapVirtualKey(key, 0);
		switch(key)
		{
			case VK_INSERT:
			case VK_DELETE:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:  
			case VK_PRIOR: 
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				sCode |= 0x100;
		}

		if (0 == GetKeyNameText(sCode << 16, szKey, ARRAYSIZE(szKey)))
			return E_UNEXPECTED;
	}
	else
	{
		if (FAILED(StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%C"), (TCHAR)key)))
			return E_UNEXPECTED;
	}

	HRESULT hr = (FALSE == firstKey) ? 
					StringCchCopyEx(pszDest, cchDest, TEXT("+"), &pszDest, &cchDest, 0) :
					S_OK;

	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(pszDest, cchDest, szKey, &pszDest, &cchDest, 0);

	if (NULL != ppszDestEnd)
		*ppszDestEnd = pszDest;
	if (NULL != pcchRemaining)
		*pcchRemaining = cchDest;

	return hr;
}
LPTSTR FormatShortcut(BYTE fVirt, WORD key, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 1) 
		return NULL;

	HRESULT hr = S_OK;
		
	LPTSTR p = pszBuffer;
	size_t remaining = cchBufferMax;

	if (SUCCEEDED(hr) && 0 != (FCONTROL & fVirt))
		hr = FormatVirtKey(VK_CONTROL, TRUE, (p == pszBuffer), p, remaining, &p, &remaining);
	
	if (SUCCEEDED(hr) && 0 != (FALT & fVirt))
		hr = FormatVirtKey(VK_MENU, TRUE, (p == pszBuffer), p, remaining, &p, &remaining);

	if (SUCCEEDED(hr) && 0 != (FSHIFT & fVirt))
		hr = FormatVirtKey(VK_SHIFT, TRUE, (p == pszBuffer), p, remaining, &p, &remaining);

	if (SUCCEEDED(hr))
		hr = FormatVirtKey(key, (0 != (FVIRTKEY & fVirt)), (p == pszBuffer), p, remaining, &p, &remaining);
		

	if (FAILED(hr))
		pszBuffer[0] = TEXT('\0');
	return (SUCCEEDED(hr)) ? pszBuffer : NULL;

}
LPTSTR FormatColumnName(INT columnId, LPTSTR pszBuffer, INT cchBufferMax)
{
	LPCTSTR columnName = (columnId < 0 || columnId >= COLUMN_LAST) ?
			NOCOLUMN : szRegisteredColumns[columnId].pszName;

	HRESULT hr = StringCchCopy(pszBuffer, cchBufferMax, columnName);
	if (FAILED(hr)) 
		pszBuffer[0] = TEXT('\0');
	
	return pszBuffer;
}



BOOL ParseLangResource(LPCTSTR pszInput, INT_PTR *resourceId)
{
	if (NULL == pszInput || NULL == resourceId)
		return FALSE;
	
	const TCHAR szTemplate[] = TEXT("langDll@");
	INT cchTemplate = ARRAYSIZE(szTemplate) - 1;
	INT cchInput = lstrlen(pszInput);
	if (cchInput <= cchTemplate)
		return FALSE;

	if (CSTR_EQUAL != CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
			pszInput, cchTemplate, szTemplate, cchTemplate))
	{
		return FALSE;
	}

	return StrToIntEx(pszInput + cchTemplate, STIF_DEFAULT, (INT*)resourceId);
}


typedef struct __VKEYNAME
{
	WORD code;
	TCHAR name[32];
} VKEYNAME;

#define reg_vkey(__key) { ##__key, { TEXT('\0'), } }
static VKEYNAME szLongNameKeys[] = 
{
	reg_vkey(VK_BACK),
	reg_vkey(VK_TAB),
	reg_vkey(VK_CLEAR),
	reg_vkey(VK_RETURN),
	reg_vkey(VK_PAUSE),
	reg_vkey(VK_CAPITAL),
	reg_vkey(VK_ESCAPE),
	reg_vkey(VK_SPACE),
	reg_vkey(VK_PRIOR),
	reg_vkey(VK_NEXT),
	reg_vkey(VK_END),
	reg_vkey(VK_HOME),
	reg_vkey(VK_LEFT),
	reg_vkey(VK_UP),
	reg_vkey(VK_RIGHT),
	reg_vkey(VK_DOWN),
	reg_vkey(VK_SELECT),
	reg_vkey(VK_INSERT),
	reg_vkey(VK_DELETE),
	reg_vkey(VK_HELP ),
	reg_vkey(VK_F1),
	reg_vkey(VK_F2),
	reg_vkey(VK_F3),
	reg_vkey(VK_F4),
	reg_vkey(VK_F5),
	reg_vkey(VK_F6),
	reg_vkey(VK_F7),
	reg_vkey(VK_F8),
	reg_vkey(VK_F9),
	reg_vkey(VK_F10),
	reg_vkey(VK_F11),
	reg_vkey(VK_F12),
	reg_vkey(VK_F13),
	reg_vkey(VK_F14),
	reg_vkey(VK_F15),
	reg_vkey(VK_F16),
	reg_vkey(VK_F17),
	reg_vkey(VK_F18),
	reg_vkey(VK_F19),
	reg_vkey(VK_F20),
	reg_vkey(VK_F21),
	reg_vkey(VK_F22),
	reg_vkey(VK_F23),
	reg_vkey(VK_F24),
	reg_vkey(VK_NUMLOCK),
	reg_vkey(VK_SCROLL),
};

typedef struct __SHORTCUTPARSER
{
	TCHAR szShift[48];
	TCHAR szControl[48];
	TCHAR szAlt[48];
	BYTE fVirt;
	WORD key;
} SHORTCUTPARSER;

static UINT CALLBACK ParseShortcut_KeyParser(LPCTSTR pszKeyword, INT cchKeyword, LPVOID user)
{
	SHORTCUTPARSER *parserData = (SHORTCUTPARSER*)user;
	
	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
							pszKeyword, cchKeyword, parserData->szShift, -1))
	{
		parserData->fVirt |= FSHIFT;
		return KWPARSER_FOUND | KWPARSER_CONTINUE;
	}

	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
							pszKeyword, cchKeyword, parserData->szControl, -1))
	{
		parserData->fVirt |= FCONTROL;
		return KWPARSER_FOUND | KWPARSER_CONTINUE;
	}

	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
							pszKeyword, cchKeyword, parserData->szAlt, -1))
	{
		parserData->fVirt |= FALT;
		return KWPARSER_FOUND | KWPARSER_CONTINUE;
	}

	if (0 != parserData->key)
		return KWPARSER_ABORT;
	
	if (1 == cchKeyword)
	{
		parserData->key = LOBYTE(VkKeyScan(pszKeyword[0]));
		if (-1 == LOBYTE(parserData->key))
		{
			parserData->key = 0;
			return KWPARSER_ABORT;
		}
		return KWPARSER_FOUND | KWPARSER_CONTINUE;
	}

	for (INT i = 0; i < ARRAYSIZE(szLongNameKeys); i++)
	{
		if ((TEXT('\0') != *szLongNameKeys[i].name ||
			0 != GetKeyNameText(MapVirtualKey(szLongNameKeys[i].code, 0) << 16, 
						szLongNameKeys[i].name, ARRAYSIZE(szLongNameKeys[i].name))) && 
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
						pszKeyword, cchKeyword, szLongNameKeys[i].name, -1))
		{
			parserData->key = szLongNameKeys[i].code;
			return KWPARSER_FOUND | KWPARSER_CONTINUE;
		}
	}

	parserData->key = 0;
	return KWPARSER_ABORT;
}

BOOL ParseShortcut(LPCTSTR pszInput, ACCEL *accelOut)
{
	if (NULL == accelOut)
		return FALSE;
	
	SHORTCUTPARSER parserData;
	if (0 == GetKeyNameText(MapVirtualKey(VK_SHIFT, 0) << 16, parserData.szShift, ARRAYSIZE(parserData.szShift)) ||
		0 == GetKeyNameText(MapVirtualKey(VK_CONTROL, 0) << 16, parserData.szControl, ARRAYSIZE(parserData.szControl)) ||
		0 == GetKeyNameText(MapVirtualKey(VK_MENU, 0) << 16, parserData.szAlt, ARRAYSIZE(parserData.szAlt)))
		return FALSE;

	parserData.key = 0;
	parserData.fVirt = 0;

	ParseKeywords(pszInput, -1, TEXT("+-"), TRUE, ParseShortcut_KeyParser, &parserData);
	
	accelOut->key = parserData.key;
	accelOut->fVirt = (0 != parserData.key) ? (parserData.fVirt | FVIRTKEY) : 0;
		
	return (0 != parserData.key);
}

INT ParseActiveColumns(ACTIVECOLUMN *pColumns, INT nMax, LPCTSTR pszString)
{
	INT count = 0;
	const LISTCOLUMN *prc = NULL;
	for (LPCTSTR pc = pszString, pBlock = NULL; TEXT('\0') != *pc; pc++)
	{
		if (TEXT('(') == *pc) pBlock = (pc + 1);
		else if (TEXT(')') == *pc)
		{
			if (pBlock && pBlock != pc)
			{
				if (count == nMax) return count;
				ZeroMemory(&pColumns[count], sizeof(ACTIVECOLUMN));
				while (TEXT(' ') == *pBlock && pBlock != pc) pBlock++;
				if (pBlock != pc)
				{
					prc = NULL;
					
					INT cchBlock = 0;
					while (TEXT(',') != pBlock[cchBlock] && TEXT(' ') != pBlock[cchBlock] && (pBlock + cchBlock) != pc) cchBlock++;
					for(int i = 0; i < ARRAYSIZE(szRegisteredColumns) && NULL == prc; i++)
					{
						if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
										szRegisteredColumns[i].pszName, -1, pBlock, cchBlock))
						{
							prc = &szRegisteredColumns[i];
						}
					}

					if (NULL != prc)
					{
						pColumns[count].order = count;
						pColumns[count].id = prc->id;
						pColumns[count].width = prc->widthMin;

						while (TEXT(',') != *pBlock && pBlock != pc) pBlock++;
						if (pBlock != pc) 
						{	
							while ((TEXT(',') == *pBlock || TEXT(' ') == *pBlock) && pBlock != pc) pBlock++;
							if (pBlock != pc)
								pColumns[count].width = StrToInt(pBlock);
							
							
						}
						if (pColumns[count].width < prc->widthMin) pColumns[count].width = prc->widthMin;
						else if (pColumns[count].width > prc->widthMax) pColumns[count].width = prc->widthMax;
						count++;
					}
				}
			}
			pBlock = NULL;
		}
	}
	return count;
}

INT ParseColumnName(LPCTSTR pszColumnName)
{
	if (NULL == pszColumnName || TEXT('\0') == pszColumnName)
		return COLUMN_INVALID;
	
	while (TEXT(' ') == *pszColumnName && TEXT('\0') != pszColumnName) pszColumnName++;
	INT cchLen = lstrlen(pszColumnName);
	if (cchLen < 1)
		return COLUMN_INVALID;


	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
						NOCOLUMN, -1, pszColumnName, cchLen))
		return COLUMN_INVALID;

	for(int i = 0; i < ARRAYSIZE(szRegisteredColumns); i++)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
						szRegisteredColumns[i].pszName, -1, pszColumnName, cchLen))
			return szRegisteredColumns[i].id;
	}

	return COLUMN_INVALID;
}

static BOOL IsCharSeparator(TCHAR ch, LPCTSTR separators)
{
	if (TEXT('\0') == ch) return TRUE;
	if (NULL == separators) return FALSE; 
	for (LPCTSTR s = separators; TEXT('\0') != *s; s++)
	{
		if (*s == ch)
			return TRUE;
	}
	return FALSE;
}

INT ParseKeywords(LPCTSTR input, INT cchInput, LPCTSTR separators, BOOL eatSpace, KWPARSERPROC callback, void *user)
{
	if (NULL == input)
		return 0;

	if (cchInput < 0)
		cchInput = lstrlen(input);
	
	if (cchInput <= 0)
		return 0;

	LPCTSTR end  = (input + cchInput);

	if(eatSpace) 
		while(input < end && TEXT(' ') == *input) input++;

	if (TEXT('\0') == *input)
		return 0;

	INT found = 0;
	
	for (;;)
	{
		LPCTSTR pBlock = input;
		while(input <= end && !IsCharSeparator(*input, separators)) input++;
		LPCTSTR last = (input - 1);
		if (eatSpace)
			while(last >= pBlock && TEXT(' ') == *last) last--;
		
		if (last >= pBlock)
		{
			UINT code = callback(pBlock, (INT)(INT_PTR)(last - pBlock) + 1, user);
			if (KWPARSER_FOUND & code) found++;
			if (KWPARSER_ABORT == (0x01 & code)) return found;
		}

		if (TEXT('\0') == *input || input >= end) return found;
		input++;
		if(eatSpace) 
			while(input < end && TEXT(' ') == *input) input++;
	}

	return found;
}


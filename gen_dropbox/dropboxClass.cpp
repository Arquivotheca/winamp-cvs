#include "main.h"
#include "plugin.h"
#include "./dropboxClass.h"

#include "./formatData.h"
#include "./wasabiApi.h"

#include <shlwapi.h>
#include <strsafe.h>

#define CLASS_FILENAME_PREFIX	TEXT("dropboxClass_")
#define CLASS_WINDOWSECTION		TEXT("WindowClass")


static LPCTSTR szStyleText[] = 
{
	TEXT("OneInstance"),
	TEXT("SkinWindow"),
	TEXT("EnableBlur"),
	TEXT("WinampGroup"),
	TEXT("RememberProfile"),
	TEXT("DoNotSave"),
	TEXT("RegisterPlaylist"),
	TEXT("ShowHeader"),
};

static HRESULT DropboxClass_FormatFileName(const UUID &classUid, LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = Plugin_GetDropboxPath(pszBuffer, cchBufferMax);
	if (FAILED(hr)) return hr;

	Plugin_EnsurePathExist(pszBuffer);
	
	INT cchPath = lstrlen(pszBuffer);
	
	RPC_WSTR pszClass;
	if (RPC_S_OK != UuidToString((UUID*)&classUid, &pszClass))
		return E_FAIL;
 
	hr = StringCchPrintf(pszBuffer + cchPath, cchBufferMax - cchPath, 
			TEXT("\\") CLASS_FILENAME_PREFIX TEXT("{%s}.ini"), pszClass);
	
	RpcStringFree(&pszClass);
	return hr;
}

static HRESULT DropboxClass_FormatStyle(UINT style, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	*pszBuffer = TEXT('\0');
	
	HRESULT hr;
	LPTSTR p = pszBuffer;
	size_t remaining = cchBufferMax;

	for (int i = 0; 0 != style; i++, style >>= 1)
	{	
		if (0 != (0x01 & style) && NULL != szStyleText[i] && TEXT('\0') != *szStyleText[i])
		{		
			if (p != pszBuffer)
			{
				hr = StringCchCopyEx(p, remaining, TEXT(" | "), &p, &remaining, 0);
				if (FAILED(hr)) return hr;
			}

			hr = StringCchCopyEx(p, remaining, szStyleText[i], &p, &remaining, 0);
			if (FAILED(hr)) return hr;
		}
	}
	return S_OK;
}

static UINT CALLBACK DropboxClass_StyleParser(LPCTSTR pszKeyword, INT cchKeyword, LPVOID user)
{
	UINT foundStyles = *(UINT*)user;
	UINT s = 0x01;
	for (INT i = 0; i < ARRAYSIZE(szStyleText); i++, s <<= 1)
	{
		if (0 == (foundStyles & s) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
								pszKeyword, cchKeyword, szStyleText[i], -1))
		{
			(*(UINT*)user) |= s;
			return KWPARSER_FOUND | KWPARSER_CONTINUE;
		}
	}
	return KWPARSER_CONTINUE;
}
static UINT DropboxClass_ParseStyle(LPCTSTR pszBuffer)
{
	UINT style = 0;
	ParseKeywords(pszBuffer, -1, TEXT("|"), TRUE, DropboxClass_StyleParser, &style);
	return style;
}

void DropboxClass_FreeString(LPTSTR  pszString)
{
	if (NULL != pszString && !IS_INTRESOURCE(pszString))
		LocalFree(pszString);
}

HRESULT DropboxClass_Save(const DROPBOXCLASSINFO *pdbc)
{
	if (NULL == pdbc)
		return E_POINTER;

	TCHAR filePath[MAX_PATH * 2];
	HRESULT hr;
	hr = DropboxClass_FormatFileName(pdbc->classUid, filePath, ARRAYSIZE(filePath));
	if (FAILED(hr))
		return hr;

	RPC_WSTR pszClass;
	if (RPC_S_OK != UuidToString((UUID*)&pdbc->classUid, &pszClass))
		return E_UNEXPECTED;

	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("classUid"), (LPCWSTR)pszClass, filePath);
	RpcStringFree(&pszClass);

	TCHAR szBuffer[2048];
	LPCTSTR pszValue;

	pszValue = pdbc->pszTitle;
	if (NULL != pszValue && IS_INTRESOURCE(pszValue))
		pszValue = FormatLangResource((INT_PTR)pszValue, szBuffer,ARRAYSIZE(szBuffer));
	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("title"), pszValue, filePath);
	
	RPC_WSTR pszProfile;
	if (RPC_S_OK == UuidToString((UUID*)&pdbc->profileUid, &pszProfile))
	{
		WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("profileUid"), (LPCWSTR)pszProfile, filePath);
		RpcStringFree(&pszProfile);
	}
	
	POINT pt = { pdbc->x, pdbc->y };
	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("windowPos"), 
		FormatPoint(pt, szBuffer, ARRAYSIZE(szBuffer)), filePath);
		
	UINT s = pdbc->style;
	hr = DropboxClass_FormatStyle(pdbc->style, szBuffer, ARRAYSIZE(szBuffer));
	pszValue = (SUCCEEDED(hr)) ? szBuffer : NULL;
	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("style"), pszValue, filePath);
		
	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("shortcut"), 
		FormatShortcut(pdbc->shortcut.fVirt, pdbc->shortcut.key, szBuffer, ARRAYSIZE(szBuffer)), filePath);
	
	return S_OK;
}

HRESULT DropboxClass_Load(const UUID &classUid, DROPBOXCLASSINFO *pdbc)
{
	if (NULL == pdbc)
		return E_POINTER;

	ZeroMemory(pdbc, sizeof(DROPBOXCLASSINFO));
	
	TCHAR filePath[MAX_PATH * 2];
	HRESULT hr;
	hr = DropboxClass_FormatFileName(classUid, filePath, ARRAYSIZE(filePath));
	if (FAILED(hr))
		return hr;

	TCHAR szBuffer[2048];
	INT_PTR resourceId;

	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("classUid"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	if (RPC_S_OK != UuidFromString((RPC_WSTR)szBuffer, &pdbc->classUid))
		return E_NOINTERFACE;
	 
	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("title"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	pdbc->pszTitle = (ParseLangResource(szBuffer, &resourceId)) ? MAKEINTRESOURCE(resourceId) : StrDup(szBuffer);
	

	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("profileUid"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	if (RPC_S_OK != UuidFromString((RPC_WSTR)szBuffer, &pdbc->profileUid))
		pdbc->profileUid = GUID_NULL;
		

	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("windowPos"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	{
		POINT pt;
		if (ParsePoint(szBuffer, &pt, NULL))
		{
			pdbc->x = pt.x;
			pdbc->y = pt.y;
		}
	}

	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("style"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	pdbc->style = DropboxClass_ParseStyle(szBuffer);

	GetPrivateProfileString(CLASS_WINDOWSECTION, TEXT("shortcut"), NULL, szBuffer, ARRAYSIZE(szBuffer), filePath);
	ACCEL accel;
	if (ParseShortcut(szBuffer,  &accel))
	{
		pdbc->shortcut.fVirt = accel.fVirt;
		pdbc->shortcut.key= accel.key;
	}

	return S_OK;
}

HRESULT DropboxClass_SaveProfile(const UUID &classUid, const UUID &profileUid)
{
	HRESULT hr;
	TCHAR filePath[MAX_PATH * 2];
	hr = DropboxClass_FormatFileName(classUid, filePath, ARRAYSIZE(filePath));
	if (FAILED(hr))
		return hr;

	hr = E_FAIL;
	RPC_WSTR pszProfile;
	if (RPC_S_OK == UuidToString((UUID*)&profileUid, &pszProfile))
	{
		WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("profileUid"), (LPCWSTR)pszProfile, filePath);
		RpcStringFree(&pszProfile);
		hr = S_OK;
	}
	return hr;
}
HRESULT DropboxClass_SavePosition(const UUID &classUid, POINT pt)
{
	HRESULT hr;
	TCHAR filePath[MAX_PATH * 2];
	hr = DropboxClass_FormatFileName(classUid, filePath, ARRAYSIZE(filePath));
	if (FAILED(hr))
		return hr;

	TCHAR szBuffer[128];
	WritePrivateProfileString(CLASS_WINDOWSECTION, TEXT("windowPos"), 
						FormatPoint(pt, szBuffer, ARRAYSIZE(szBuffer)), filePath);

	return hr;
}
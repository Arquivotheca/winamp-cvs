#include "main.h"
#include "./common.h"
#include "./wasabi.h"

#include "../winamp/wa_ipc.h"

#include <strsafe.h>

LPWSTR Plugin_MallocString(size_t cchLen)
{
	return (LPWSTR)malloc(sizeof(WCHAR) * cchLen);
}

void Plugin_FreeString(LPWSTR pszString)
{
	if (NULL != pszString)
	{
		free(pszString);
	}
}

LPWSTR Plugin_ReAllocString(LPWSTR pszString, size_t cchLen)
{
	return (LPWSTR)realloc(pszString, sizeof(WCHAR) * cchLen);
}

LPWSTR Plugin_CopyString(LPCWSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenW(pszSource) + 1;
		
	LPWSTR copy = Plugin_MallocString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(WCHAR) * cchSource);
	}
	return copy;
}

LPSTR Plugin_MallocAnsiString(size_t cchLen)
{
	return (LPSTR)malloc(sizeof(CHAR) * cchLen);
}

LPSTR Plugin_CopyAnsiString(LPCSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenA(pszSource) + 1;
		
	LPSTR copy = Plugin_MallocAnsiString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(CHAR) * cchSource);
	}
	return copy;

}
void Plugin_FreeAnsiString(LPSTR pszString)
{
	Plugin_FreeString((LPWSTR)pszString);
}

LPSTR Plugin_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	INT cchBuffer = WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, NULL, 0, lpDefaultChar, lpUsedDefaultChar);
	if (0 == cchBuffer) return NULL;
	
	LPSTR buffer = Plugin_MallocAnsiString(cchBuffer);
	if (NULL == buffer) return NULL; 
		
	if (0 == WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, buffer, cchBuffer, lpDefaultChar, lpUsedDefaultChar))
	{
		Plugin_FreeAnsiString(buffer);
		return NULL;
	}
	return buffer;
}

LPWSTR Plugin_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte)
{
	if (NULL == lpMultiByteStr) return NULL;
	INT cchBuffer = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, NULL, 0);
	if (NULL == cchBuffer) return NULL;
	
	if (cbMultiByte > 0) cchBuffer++;
	
	LPWSTR buffer = Plugin_MallocString(cchBuffer);
	if (NULL == buffer) return NULL;

	if (0 == MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, buffer, cchBuffer))
	{
		Plugin_FreeString(buffer);
		return NULL;
	}

	if (cbMultiByte > 0)
	{
		buffer[cchBuffer - 1] = L'\0';
	}
	return buffer;
}


LPWSTR Plugin_DuplicateResString(LPCWSTR pszResource)
{
	return (IS_INTRESOURCE(pszResource)) ? 
			(LPWSTR)pszResource : 
			Plugin_CopyString(pszResource);
}

void Plugin_FreeResString(LPWSTR pszResource)
{
	if (!IS_INTRESOURCE(pszResource))
		Plugin_FreeString(pszResource);
}

HRESULT Plugin_CopyResString(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszString)
{
	if (NULL == pszBuffer)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	if (NULL == pszString)
	{
		pszBuffer[0] = L'\0';
	}
	else if (IS_INTRESOURCE(pszString))
	{
		if (NULL == WASABI_API_LNG)
			hr = E_FAIL;
		else
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszString, pszBuffer, cchBufferMax);
	}
	else
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, pszString);
	}
	return hr;
}

void Plugin_SafeRelease(IUnknown *pUnk)
{
	if (NULL != pUnk)
		pUnk->Release();
}

HRESULT Plugin_MakeResourcePath(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszType, LPCWSTR pszName, UINT flags)
{
	HINSTANCE hInstance = WASABI_API_LNG_HINST;
	if (NULL == hInstance || NULL == FindResource(hInstance, pszName, pszType))
		hInstance = Plugin_GetInstance();

	if (NULL == OMUTILITY)
		return E_UNEXPECTED;

	return OMUTILITY->MakeResourcePath(pszBuffer, cchBufferMax, hInstance, pszType, pszName, flags);
}

HWND Plugin_GetDialogOwner(void)
{
	HWND hOwner= Plugin_GetLibrary();
	if (NULL == hOwner || FALSE == IsWindowVisible(hOwner) ||
		FALSE == IsWindowEnabled(hOwner))
	{
		hOwner = Plugin_GetWinamp();
		if (NULL != hOwner)
		{
			HWND hDlgParent = (HWND)SENDWAIPC(hOwner, IPC_GETDIALOGBOXPARENT, 0L);
			if (NULL != hDlgParent) 
				hOwner = hDlgParent;
		}
	}
	return hOwner;
}

HRESULT Plugin_BuildActionUrl(LPWSTR *ppStringOut, LPCWSTR pszAction, UINT *pServiceUid, size_t cchServiceUid)
{
	if (NULL == ppStringOut) 
		return E_POINTER;
	
	*ppStringOut = NULL;

	if (NULL == pszAction || L'\0' == *pszAction || 
		NULL == pServiceUid || 0 == cchServiceUid)
	{
		return E_INVALIDARG;
	}
	
	const WCHAR szPrefix[] = L"http://services.winamp.com/svc/action?action=%s\0";
	const WCHAR szService[] = L"&svc_id=%u\0";
	const WCHAR szClient[] = L"&unique_id=%s\0";

	size_t cchBuffer = ARRAYSIZE(szPrefix) + ARRAYSIZE(szService) + ARRAYSIZE(szClient);
	cchBuffer += lstrlen(pszAction);
	cchBuffer += (cchServiceUid * 11);
	cchBuffer += 32; // unique id
	
	LPWSTR buffer = Plugin_MallocString(cchBuffer);
	if (NULL == buffer)
		return E_OUTOFMEMORY;

	HRESULT hr;
	LPWSTR cursor = buffer;
	size_t remaining = cchBuffer;


	hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
							szPrefix, pszAction);
	
	for (size_t i = 0; i < cchServiceUid && SUCCEEDED(hr); i++)
	{
		hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
							((0 == i) ? szService : L",%u"), pServiceUid[i]);
	}

	if (SUCCEEDED(hr))
	{
		WCHAR szTemp[128];
		hr = OMBROWSERMNGR->GetClientId(szTemp, ARRAYSIZE(szTemp));
		if (SUCCEEDED(hr))
		{
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 	szClient, szTemp);
		}
	}
	
	if (FAILED(hr))
	{
		Plugin_FreeString(buffer);
		hr = E_FAIL;
	}
	else
	{
		*ppStringOut = buffer;
	}
	return hr;
}

INT Plugin_ParseKeywords(LPCWSTR input, INT cchInput, WCHAR separator, BOOL eatSpace, KWPARSERPROC callback, ULONG_PTR user)
{
	if (NULL == input)
		return 0;

	if (cchInput < 0)
		cchInput = lstrlen(input);
	
	if (cchInput <= 0)
		return 0;

	LPCWSTR end  = (input + cchInput);

	if(eatSpace) 
		while(input < end && L' ' == *input) input++;

	if (L'\0' == *input)
		return 0;

	INT found = 0;
	
	for (;;)
	{
		LPCWSTR pBlock = input;
		while(input <= end && separator != *input) input++;
		LPCWSTR last = (input - 1);
		if (eatSpace)
			while(last >= pBlock && L' ' == *last) last--;
		
		if (last >= pBlock)
		{
			UINT code = callback(pBlock, (INT)(INT_PTR)(last - pBlock) + 1, user);
			if (KWPARSER_FOUND & code) found++;
			if (KWPARSER_ABORT == (0x01 & code)) 
				return found;
		}

		if (input >= end || L'\0' == *input) 
			return found;

		input++;
		if(eatSpace) 
			while(input < end && L' ' == *input) input++;
	}

	return found;
}

INT Plugin_MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	HWND hHost;
	
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL != hLibrary && FALSE != IsWindowVisible(hLibrary))
	{
		hHost = (HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0);
		if (NULL == hHost || FALSE == IsWindowVisible(hHost)) 
			hHost = hLibrary;
	}
	else
		hHost = Plugin_GetDialogOwner();
		
	WCHAR szText[2048], szCaption[128];
	
	if(IS_INTRESOURCE(lpText) && NULL != lpText)
	{
		lpText = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)lpText, szText, ARRAYSIZE(szText));
	}

	if(IS_INTRESOURCE(lpCaption) && NULL != lpCaption)
	{
		lpCaption = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)lpCaption, szCaption, ARRAYSIZE(szCaption));
	}

	return MessageBox(hHost, lpText, lpCaption, uType);
}
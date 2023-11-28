#include "./explorePath.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

typedef HRESULT (STDAPICALLTYPE *SHOPENFOLDERANDSELECTITEMS)(LPITEMIDLIST /*pidlFolder*/, UINT /*cidl*/,
																LPITEMIDLIST* /*apidl*/, DWORD /*dwFlags*/);

typedef struct __EXPLORETHREADPARAM
{
	HWND hHost;
	TCHAR szFilePath[MAX_PATH];
} EXPLORETHREADPARAM;

static void* LoadShellFunction(LPCSTR pszFunctionName)
{
	void *fn = NULL;

	UINT prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HMODULE hModule = LoadLibrary(TEXT("shell32.dll"));
	SetErrorMode(prevErrorMode);

	if (hModule) 
	{
		fn = GetProcAddress(hModule, pszFunctionName);
		FreeLibrary(hModule);
	}
	return fn;
}

static BOOL ExploreViaShell(HWND hwnd, LPCTSTR pszFilePath)
{

	LPITEMIDLIST pidl;
	IShellFolder *psf;
	SHOPENFOLDERANDSELECTITEMS shellFn;
	HRESULT hr;

	shellFn = (SHOPENFOLDERANDSELECTITEMS)LoadShellFunction("SHOpenFolderAndSelectItems");
	if (NULL == shellFn)
		return FALSE;
	
	hr = CoInitialize(NULL);
	if (FAILED(hr))
		return FALSE;

	hr = SHGetDesktopFolder(&psf);
	if (SUCCEEDED(hr))
	{		
		psf->ParseDisplayName(NULL, NULL, (LPTSTR)pszFilePath, NULL, &pidl, NULL);
		if(SUCCEEDED(hr))
		{
			hr = shellFn(pidl, 0, NULL, 0);

			IMalloc *pMalloc = NULL;
			if (SUCCEEDED(SHGetMalloc(&pMalloc)))
			{
				pMalloc->Free(pidl);
				pMalloc->Release();
			}
		}
		psf->Release();
	}

	CoUninitialize();

	return SUCCEEDED(hr);
}

static HRESULT ExploreFile_Worker(HWND hwnd, LPCTSTR pszFilePath)
{
	HRESULT hr;
	TCHAR szCommand[MAX_PATH * 2];

	BOOL bNewWindow = (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));

	if (!bNewWindow)
	{
		if (ExploreViaShell(hwnd, pszFilePath))
			return S_OK;
	}

	hr = StringCchPrintf(szCommand, ARRAYSIZE(szCommand), TEXT("%s/select,\"%s\""), 
							((bNewWindow) ? TEXT("/n, ") : TEXT("")), pszFilePath);
	if (SUCCEEDED(hr))
	{
		HINSTANCE hInst = ShellExecute(hwnd, TEXT("open"), TEXT("explorer.exe"), szCommand, NULL, SW_SHOWNORMAL);
		hr = ((INT_PTR)hInst > 32) ? S_OK : E_FAIL;
	}
	return hr;
}

static DWORD CALLBACK ExploreFile_ThreadProc(LPVOID param)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

	EXPLORETHREADPARAM *exploreParam = (EXPLORETHREADPARAM*)param;
	if (NULL != exploreParam)
	{
		ExploreFile_Worker(exploreParam->hHost, exploreParam->szFilePath);
		free(exploreParam);
	}
	return 0;
}

HRESULT ExploreFile(HWND hwnd, LPCTSTR pszFilePath, BOOL bAsync)
{
	if (NULL == pszFilePath || TEXT('\0') == *pszFilePath)
		return E_INVALIDARG;

	HRESULT hr;
	
	if (bAsync)
	{		
		EXPLORETHREADPARAM *threadParam = (EXPLORETHREADPARAM*)malloc(sizeof(EXPLORETHREADPARAM));
		if (NULL == threadParam)
			hr = E_OUTOFMEMORY;
		else
		{
			threadParam->hHost = hwnd;
			hr = StringCchCopy(threadParam->szFilePath, ARRAYSIZE(threadParam->szFilePath), pszFilePath);
			if (S_OK != hr)
				hr = E_UNEXPECTED;
			else
			{

				DWORD threadId;
				HANDLE hThread = CreateThread(NULL, 0, ExploreFile_ThreadProc, threadParam, 0, &threadId);
				if (NULL != hThread)
					CloseHandle(hThread);
				hr = (NULL != hThread) ? S_OK : E_FAIL;
			}

			if (FAILED(hr) && NULL != threadParam)
				free(threadParam);
		}
	}
	else 
		hr = ExploreFile_Worker(hwnd, pszFilePath);

	return hr;
}
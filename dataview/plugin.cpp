#include "main.h"
#include "./plugin.h"
#include "./api_dataview.h"
#include "./performanceTimerDebugOutput.h"
#include "./numberFormatEx.h"
#include <strsafe.h>

typedef struct NumberFormatCache
{
	LCID localeId;
	NumberFormatEx formatEx;
} NumberFormatCache;

static HINSTANCE pluginInstance = NULL;
static ATOM	windowAtom = 0;
static Component component;
static Theme *activeTheme = NULL;
static LCID userLocaleId = 0;
static StringArticle *stringArticle = NULL;
static _locale_t localeC = NULL;
static NumberFormatCache numberFormatCache = {0, };
static PerformanceTimerDebugOutput *performanceTimerDebugOutput = NULL;

HINSTANCE 
Plugin_GetInstance()
{
	return pluginInstance;
}

HWND 
Plugin_GetWinampWindow(void)
{
	static HWND winampWindow = NULL;

	if (NULL == winampWindow)
	{
		if (NULL == WASABI_API_WINAMP)
			return NULL;

		winampWindow = WASABI_API_WINAMP->GetMainWindow();
	}

	return winampWindow;
}

ATOM
Plugin_GetWindowAtom()
{
	if (0 == windowAtom)
	{
		windowAtom = GlobalAddAtomW(L"NULLSOFT_WINAMP_DATAVIEW_WINDOW_DATA");
	}
	return windowAtom;
}

void *
Plugin_GetWindowData(HWND hwnd)
{
	return GetPropW(hwnd, (const wchar_t*)MAKEINTATOM(Plugin_GetWindowAtom()));
}

BOOL
Plugin_SetWindowData(HWND hwnd, void *data)
{
	return SetPropW(hwnd, (const wchar_t*)MAKEINTATOM(Plugin_GetWindowAtom()), data);
}

void *
Plugin_RemoveWindowData(HWND hwnd)
{
	return RemovePropW(hwnd, (const wchar_t*)MAKEINTATOM(Plugin_GetWindowAtom()));
}

HRESULT
Plugin_GetDataView(api_dataview **instance)
{
	return component.GetDataView(instance);
}

HRESULT
Plugin_GetColumnManager(ifc_viewcolumnmanager **instance)
{
	HRESULT hr;
	api_dataview *dataView;

	hr = Plugin_GetDataView(&dataView);
	if (FAILED(hr))
		return hr;

	hr = dataView->GetColumnManager(instance);

	dataView->Release();

	return hr;
}

HRESULT
Plugin_GetGroupManager(ifc_groupmanager **instance)
{
	HRESULT hr;
	api_dataview *dataView;

	hr = Plugin_GetDataView(&dataView);
	if (FAILED(hr))
		return hr;

	hr = dataView->GetGroupManager(instance);

	dataView->Release();

	return hr;
}

HRESULT
Plugin_GetTheme(Theme **_theme)
{
	if (NULL == _theme)
		return E_POINTER;

	if (NULL == activeTheme)
	{
		HRESULT hr;
		hr = Theme::CreateInstance(&activeTheme);
		if(FAILED(hr))
			return hr;
	}

	*_theme = activeTheme;
	activeTheme->AddRef();

	return S_OK;
}

HRESULT
Plugin_SetControlTheme(HWND hwnd, unsigned int type, unsigned int style)
{
	HRESULT hr;
	Theme *theme;

	hr = Plugin_GetTheme(&theme);
	if (FAILED(hr))
		return hr;

	hr = theme->SkinControl(hwnd, type, style);

	theme->Release();

	return hr;
}

HRESULT
Plugin_EnsurePathExist(const wchar_t *path)
{
	unsigned long errorCode;
	unsigned int errorMode;
	
	errorCode = ERROR_SUCCESS;
	errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	if (0 == CreateDirectoryW(path, NULL))
	{
		errorCode = GetLastError();
		if (ERROR_PATH_NOT_FOUND == errorCode)
		{
			const wchar_t *block, *cursor;
			wchar_t buffer[MAX_PATH];
			
			block = path;
			cursor = PathFindNextComponentW(block);

			errorCode = (cursor == block || 
						S_OK != StringCchCopyNW(buffer, ARRAYSIZE(buffer), block, (cursor - block))) ?
						ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			block = cursor;
			
			while(ERROR_SUCCESS == errorCode && 
				  NULL != (cursor = PathFindNextComponentW(block)))
			{
				if (cursor == block || 
					S_OK != StringCchCatNW(buffer, ARRAYSIZE(buffer), block, (cursor - block)))
				{
					errorCode = ERROR_INVALID_NAME;
				}

				if (ERROR_SUCCESS == errorCode && 
					FALSE == CreateDirectoryW(buffer, NULL))
				{
					errorCode = GetLastError();
					if (ERROR_ALREADY_EXISTS == errorCode) 
						errorCode = ERROR_SUCCESS;
				}
				block = cursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == errorCode) 
			errorCode = ERROR_SUCCESS;
	}

	SetErrorMode(errorMode);
	SetLastError(errorCode);
	return HRESULT_FROM_WIN32(errorCode);
}

typedef LCID (WINAPI *FnLocalNameToLCID)(const wchar_t *name, unsigned long flags);

LCID
Plugin_GetUserLocaleId()
{	
	if (0 == userLocaleId)
	{
		if (NULL != WASABI_API_LNG)
		{
			FnLocalNameToLCID fnLocalNameToLCID;
			HMODULE libraryModule = NULL;

			libraryModule = LoadLibrary(L"kernel32.dll");
			if (NULL != libraryModule)
				fnLocalNameToLCID = (FnLocalNameToLCID)GetProcAddress(libraryModule, "LocaleNameToLCID");
			else 
				fnLocalNameToLCID = NULL;

			if (NULL == fnLocalNameToLCID)
			{
				if (NULL != libraryModule)
					FreeLibrary(libraryModule);

				libraryModule = LoadLibrary(L"nlsdl.dll");
				if (NULL != libraryModule)
					fnLocalNameToLCID = (FnLocalNameToLCID)GetProcAddress(libraryModule, "DownlevelLocaleNameToLCID");
			}

			if (NULL != fnLocalNameToLCID)
			{
				const wchar_t *localeName;
				
				localeName = WASABI_API_LNG->GetLanguageIdentifier(LANG_IDENT_STR);
				userLocaleId = fnLocalNameToLCID(localeName, 0);
			}

			if (NULL != libraryModule)
				FreeLibrary(libraryModule);
		}

		if (0 == userLocaleId)
			userLocaleId = LOCALE_USER_DEFAULT;
	}

	return userLocaleId;
}

_locale_t
Plugin_GetCLocale()
{
	if (NULL == localeC)
	{
		localeC = _create_locale(LC_ALL, "C");
	}

	return localeC;
}

HRESULT
Plugin_GetNumberFormat(LCID localeId, NumberFormatEx *instance)
{
	HRESULT hr;

	if (localeId != numberFormatCache.localeId)
	{
		hr = NumberFormatEx_Init(&numberFormatCache.formatEx, localeId);
		if (FAILED(hr))
			return hr;

		numberFormatCache.localeId = localeId;
	}

	hr = NumberFormatEx_Copy(&numberFormatCache.formatEx, instance);
	return hr;
}

StringArticle *
Plugin_GetStringArticle()
{
	if (NULL == stringArticle)
	{
		const wchar_t * defaultArticles[] = 
		{
			L"the",
			L"a",
		};

		stringArticle = new (std::nothrow) StringArticle();
		if (NULL == stringArticle)
			return NULL;

		stringArticle->Register(defaultArticles, ARRAYSIZE(defaultArticles));
	}

	return stringArticle;
}

PerformanceTimerDebugOutput *
Plugin_GetPerformanceTimerDebugOutput()
{
	if (NULL == performanceTimerDebugOutput)
	{
		if (FAILED(PerformanceTimerDebugOutput::CreateInstance(&performanceTimerDebugOutput)))
			return NULL;
	}

	return performanceTimerDebugOutput;
}

static void
Plugin_Uninitialize()
{
	if (0 != windowAtom)
	{
		GlobalDeleteAtom(windowAtom);
		windowAtom = 0;
	}

	SafeRelease(activeTheme);
	SafeDelete(stringArticle);
	SafeRelease(performanceTimerDebugOutput);

	if(NULL != localeC)
		_free_locale(localeC);
}

extern "C" __declspec(dllexport) api_wa5component *
GetWinamp5SystemComponent()
{
	return &component;
}

BOOL APIENTRY 
DllMain(HANDLE hModule, DWORD  uReason, void *reserved)
{
    switch(uReason) 
	{
		case DLL_PROCESS_ATTACH:
			pluginInstance = (HINSTANCE)hModule;
			break;
		case DLL_PROCESS_DETACH:
			Plugin_Uninitialize();
			break;
    }
    return TRUE;
}

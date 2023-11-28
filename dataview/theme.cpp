#include "main.h"
#include "./theme.h"

#include <strsafe.h>

#define ENSURE_FUNCTION_LOADED(__function) {\
	HRESULT hr;\
	if (NULL == (__function)) 	{\
		hr = LoadLibraryModule();\
		if (SUCCEEDED(hr) && NULL == (__function)) hr = E_UNEXPECTED;\
		if (FAILED(hr)) return hr; }}

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : E_FAIL)


Theme::Theme()
	: ref(1), libraryModule(NULL), libraryLoadResult(S_FALSE), colorInitRequired(TRUE), 
	textFont(NULL), mlSkinWindowEx(NULL), mlGetSkinColor(NULL), mlResetSkinColor(NULL),
	mlTrackSkinnedPopupMenuEx(NULL), mlIsSkinnedPopupEnabled(NULL), mlInitSkinnedPopupHook(NULL),
	mlRemoveSkinnedPopupHook(NULL), mlRatingDraw(NULL), mlRatingHitTest(NULL), mlRatingCalcMinRect(NULL)
{
}

Theme::~Theme()
{
	if (NULL != textFont)
		DeleteObject(textFont);


	if (NULL != libraryModule)
		FreeLibrary(libraryModule);
}

HRESULT Theme::CreateInstance(Theme **instance)
{
	if (NULL == instance) 
		return E_POINTER;
	
	*instance = NULL;
	
	*instance = new (std::nothrow) Theme();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t Theme::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Theme::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

COLORREF Theme::GetColor(unsigned int colorIndex)
{
	if (colorIndex >= WADLG_NUM_COLORS)
		return RGB(255, 0, 255);
	
	if (FALSE != colorInitRequired)
		InitializeColorData();
		
	return WADlg_getColor(colorIndex);
}

COLORREF Theme::GetColorEx(UINT uObject, UINT uPart, UINT uState)
{
	COLORREF color;
	ENSURE_FUNCTION_LOADED(mlGetSkinColor);
	if (FALSE == mlGetSkinColor(uObject, uPart, uState, &color))
		return RGB(255, 0, 255);

	return color;
}

HFONT Theme::GetTextFont()
{
	if (NULL == textFont)
		textFont = CreateThemeFont();

	return textFont;
}

HRESULT Theme::SkinControl(HWND hwnd, UINT type, UINT style)
{
	BOOL result;
	ENSURE_FUNCTION_LOADED(mlSkinWindowEx);
	
	result = mlSkinWindowEx(hwnd, type, style);
	return BOOL2HRESULT(result);
}

HRESULT Theme::Rating_Draw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle)
{
	ENSURE_FUNCTION_LOADED(mlRatingDraw);
	BOOL result = mlRatingDraw(hdc, maxValue, value, trackingVal, NULL, 0, prc, fStyle);
	return BOOL2HRESULT(result);
}

HRESULT Theme::Rating_HitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, long *result)
{
	if (NULL == result) 
		return E_POINTER;

	ENSURE_FUNCTION_LOADED(mlRatingHitTest);
	*result = mlRatingHitTest(pt, maxValue, NULL, prc, fStyle);

	return S_OK;
}

HRESULT Theme::Rating_CalcMinRect(INT maxValue, RECT *prc)
{
	ENSURE_FUNCTION_LOADED(mlRatingCalcMinRect);
	BOOL result = mlRatingCalcMinRect(maxValue, NULL, prc);
	return BOOL2HRESULT(result);
}

BOOL Theme::PopupMenu_Track(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm)
{
	if (S_OK == PopupMenu_IsEnabled())
	{			
		BOOL result;
		HMLIMGLST hmlil = NULL;
		INT width = 0;
		UINT skinStyle = SMS_USESKINFONT;
		MENUCUSTOMIZEPROC customProc = NULL;
		ULONG_PTR customParam = 0;
		
		HRESULT hr = MlTrackSkinnedPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm, 
			hmlil, width, skinStyle, customProc, customParam, &result);
		
		if (SUCCEEDED(hr))
			return result;
	}
	
	return TrackPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm);
}

HRESULT Theme::PopupMenu_IsEnabled(void)
{
	ENSURE_FUNCTION_LOADED(mlIsSkinnedPopupEnabled);
	return (FALSE != mlIsSkinnedPopupEnabled()) ? S_OK : S_FALSE;
}

HANDLE Theme::PopupMenu_InitHook(HWND hwnd)
{
	if (S_OK == PopupMenu_IsEnabled())
	{			
		HMLIMGLST hmlil = NULL;
		INT width = 0;
		UINT skinStyle = SMS_USESKINFONT;
		MENUCUSTOMIZEPROC customProc = NULL;
		ULONG_PTR customParam = 0;
	
		HANDLE hook;
		HRESULT hr = MlInitSkinnedPopupHook(hwnd, hmlil, width, skinStyle, customProc, customParam, &hook);
		if (FAILED(hr))
			hook = NULL;

		return hook;
	}
	
	return NULL;
}

HRESULT Theme::PopupMenu_RemoveHook(HANDLE popupHook)
{
	ENSURE_FUNCTION_LOADED(mlRemoveSkinnedPopupHook);
	mlRemoveSkinnedPopupHook(popupHook);
	return S_OK;
}

HRESULT Theme::MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut)
{
	if (NULL != resultOut)
		*resultOut = 0;

	ENSURE_FUNCTION_LOADED(mlTrackSkinnedPopupMenuEx);
	INT result = mlTrackSkinnedPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam);

	if (NULL != resultOut)
		*resultOut = result;

	return S_OK;
}

HRESULT Theme::MlInitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, HANDLE *hookOut)
{
	if (NULL == hookOut) return E_POINTER;
	*hookOut = NULL;
	ENSURE_FUNCTION_LOADED(mlInitSkinnedPopupHook);

	*hookOut = mlInitSkinnedPopupHook(hwnd, hmlil, width, skinStyle, customProc, customParam);
	if (NULL == *hookOut) 
		return E_FAIL;

	return S_OK;
}

HFONT Theme::CreateThemeFont()
{
	HWND winampWindow;

	winampWindow = Plugin_GetWinampWindow();
	if (NULL == winampWindow)
		return NULL;

	LOGFONT lf = 
	{
		0, /* lfHeight */
		0, /* lfWidth */
		0, /* lfEscapement */
		0, /* lfOrientation */
		FW_NORMAL, /* lfWeight */
		FALSE, /* lfItalic */
		FALSE, /* lfUnderline */
		FALSE, /* lfStrikeOut */
		DEFAULT_CHARSET, /* lfCharSet */
		OUT_DEFAULT_PRECIS, /* lfOutPrecision */
		CLIP_DEFAULT_PRECIS, /* lfClipPrecision */
		DEFAULT_QUALITY, /* lfQuality */
		DEFAULT_PITCH | FF_DONTCARE, /* lfPitchAndFamily */
		L"", /* lfFaceName */
	};

						
	lf.lfHeight = -(INT)SENDWAIPC(winampWindow, IPC_GET_GENSKINBITMAP, 3);
	lf.lfCharSet = (BYTE)SENDWAIPC(winampWindow, IPC_GET_GENSKINBITMAP, 2);
					
	LPCSTR faceNameAnsi = (LPCSTR)SENDWAIPC(winampWindow,IPC_GET_GENSKINBITMAP, 1);
	if (NULL != faceNameAnsi && '\0' != faceNameAnsi)
	{
		INT count = MultiByteToWideChar(CP_ACP, 0, faceNameAnsi, -1, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
		if (count > 0) count--;
		lf.lfFaceName[count] = L'\0';
	}

	return CreateFontIndirect(&lf);	
}

HRESULT Theme::LoadLibraryModule()
{
	HWND winampWindow;
	wchar_t szPath[MAX_PATH*2];

	if (S_FALSE != libraryLoadResult)
		return libraryLoadResult;

	szPath[0] = L'\0';

	winampWindow = Plugin_GetWinampWindow();
	if (NULL == winampWindow)
		return E_FAIL;

	const char* pathAnsi = (const char*)SENDWAIPC(winampWindow, IPC_GETPLUGINDIRECTORY, 0);
	int cchLen = 0;
	
	if (FALSE == IS_STRING_EMPTY(pathAnsi))
	{
		cchLen = MultiByteToWideChar(CP_ACP, 0, pathAnsi, -1, szPath, ARRAYSIZE(szPath));
		if (0 != cchLen)
		{
			cchLen--;
			if (cchLen > 1 && L'\\' == szPath[cchLen - 1] && L'\\' == szPath[cchLen - 2])
				cchLen -= 2;
			else if (cchLen > 0 && L'/' == szPath[cchLen - 1])
				cchLen -= 1;
		}
		szPath[cchLen] = L'\0';
	}
	
	libraryLoadResult = StringCchCopy(szPath + cchLen, ARRAYSIZE(szPath) - cchLen, ((0 != cchLen) ? L"\\gen_ml.dll" : L"gen_ml.dll"));
	
	if (SUCCEEDED(libraryLoadResult))
	{
		libraryModule = LoadLibrary(szPath);
		if (NULL == libraryModule)
			libraryLoadResult = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(libraryLoadResult))
	{
		mlSkinWindowEx = (MLSKINWINDOWEX)GetProcAddress(libraryModule, "MlSkinWindowEx");
		mlTrackSkinnedPopupMenuEx = (MLTRACKSKINNEDPOPUPMENUEX)GetProcAddress(libraryModule, "MlTrackSkinnedPopupMenuEx");
		mlIsSkinnedPopupEnabled = (MLISSKINNEDPOPUPENABLED)GetProcAddress(libraryModule, "MlIsSkinnedPopupEnabled");
		mlGetSkinColor = (MLGETSKINCOLOR)GetProcAddress(libraryModule, "MlGetSkinColor");
		mlResetSkinColor = (MLRESETSKINCOLOR)GetProcAddress(libraryModule, "MlResetSkinColor");
		mlInitSkinnedPopupHook = (MLINITSKINNEDPOPUPHOOK)GetProcAddress(libraryModule, "MlInitSkinnedPopupHook");
		mlRemoveSkinnedPopupHook = (MLREMOVESKINNEDPOPUPHOOK)GetProcAddress(libraryModule, "MlRemoveSkinnedPopupHook");
		mlRatingDraw =  (MLRATINGDRAW)GetProcAddress(libraryModule, "MlRatingDraw");
		mlRatingHitTest =  (MLRATINGHITTEST)GetProcAddress(libraryModule, "MlRatingHitTest");
		mlRatingCalcMinRect = (MLRATINGCALCMINRECT)GetProcAddress(libraryModule, "MlRatingCalcMinRect");
	}
	return libraryLoadResult;
}

HRESULT Theme::InitializeColorData()
{
	HWND winampWindow;

	winampWindow = Plugin_GetWinampWindow();
	if (NULL == winampWindow)
		return E_FAIL;

	WADlg_init(winampWindow);
	colorInitRequired = FALSE;

	return S_OK;
}
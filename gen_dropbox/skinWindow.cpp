#include "./main.h"
#include "./plugin.h"
#include "./skinWindow.h"

#define WA_DLG_IMPLEMENT
#include "../winamp/wa_dlg.h"

#include "../gen_ml/ml_ipc_0313.h"
#include "../gen_ml/colors.h"

#include <windows.h>
#include <strsafe.h>


#define ENSURE_FUNCTION_LOADED(__function) {\
	HRESULT hr;\
	if (NULL == (__function)) 	{\
		hr = LoadMlLibrary();\
		if (SUCCEEDED(hr) && NULL == (__function)) hr = E_UNEXPECTED;\
		if (FAILED(hr)) return hr; }}\

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : E_FAIL)


#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (__cdecl *MLSKINWINDOWEX)(HWND /*hwnd*/, INT /*type*/, UINT /*style*/);
typedef BOOL (__cdecl *MLUNSKINWINDOW)(HWND /*hwnd*/);
typedef BOOL (__cdecl *MLGETSKINCOLOR)(UINT /*uObject*/, UINT /*uPart*/, UINT /*uState*/, COLORREF* /*pColor*/);
typedef BOOL (__cdecl *MLTRACKSKINNEDPOPUPMENUEX)(HMENU /*hmenu*/, UINT /*fuFlags*/, INT /*x*/, INT /*y*/, HWND /*hwnd*/,
												LPTPMPARAMS /*lptpm*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
typedef BOOL (__cdecl *MLISSKINNEDPOPUPENABLED)(void);

#ifdef __cplusplus
}
#endif


#define MLCOLOR_FIRST		(WADLG_NUM_COLORS)
#define MLCOLOR_MENU			(MLCOLOR_FIRST + 0)
#define MLCOLOR_MENUTEXT		(MLCOLOR_FIRST + 1)
#define MLCOLOR_LAST			MLCOLOR_MENUTEXT	

static HMODULE hGenML = NULL;
static HRESULT loadResult = S_FALSE;
static BOOL initilalizeSkinData = TRUE;
static HBRUSH skinBrushes[WADLG_NUM_COLORS + (MLCOLOR_LAST - MLCOLOR_FIRST)] = {NULL,};
static HFONT skinFont = NULL;

static MLSKINWINDOWEX mlSkinWindowEx = NULL;
static MLUNSKINWINDOW mlUnskinWindow = NULL;
static MLTRACKSKINNEDPOPUPMENUEX mlTrackSkinnedPopupMenuEx = NULL;
static MLISSKINNEDPOPUPENABLED mlIsSkinnedPopupEnabled = NULL;
static MLGETSKINCOLOR mlGetSkinColor = NULL;

static HRESULT GetPluginPath(LPTSTR pszBuffer, INT cchBufferMax)
{
	*pszBuffer = TEXT('\0');

	LPCSTR pathA = (LPCSTR)SENDWAIPC(plugin.hwndParent, IPC_GETPLUGINDIRECTORY, 0);
	if (!pathA)
		return E_FAIL;
		
	INT cchLen = MultiByteToWideChar(CP_ACP, 0, pathA, -1, pszBuffer, cchBufferMax);
	pszBuffer[cchLen] = TEXT('\0');
	if (0 == cchLen)
		return E_FAIL;

	return S_OK;
}

static HRESULT LoadMlLibrary()
{
	if (S_FALSE != loadResult)
		return loadResult;

	TCHAR szPath[MAX_PATH];
	loadResult = GetPluginPath(szPath, ARRAYSIZE(szPath));
	
	if (SUCCEEDED(loadResult)) 
		loadResult = StringCchCat(szPath, ARRAYSIZE(szPath), TEXT("\\gen_ml.dll"));

	if (SUCCEEDED(loadResult))
	{
		hGenML = LoadLibrary(szPath);
		if (NULL == hGenML)
			loadResult = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(loadResult))
	{
		mlSkinWindowEx = (MLSKINWINDOWEX)GetProcAddress(hGenML, "MlSkinWindowEx");
		mlUnskinWindow = (MLUNSKINWINDOW)GetProcAddress(hGenML, "MlUnskinWindow");
		mlTrackSkinnedPopupMenuEx = (MLTRACKSKINNEDPOPUPMENUEX)GetProcAddress(hGenML, "MlTrackSkinnedPopupMenuEx");
		mlIsSkinnedPopupEnabled = (MLISSKINNEDPOPUPENABLED)GetProcAddress(hGenML, "MlIsSkinnedPopupEnabled");
		mlGetSkinColor = (MLGETSKINCOLOR)GetProcAddress(hGenML, "MlGetSkinColor");
		
	}
	return loadResult;
}

HRESULT SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF)
{
	SKINWINDOWPARAM swp;
	swp.cbSize = sizeof(SKINWINDOWPARAM);
	swp.hwndToSkin = hwnd;
	swp.windowGuid = *windowGuid;
	swp.flagsEx = flagsEx;
	swp.callbackFF = callbackFF;

	BOOL r = (BOOL)SENDWAIPC(plugin.hwndParent, IPC_SKINWINDOW, (WPARAM)&swp);
	return (r) ? S_OK : E_FAIL;
}

HRESULT MlSkinWindow(HWND hwnd, UINT style)
{
	return MlSkinWindowEx(hwnd, SKINNEDWND_TYPE_AUTO, style);
}

HRESULT MlSkinWindowEx(HWND hwnd, INT type, UINT style)
{
	ENSURE_FUNCTION_LOADED(mlSkinWindowEx);
	return BOOL2HRESULT(mlSkinWindowEx(hwnd, type, style));
}

HRESULT MlUnskinWindow(HWND hwnd)
{
	ENSURE_FUNCTION_LOADED(mlUnskinWindow);
	return BOOL2HRESULT(mlUnskinWindow(hwnd));
}

HRESULT MlGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor)
{
	ENSURE_FUNCTION_LOADED(mlGetSkinColor);
	return BOOL2HRESULT(mlGetSkinColor(uObject, uPart, uState, pColor));
}

HRESULT MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut)
{
	ENSURE_FUNCTION_LOADED(mlTrackSkinnedPopupMenuEx);
	INT result = mlTrackSkinnedPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam);
	if (NULL != resultOut)
		*resultOut = result;
	return S_OK;
}

HRESULT MlIsSkinnedPopupEnabled()
{
	ENSURE_FUNCTION_LOADED(mlIsSkinnedPopupEnabled);
	return (FALSE != mlIsSkinnedPopupEnabled()) ? S_OK : S_FALSE;
}

static COLORREF GetMlColorByIndex(INT mlColorIndex)
{
	COLORREF rgb = RGB(255, 0, 255);
	switch(mlColorIndex)
	{
		case MLCOLOR_MENU:
			MlGetSkinColor(MLSO_MENU, MP_BACKGROUND, MBS_NORMAL, &rgb);
			break;
		case MLCOLOR_MENUTEXT:
			MlGetSkinColor(MLSO_MENU, MP_TEXT, MTS_NORMAL, &rgb);
			break;
	}
	return rgb;
}

static INT GetSkinColorIndex(INT systemIndex)
{
	switch(systemIndex)
	{
		case COLOR_WINDOW:			return WADLG_ITEMBG;
		case COLOR_WINDOWTEXT:		return WADLG_ITEMFG;
		case COLOR_HIGHLIGHT:		return WADLG_SELBAR_BGCOLOR;
		case COLOR_HIGHLIGHTTEXT:	return WADLG_SELBAR_FGCOLOR;
		case COLOR_BTNFACE:			return WADLG_LISTHEADER_BGCOLOR;
		case COLOR_BTNTEXT:			return WADLG_LISTHEADER_FONTCOLOR;
		case COLOR_BTNSHADOW:		return WADLG_LISTHEADER_FRAME_BOTTOMCOLOR;
		case COLOR_BTNHIGHLIGHT:	return WADLG_LISTHEADER_FRAME_TOPCOLOR;
		case COLOR_WINDOWFRAME:		return WADLG_HILITE;
		case COLOR_DIALOG:			return WADLG_WNDBG;
		case COLOR_DIALOGTEXT:		return WADLG_WNDFG;
		case COLOR_MENU:            return MLCOLOR_MENU;
		case COLOR_MENUTEXT:        return MLCOLOR_MENUTEXT;
	}

	return -1;
}

COLORREF WINAPI GetSkinColor(INT index)
{
	if (initilalizeSkinData)
		UpdateSkinCache(UPDATESKIN_COLOR);
	index = GetSkinColorIndex(index);
	if (index >= 0 && index < WADLG_NUM_COLORS)
		return WADlg_getColor(index);

	return GetMlColorByIndex(index);
}

const static INT extendedColorTable[] =
{
	COLOR_3DFACE,
	COLOR_BTNTEXT,
};

static INT ConvertSystemColorIndex(INT index)
{
	if (index >= EXTENDED_COLORS)
	{
		return extendedColorTable[index - EXTENDED_COLORS];
	}
	return index;
}
COLORREF WINAPI GetSystemColor(INT index)
{
	switch(index)
	{
		case COLOR_DIALOG:			index = COLOR_3DFACE; break;
		case COLOR_DIALOGTEXT:		index = COLOR_BTNTEXT; break;
	}
	return GetSysColor(ConvertSystemColorIndex(index));
}

HBRUSH WINAPI GetSystemBrush(INT index)
{
	return GetSysColorBrush(ConvertSystemColorIndex(index));
}

HBRUSH WINAPI GetSkinBrush(INT index)
{
	index = GetSkinColorIndex(index);
	INT brushIndex = (index < WADLG_NUM_COLORS) ? index : (WADLG_NUM_COLORS + (index - MLCOLOR_FIRST));
	if (brushIndex < 0  || brushIndex >= ARRAYSIZE(skinBrushes))
		return NULL;
		
	if (initilalizeSkinData)
		UpdateSkinCache(UPDATESKIN_COLOR);

	if (NULL == skinBrushes[brushIndex])
	{
		COLORREF rgb = (index < WADLG_NUM_COLORS) ? WADlg_getColor(index) :  GetMlColorByIndex(index);
		skinBrushes[brushIndex] = CreateSolidBrush(rgb);
	}
	
	return skinBrushes[brushIndex];
}

static void CALLBACK UninitializeSkinSupport()
{
	for(int i = 0; i < ARRAYSIZE(skinBrushes); i++)
	{
		if (NULL != skinBrushes[i])
		{
			DeleteObject(skinBrushes[i]);
			skinBrushes[i] = NULL;
		}
	}
	initilalizeSkinData = TRUE;

	if (NULL != skinFont)
	{
		DeleteObject(skinFont);
		skinFont = NULL;
	}
}

BOOL InitializeSkinSupport()
{
	for(int i = 0; i < ARRAYSIZE(skinBrushes); i++)
		skinBrushes[i] = NULL;
	Plugin_RegisterUnloadCallback(UninitializeSkinSupport);
	return TRUE;
}


HFONT CreateSkinFont()
{
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
		TEXT(""), /* lfFaceName */
	};

						
	lf.lfHeight = -(INT)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 3);
	lf.lfCharSet = (BYTE)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 2);
					
	LPCSTR faceNameA = (LPCSTR)SENDWAIPC(plugin.hwndParent,IPC_GET_GENSKINBITMAP, 1);
	if (NULL != faceNameA && '\0' != faceNameA)
	{
		INT count = MultiByteToWideChar(CP_ACP, 0, faceNameA, -1, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
		if (count > 0) count--;
		lf.lfFaceName[count] = TEXT('\0');
	}
	return CreateFontIndirect(&lf);	
}

HFONT GetSkinFont()
{
	if (NULL == skinFont)
		skinFont = CreateSkinFont();

	return skinFont;
}

void UpdateSkinCache(INT updateFlags)
{
	if (UPDATESKIN_COLOR & updateFlags)
	{
		for(int i = 0; i < ARRAYSIZE(skinBrushes); i++)
		{
			if (NULL != skinBrushes[i])
			{
				DeleteObject(skinBrushes[i]);
				skinBrushes[i] = NULL;
			}
		}
		WADlg_init(plugin.hwndParent);
		initilalizeSkinData = FALSE;
	}

	if (UPDATESKIN_FONT & updateFlags)
	{
		if (NULL != skinFont)
		{
			DeleteObject(skinFont);
			skinFont = NULL;
		}
		
	}
}

BOOL TrackPopupEx(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, BOOL fForceUnskinned, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{	
	if (FALSE == fForceUnskinned)
	{
		HRESULT hr = MlIsSkinnedPopupEnabled();
		if ((FAILED(hr) || S_OK == hr))
		{
			INT result;
			hr = MlTrackSkinnedPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam, &result);
			return result;
		}
	}

	return TrackPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm);
}

BOOL TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, BOOL fForceUnskinned)
{
	return TrackPopupEx(hMenu, fuFlags, x, y, hwnd, fForceUnskinned, NULL, NULL, 0, SMS_USESKINFONT, NULL, NULL);
}
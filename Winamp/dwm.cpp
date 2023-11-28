#include "main.h"
#include "config.h"
#include "wintheme.h"
#include <shobjidl.h>
#include <tataki/canvas/bltcanvas.h>
#include <tataki/bitmap/bitmap.h>
#include "../Agave/Language/api_language.h"

typedef HRESULT(WINAPI *DWMREGISTERTHUMBNAIL)(HWND hwndDestination, HWND hwndSource, void **phThumbnailId);
typedef	HRESULT(WINAPI *DWMUPATETHUMBNAILPROPERTIES)(void* hThumbnailId, void* ptnProperties);
typedef HRESULT(WINAPI *DWMUNREGISTERTHUMBNAIL)(void *hThumbnailId);
typedef HRESULT(WINAPI *DWMSETWINDOWATTRIBUTE)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
typedef HRESULT (WINAPI *DWMENABLEMMCSS)(BOOL fEnableMMCSS);
typedef HRESULT (WINAPI *DWMSETICONICTHUMBNAIL)(HWND hwnd,    HBITMAP hbmp,    DWORD dwSITFlags);
typedef HRESULT (WINAPI *DWMINVALIDATEICONICBITMAPS)(          HWND hwnd);
typedef HRESULT (WINAPI *DWMSETICONICLIVEPREVIEWBITMAP )(HWND hwnd,    HBITMAP hbmp, POINT *pptClient,  DWORD dwSITFlags);
static HMODULE dwmapi;
static void *thumbnail;
static DWMREGISTERTHUMBNAIL regThumbnail;
static DWMUPATETHUMBNAILPROPERTIES updateProp;
static DWMUNREGISTERTHUMBNAIL unregThumbnail;
static DWMSETWINDOWATTRIBUTE setWindowAttribute;
static DWMENABLEMMCSS dwmEnableMMCSS;
static DWMSETICONICTHUMBNAIL dwmSetIconicThumbnail;
static DWMINVALIDATEICONICBITMAPS dwmInvalidateIconicBitmaps;
static DWMSETICONICLIVEPREVIEWBITMAP dwmSetIconicLivePreviewBitmap;

static HIMAGELIST toolbarIcons = NULL;

typedef struct
{
	DWORD dwFlags;
	RECT rcDestination;
	RECT rcSource;
	BYTE opacity;
	BOOL fVisible;
	BOOL fSourceClientAreaOnly;
} thumbprop;

BOOL atti_present=false;
static bool triedLoad=false;
static bool LoadDWMApi()
{
	if (!triedLoad)
	{
		wchar_t gen_win7shell_path[MAX_PATH];
		PathCombineW(gen_win7shell_path, PLUGINDIR, L"gen_win7shell.dll");
		HMODULE gen_win7shell = LoadLibraryW(gen_win7shell_path);
		if (gen_win7shell)
		{
			atti_present=true;
			FreeLibrary(gen_win7shell);
		}

		dwmapi = LoadLibrary("dwmapi.dll");
		{
			regThumbnail = (DWMREGISTERTHUMBNAIL)GetProcAddress(dwmapi, "DwmRegisterThumbnail");
			updateProp = (DWMUPATETHUMBNAILPROPERTIES)GetProcAddress(dwmapi, "DwmUpdateThumbnailProperties");
			unregThumbnail = (DWMUNREGISTERTHUMBNAIL)GetProcAddress(dwmapi, "DwmUnregisterThumbnail");
			setWindowAttribute = (DWMSETWINDOWATTRIBUTE)GetProcAddress(dwmapi, "DwmSetWindowAttribute");
			dwmEnableMMCSS = (DWMENABLEMMCSS)GetProcAddress(dwmapi, "DwmEnableMMCSS");
			dwmSetIconicThumbnail = (DWMSETICONICTHUMBNAIL)GetProcAddress(dwmapi, "DwmSetIconicThumbnail");
			dwmInvalidateIconicBitmaps = (DWMINVALIDATEICONICBITMAPS)GetProcAddress(dwmapi, "DwmInvalidateIconicBitmaps");
			dwmSetIconicLivePreviewBitmap = (DWMSETICONICLIVEPREVIEWBITMAP)GetProcAddress(dwmapi, "DwmSetIconicLivePreviewBitmap");
		}
		triedLoad = true;
	}

	return dwmapi &&  regThumbnail && updateProp && unregThumbnail && setWindowAttribute && dwmEnableMMCSS;
}

bool LoadToolbarIcons()
{
	//toolbarIcons already loaded
	if (toolbarIcons != NULL)
		return true;
	
	//load toolbarIcons 
	toolbarIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32, 5, 0);
	if (toolbarIcons == NULL)
		return false;

	HICON hIcon;
	hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_TBICON1)); 
	if (hIcon != NULL)
		ImageList_AddIcon(toolbarIcons, hIcon);
	DestroyIcon(hIcon);

	hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_TBICON2)); 
	if (hIcon != NULL)
		ImageList_AddIcon(toolbarIcons, hIcon);
	DestroyIcon(hIcon);

	hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_TBICON3)); 
	if (hIcon != NULL)
		ImageList_AddIcon(toolbarIcons, hIcon);
	DestroyIcon(hIcon);

	hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_TBICON4)); 
	if (hIcon != NULL)
		ImageList_AddIcon(toolbarIcons, hIcon);
	DestroyIcon(hIcon);

	hIcon = LoadIcon(hMainInstance, MAKEINTRESOURCE(IDI_TBICON5)); 
	if (hIcon != NULL)
		ImageList_AddIcon(toolbarIcons, hIcon);
	DestroyIcon(hIcon);

	return true;
}

void UnregisterThumbnailTab(HWND hWnd)
{
	if (pTaskbar3)
	{
		if (hWnd)
		{
			pTaskbar3->UnregisterTab(hWnd);
		}
	//else
		//pTaskbar->UnregisterTab(hMainWindow);
	}
}

static void addToolbarButtons(HWND hWnd, BOOL update)
{
	THUMBBUTTON thbButtons[5];
	
	DWORD dwMask = 0x1 /*THB_BITMAP*/ | 0x4 /*THB_TOOLTIP*/;

	thbButtons[0].dwMask = (THUMBBUTTONMASK)dwMask;
	thbButtons[0].iId = 0;
	thbButtons[0].iBitmap = 0;
	StringCbCopyW(thbButtons[0].szTip, sizeof(thbButtons[0].szTip), getStringW(IDS_WIN7TOOLBAR_TOOLTIP_PREVIOUS, NULL, 0));

	thbButtons[1].dwMask = (THUMBBUTTONMASK)dwMask;
	thbButtons[1].iId = 1;
	thbButtons[1].iBitmap = 1;
	StringCbCopyW(thbButtons[1].szTip, sizeof(thbButtons[1].szTip), getStringW(IDS_WIN7TOOLBAR_TOOLTIP_PLAY, NULL, 0));

	thbButtons[2].dwMask = (THUMBBUTTONMASK)dwMask;
	thbButtons[2].iId = 2;
	thbButtons[2].iBitmap = 2;
	StringCbCopyW(thbButtons[2].szTip, sizeof(thbButtons[2].szTip), getStringW(IDS_WIN7TOOLBAR_TOOLTIP_PAUSE, NULL, 0));

	thbButtons[3].dwMask = (THUMBBUTTONMASK)dwMask;
	thbButtons[3].iId = 3;
	thbButtons[3].iBitmap = 3;
	StringCbCopyW(thbButtons[3].szTip, sizeof(thbButtons[3].szTip), getStringW(IDS_WIN7TOOLBAR_TOOLTIP_STOP, NULL, 0));

	thbButtons[4].dwMask = (THUMBBUTTONMASK)dwMask;
	thbButtons[4].iId = 4;
	thbButtons[4].iBitmap = 4;
	StringCbCopyW(thbButtons[4].szTip, sizeof(thbButtons[4].szTip), getStringW(IDS_WIN7TOOLBAR_TOOLTIP_NEXT, NULL, 0));

	if (update) 
		pTaskbar3->ThumbBarUpdateButtons(hWnd, 5, &thbButtons[0]);
	else
		pTaskbar3->ThumbBarAddButtons(hWnd, 5, &thbButtons[0]);
}


void RegisterThumbnailTab(HWND hWnd)
{
	if (LoadDWMApi() && !atti_present)
	{
		if (dwmInvalidateIconicBitmaps) // even if DWM is loaded, this only exists on win7 (NT6.1)
			dwmInvalidateIconicBitmaps(hMainWindow);

		if (!hWnd)
		{
			hWnd = hMainWindow;
		}

		if (hWnd != hMainWindow)
		{
			wchar_t title[512];
			GetWindowTextW(hMainWindow, title, sizeof(title));
			SetWindowTextW(hWnd, title);

			//HICON hIcon = (HICON)SendMessage(hMainWindow, WM_GETICON, (WPARAM)ICON_SMALL, 0);
			HICON hIcon = (HICON)GetClassLong(hMainWindow, GCL_HICONSM);
			SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
		}

		if (pTaskbar3 != NULL) 
		{
			BOOL dwm_setting=FALSE;
			setWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &dwm_setting, sizeof(dwm_setting));		
			setWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &dwm_setting, sizeof(dwm_setting));
			pTaskbar3->RegisterTab(hWnd, hMainWindow);
			pTaskbar3->SetTabOrder(hWnd, NULL);
			pTaskbar3->SetTabActive(hWnd, hMainWindow, 0);

			if (LoadToolbarIcons())
			{
				HRESULT hr = pTaskbar3->ThumbBarSetImageList(hWnd, toolbarIcons);
				if (SUCCEEDED(hr)) 
				{
					addToolbarButtons(hWnd, false);
				}
			}

			if (hWnd != hMainWindow)
			{
				dwm_setting=TRUE;
				setWindowAttribute(hMainWindow, DWMWA_FORCE_ICONIC_REPRESENTATION, &dwm_setting, sizeof(dwm_setting));		
				setWindowAttribute(hMainWindow, DWMWA_HAS_ICONIC_BITMAP, &dwm_setting, sizeof(dwm_setting));
			}
		}
	}
}


void DisableVistaPreview()
{
	if (LoadDWMApi())
	{
		BOOL blah=TRUE;
		setWindowAttribute(hMainWindow, DWMWA_FORCE_ICONIC_REPRESENTATION, &blah, sizeof(blah));
		setWindowAttribute(hMainWindow, DWMWA_HAS_ICONIC_BITMAP, &blah, sizeof(blah));
	}
}

static bool done_the_dance=false;
void DoTheVistaVideoDance()
{
	if (!done_the_dance && LoadDWMApi())
	{
		dwmEnableMMCSS(TRUE); // the magic "make my program not suck" function
		/* TODO: 
DWM_PRESENT_PARAMETERS dpp;
memset(&dpp, 0, sizeof(dpp));
dpp.cbSize = sizeof(dpp);
dpp.fQueue = true;
dpp.cBuffer = 8;
dpp.fUseSourceRate = false;
dpp.cRefreshesPerFrame = 1;
dpp.eSampling = DWM_SOURCE_FRAME_SAMPLING_POINT;
HRESULT hr = DwmSetPresentParameters(hWnd, &dpp);
*/

		// also, unrelated, but check out AvSetMmThreadCharacteristics sometime.
	}
done_the_dance = true;
}

static void Adjust(int bmpw, int bmph, RECT &r)
{
	// maintain 'square' stretching
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	double aspX = (double)(w)/(double)bmpw;
	double aspY = (double)(h)/(double)bmph;
	double asp = min(aspX, aspY);
	int newW = (int)(bmpw*asp);
	int newH = (int)(bmph*asp);
	r.left = (w - newW)/2;
	r.top = (h - newH)/2;
	r.right = r.left + newW;
	r.bottom = r.top + newH;
}

void RefreshIconicThumbnail()
{
	if (LoadDWMApi() && !atti_present)
	{
		if (dwmInvalidateIconicBitmaps) // even if DWM is loaded, this only exists on win7 (NT6.1)
			dwmInvalidateIconicBitmaps(hMainWindow);
	}
}

void OnIconicThumbnail(int width, int height)
{
	static BltCanvas *iconic_thumbnail_bitmap=0;

	HWND hWnd = g_dialog_box_parent?g_dialog_box_parent:hMainWindow;

	RECT client_size;
	GetClientRect(hWnd, &client_size);

	if (!iconic_thumbnail_bitmap)
	{
		iconic_thumbnail_bitmap = new BltCanvas(client_size.right, client_size.bottom, hMainWindow);
	}
	else
	{
		iconic_thumbnail_bitmap->DestructiveResize(client_size.right, client_size.bottom);
	}

	SendMessage(hWnd, WM_PRINTCLIENT, (WPARAM) iconic_thumbnail_bitmap->getHDC(),  PRF_CHILDREN | PRF_CLIENT | /*PRF_ERASEBKGND |*/ PRF_NONCLIENT /*| PRF_OWNED*/);
	
	void *bits=0;
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	HBITMAP hbmp = CreateDIBSection(iconic_thumbnail_bitmap->getHDC(), &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	
	if (hbmp)
	{
		BltCanvas resizedBitmap(hbmp);
		int x=0, y=0;
		
		RECT dest;
		dest.left = x;
		dest.top = y;
		dest.right = width;
		dest.bottom = height;
		resizedBitmap.drawRect(&dest, 1, 0);
		Adjust(client_size.right, client_size.bottom, dest);
		iconic_thumbnail_bitmap->stretchToRectAlpha(&resizedBitmap, &client_size, &dest);
		dwmSetIconicThumbnail(hMainWindow, hbmp, 0);
	}
}

void OnThumbnailPreview()
{
	static BltCanvas *thumbnail_preview_bitmap=0;

	HWND hWnd = g_dialog_box_parent?g_dialog_box_parent:hMainWindow;

	RECT client_size;
	GetClientRect(hWnd, &client_size);

	if (!thumbnail_preview_bitmap)
	{
		thumbnail_preview_bitmap = new BltCanvas(client_size.right, client_size.bottom, hWnd);
	}
	else
	{
		thumbnail_preview_bitmap->DestructiveResize(client_size.right, client_size.bottom);
	}

	SendMessage(hWnd, WM_PRINT, (WPARAM) thumbnail_preview_bitmap->getHDC(), PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT /*| PRF_OWNED*/);
	
	void *bits=0;
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = client_size.right - client_size.left;
	bmi.bmiHeader.biHeight = client_size.top - client_size.bottom;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	HBITMAP hbmp = CreateDIBSection(thumbnail_preview_bitmap->getHDC(), &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	
	if (hbmp)
	{
		POINT offset;
		offset.x = client_size.left;
		offset.y = client_size.top;
		if (dwmSetIconicLivePreviewBitmap(hWnd, hbmp, &offset,  1) ==  S_OK)
		{
			MessageBoxA(NULL, "winamp/live", "winamp/live", MB_OK);
		}
		else 
		{
			MessageBoxA(NULL, "winamp/no live", "winamp/no live", MB_OK);
		}
		
	}

}
#include "main.h"
#include "./toolbarHost.h"


typedef struct ToolbarHost
{
	Toolbar	*toolbar;
} ToolbarHost;

typedef struct ToolbarHostCreateParam
{
	Toolbar	*toolbar;
} ToolbarHostCreateParam;

#define TOOLBARHOST(_hwnd) ((ToolbarHost*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define TOOLBARHOST_RET_VOID(_self, _hwnd) {(_self) = TOOLBARHOST((_hwnd)); if (NULL == (_self)) return;}
#define TOOLBARHOST_RET_VAL(_self, _hwnd, _error) {(_self) = TOOLBARHOST((_hwnd)); if (NULL == (_self)) return (_error);}

#define TOOLBAR(_self)		(((_self))->toolbar)

static LRESULT CALLBACK 
ToolbarHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);


static ATOM 
ToolbarHost_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, TOOLBARHOST_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = CS_DBLCLKS;
	klass.lpfnWndProc = ToolbarHost_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(ToolbarHost*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = TOOLBARHOST_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

HWND
ToolbarHost_CreateWindow(Toolbar *toolbar, HWND parentWindow, int x, int y, 
						 int width, int height, int controlId)
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;
	ToolbarHostCreateParam param;
		
	if (NULL == toolbar)
		return NULL;

	if (NULL == parentWindow || FALSE == IsWindow(parentWindow))
		return NULL;

	instance = Plugin_GetInstance();
	klassAtom = ToolbarHost_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;

	param.toolbar = toolbar;
	
	hwnd = CreateWindowEx(WS_EX_NOPARENTNOTIFY, 
						  (LPCWSTR)MAKEINTATOM(klassAtom), NULL,
						  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
						  x, y, width, height, 
						  parentWindow, (HMENU)controlId, instance, &param);

	return hwnd;
}


static LRESULT
ToolbarHost_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	ToolbarHost *self;
	ToolbarHostCreateParam *param;
	
	param = (ToolbarHostCreateParam*)createStruct->lpCreateParams;
	if (NULL == param)
		return -1;

	self = (ToolbarHost*)malloc(sizeof(ToolbarHost));
	if (NULL == self)
		return -1;
	
	memset(self, 0, sizeof(ToolbarHost));

	TOOLBAR(self) = param->toolbar;
	if (NULL == TOOLBAR(self))
		return -1;

	TOOLBAR(self)->SetHost(hwnd);
	TOOLBAR(self)->AddRef();

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;
		
	Plugin_SetControlTheme(hwnd, SKINNEDWND_TYPE_WINDOW, 
						   SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	TOOLBAR(self)->UpdateColors();
	return 0;
}

static void
ToolbarHost_OnDestroy(HWND hwnd)
{
	ToolbarHost *self;

	self = (ToolbarHost*)(LONG_PTR)SetWindowLongPtr(hwnd, 0, 0);
	if (NULL == self)
		return;

	if (NULL != TOOLBAR(self))
	{
		TOOLBAR(self)->SetHost(NULL);
		TOOLBAR(self)->Release();
	}

	free(self);
}

static void
ToolbarHost_OnPaint(HWND hwnd)
{
	ToolbarHost *self;
	PAINTSTRUCT ps;
	
	TOOLBARHOST_RET_VOID(self, hwnd);

	if (NULL != BeginPaint(hwnd, &ps))
	{			
		TOOLBAR(self)->Paint(ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
ToolbarHost_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		ToolbarHost *self;
		TOOLBARHOST_RET_VOID(self, hwnd);
		TOOLBAR(self)->Paint(hdc, &clientRect, TRUE);
	}
}

static void
ToolbarHost_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
	{
		ToolbarHost *self;
		TOOLBARHOST_RET_VOID(self, hwnd);
		TOOLBAR(self)->Layout(0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static void
ToolbarHost_OnContextMenu(HWND hwnd, HWND targetWindow, long cursor_s)
{
	DefWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)targetWindow, (LPARAM)cursor_s);
}

static long
ToolbarHost_OnGetIdealHeight(HWND hwnd)
{
	ToolbarHost *self;
	TOOLBARHOST_RET_VAL(self, hwnd, 0);
	return TOOLBAR(self)->GetIdealHeight();
}

static LRESULT CALLBACK 
ToolbarHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return ToolbarHost_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			ToolbarHost_OnDestroy(hwnd); return 0;
		case WM_PAINT:				ToolbarHost_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		ToolbarHost_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	ToolbarHost_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_CONTEXTMENU:		ToolbarHost_OnContextMenu(hwnd, (HWND)wParam, (long)lParam); return 0;

		case TBM_GETIDEALHEIGHT:	return ToolbarHost_OnGetIdealHeight(hwnd);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#include <precomp.h>

#include "std_wnd.h"
#include "api.h"
#include <api/wnd/api_window.h>
#include <api/wnd/wndevent.h>
#include <api/wnd/wndevent.h>
#include <shobjidl.h>
#include "../winamp/wa_ipc.h"

#ifndef AC_SRC_ALPHA
const int AC_SRC_ALPHA = 1;
#endif

#ifndef THBN_CLICKED
#define THBN_CLICKED 0x1800
#endif

static int nreal = 0;

#ifdef __APPLE__
OSStatus MyWindowEventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData);
OSStatus MyControlEventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData);
#endif

#ifdef _WIN32
static HINSTANCE gdi32instance = NULL;

static int(WINAPI *getRandomRgn)(HDC dc, HRGN rgn, int i) = NULL;

static int grrfailed = 0;

static void register_wndClass(HINSTANCE hInstance);

static int versionChecked = 0;
static int isNT = 0;

static int IsNT()
{
	if (versionChecked)
		return isNT;
  
	if (GetVersion() < 0x80000000)
		isNT = 1;
  
	versionChecked = 1;
	return isNT;
}

int StdWnd::alphaStretchBlit(HDC destHDC, int dstx, int dsty, int dstw, int dsth, HDC sourceHDC, int srcx, int srcy, int srcw, int srch)
{
	if (IsNT())
	{
		SetStretchBltMode(destHDC, HALFTONE);
		StretchBlt(destHDC, dstx, dsty, dstw, dsth, sourceHDC, srcx , srcy, srcw, srch, SRCCOPY);
		return 1;
	}
	else
		return 0;
}
#endif

#ifdef __APPLE__
enum
{
	kWasabi	= 'WASA'
};

const ControlID	kWasabiID	= { kWasabi, 0 };

void GetWasabiHIView(WindowRef window, HIViewRef *control)
{
  GetControlByID(window, &kWasabiID, control);
}


OSStatus CreateHIView ( WindowRef inWindow, const Rect* inBounds,
                        ControlRef* outControl )
{
  OSStatus            err;
  ControlRef          root;
  EventRef            event;
  
  
  // Make an initialization event
  err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize,
                     GetCurrentEventTime(), 0, &event );
  require_noerr( err, CantCreateEvent );
  
  // If bounds were specified, push the them into the initialization event
  // so that they can be used in the initialization handler.
  if ( inBounds != NULL )
  {
    err = SetEventParameter( event, 'boun', typeQDRectangle,
                             sizeof( Rect ), inBounds );
    require_noerr( err, CantSetParameter );
  }
  
  err = HIObjectCreate( kHIViewClassID, event, (HIObjectRef*)
                        outControl );
  require_noerr( err, CantCreate );
  
  
  // If a parent window was specified, place the new view into the
  // parent window.
  if ( inWindow != NULL )
  {
    err = GetRootControl( inWindow, &root );
    require_noerr( err, CantGetRootControl );
    
    
    err = HIViewAddSubview( root, *outControl );
    SetControlID( *outControl, &kWasabiID );
    HIViewSetVisible(*outControl, true);
  }
  
  
CantCreate:
CantGetRootControl:
CantSetParameter:
CantCreateEvent:
    ReleaseEvent( event );
  
  
CantRegister:
    return err;
}
#endif

OSWINDOWHANDLE StdWnd::createWnd(RECT *r, int nochild, int acceptdrops, OSWINDOWHANDLE parent, OSMODULEHANDLE module, ifc_window *rw)
{
#ifdef _WIN32
	register_wndClass(module);
  
  int style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
  int exstyle=0;

  if (parent == NULL) {
    exstyle |= WS_EX_TOOLWINDOW;
    style |= WS_POPUP;
  } else
    style |= WS_CHILD;

  if (nochild) style=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  if (acceptdrops) exstyle |= WS_EX_ACCEPTFILES;
  
	HWND ret = CreateWindowExW(exstyle, BASEWNDCLASSNAME, NULL, style,
	                          r->left, r->top, r->right - r->left, r->bottom - r->top, parent, NULL, module, (LPVOID)rw);
	if (ret != INVALIDOSWINDOWHANDLE)
	{
		nreal++;
		if(NULL != WASABI_API_APP && 0 != (WS_POPUP & style))
			WASABI_API_APP->app_registerGlobalWindow(ret);
	}
	return ret;
#elif defined(__APPLE__)
	Rect wndRect;
	SetRect(&wndRect, r->left, r->top, r->right, r->bottom);
	WindowRef newWnd;
	CreateNewWindow(kOverlayWindowClass,  kWindowCompositingAttribute, &wndRect, &newWnd);
	SetWindowGroup(newWnd, GetWindowGroupOfClass(kDocumentWindowClass));
	// install a window event handler
	const EventTypeSpec	windowEventList[] =
  {
  {
    kEventClassCommand, kEventProcessCommand
  },
//      { kEventClassWindow, kEventWindowBoundsChanging },
  { kEventClassWindow, kEventWindowBoundsChanged },
//  { kEventClassWindow,    kEventWindowInit },
    
  {kEventClassMouse, kEventMouseDown},
  {kEventClassMouse, kEventMouseUp},
  {kEventClassMouse, kEventMouseMoved},
  {kEventClassMouse, kEventMouseDragged},
    //{kEventClassMouse, kEventMouseEntered},
    //{kEventClassMouse, kEventMouseExited},
    //{kEventClassMouse, kEventMouseWheelMoved},
    
  {kEventClassKeyboard, kEventRawKeyDown},
  {kEventClassKeyboard, kEventRawKeyUp},
        
  };
	InstallWindowEventHandler(newWnd, MyWindowEventHandler, 	GetEventTypeCount(windowEventList), windowEventList, rw, NULL);

  // create the content view
  HIViewRef myHIView;
  CreateHIView(newWnd,&wndRect,(ControlRef*)&myHIView);
  const EventTypeSpec	controlEventList[] =
  {
  {    kEventClassControl, kEventControlDraw},
  {    kEventClassControl, kEventControlApplyBackground},
  };

  InstallEventHandler(GetControlEventTarget(myHIView), MyControlEventHandler, GetEventTypeCount(controlEventList), controlEventList, rw, NULL);  

	return newWnd;
#endif
}

void StdWnd::destroyWnd(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32

	if(NULL != WASABI_API_APP && 0 != (WS_POPUP & GetWindowLongPtr(wnd, GWL_STYLE)))
		WASABI_API_APP->app_unregisterGlobalWindow(wnd);

	DestroyWindow(wnd);
	nreal--;
	if (nreal == 0)
	{
		if (gdi32instance) FreeLibrary(gdi32instance);
    
		gdi32instance = NULL;
		getRandomRgn = NULL;
	}
#elif defined(__APPLE__)
	DisposeWindow(wnd);
#endif
}

int StdWnd::isValidWnd(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	return IsWindow(wnd);
#elif defined(__APPLE__)
	// TODO: docs suggest that this function is slow
  if (!wnd)
    return 0;
	return IsValidWindowPtr(wnd);
#endif
}

#ifdef _WIN32
void StdWnd::setLayeredWnd(OSWINDOWHANDLE wnd, int layered)
{
	if (layered)
	{
		// have to clear and reset, can't just set
		SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	}
	else
	{
		SetWindowLong(wnd, GWL_EXSTYLE, GetWindowLong(wnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
	}
}

int StdWnd::isLayeredWnd(OSWINDOWHANDLE wnd)
{
	DWORD dwLong = GetWindowLong(wnd, GWL_EXSTYLE);
	return !!(dwLong & WS_EX_LAYERED);
}



void StdWnd::setLayeredAlpha(OSWINDOWHANDLE wnd, int amount)
{
	if (!isDesktopAlphaAvailable()) return;
	SetLayeredWindowAttributes(wnd, RGB(0, 0, 0), amount, LWA_ALPHA);
}


void StdWnd::updateLayeredWnd(OSWINDOWHANDLE wnd, int x, int y, int w, int h, HDC surfdc, int alpha)
{
	if (!isDesktopAlphaAvailable()) return;
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
	POINT sp = {x, y}, pt = {0, 0};
	SIZE ss = { w, h };
	//HDC sysdc = GetDC(NULL);
	UpdateLayeredWindow(wnd, NULL/*sysdc*/, &sp, &ss, surfdc, &pt, 0, &blend, ULW_ALPHA);
	//ReleaseDC(NULL, sysdc);
}
#endif

void StdWnd::setWndPos(OSWINDOWHANDLE wnd, OSWINDOWHANDLE zorder, int x, int y, int w, int h,
                       int nozorder, int noactive, int nocopybits, int nomove, int noresize)
{
#ifdef _WIN32
	SetWindowPos(wnd, zorder, x, y, w, h,
	             SWP_DEFERERASE |		// we ignore WM_SYNCPAINT anyway
	             (nozorder ? SWP_NOZORDER : 0) |
	             (noactive ? SWP_NOACTIVATE : 0) |
	             (nocopybits ? SWP_NOCOPYBITS : 0) |
	             (nomove ? SWP_NOMOVE : 0) |
	             (noresize ? SWP_NOSIZE : 0));
#elif defined(__APPLE__)
  if (!nomove)
  {
    MoveWindow(wnd, x, y, false);
  }
  if (!noresize)
  {
    Rect newRect;
    newRect.left = x;
    newRect.right=x+w;
    newRect.top = y;
    newRect.bottom = y+h;
//    SetWindowBounds(wnd, kWindowStructureRgn, &newRect);
    SizeWindow(wnd, w, h, false);
  }
  if (!noactive)
    SelectWindow(wnd);
  if (!nocopybits)
    StdWnd::invalidateRect(wnd, 0);
  
#endif
}

void StdWnd::bringToFront(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	SetWindowPos(wnd, HWND_TOP, 0, 0, 0, 0,
	             SWP_NOMOVE | SWP_NOSIZE | SWP_DEFERERASE | SWP_NOOWNERZORDER);
#elif defined(__APPLE__)
	BringToFront(wnd);
#endif
}

void StdWnd::sendToBack(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	SetWindowPos(wnd, HWND_BOTTOM, 0, 0, 0, 0,
	             SWP_NOMOVE | SWP_NOSIZE | SWP_DEFERERASE | SWP_NOOWNERZORDER);
#elif defined(__APPLE__)
	SendBehind(wnd, 0);
#endif
}

OSWINDOWHANDLE StdWnd::getWindowFromPoint(POINT pt)
{
#ifdef _WIN32
  return ::WindowFromPoint(pt);
#else
#warning port me
return 0;
#endif
}

int StdWnd::isWndVisible(OSWINDOWHANDLE wnd)
{
	return IsWindowVisible(wnd);
}

void StdWnd::showWnd(OSWINDOWHANDLE wnd, int noactivate)
{
#ifdef _WIN32
	ShowWindow(wnd, noactivate ? SW_SHOWNA : SW_SHOWNORMAL);
#elif defined(__APPLE__)
	ShowWindow(wnd);
	if (!noactivate)
		ActivateWindow(wnd, 1);
#endif
}

void StdWnd::hideWnd(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	ShowWindow(wnd, SW_HIDE);
#elif defined(__APPLE__)
	HideWindow(wnd);
#endif
}

int StdWnd::isPopup(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	return !!(GetWindowLong(wnd, GWL_STYLE) & WS_POPUP);
#elif defined(__APPLE__)
	return 0; // TODO: maybe use window class or window group to determine
#endif
}

#ifdef _WIN32
void StdWnd::setEnabled(OSWINDOWHANDLE wnd, int enabled)
{
	EnableWindow(wnd, enabled);
}
#endif

void StdWnd::setFocus(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	SetFocus(wnd);
#elif defined(__APPLE__)
	SetUserFocusWindow(wnd);
#endif
}

OSWINDOWHANDLE StdWnd::getFocus()
{
#ifdef _WIN32
	return GetFocus();
#elif defined(__APPLE__)
	return GetUserFocusWindow();
#endif
}

#ifdef _WIN32
void StdWnd::setTopmost(OSWINDOWHANDLE wnd, int topmost)
{
	SetWindowPos(wnd, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}
#endif

void StdWnd::invalidateRect(OSWINDOWHANDLE wnd, RECT *r)
{
#ifdef _WIN32
	OSREGIONHANDLE reg = NULL;
	if (r == NULL)
	{
		RECT cr;
		if (!IsWindow(wnd))
			return;
    
		GetClientRect(wnd, &cr);
		reg = CreateRectRgnIndirect(&cr);
	}
	else
		reg = CreateRectRgnIndirect(r);
	invalidateRegion(wnd, reg);
	DeleteObject(reg);
#elif defined(__APPLE__)
  HIViewRef view;
  GetWasabiHIView(wnd, &view);
	if (r == 0)
  {
    HIViewSetNeedsDisplay(view, true);
  }
  else
  {
    HIRect rect = CGRectMake(r->left, r->top, r->right-r->left, r->bottom - r->top);
    HIViewSetNeedsDisplayInRect(view, &rect, true);
  }
#endif
}


void StdWnd::invalidateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region)
{
#ifdef _WIN32
	clipOSChildren(wnd, region);
	InvalidateRgn(wnd, region, FALSE);
#elif defined(__APPLE__)
  HIViewRef view;
  GetWasabiHIView(wnd, &view);
  HIViewSetNeedsDisplayInShape(view, region, true);
#endif
}

void StdWnd::validateRect(OSWINDOWHANDLE wnd, RECT *r)
{
#ifdef _WIN32
	ValidateRect(wnd, r);
#elif  defined(__APPLE__)
  HIViewRef view;
  GetWasabiHIView(wnd, &view);
	if (r == 0)
  {
    HIViewSetNeedsDisplay(view, false);
  }
  else
  {
    HIRect rect = CGRectMake(r->left, r->top, r->right-r->left, r->bottom - r->top);
    HIViewSetNeedsDisplayInRect(view, &rect, false);
  }
#endif
}

void StdWnd::validateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region)
{
#ifdef _WIN32
	ValidateRgn(wnd, region);
#elif defined(__APPLE__)
  HIViewRef view;
  GetWasabiHIView(wnd, &view);
  HIViewSetNeedsDisplayInShape(view, region, false);  
#endif
}

void StdWnd::update(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	if (wnd != NULL) UpdateWindow(wnd);
#elif defined(__APPLE__)
  if (wnd)
  {
  HIViewRef view;
  GetWasabiHIView(wnd, &view);
  HIViewRender(view);  
  }
#endif
}


int StdWnd::getUpdateRect(OSWINDOWHANDLE wnd, RECT *r)
{
#ifdef _WIN32
	return GetUpdateRect(wnd, r, FALSE);
#else
  Rect updateRect;
		GetWindowBounds(wnd, kWindowUpdateRgn, &updateRect);
    r->left=updateRect.left;
    r->right = updateRect.right;
    r->top = updateRect.top;
    r->bottom = updateRect.bottom;
#endif
}

#ifdef _WIN32
void StdWnd::getUpdateRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region)
{
	GetUpdateRgn(wnd, region, FALSE);
}
#elif defined(__APPLE__)
// TODO:  GetWindowRegion(wnd, kWindowUpdateRgn, region);
#endif

#ifdef _WIN32
int StdWnd::haveGetRandomRegion()
{
	// I assume linux will just return FALSE
	if (gdi32instance == NULL && !grrfailed)
	{
		gdi32instance = LoadLibrary("GDI32.dll");
		if (gdi32instance != NULL)
		{
			getRandomRgn = (int(WINAPI *)(HDC, HRGN, int)) GetProcAddress(gdi32instance, "GetRandomRgn");
			if (getRandomRgn == NULL)
			{
				grrfailed = 1;
				FreeLibrary(gdi32instance);
				gdi32instance = NULL;
			}
		}
		else
		{
			grrfailed = 1;
		}
	}
	return (getRandomRgn != NULL);
}

void StdWnd::getRandomRegion(HDC hdc, OSREGIONHANDLE region)
{
	if (!haveGetRandomRegion()) return;
	(*getRandomRgn)(hdc, region, SYSRGN);
}
#endif
void StdWnd::setWndRegion(OSWINDOWHANDLE wnd, OSREGIONHANDLE region, int redraw)
{
#ifdef _WIN32
	SetWindowRgn(wnd, region, !!redraw);
#elif defined(__APPLE__)
#warning port me?  
#endif
}
#ifdef _WIN32
int StdWnd::isDesktopAlphaAvailable()
{
	return 1; // we're only targetting windows 2000 and up, so it's always available
}
#endif
int StdWnd::isTransparencyAvailable()
{
#ifdef _WIN32 
  // there is no win32 implementation that supports setLayeredWindowAttributes but not updateLayeredWindow
	return StdWnd::isDesktopAlphaAvailable();
#elif defined(__APPLE__)
  return 1;
#else
#error port me
#endif
}

void StdWnd::getClientRect(OSWINDOWHANDLE wnd, RECT *r)
{
#ifdef _WIN32
	GetClientRect(wnd, r);
#elif defined(__APPLE__)
	Rect temp;
	GetWindowBounds(wnd, kWindowContentRgn, &temp);
  r->left = 0;
  r->top = 0;
  r->right = temp.right-temp.left;
  r->bottom = temp.bottom-temp.top;
#endif
}

void StdWnd::getWindowRect(OSWINDOWHANDLE wnd, RECT *r)
{
#ifdef _WIN32
	GetWindowRect(wnd, r);
#elif defined(__APPLE__)
	Rect temp;
	GetWindowBounds(wnd, kWindowGlobalPortRgn, &temp);
  r->left = temp.left;
  r->top = temp.top;
  r->right = temp.right;
  r->bottom = temp.bottom;
#endif
}

void StdWnd::clientToScreen(OSWINDOWHANDLE wnd, int *x, int *y)
{
#ifdef _WIN32
	POINT p = { x ? *x : 0, y ? *y : 0 };
	ClientToScreen(wnd, &p);
	if (x) *x = p.x;
	if (y) *y = p.y;
#elif defined(__APPLE__)
  Point pt;
  pt.h = x?*x:0;
  pt.v = y?*y:0;
	QDLocalToGlobalPoint(GetWindowPort(wnd) , &pt);
	if (x) *x = pt.h;
	if (y) *y = pt.v;  
#endif
}

void StdWnd::screenToClient(OSWINDOWHANDLE wnd, int *x, int *y)
{
#ifdef _WIN32
	POINT p = { x ? *x : 0, y ? *y : 0 };
	ScreenToClient(wnd, &p);
	if (x) *x = p.x;
	if (y) *y = p.y;
#elif defined(__APPLE__)
  Point pt;
  pt.h = x?*x:0;
  pt.v = y?*y:0;
		QDGlobalToLocalPoint(GetWindowPort(wnd) , &pt);
    if (x) *x = pt.h;
    if (y) *y = pt.v;  
#endif
}

#ifdef _WIN32
void StdWnd::setParent(OSWINDOWHANDLE child, OSWINDOWHANDLE newparent)
{
	SetParent(child, newparent);
}
#endif

OSWINDOWHANDLE StdWnd::getParent(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	return GetParent(wnd);
#else
	return 0;
#endif
}

#ifdef _WIN32
OSWINDOWHANDLE StdWnd::getTopmostChild(OSWINDOWHANDLE wnd)
{
	return GetWindow(wnd, GW_CHILD);
}

void StdWnd::setCapture(OSWINDOWHANDLE wnd)
{
	SetCapture(wnd);
}

void StdWnd::releaseCapture()
{
	ReleaseCapture();
}

OSWINDOWHANDLE StdWnd::getCapture()
{
	return GetCapture();
}

void StdWnd::revokeDragNDrop(OSWINDOWHANDLE wnd)
{
	if (wnd == NULL) return;
	RevokeDragDrop(wnd);
}
#endif
void StdWnd::setWndName(OSWINDOWHANDLE wnd, const wchar_t *name)
{
#ifdef _WIN32
	SetWindowTextW(wnd, name);
#elif defined(__APPLE__)
	// TODO:
	//SetWindowTitleWithCFString(wnd,
#endif
}


void StdWnd::getWndName(OSWINDOWHANDLE wnd, wchar_t *name, int maxlen)
{
  #ifdef _WIN32
	GetWindowTextW(wnd, name, maxlen);
#else
  name[0]=0;
#warning port me
#endif
}

void StdWnd::setIcon(OSWINDOWHANDLE wnd, OSICONHANDLE icon, int large)
{
#ifdef _WIN32
	SendMessageW(wnd, WM_SETICON, !large ? ICON_SMALL : ICON_BIG, (LPARAM)icon);
#else
#warning port me
#endif
}

OSWINDOWHANDLE StdWnd::getActiveWindow()
{
#ifdef _WIN32
	return GetActiveWindow();
#elif defined(__APPLE__)
  return ActiveNonFloatingWindow();
#endif
}

void StdWnd::setActiveWindow(OSWINDOWHANDLE wnd)
{
#ifdef _WIN32
	SetActiveWindow(wnd);
#elif defined(__APPLE__)
  SelectWindow(wnd);
#endif
}

#ifdef _WIN32
// clip oswindow children off the invalidation region
void StdWnd::clipOSChildren(OSWINDOWHANDLE wnd, OSREGIONHANDLE reg)
{
	HWND w = GetWindow(wnd, GW_CHILD);
	while (w != NULL)
	{
		if (IsWindowVisible(w))
		{
			RECT r;
			GetClientRect(w, &r);
			POINT p = {r.left, r.top};
			ClientToScreen(w, &p);
			ScreenToClient(wnd, &p);
			OffsetRect(&r, p.x, p.y);
			OSREGIONHANDLE cr = CreateRectRgnIndirect(&r);
			OSREGIONHANDLE or = CreateRectRgn(0, 0, 0, 0);
			CombineRgn(or, reg, 0, RGN_COPY);
			CombineRgn(reg, or, cr, RGN_DIFF);
			DeleteObject(cr);
			DeleteObject(or);
		}
		w = GetWindow(w, GW_HWNDNEXT);
	}
}

//////////////////////////

static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// fetch out the api_window *
	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
		ASSERT(cs->lpCreateParams != NULL);
		// stash pointer to self
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}
  
	// pass the messages into the BaseWnd
	ifc_window *rootwnd = (ifc_window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (rootwnd == NULL || rootwnd->getOsWindowHandle() != hWnd)
	{
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		LRESULT r = 0;
		switch (uMsg)
		{
			case WM_CLOSE: 
				PostMessage(GetParent(hWnd), WM_CLOSE, wParam, lParam); 
				return 0;
			case WM_NCPAINT: return 0;
			case WM_SYNCPAINT: return 0;
        //      case WM_SETFOCUS: r = rootwnd->windowEvent(WndEvent::SETFOCUS);
        //      case WM_KILLFOCUS: r = rootwnd->windowEvent(WndEvent::KILLFOCUS);
			case WM_ACTIVATE:
				if ( (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) && IsIconic(GetParent(hWnd)) )
				{
					ShowWindow(GetParent(hWnd), SW_RESTORE);
				}
				r = rootwnd->wndProc(hWnd, uMsg, wParam, lParam);
				break;
			case WM_COMMAND:		
				if (HIWORD(wParam) == THBN_CLICKED)
				{
					HWND parentWnd = GetParent(hWnd);
					if (parentWnd == NULL) 
						parentWnd = hWnd;

					switch (LOWORD(wParam))
					{
					case 0: //previous
						{
							SendMessage(parentWnd, WM_COMMAND, 40044, 0);
						}
						break;
					case 1: //play
						{
							SendMessage(parentWnd, WM_COMMAND, 40045, 0);									
						}
						break;
					case 2: //pause
						{
							SendMessage(parentWnd, WM_COMMAND, 40046, 0);									
						}
						break;
					case 3: //stop
						{
							SendMessage(parentWnd, WM_COMMAND, 40047, 0);
						}
						break;
					case 4: //next
						{
							SendMessage(parentWnd, WM_COMMAND, 40048, 0);
						}
						break;					
					}
				}
			default:
				r = rootwnd->wndProc(hWnd, uMsg, wParam, lParam);
		}
		if (IsWindow(hWnd)) // wndProc may have switched skin and killed this window
			rootwnd->performBatchProcesses();
		return r;
	}
}

static void register_wndClass(HINSTANCE hInstance)
{
	WNDCLASSW wc;
	if (GetClassInfoW(hInstance, BASEWNDCLASSNAME, &wc)) return;
  
	// regiszter pane class
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon		= NULL;
	wc.hCursor		= NULL;
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= BASEWNDCLASSNAME;
  
	int r = RegisterClassW(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			char florp[WA_MAX_PATH];
			SPRINTF(florp, "Failed to register class, err %d", res);
			ASSERTPR(0, florp);
		}
	}
}
#endif

#ifdef __APPLE__

// TODO: call into BaseWnd and take this out of here
#include <tataki/canvas/canvas.h>
#include <tataki/region/region.h>
#include <api/wnd/basewnd.h>

OSStatus MyWindowEventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	HICommand hiCommand;
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(inEvent);
	UInt32 eventKind = GetEventKind(inEvent);
	ifc_window *wasabiWnd = (ifc_window *)inUserData;
	WindowRef window = wasabiWnd->getOsWindowHandle();
  
	switch (eventClass)
	{
		case kEventClassKeyboard:
			switch (eventKind)
			{
				case kEventRawKeyDown:
				{
					UInt32 keyCode;
					GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32,
					                  NULL, sizeof(keyCode), NULL, &keyCode);
          
					if (wasabiWnd->onKeyDown(keyCode))
						err = noErr;
				}
          break;
				case kEventRawKeyUp:
				{
					UInt32 keyCode;
					GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32,
					                  NULL, sizeof(keyCode), NULL, &keyCode);
          
					if (wasabiWnd->onKeyUp(keyCode))
						err = noErr;
				}
          break;
			}
			break;
		case kEventClassMouse:
			switch (eventKind)
			{
				case kEventMouseDown:
				{
					HIPoint point;
          
					GetEventParameter(inEvent, kEventParamWindowMouseLocation, typeHIPoint,
					                  NULL, sizeof(point), NULL, &point);
          
					EventMouseButton button;
          
					GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton,
					                  NULL, sizeof(button), NULL, &button);
          
          UInt32 modifiers;
          GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32,
					                  NULL, sizeof(modifiers), NULL, &modifiers);
          
          
					switch (button)
					{
						case kEventMouseButtonPrimary:
              if (modifiers & controlKey) // fake right click
                wasabiWnd->onRightButtonDown(point.x, point.y);
              else
							  wasabiWnd->onLeftButtonDown(point.x, point.y);
							break;
						case kEventMouseButtonSecondary:
							wasabiWnd->onRightButtonDown(point.x, point.y);
							break;
					}
				}
          break;
				case kEventMouseUp:
				{
					HIPoint point;
					GetEventParameter(inEvent, kEventParamWindowMouseLocation, typeHIPoint,
					                  NULL, sizeof(point), NULL, &point);
          
					EventMouseButton button;
					GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton,
					                  NULL, sizeof(button), NULL, &button);
          
					switch (button)
					{
						case kEventMouseButtonPrimary:
							wasabiWnd->onLeftButtonUp(point.x, point.y);
							break;
						case kEventMouseButtonSecondary:
							wasabiWnd->onRightButtonUp(point.x, point.y);
							break;
					}
				}
          break;
				case kEventMouseMoved:
				case kEventMouseDragged:
				{
					HIPoint point;
					GetEventParameter(inEvent, kEventParamWindowMouseLocation, typeHIPoint,
					                  NULL, sizeof(point), NULL, &point);
          
					wasabiWnd->onMouseMove(point.x, point.y);
          
				}
          break;
			}
			break;
		case kEventClassCommand:
			if (eventKind == kEventProcessCommand)
			{
				err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
				                        NULL, sizeof(HICommand), NULL, &hiCommand);
        
				switch (hiCommand.commandID)
				{
					default:
						err = eventNotHandledErr;
						break;
				}
			}
			break;
		case kEventClassWindow:
			switch (eventKind)
			{
        case  kEventWindowBoundsChanging:
          break;
				case  kEventWindowBoundsChanged:
				{
          		HIRect		bounds;
					UInt32		attributes;
					Rect		origBounds;
					Rect		currBounds;
          		ControlRef	control;
              
					// Get the change attributes
					err = GetEventParameter(inEvent, kEventParamAttributes, typeUInt32,
					                        NULL, sizeof(UInt32), NULL, &attributes);
          
					// If the window size changed
          //kWindowBoundsChangeOriginChanged
					if (attributes & kWindowBoundsChangeSizeChanged)
					{
						err = GetEventParameter(inEvent, kEventParamOriginalBounds, typeQDRectangle,
						                        NULL, sizeof(Rect), NULL, &origBounds);
            
						err = GetEventParameter(inEvent, kEventParamCurrentBounds, typeQDRectangle,
						                        NULL, sizeof(Rect), NULL, &currBounds);
            
						// Get the clock control
									err = GetControlByID( window, &kWasabiID, &control );
            
						// Hide it (to avoid remnants when downsizing)
									//HIViewSetVisible( control, false );
            
						// Resize it
									HIViewGetFrame( control, &bounds );
#define QDRECTWIDTH(R)	((R).right - (R).left)
#define QDRECTHEIGHT(R)	((R).bottom - (R).top)

									bounds.size.width += QDRECTWIDTH( currBounds ) - QDRECTWIDTH( origBounds );
									bounds.size.height += QDRECTHEIGHT( currBounds ) - QDRECTHEIGHT( origBounds );
									HIViewSetFrame( control, &bounds );
            
						// Show it
									//HIViewSetVisible( control, true );

					}
          err = noErr;
				}
          break;
        case kEventWindowInit:
          break;
			}
			break;
      
	}
  
	return err;
}
OSStatus MyControlEventHandler(EventHandlerCallRef	inHandlerCallRef, EventRef inEvent, void *inUserData)
{
  HICommand hiCommand;
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(inEvent);
	UInt32 eventKind = GetEventKind(inEvent);
	ifc_window *wasabiWnd = (ifc_window *)inUserData;
  switch(eventClass)
  {
    case kEventClassControl:
      switch(eventKind)
      {

        case kEventControlDraw:
        {
          CGContextRef context;
          RgnHandle rgn;
          HIShapeRef shape;
          HIViewRef view;
          if (GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context) == noErr
              && GetEventParameter(inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn) == noErr
               && GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(view), NULL, &view) == noErr)
          {
          GetEventParameter(inEvent, kEventParamShape, typeHIShapeRef, NULL, sizeof(shape), NULL, &shape);
            
            HIRect bounds;
            HIViewGetBounds(view, &bounds);
                        
          Canvas canvas(context);
//          RegionI region(rgn);
          RegionI region(shape);

          //((RootWndI *)wasabiWnd)->rootwnd_paintTree(&canvas, &region); 
          wasabiWnd->paint(&canvas, &region); // TODO: should be virtualOnPaint()

          err = noErr;            
          }
        }
          break;
      }
      break;
  }
  return err;
}
#endif

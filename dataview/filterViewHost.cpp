#include "main.h"
#include "./filterViewHost.h"
#include "./filterViewController.h"
#include "./filterChain.h"
#include "./viewContents.h"
#include "./config.h"
#include "./dataViewHost.h"

#include "./listView.h"

#include "./ifc_viewwindow.h"
#include "./ifc_viewgroupfilter.h"
#include "./ifc_reflectedmessageproc.h"

#include "../nu/vector.h"

typedef Vector<HWND> WindowList;

typedef struct FilterViewHost
{
	WindowList windows;
	HFONT font;
	COLORREF backColor;
	LengthUnit sizerWidth;
	FilterChain *filterChain;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;
} FilterViewHost;

typedef struct FilterViewHostCreateParam
{
	FilterChain *filterChain;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;
} FilterViewHostCreateParam;

#define FILTERVIEWHOST(_hwnd) ((FilterViewHost*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define FILTERVIEWHOST_RET_VOID(_self, _hwnd) {(_self) = FILTERVIEWHOST((_hwnd)); if (NULL == (_self)) return;}
#define FILTERVIEWHOST_RET_VAL(_self, _hwnd, _error) {(_self) = FILTERVIEWHOST((_hwnd)); if (NULL == (_self)) return (_error);}

static LRESULT CALLBACK 
FilterViewHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

static ATOM 
FilterViewHost_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, FILTERVIEWHOST_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = CS_DBLCLKS;
	klass.lpfnWndProc = FilterViewHost_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(FilterViewHost*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = FILTERVIEWHOST_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

HWND 
FilterViewHost_CreateWindow(FilterChain *filterChain, ifc_viewconfig *config, ifc_viewcontroller *controller,
							HWND parentWindow, int x, int y, int width, int height, int controlId)
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;
	FilterViewHostCreateParam param;
		
	if (NULL == filterChain)
		return NULL;

	if (NULL == parentWindow || FALSE == IsWindow(parentWindow))
		return NULL;

	instance = Plugin_GetInstance();
	klassAtom = FilterViewHost_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;

	param.config = config;
	param.controller = controller;
	param.filterChain = filterChain;
	
	hwnd = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTROLPARENT, 
						  (LPCWSTR)MAKEINTATOM(klassAtom), NULL,
						  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
						  x, y, width, height, 
						  parentWindow, (HMENU)controlId, instance, &param);

	return hwnd;
}

static void
FilterViewHost_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	FilterViewHost *self;
		
	FILTERVIEWHOST_RET_VOID(self, hwnd);
		
	if (FALSE != erase)
	{
		COLORREF prevBkColor;
		
		prevBkColor = SetBkColor(hdc, self->backColor);
		
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, paintRect, NULL, 0, NULL);
		
		SetBkColor(hdc, prevBkColor);
	}
}

static void 
FilterViewHost_UpdateColors(HWND hwnd, BOOL redraw)
{
	FilterViewHost *self;
	Theme *theme;

	FILTERVIEWHOST_RET_VOID(self, hwnd);
	
	if (SUCCEEDED(Plugin_GetTheme(&theme)))
	{
		self->backColor = theme->GetColor(WADLG_ITEMBG);
		theme->Release();
	}
}

static void 
FilterViewHost_Layout(HWND hwnd, BOOL redraw)
{
	FilterViewHost *self;
	RECT clientRect;
	long filterWidth;
	long x, y, width;
	size_t index, windowCount;
	HWND controlWindow;

	
	FILTERVIEWHOST_RET_VOID(self, hwnd);

	GetClientRect(hwnd, &clientRect);
	
	windowCount = self->windows.size();
	filterWidth = RECTWIDTH(clientRect);
	if (0 != windowCount)
	{
		filterWidth -= (windowCount/2) * 6/*self->sizerWidth*/;
		filterWidth = filterWidth / ((windowCount + 1)/2);

		x = clientRect.left;
		y = clientRect.top;

		for (index = 0; index < windowCount; index++)
		{
			controlWindow = self->windows[index];
			
			if((index + 1) == windowCount)
				width = RECTWIDTH(clientRect) - x;
			else
				width = (0 == (index%2)) ? filterWidth : 6;

			SetWindowPos(controlWindow, NULL, 
						 x, y, width, RECTHEIGHT(clientRect), 
						 SWP_NOACTIVATE | SWP_NOZORDER);

			x += width;
		}
	}
}

static void CALLBACK 
FilterViewHost_SizerMovedCb(HWND sizerWindow, int position, LPARAM param)
{

}

static BOOL
FilterViewHost_ReflectMessage(HWND hwnd, HWND targetWindow, unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
	HRESULT hr;
	ifc_viewwindow *window;
	ifc_reflectedmessageproc *messageProc;
	

	if (FAILED(ViewWindow::GetObject(targetWindow, &window)))
		return FALSE;

	if (SUCCEEDED(window->QueryInterface(IFC_ReflectedMessageProc, (void**)&messageProc)))
	{
		hr = messageProc->ReflectedMessage(message, wParam, lParam, result);
		messageProc->Release();
	}
	else
		hr = E_FAIL;

	window->Release();
	return (S_OK == hr);
}

static size_t
FilterViewHost_InsertFilterWindow(HWND hwnd, HWND filterWindow, size_t position)
{
	FilterViewHost *self;
	size_t windowCount, filterCount;

	FILTERVIEWHOST_RET_VAL(self, hwnd, (size_t)-1);

	if (NULL == filterWindow)
		return (size_t)-1;

	windowCount = self->windows.size();
	filterCount = (windowCount + 1) / 2;

	if (position > filterCount)
		position = filterCount;

	if (windowCount > 0)
	{
		HWND sizerWindow;
		sizerWindow = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Static", NULL, 
						WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_ETCHEDFRAME,
						0, 0, 0, 0, hwnd, NULL, Plugin_GetInstance(), NULL);

		if (NULL == sizerWindow)
			return (size_t)-1;
		
		Plugin_SetControlTheme(sizerWindow, SKINNEDWND_TYPE_DIVIDER, 
							   SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWDIV_VERT);
				
		MLSkinnedDivider_SetCallback(sizerWindow, FilterViewHost_SizerMovedCb, 0L);

		if (position == filterCount)
			self->windows.push_back(sizerWindow);
		else
		{
			size_t index;
			index = position*2;
			if (index > 0)
				index--;
			self->windows.insert(index, sizerWindow);
		}
	}

	if (position == filterCount)
		self->windows.push_back(filterWindow);
	else
		self->windows.insert(position*2, filterWindow);

	return position;
}

static size_t
FilterViewHost_InsertFilter(HWND hwnd, ifc_viewfilter *filter, size_t position)
{
	HRESULT hr;
	FilterViewHost *self;
	ifc_viewgroupfilter *groupFilter;
	ifc_dataprovider *provider;
	size_t insertedPosition;
	
	FILTERVIEWHOST_RET_VAL(self, hwnd, (size_t)-1);

	if (NULL == filter ||
		FAILED(filter->QueryInterface(IFC_ViewGroupFilter, (void**)&groupFilter)))
	{
		return (size_t)-1;
	}
	
	insertedPosition = (size_t)-1;

	hr = groupFilter->GetProvider(&provider);
	if (SUCCEEDED(hr))
	{
		FilterViewController *controller;

		hr = FilterViewController::CreateInstance(filter, self->controller, &controller);
		if (SUCCEEDED(hr))
		{
			ViewContents *viewContents;

			hr = ViewContents::CreateInstance(filter->GetName(), provider, self->config, 
				  							  controller, filter, &viewContents);
			if (SUCCEEDED(hr))
			{
				ifc_dataobjectlist *objects;
				hr = groupFilter->GetObjects(&objects);
				if (SUCCEEDED(hr))
				{
					hr = viewContents->AttachObjects(objects);
					if (SUCCEEDED(hr))
					{
						HWND filterWindow;

						filterWindow = ListView::CreateInstance(viewContents, 0,
											WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
											hwnd, 0, 0, 0, 0, 0);
						
						if (NULL != filterWindow)
						{
							HWND parentWindow;
							ifc_performancetimer *performanceTimer;

							insertedPosition = FilterViewHost_InsertFilterWindow(hwnd, filterWindow, position);
							if ((size_t)-1 == insertedPosition)
							{
								DestroyWindow(filterWindow);
								filterWindow = NULL;
							}
							else
							{
								viewContents->RegisterEventHandler(controller);
							}

							parentWindow =GetAncestor(hwnd, GA_PARENT);
							if (NULL != parentWindow && 
								FALSE != DATAVIEWHOST_GET_PERFORMANCE_TIMER(parentWindow, &performanceTimer))
							{
								ViewWindow::RegisterPerformanceTimer(filterWindow, performanceTimer);
								performanceTimer->Release();
							}
						}

						if (NULL == filterWindow)
							viewContents->Destroy();
					}

					SafeRelease(objects);
				}

				viewContents->Release();
			}
			controller->Release();
		}
		provider->Release();
	}

	groupFilter->Release();
	return insertedPosition;
}

static size_t
FilterViewHost_LoadFilters(HWND hwnd)
{
	FilterViewHost *self;
	ifc_viewfilter *filter;
	ifc_viewfilterenum *enumerator;
	size_t count;

	FILTERVIEWHOST_RET_VAL(self, hwnd, 0);

	if (NULL == self->filterChain)
		return 0;

	if (FAILED(self->filterChain->EnumerateFilters(&enumerator)))
		return 0;

	count = 0;

	while(S_OK == enumerator->Next(&filter, 1, NULL))
	{
		if ((size_t)-1 != FilterViewHost_InsertFilter(hwnd, filter, count))
			count++;

		filter->Release();
	}
	
	enumerator->Release();
	
	return count;
}

static LRESULT
FilterViewHost_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	FilterViewHost *self;
	FilterViewHostCreateParam *param;
	
	param = (FilterViewHostCreateParam*)createStruct->lpCreateParams;
	if (NULL == param)
		return -1;

	self = new (std::nothrow) FilterViewHost();
	if (NULL == self)
		return -1;
	
	self->config = param->config;
	if (NULL != self->config)
		self->config->AddRef();

	self->controller = param->controller;
	if (NULL != self->controller)
		self->controller->AddRef();

	self->filterChain = param->filterChain;
	if (NULL == self->filterChain)
		return -1;

	self->filterChain->AddRef();

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;
		
	LengthUnit_Set(&self->sizerWidth, 3, UnitType_Dlu);
	self->font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	Plugin_SetControlTheme(hwnd, SKINNEDWND_TYPE_WINDOW, 
						   SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	FilterViewHost_UpdateColors(hwnd, FALSE);
	FilterViewHost_LoadFilters(hwnd);
	return 0;
}

static void
FilterViewHost_OnDestroy(HWND hwnd)
{
	FilterViewHost *self;
	HWND controlWindow;
	ifc_viewwindow *viewWindow;
	ifc_viewcontents *contents;
	size_t index;
		
	self = (FilterViewHost*)(LONG_PTR)SetWindowLongPtr(hwnd, 0, 0);
	if (NULL == self)
		return;

	index = self->windows.size();
	while(index--)
	{
		controlWindow = self->windows[index];
		if (NULL != controlWindow)
		{
			if (SUCCEEDED(ViewWindow::GetObject(controlWindow, &viewWindow)))
			{
				if (SUCCEEDED(viewWindow->GetContents(&contents)))
				{
					ViewContents *viewContents;
					viewContents = (ViewContents*)contents;
					viewContents->Destroy();
					contents->Release();
				}
				viewWindow->Release();
			}
			DestroyWindow(controlWindow);
		}
	}

	SafeRelease(self->config);
	SafeRelease(self->controller);
	SafeRelease(self->filterChain);
	
	delete self;
}

static void
FilterViewHost_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	
	if (NULL != BeginPaint(hwnd, &ps))
	{		
		FilterViewHost_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
FilterViewHost_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		FilterViewHost_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
FilterViewHost_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
	{
		FilterViewHost *self;
		FILTERVIEWHOST_RET_VOID(self, hwnd);

		FilterViewHost_Layout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static void
FilterViewHost_OnContextMenu(HWND hwnd, HWND targetWindow, long cursor_s)
{
	LRESULT result;

	if (FALSE != FilterViewHost_ReflectMessage(hwnd, targetWindow, WM_CONTEXTMENU,
											 (WPARAM)targetWindow, (LPARAM)cursor_s, &result))
	{
		return;
	}
		
	DefWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)targetWindow, (LPARAM)cursor_s);
}

static BOOL
FilterViewHost_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *measureItem)
{
	HWND targetWindow;
	targetWindow = GetDlgItem(hwnd, measureItem->CtlID);

	if (NULL != targetWindow)
	{
		LRESULT result;
		result = 0;
		if (FALSE != FilterViewHost_ReflectMessage(hwnd, targetWindow, WM_MEASUREITEM, 
												 0, (LPARAM)measureItem, &result))
		{
			return result;
		}
	}
		
	return FALSE;
}

static LRESULT
FilterViewHost_OnNotify(HWND hwnd, NMHDR *notification)
{
	LRESULT result;
	
	result = 0;

	if (FALSE != FilterViewHost_ReflectMessage(hwnd, notification->hwndFrom, WM_NOTIFY, 
											 (WPARAM)notification->idFrom, (LPARAM)notification, &result))
	{
		return result;
	}

	return DefWindowProc(hwnd, WM_NOTIFY, (WPARAM)notification->idFrom, (LPARAM)notification);
}


static LRESULT CALLBACK 
FilterViewHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return FilterViewHost_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			FilterViewHost_OnDestroy(hwnd); return 0;
		case WM_PAINT:				FilterViewHost_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		FilterViewHost_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	FilterViewHost_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_CONTEXTMENU:		FilterViewHost_OnContextMenu(hwnd, (HWND)wParam, (long)lParam); return 0;
		case WM_MEASUREITEM:		return FilterViewHost_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_NOTIFY:				return FilterViewHost_OnNotify(hwnd, (NMHDR*)lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#include "main.h"
#include "./dataViewHost.h"
#include "./api_dataview.h"
#include "./filteredObjectList.h"
#include "./filteredObjectListViewFilterHandler.h"
#include "./filterChain.h"
#include "./filterViewHost.h"
#include "./viewContents.h"
#include "./config.h"
#include "./groupFilter.h"
#include "./performanceTimer.h"
#include "./performanceTimerDebugOutput.h"

#include "./toolbarHost.h"
#include "./listView.h"

#include "./ifc_viewwindow.h"
#include "./ifc_reflectedmessageproc.h"

#include <strsafe.h>

#define VIEW_CONFIG_PATH_SUFFIX		L"Plugins\\dataView"
#define VIEW_CONFIG_FILE_PREFIX		L"dataView_"


#define IDC_TOOLBAR				10000
#define IDC_FILTER_HOST			10001
#define IDC_SIZER				10002			
#define IDC_REFINEBAR			10003
#define IDC_RESULT_WINDOW		10004
#define IDC_STATUSBAR			10005

typedef struct DataViewHost
{
	char *name;
	ifc_dataprovider *provider;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;
	FilterChain *filterChain;
	FilteredObjectList *objectList;
	FilteredObjectListViewFilterHandler *objectListFilterHandler;
	ViewContents *resultContents;
	LengthUnit filterHeight;
	LengthUnit sizerHeight;
	COLORREF backColor;
	PerformanceTimer *performanceTimer;

} DataViewHost;

typedef struct DataViewHostCreateParam
{
	const char *name;
	ifc_dataprovider *provider;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;

} DataViewHostCreateParam;

#define DATAVIEWHOST(_hwnd) ((DataViewHost*)(LONGX86)GetWindowLongPtrW((_hwnd), 0))
#define DATAVIEWHOST_RET_VOID(_self, _hwnd) {(_self) = DATAVIEWHOST((_hwnd)); if (NULL == (_self)) return;}
#define DATAVIEWHOST_RET_VAL(_self, _hwnd, _error) {(_self) = DATAVIEWHOST((_hwnd)); if (NULL == (_self)) return (_error);}

static LRESULT CALLBACK 
DataViewHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam);

static ATOM 
DataViewHost_GetClassAtom(HINSTANCE instance)
{
	WNDCLASSEXW klass;
	ATOM klassAtom;

	klassAtom = (ATOM)GetClassInfoExW(instance, DATAVIEWHOST_WINDOW_CLASS, &klass);
	if (0 != klassAtom)
		return klassAtom;

	memset(&klass, 0, sizeof(klass));
	klass.cbSize = sizeof(klass);
	klass.style = CS_DBLCLKS;
	klass.lpfnWndProc = DataViewHost_WindowProc;
	klass.cbClsExtra = 0;
	klass.cbWndExtra = sizeof(DataViewHost*);
	klass.hInstance = instance;
	klass.hIcon = NULL;
	klass.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	klass.hbrBackground = NULL;
	klass.lpszMenuName = NULL;
	klass.lpszClassName = DATAVIEWHOST_WINDOW_CLASS;
	klass.hIconSm = NULL;
	klassAtom = RegisterClassExW(&klass);
	
	return klassAtom;
}

static ifc_viewconfig *
DataViewHost_CreateDefaultConfig(const char *name)
{
	HRESULT hr;
	ifc_viewconfig *config;
	const wchar_t *userPath;
	wchar_t configPath[1024];
	wchar_t *nameUnicode, *cursor;
	size_t remaining;

	if (NULL == WASABI_API_APP)
		return NULL;

	userPath = WASABI_API_APP->path_getUserSettingsPath();
	if (NULL == userPath)
		return NULL;

	nameUnicode = AnsiString_ToUnicode(CP_UTF8, 0, name, -1);
	if (NULL == nameUnicode)
		return NULL;

	cursor = configPath;
	remaining = ARRAYSIZE(configPath);

	hr = StringCchCopyEx(cursor, remaining, L"\\\\?\\", &cursor, &remaining, 0);
		
	if (SUCCEEDED(hr) && cursor > configPath && L'\\' != *(cursor-1) && L'/' != *(cursor-1))
		hr = StringCchCopyEx(cursor, remaining, L"\\", &cursor, &remaining, 0);
	
	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(cursor, remaining, userPath, &cursor, &remaining, 0);

	if (SUCCEEDED(hr) && cursor > configPath && L'\\' != *(cursor-1) && L'/' != *(cursor-1))
		hr = StringCchCopyEx(cursor, remaining, L"\\", &cursor, &remaining, 0);

	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(cursor, remaining, VIEW_CONFIG_PATH_SUFFIX, &cursor, &remaining, 0);

	if (SUCCEEDED(hr) && cursor > configPath && L'\\' != *(cursor-1) && L'/' != *(cursor-1))
		hr = StringCchCopyEx(cursor, remaining, L"\\", &cursor, &remaining, 0);
	
	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(cursor, remaining, VIEW_CONFIG_FILE_PREFIX, &cursor, &remaining, 0);

	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(cursor, remaining, nameUnicode, &cursor, &remaining, 0);

	if (SUCCEEDED(hr))
		hr = StringCchCopyEx(cursor, remaining, L".ini", &cursor, &remaining, 0);

	if (SUCCEEDED(hr))
		hr = Config::CreateInstance(configPath, "View", (Config**)&config);
		
	String_Free(nameUnicode);

	if (FAILED(hr))
		return NULL;

	return config;
}

HWND 
DataViewHost_CreateWindow(const char *name, ifc_dataprovider *provider, 
						  ifc_viewconfig *config, ifc_viewcontroller *controller, 
						  HWND parentWindow, int x, int y, int width, int height, int controlId)
{
	HINSTANCE instance;
	ATOM klassAtom;
	HWND hwnd;
	DataViewHostCreateParam param;
		
	if (NULL == parentWindow || FALSE == IsWindow(parentWindow))
		return NULL;

	if (IS_STRING_EMPTY(name))
		return NULL;

	if (NULL == provider || NULL == controller)
		return NULL;

	instance = Plugin_GetInstance();
	klassAtom = DataViewHost_GetClassAtom(instance);
	if (0 == klassAtom)
		return NULL;

	param.name = name;
	param.provider = provider;
	param.controller = controller;

	
	if (NULL != config)
	{
		if (USE_DEFAULT_CONFIG == config)
			param.config = DataViewHost_CreateDefaultConfig(name);
		else if (FAILED(config->QueryGroup("View", &param.config)))
			param.config = NULL;

		if (NULL == param.config)
			return NULL;
	}
	else
		param.config = NULL;

	
	hwnd = CreateWindowExW(WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY, 
						   (LPCWSTR)MAKEINTATOM(klassAtom), NULL,
						   WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
						   x, y, width, height, 
						   parentWindow, (HMENU)controlId, instance, &param);

	SafeRelease(param.config);

	return hwnd;
}

static BOOL
DataViewHost_ReflectMessage(HWND hwnd, HWND targetWindow, unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
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

static BOOL
DataViewHost_GetClientRect(HWND hwnd, RECT *rect)
{
	if (FALSE == GetClientRect(hwnd, rect))
		return FALSE;

	rect->top += 2;
	rect->right -= 4;

	if (rect->top > rect->bottom)
		rect->top = rect->bottom;

	if (rect->right < rect->left)
		rect->right = rect->left;

	return TRUE;
}

static long
DataViewHost_GetToolbarIdealHeight(HWND hwnd, unsigned int controlId, BOOL checkVisibility)
{
	HWND controlWindow;
	
	controlWindow = GetDlgItem(hwnd, controlId);
	if (NULL == controlWindow)
		return 0;

	if (FALSE != checkVisibility && 
		0 == (WS_VISIBLE & GetWindowStyle(controlWindow)))
	{
		return 0;
	}
	
	return Toolbar_GetIdealHeight(controlWindow);
}

static BOOL
DataViewHost_GetControlRect(HWND hwnd, unsigned int controlId, BOOL checkVisibility, RECT *rect)
{
	HWND controlWindow;
	
	controlWindow = GetDlgItem(hwnd, controlId);
	if (NULL == controlWindow)
		return FALSE;

	if (FALSE != checkVisibility && 
		0 == (WS_VISIBLE & GetWindowStyle(controlWindow)))
	{
		return FALSE;
	}
	
	if (FALSE == GetWindowRect(controlWindow, rect))
		return FALSE;
	
	return TRUE;
}

static BOOL
DataViewHost_GetPanelsRect(HWND hwnd, RECT *rect)
{
	DataViewHost *self;
	
	if (FALSE == DataViewHost_GetClientRect(hwnd, rect))
		return FALSE;

	DATAVIEWHOST_RET_VAL(self, hwnd, FALSE);
	
	return TRUE;
}

static long
DataViewHost_GetMinHeight(HWND hwnd, const LengthConverter *lengthConverter)
{
	DataViewHost *self;
	long height;
	HWND controlWindow;
	MINMAXINFO minMax;

	const unsigned int panels[] = 
	{
		IDC_FILTER_HOST,
		IDC_RESULT_WINDOW,
	};

	const unsigned int toolbars[] =
	{
		IDC_TOOLBAR,
		IDC_STATUSBAR,
	};


	DATAVIEWHOST_RET_VAL(self, hwnd, 0);
	
	height = 0;
	
	
	controlWindow = GetDlgItem(hwnd, IDC_SIZER);
	if (NULL != controlWindow && 
		0 != (WS_VISIBLE & GetWindowStyle(controlWindow)))
	{
		height += (long)LengthUnit_GetVertPx(&self->sizerHeight, lengthConverter);
	}

	for (size_t index = 0; index < ARRAYSIZE(panels); index++)
	{
		controlWindow = GetDlgItem(hwnd, panels[index]);
		if (NULL != controlWindow && 
			0 != (WS_VISIBLE & GetWindowStyle(controlWindow)))
		{
			memset(&minMax, 0, sizeof(minMax));
			SendMessage(controlWindow, WM_GETMINMAXINFO, 0, (LPARAM)&minMax);
		
			if (minMax.ptMinTrackSize.y > 0)
				height += minMax.ptMinTrackSize.y;
		}
	}

	for (size_t index = 0; index < ARRAYSIZE(toolbars); index++)
	{
		height += DataViewHost_GetToolbarIdealHeight(hwnd, toolbars[index], TRUE);
	}

	return height;
}

static long
DataViewHost_GetFilterHeight(HWND hwnd, const LengthConverter *converter)
{
	DataViewHost *self;
	HWND filterWindow, resultWindow;
	MINMAXINFO minMax;
	long filterMin, resultMin, filterHeight;
	
	DATAVIEWHOST_RET_VAL(self, hwnd, 0);
	
	filterWindow = GetDlgItem(hwnd, IDC_FILTER_HOST);
	if (NULL == filterWindow ||
		0 == (WS_VISIBLE & GetWindowStyle(filterWindow)))
	{
		return 0;
	}
	
	memset(&minMax, 0, sizeof(minMax));
	SendMessage(filterWindow, WM_GETMINMAXINFO, 0, (LPARAM)&minMax);
	if (minMax.ptMinTrackSize.y > 0)
		filterMin = minMax.ptMinTrackSize.y;
	else
		filterMin = 0;
		
	resultWindow = GetDlgItem(hwnd, IDC_RESULT_WINDOW);
	if (NULL != resultWindow  &&
		0 != (WS_VISIBLE & GetWindowStyle(resultWindow)))
	{
		memset(&minMax, 0, sizeof(minMax));
		SendMessage(resultWindow, WM_GETMINMAXINFO, 0, (LPARAM)&minMax);
		if (minMax.ptMinTrackSize.y > 0)
			resultMin = minMax.ptMinTrackSize.y;
		else
			resultMin = 0;
	}
	else
	{
		resultWindow = NULL;
		resultMin = 0;
	}

	
	filterHeight = (long)LengthUnit_GetVertPx(&self->filterHeight, converter);
		
	if (((long)converter->blockHeight - filterHeight) < resultMin)
		filterHeight = (long)converter->blockHeight - resultMin;

	if (filterHeight < filterMin)
		filterHeight = filterMin;
	
	return filterHeight;
}

static void
DataViewHost_UpdateFont(HWND hwnd, BOOL redraw)
{	
	LengthConverter lengthConverter;
	HWND controlWindow;
	MLSKINNEDMINMAXINFO minMax;
	
	if (FALSE == LengthConverter_InitFromWindow(&lengthConverter, hwnd))
		return;

	memset(&minMax, 0, sizeof(minMax));

	controlWindow = GetDlgItem(hwnd, IDC_FILTER_HOST);
	if (NULL != controlWindow)
	{
		minMax.min.cy = (long)LengthConverter_EmToPixelsY(&lengthConverter, 3.0f);
		MLSkinnedWnd_SetMinMaxInfo(controlWindow, &minMax); 
	}

	controlWindow = GetDlgItem(hwnd, IDC_RESULT_WINDOW);
	if (NULL != controlWindow)
	{
		minMax.min.cy = (long)LengthConverter_EmToPixelsY(&lengthConverter, 4.0f);
		MLSkinnedWnd_SetMinMaxInfo(controlWindow, &minMax); 
	}

}

static void 
DataViewHost_UpdateColors(HWND hwnd, BOOL redraw)
{
	DataViewHost *self;
	Theme *theme;

	DATAVIEWHOST_RET_VOID(self, hwnd);
	
	if (SUCCEEDED(Plugin_GetTheme(&theme)))
	{
		self->backColor = theme->GetColor(WADLG_WNDBG);
		theme->Release();
	}
}

static void
DataViewHost_Paint(HWND hwnd, HDC hdc, const RECT *paintRect, BOOL erase)
{
	DataViewHost *self;
		
	DATAVIEWHOST_RET_VOID(self, hwnd);
		
	if (FALSE != erase)
	{
		COLORREF prevBkColor;
		
		prevBkColor = SetBkColor(hdc, self->backColor);
		
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, paintRect, NULL, 0, NULL);
		
		SetBkColor(hdc, prevBkColor);
	}
}


static void 
DataViewHost_Layout(HWND hwnd, BOOL redraw)
{
	DataViewHost *self;
	RECT rect;
	HDWP hdwp;
	long left, right, top, bottom;
	long filterHeight, sizerHeight, toolbarHeight, statusbarHeight;
	HWND controlWindow, sizerWindow;
	LengthConverter lengthConverter;
	unsigned int layoutFlags;

	DATAVIEWHOST_RET_VOID(self, hwnd);

	if (FALSE == LengthConverter_InitFromWindow(&lengthConverter, hwnd))
		return;

	if (FALSE == DataViewHost_GetClientRect(hwnd, &rect))
		return;

	layoutFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == redraw)
		layoutFlags |= (SWP_NOREDRAW | SWP_NOCOPYBITS);

	left = rect.left;
	right = rect.right;
	top = rect.top;
	bottom = rect.bottom;

	rect.bottom = rect.top + DataViewHost_GetMinHeight(hwnd, &lengthConverter);
	if (bottom < rect.bottom)
		bottom = rect.bottom;

	lengthConverter.blockHeight = (float)(bottom - top);
	lengthConverter.blockWidth = (float)(right - left);
	
	sizerWindow = GetDlgItem(hwnd, IDC_SIZER);
	if (NULL != sizerWindow &&
		0 != (WS_VISIBLE & GetWindowStyle(sizerWindow)))
	{
		sizerHeight = (long)LengthUnit_GetVertPx(&self->sizerHeight, &lengthConverter);
	}
	else
		sizerHeight = 0;

	toolbarHeight = DataViewHost_GetToolbarIdealHeight(hwnd, IDC_TOOLBAR, TRUE);
	statusbarHeight = DataViewHost_GetToolbarIdealHeight(hwnd, IDC_STATUSBAR, TRUE);
	
	
	lengthConverter.blockHeight -= (sizerHeight + toolbarHeight + statusbarHeight);
	if (lengthConverter.blockHeight < 0.0f)
		lengthConverter.blockHeight = 0.0f;

	filterHeight = DataViewHost_GetFilterHeight(hwnd, &lengthConverter);
				

	hdwp = BeginDeferWindowPos(5);

	controlWindow = GetDlgItem(hwnd, IDC_TOOLBAR);
	if (NULL != hdwp && NULL != controlWindow)
	{
		hdwp = DeferWindowPos(hdwp, controlWindow, NULL, 
							  left, top, right - left, toolbarHeight,
							  layoutFlags);

		top += toolbarHeight;
	}

	controlWindow = GetDlgItem(hwnd, IDC_STATUSBAR);
	if (NULL != hdwp && NULL != controlWindow)
	{
		hdwp = DeferWindowPos(hdwp, controlWindow, NULL, 
							  left, bottom - statusbarHeight, right - left, statusbarHeight,
							  layoutFlags);

		bottom -= statusbarHeight;
	}

	controlWindow = GetDlgItem(hwnd, IDC_FILTER_HOST);
	if (NULL != hdwp && NULL != controlWindow)
	{
		hdwp = DeferWindowPos(hdwp, controlWindow, NULL, 
							  left, top, right - left, filterHeight,
							  layoutFlags);
		top += filterHeight;
	}
			
	if (NULL != hdwp && NULL != sizerWindow)
	{
		hdwp = DeferWindowPos(hdwp, sizerWindow, NULL, 
							  left, top, right - left, sizerHeight,
							  layoutFlags);
		top += sizerHeight;
	}

	controlWindow = GetDlgItem(hwnd, IDC_RESULT_WINDOW);
	if (NULL != hdwp && NULL != controlWindow)
	{
		hdwp = DeferWindowPos(hdwp, controlWindow, NULL, 
							  left, top, right - left, bottom - top,
							  layoutFlags);
	}
	
	if (NULL == hdwp)
		return;
	
	EndDeferWindowPos(hdwp);
}

static long
DataViewHost_AdjustSizerDelta(HWND hwnd, long delta)
{
	DataViewHost *self;
	HWND panelWindow;
	unsigned int panelId;
	long carryOver, direction, height;
	MINMAXINFO minMax;
	RECT rect;
	
	if (0 == delta)
		return 0;

	DATAVIEWHOST_RET_VAL(self, hwnd, 0);
		
	for(size_t run = 0; run < 2; run++)
	{
		switch(run)
		{
			case 0: 	direction = -1; panelId = IDC_FILTER_HOST; break;
			case 1:		direction = 1; panelId = IDC_RESULT_WINDOW; break;
		}				

		carryOver = delta * (-direction);

		if (0 != carryOver)
		{
			panelWindow = GetDlgItem(hwnd, panelId);
			if (NULL != panelWindow &&
				FALSE != GetWindowRect(panelWindow, &rect))
			{
				height = RECTHEIGHT(rect) + carryOver;

				memset(&minMax, 0, sizeof(minMax));
				SendMessage(panelWindow, WM_GETMINMAXINFO, 0, (LPARAM)&minMax);
				
				if (minMax.ptMinTrackSize.y > 0 && 
					height < minMax.ptMinTrackSize.y)
				{
					carryOver = height - minMax.ptMinTrackSize.y;
				}
				else
					carryOver = 0;
			}
		}

		delta -= (carryOver * (-direction));
		if (0 == delta)
			break;
	}

	return delta;
}


static void CALLBACK 
DataViewHost_SizerMovedCb(HWND dividerWindow, int position, LPARAM param)
{
	HWND hwnd;
	DataViewHost *self;
	RECT rect;
	long delta, sizerHeight;
	
	hwnd = GetAncestor(dividerWindow, GA_PARENT);
	if (NULL == hwnd)
		return;
	
	DATAVIEWHOST_RET_VOID(self, hwnd);
	
	if (FALSE == GetWindowRect(dividerWindow, &rect))
		return;

	sizerHeight = RECTHEIGHT(rect);

	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
	delta = position - rect.top;
	if (0 == delta)
		return;
		
	delta = DataViewHost_AdjustSizerDelta(hwnd, delta);

	if (0 != delta &&
		FALSE != DataViewHost_GetPanelsRect(hwnd, &rect))
	{
		HWND filterWindow;
		float panelAreaHeight, filterHeight;

		panelAreaHeight = (float)(RECTHEIGHT(rect) - sizerHeight);
		if (panelAreaHeight < 1.0f)
			panelAreaHeight = 1.0f;
		
		filterWindow = GetDlgItem(hwnd, IDC_FILTER_HOST);
		if (NULL != filterWindow && 
			FALSE != GetWindowRect(filterWindow, &rect))
		{
			filterHeight = (float)(RECTHEIGHT(rect) + delta);
						
			if (filterHeight < 1.0f)
				filterHeight = 1.0f;
			else if (filterHeight > panelAreaHeight)
				filterHeight = panelAreaHeight;

			filterHeight = filterHeight * 100.0f / panelAreaHeight;
			if (filterHeight != self->filterHeight.value)
			{
				LengthUnit_Set(&self->filterHeight, filterHeight, UnitType_Percent);
				DataViewHost_Layout(hwnd, TRUE);
			}
		}
	}
}

static HRESULT
DataViewHost_Bind(HWND hwnd)
{
	DataViewHost *self;
	ViewContents *viewContents;
	HWND controlWindow;
	HRESULT hr;

	DATAVIEWHOST_RET_VAL(self, hwnd, E_INVALIDARG);

	if (NULL != self->objectList)
		self->objectList->Objects_Clear();
	else
	{
		hr = FilteredObjectList::CreateInstance(&self->objectList);
		if (FAILED(hr))
			return hr;
	}
	
	SafeRelease(self->resultContents);

	if (NULL != self->filterChain)
	{
		if (NULL != self->objectListFilterHandler)
			self->filterChain->UnregisterEventHandler(self->objectListFilterHandler);

		self->filterChain->Destroy();
		self->filterChain->Release();
		self->filterChain = NULL;
	}

	SafeRelease(self->objectListFilterHandler);

	if (NULL == self->provider)
		return E_FAIL;

	hr = ViewContents::CreateInstance("Result", self->provider, self->config, 
										self->controller, NULL, &viewContents);
	if (FAILED(hr))
		return hr;

	viewContents->AttachObjects(self->objectList);
	self->resultContents = viewContents;

	

	if (FAILED(FilterChain::CreateInstance(&self->filterChain)))
		self->filterChain = NULL;
	else
	{
		ifc_groupmanager *groupManager;
		ifc_groupprovider *groupProvider;
		GroupFilter *groupFilter;

		if (SUCCEEDED(FilteredObjectListViewFilterHandler::CreateInstance(self->objectList, 
																&self->objectListFilterHandler)))
		{
			if (FAILED(self->filterChain->RegisterEventHandler(self->objectListFilterHandler)))
			{
				self->objectListFilterHandler->Release();
				self->objectListFilterHandler = NULL;
			}
		}
		else
			self->objectListFilterHandler = NULL;

		if (SUCCEEDED(Plugin_GetGroupManager(&groupManager)))
		{
			const char *filters[] = {"Genre", "Artist", "Album"};
			for(size_t index = 0; index < ARRAYSIZE(filters); index++)
			{
				if (S_OK == groupManager->Find(filters[index], &groupProvider))
				{
					if (SUCCEEDED(GroupFilter::CreateInstance(groupProvider, &groupFilter)))
					{
						if (SUCCEEDED(groupFilter->Bind(self->provider)))
						{
							self->filterChain->InsertFilter((size_t)-1, groupFilter);
						}
						groupFilter->Release();
					}
					groupProvider->Release();
				}
			}
			
			groupManager->Release();
		}
		
	}
	
	controlWindow = FilterViewHost_CreateWindow(self->filterChain, self->config, self->controller, 
												hwnd, 0, 0, 0, 0, IDC_FILTER_HOST);

	if (NULL != controlWindow)
	{
		controlWindow = CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"Static", NULL, 
							WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_ETCHEDFRAME,
							0, 0, 0, 0, hwnd, (HMENU)IDC_SIZER, Plugin_GetInstance(), NULL);

		if (NULL != controlWindow)
		{
			Plugin_SetControlTheme(controlWindow, SKINNEDWND_TYPE_DIVIDER,
								   SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT | SWDIV_HORZ);
			
			MLSkinnedDivider_SetCallback(controlWindow, DataViewHost_SizerMovedCb, 0L);
		}
	}
	
	controlWindow = ListView::CreateInstance(self->resultContents, 
									WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
									WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
									hwnd, 0, 0, 0, 0, IDC_RESULT_WINDOW);

	if (NULL == controlWindow)
		return E_FAIL;

	ViewWindow::RegisterPerformanceTimer(controlWindow, self->performanceTimer);


	return S_OK;
}


static void
DataViewHost_ReloadData(HWND hwnd)
{
	DataViewHost *self;
	HRESULT hr;
	ifc_dataobjectenum *enumerator;
	ifc_dataobject **buffer;
	void *data;
	size_t count, fetched, size;

	DATAVIEWHOST_RET_VOID(self, hwnd);

	if (NULL != self->performanceTimer)
		self->performanceTimer->Start();

	self->objectList->Objects_Clear();

	hr = self->provider->Enumerate(&enumerator);
	if (SUCCEEDED(hr))
	{
		count = 0;
		size = 16384;
		buffer = NULL;

		do
		{
			data = realloc(buffer, size * 2 * sizeof(ifc_dataobject*));
			if (NULL == data)
				break;
			
			size = size * 2;
			buffer  = (ifc_dataobject**)data;
			fetched = 0;

			hr = enumerator->Next(buffer + count, size - count, &fetched);
			if (SUCCEEDED(hr))
				count += fetched;
		}
		while (S_OK == hr);
		
		if (count > 0)
		{
			self->objectList->Objects_Add(buffer, count);
			if (NULL != self->filterChain)
			{
				ifc_dataobjectlist *directList;
				if (FAILED(self->objectList->Objects_GetList(&directList)))
				{
					directList = self->objectList;
					directList->AddRef();

				}
				self->filterChain->Init(directList);
				self->filterChain->Update();

				directList->Release();
			}

			while(count--)
			{
				buffer[count]->Release();
			}
		}

		free(buffer);

		enumerator->Release();
	}

	if (NULL != self->performanceTimer)
		self->performanceTimer->Stop();
	
}

static BOOL
DataViewHost_CreateToolbar(HWND hwnd)
{
	DataViewHost *self;
	Toolbar *toolbar;
	HWND toolbarWindow;
	BOOL result;

	DATAVIEWHOST_RET_VAL(self, hwnd, FALSE);

	result = FALSE;

	if (FAILED(Toolbar::CreateInstance("Toolbar", self->config, self->controller, &toolbar)))
		return FALSE;

	toolbarWindow = ToolbarHost_CreateWindow(toolbar, hwnd, 0, 0, 0, 0, IDC_TOOLBAR);
	if (NULL != toolbarWindow)
		result = TRUE;
	
	toolbar->Release();

	return result;
}

static BOOL
DataViewHost_CreateStatusbar(HWND hwnd)
{
	DataViewHost *self;
	Toolbar *toolbar;
	HWND toolbarWindow;
	BOOL result;

	DATAVIEWHOST_RET_VAL(self, hwnd, FALSE);

	result = FALSE;

	if (FAILED(Toolbar::CreateInstance("Statusbar", self->config, self->controller, &toolbar)))
		return FALSE;

	toolbarWindow = ToolbarHost_CreateWindow(toolbar, hwnd, 0, 0, 0, 0, IDC_STATUSBAR);
	if (NULL != toolbarWindow)
		result = TRUE;
	
	toolbar->Release();

	return result;
}

static LRESULT
DataViewHost_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{	
	DataViewHost *self;
	DataViewHostCreateParam *param;
		
	param = (DataViewHostCreateParam*)createStruct->lpCreateParams;
	if (NULL == param)
		return -1;

	self = (DataViewHost*)malloc(sizeof(DataViewHost));
	if (NULL == self)
		return -1;
	
	memset(self, 0, sizeof(DataViewHost));

	self->name = AnsiString_Duplicate(param->name);
	
	self->provider = param->provider;
	if (NULL != self->provider)
		self->provider->AddRef();

	self->config = param->config;
	if (NULL != self->config)
		self->config->AddRef();

	self->controller = param->controller;
	if (NULL != self->controller)
		self->controller->AddRef();

	if (FAILED(PerformanceTimer::CreateInstance(self->name, &self->performanceTimer)))
		self->performanceTimer = NULL;
	else
	{
		PerformanceTimerDebugOutput *timerOutput;
		timerOutput = Plugin_GetPerformanceTimerDebugOutput();
		if (NULL != timerOutput)
			self->performanceTimer->RegisterEventHandler(timerOutput);
	}
 
	LengthUnit_Set(&self->filterHeight, 50.0, UnitType_Percent);
	LengthUnit_Set(&self->sizerHeight, 0.5, UnitType_Em);

	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)self) && ERROR_SUCCESS != GetLastError())
		return -1;
	
	Plugin_SetControlTheme(hwnd, SKINNEDWND_TYPE_WINDOW, 
						   SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	
	if (FALSE == DataViewHost_CreateToolbar(hwnd) ||
		FALSE == DataViewHost_CreateStatusbar(hwnd))
	{
		return -1;
	}
	

	if (FAILED(DataViewHost_Bind(hwnd)))
		return -1;

	DataViewHost_UpdateFont(hwnd, FALSE);
	DataViewHost_UpdateColors(hwnd, FALSE);
	
	DataViewHost_ReloadData(hwnd);
	return 0;
}

static void
DataViewHost_OnDestroy(HWND hwnd)
{
	DataViewHost *self;
	HWND controlWindow;
	const int controlList[] = 
	{ 
		IDC_TOOLBAR, IDC_FILTER_HOST, IDC_SIZER, 
		IDC_REFINEBAR, IDC_RESULT_WINDOW, IDC_STATUSBAR
	};
	
	self = (DataViewHost*)(LONG_PTR)SetWindowLongPtr(hwnd, 0, 0);
	if (NULL == self)
		return;

	for (size_t index = 0; index < ARRAYSIZE(controlList); index++)
	{
		controlWindow = GetDlgItem(hwnd, controlList[index]);
		if (NULL != controlWindow)
			DestroyWindow(controlWindow);
	}

	
	if (NULL != self->resultContents)
	{
		self->resultContents->Destroy();
		self->resultContents->Release();
	}


	if (NULL != self->objectListFilterHandler)
	{
		if (NULL != self->filterChain)
			self->filterChain->UnregisterEventHandler(self->objectListFilterHandler);

		self->objectListFilterHandler->Release();
	}

	SafeRelease(self->objectList);

	if (NULL != self->filterChain)
	{
		self->filterChain->Destroy();
		self->filterChain->Release();
	}

	SafeRelease(self->provider);
	SafeRelease(self->config);
	SafeRelease(self->controller);
	SafeRelease(self->performanceTimer);

	AnsiString_Free(self->name);

	free(self);
}

static void
DataViewHost_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	
	if (NULL != BeginPaint(hwnd, &ps))
	{		
		DataViewHost_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void 
DataViewHost_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	{
		DataViewHost_Paint(hwnd, hdc, &clientRect, TRUE);
	}
}

static void
DataViewHost_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
	{
		DataViewHost *self;
		DATAVIEWHOST_RET_VOID(self, hwnd);

		DataViewHost_Layout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));
	}
}

static void
DataViewHost_OnContextMenu(HWND hwnd, HWND targetWindow, long cursor_s)
{
	LRESULT result;

	if (FALSE != DataViewHost_ReflectMessage(hwnd, targetWindow, WM_CONTEXTMENU,
											 (WPARAM)targetWindow, (LPARAM)cursor_s, &result))
	{
		return;
	}
		
	DefWindowProc(hwnd, WM_CONTEXTMENU, (WPARAM)targetWindow, (LPARAM)cursor_s);
}

static BOOL
DataViewHost_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *measureItem)
{
	HWND targetWindow;
	targetWindow = GetDlgItem(hwnd, measureItem->CtlID);

	if (NULL != targetWindow)
	{
		LRESULT result;
		result = 0;
		if (FALSE != DataViewHost_ReflectMessage(hwnd, targetWindow, WM_MEASUREITEM, 
												 0, (LPARAM)measureItem, &result))
		{
			return result;
		}
	}
		
	return FALSE;
}

static LRESULT
DataViewHost_OnNotify(HWND hwnd, NMHDR *notification)
{
	LRESULT result;
	
	result = 0;

	if (FALSE != DataViewHost_ReflectMessage(hwnd, notification->hwndFrom, WM_NOTIFY, 
											 (WPARAM)notification->idFrom, (LPARAM)notification, &result))
	{
		return result;
	}

	return DefWindowProc(hwnd, WM_NOTIFY, (WPARAM)notification->idFrom, (LPARAM)notification);
}



static BOOL
DataViewHost_OnGetPerformanceTimer(HWND hwnd, ifc_performancetimer **timer)
{
	DataViewHost *self;
	DATAVIEWHOST_RET_VAL(self, hwnd, FALSE);

	if (NULL == timer)
		return FALSE;

	*timer = self->performanceTimer;
	if (NULL == self->performanceTimer)
		return FALSE;

	self->performanceTimer->AddRef();
	return TRUE;
}

static LRESULT CALLBACK 
DataViewHost_WindowProc(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return DataViewHost_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			DataViewHost_OnDestroy(hwnd); return 0;
		case WM_PAINT:				DataViewHost_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		DataViewHost_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_PRINT:				return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	DataViewHost_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOVE:				return 0;
		case WM_CONTEXTMENU:		DataViewHost_OnContextMenu(hwnd, (HWND)wParam, (long)lParam); return 0;
		case WM_MEASUREITEM:		return DataViewHost_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_NOTIFY:				return DataViewHost_OnNotify(hwnd, (NMHDR*)lParam);

		case DATAVIEWHOST_WM_GETPERFORMANCETIMER:	
									return DataViewHost_OnGetPerformanceTimer(hwnd, (ifc_performancetimer**)lParam);
					
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
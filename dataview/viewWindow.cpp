#include "main.h"
#include "./viewWindow.h"

static unsigned int messageGetObject = WM_NULL;

ViewWindow::ViewWindow(HWND _hwnd, const char *_name, ifc_viewcontents *_contents)
	: ref(1), hwnd(_hwnd), name(NULL), contents(_contents), 
	  windowFlags(WindowFlag_None), previousWndProc(NULL), 
	  performanceTimer(NULL)
{
	
	if (NULL != contents)
	{
		ifc_viewgroupfilter *groupFilter;

		contents->AddRef();
		contents->RegisterEventHandler(this);

		if(SUCCEEDED(GetGroupFilter(&groupFilter)))
		{
			groupFilter->RegisterEventHandler(this);
			groupFilter->Release();
		}
	}

	name = AnsiString_Duplicate(_name);

	if (WM_NULL == messageGetObject)
	{
		messageGetObject = RegisterWindowMessage(VIEWWINDOW_WM_GET_OBJECT);
	}
}

ViewWindow::~ViewWindow()
{
	Destroy();
	AnsiString_Free(name);
}

void ViewWindow::Destroy()
{
	if (NULL != contents)
	{
		ifc_viewgroupfilter *groupFilter;

		if(SUCCEEDED(GetGroupFilter(&groupFilter)))
		{
			groupFilter->UnregisterEventHandler(this);
			groupFilter->Release();
		}

		contents->UnregisterEventHandler(this);
		contents->Release();
		contents = NULL;
	}

	SafeRelease(performanceTimer);

	if (NULL != hwnd)
	{
		Plugin_RemoveWindowData(hwnd);

		if (NULL != previousWndProc)
		{
			if (0 != (ViewWindow::WindowFlag_UnicodeWindow & windowFlags))
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
			else
				SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
		}

		windowFlags &= ~ViewWindow::WindowFlag_UnicodeWindow;
		DestroyWindow(hwnd);
		hwnd = NULL;
	}

}

HRESULT ViewWindow::GetObject(HWND hwnd, ifc_viewwindow **window)
{
	if (NULL == window)
		return E_POINTER;

	if (WM_NULL == messageGetObject)
		return E_UNEXPECTED;

	if (FALSE == SendMessage(hwnd, messageGetObject, 0, (LPARAM)window))
		return E_FAIL;

	return S_OK;
}

HRESULT ViewWindow::RegisterPerformanceTimer(HWND hwnd, ifc_performancetimer *timer)
{
	HRESULT hr;
	ifc_viewwindow *windowObject;
	ifc_performanceprovider *performanceProvider;

	hr = ViewWindow::GetObject(hwnd, &windowObject);
	if (SUCCEEDED(hr))
	{
		hr = windowObject->QueryInterface(IFC_PerformanceProvider, (void**)&performanceProvider);
		if (SUCCEEDED(hr))
		{
			hr = performanceProvider->RegisterTimer(timer);
			performanceProvider->Release();
		}
		windowObject->Release();
	}
	return hr;
}

size_t ViewWindow::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ViewWindow::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ViewWindow::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewWindow))
		*object = static_cast<ifc_viewwindow*>(this);
	else if (IsEqualIID(interface_guid, IFC_ReflectedMessageProc))
		*object = static_cast<ifc_reflectedmessageproc*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewContentsEvent))
		*object = static_cast<ifc_viewcontentsevent*>(this);
	else if (IsEqualIID(interface_guid, IFC_PerformanceProvider))
		*object = static_cast<ifc_performanceprovider*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

const char *ViewWindow::GetName()
{
	return name;
}

HRESULT ViewWindow::GetConfig(ifc_viewconfig **config)
{	
	if (NULL != contents)
	{
		HRESULT hr;
		ifc_viewconfig *contentsConfig;
		
		hr = contents->GetConfig(&contentsConfig);
		if (FAILED(hr))
			return hr;
		
		if (NULL == contentsConfig)
		{
			if (NULL == config)
				hr = E_POINTER;
			else
				*config = NULL;
		}
		else
		{
			hr = contentsConfig->QueryGroup(name, config);
			contentsConfig->Release();
		}

		return hr;
	}

	if (NULL == config)
		return E_POINTER;
		
	*config = NULL;

	return S_OK;
}

HRESULT ViewWindow::GetContents(ifc_viewcontents **_contents)
{
	if  (NULL == _contents)
		return E_POINTER;

	*_contents = contents;
	if (NULL == contents)
		return E_UNEXPECTED;

	contents->AddRef();
	return S_OK;
}

HRESULT ViewWindow::AttachWindow()
{
	if (NULL == hwnd)
		return E_UNEXPECTED;

	if (NULL != previousWndProc)
		return E_FAIL;
	
	if (FALSE != IsWindowUnicode(hwnd))
		windowFlags |= WindowFlag_UnicodeWindow;
	else
		windowFlags &= ~WindowFlag_UnicodeWindow;

	SetLastError(ERROR_SUCCESS);
	previousWndProc = (WNDPROC)(LONG_PTR)((0 != (WindowFlag_UnicodeWindow & windowFlags)) ? 
							SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ViewWindow_WindowProc) :
							SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ViewWindow_WindowProc));
	if (NULL == previousWndProc)
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
			return HRESULT_FROM_WIN32(errorCode);
	}
	
	if (FALSE == Plugin_SetWindowData(hwnd, this))
	{
		if (0 != (WindowFlag_UnicodeWindow & windowFlags))
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
		else
			SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);

		return E_FAIL;
	}

	return S_OK;
}

HWND ViewWindow::GetWindow()
{
	return hwnd;
}

BOOL ViewWindow::OnGetObject(ifc_viewwindow **window)
{
	if (NULL == window)
		return FALSE;

	*window = this;
	this->AddRef();

	return TRUE;
}

void ViewWindow::OnRedrawEnabled(BOOL enabled)
{
	PreviousWindowProc(WM_SETREDRAW, (WPARAM)enabled, 0L);
}

void ViewWindow::OnContextMenu(HWND targetWindow, long cursor)
{
	PreviousWindowProc(WM_CONTEXTMENU, (WPARAM)targetWindow, (LPARAM)cursor);
}


LRESULT ViewWindow::PreviousWindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	return (0 != (WindowFlag_UnicodeWindow & windowFlags)) ? 
			CallWindowProcW(previousWndProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(previousWndProc, hwnd, uMsg, wParam, lParam);
}


LRESULT ViewWindow::WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_SETREDRAW:
			OnRedrawEnabled((BOOL)wParam);
			return 0;
		case WM_CONTEXTMENU:
			OnContextMenu((HWND)wParam, (long)lParam);
			return 0;
	}

	if (uMsg == messageGetObject && WM_NULL != messageGetObject)
	{
		return OnGetObject((ifc_viewwindow**)lParam);
	}

	return PreviousWindowProc(uMsg, wParam, lParam);
}

HRESULT ViewWindow::ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
	return E_NOTIMPL;
}


HRESULT ViewWindow::RegisterTimer(ifc_performancetimer *timer)
{
	SafeRelease(performanceTimer);

	performanceTimer = timer;
	if (NULL != performanceTimer)
		performanceTimer->AddRef();

	return S_OK;
}

void ViewWindow::ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects)
{
}

void ViewWindow::ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
}

void ViewWindow::ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
}

void ViewWindow::ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
}

void ViewWindow::ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
}

void ViewWindow::ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list)
{
}

void ViewWindow::ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
}


void ViewWindow::ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason)
{
}

void ViewWindow::ContentsEvent_ColumnsChanged(ifc_viewcontents *contents)
{	
}

void ViewWindow::GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled)
{
}

void ViewWindow::UpdateColors()
{
}

void ViewWindow::UpdateFont()
{
}


HRESULT ViewWindow::GetController(ifc_viewcontroller **controller)
{
	if  (NULL == controller)
		return E_POINTER;

	if (NULL == contents)
		return E_UNEXPECTED;

	return contents->GetController(controller);
}

HRESULT ViewWindow::GetSelectionTracker(ifc_viewselection **selection)
{
	if (NULL == contents)
		return E_UNEXPECTED;

	return contents->GetSelectionTracker(selection);
}

HRESULT ViewWindow::GetGroupFilter(ifc_viewgroupfilter **groupFilter)
{
	HRESULT hr;
	ifc_viewcontents *contents;
	ifc_viewfilter *filter;
	
	hr = GetContents(&contents);
	if (FAILED(hr))
		return hr;

	hr = contents->GetFilter(&filter);
	if (S_OK == hr)
	{
		hr = filter->QueryInterface(IFC_ViewGroupFilter, (void**)groupFilter);
		filter->Release();
	}
	else
		hr = E_NOTIMPL;

	contents->Release();

	return hr;
}

HRESULT ViewWindow::GetSummaryObject(ifc_dataobject **object)
{
	HRESULT hr;
	ifc_viewgroupfilter *groupFilter;

	hr = GetGroupFilter(&groupFilter);
	if (SUCCEEDED(hr))
	{
		hr = groupFilter->GetSummaryObject(object);
		groupFilter->Release();
	}

	return hr;
}

static LRESULT CALLBACK 
ViewWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	ViewWindow *self;

	self = (ViewWindow*)Plugin_GetWindowData(hwnd);
	if (NULL == self)
	{
		return (FALSE != IsWindowUnicode(hwnd)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	if (WM_DESTROY == uMsg)
	{
		result = self->WindowProc(uMsg, wParam, lParam);
		self->Destroy();
		self->Release();
	}
	else
	{
		result = self->WindowProc(uMsg, wParam, lParam);
	}
	
	return result;
}

#define CBCLASS ViewWindow
START_MULTIPATCH;
	START_PATCH(MPIID_VIEWWINDOW)
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, ADDREF, AddRef);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, RELEASE, Release);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, API_GETNAME, GetName);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, API_GETCONFIG, GetConfig);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, API_GETCONTENTS, GetContents);
		M_CB(MPIID_VIEWWINDOW, ifc_viewwindow, API_GETWINDOW, GetWindow);
	NEXT_PATCH(MPIID_REFLECTEDMESSAGEPROC)
		M_CB(MPIID_REFLECTEDMESSAGEPROC, ifc_reflectedmessageproc, ADDREF, AddRef);
		M_CB(MPIID_REFLECTEDMESSAGEPROC, ifc_reflectedmessageproc, RELEASE, Release);
		M_CB(MPIID_REFLECTEDMESSAGEPROC, ifc_reflectedmessageproc, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_REFLECTEDMESSAGEPROC, ifc_reflectedmessageproc, API_REFLECTEDMESSAGE, ReflectedMessage);
	NEXT_PATCH(MPIID_VIEWCONTENTSEVENT)
		M_CB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, ADDREF, AddRef);
		M_CB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, RELEASE, Release);
		M_CB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTLISTCHANGED, ContentsEvent_ObjectListChanged);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSADDED, ContentsEvent_ObjectsAdded);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSREMOVED, ContentsEvent_ObjectsRemoved);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSREMOVEDALL, ContentsEvent_ObjectsRemovedAll);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSCHANGED, ContentsEvent_ObjectsChanged);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSUPDATESTARTED, ContentsEvent_ObjectsUpdateStarted);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSUPDATEFINISHED, ContentsEvent_ObjectsUpdateFinished);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_SELECTIONCHANGED, ContentsEvent_SelectionChanged);
		M_VCB(MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_COLUMNSCHANGED, ContentsEvent_ColumnsChanged);
	NEXT_PATCH(MPIID_VIEWGROUPFILTEREVENT)
		M_CB(MPIID_VIEWGROUPFILTEREVENT, ifc_viewgroupfilterevent, ADDREF, AddRef);
		M_CB(MPIID_VIEWGROUPFILTEREVENT, ifc_viewgroupfilterevent, RELEASE, Release);
		M_CB(MPIID_VIEWGROUPFILTEREVENT, ifc_viewgroupfilterevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_VIEWGROUPFILTEREVENT, ifc_viewgroupfilterevent, API_GROUPFILTEREVENT_BYPASSMODECHANGED, GroupFilterEvent_BypassModeChanged);
	NEXT_PATCH(MPIID_PERFORMANCEPROVIDER)
		M_CB(MPIID_PERFORMANCEPROVIDER, ifc_performanceprovider, ADDREF, AddRef);
		M_CB(MPIID_PERFORMANCEPROVIDER, ifc_performanceprovider, RELEASE, Release);
		M_CB(MPIID_PERFORMANCEPROVIDER, ifc_performanceprovider, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_PERFORMANCEPROVIDER, ifc_performanceprovider, API_REGISTERTIMER, RegisterTimer);
	END_PATCH
END_MULTIPATCH;
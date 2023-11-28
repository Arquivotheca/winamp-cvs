#include "main.h"
#include "./filterViewController.h"
#include "./ifc_viewgroupfilter.h"
#include "./ifc_groupprovider.h"


#include <strsafe.h>


FilterViewController::FilterViewController(ifc_viewfilter *_filter, ifc_viewcontroller *_controller)
	: ref(1), filter(_filter), controller(_controller)
{
	if (NULL != filter)
		filter->AddRef();

	if (NULL != controller)
		controller->AddRef();
}

FilterViewController::~FilterViewController()
{
	SafeRelease(filter);
	SafeRelease(controller);
}

HRESULT FilterViewController::CreateInstance(ifc_viewfilter *filter, ifc_viewcontroller *controller, 
								  FilterViewController **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	if (NULL == filter || NULL == controller)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}
		
	*instance = new (std::nothrow) FilterViewController(filter, controller);

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t FilterViewController::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilterViewController::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilterViewController::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewController))
		*object = static_cast<ifc_viewcontroller*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewContentsEvent))
		*object = static_cast<ifc_viewcontentsevent*>(this);
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

void FilterViewController::FreeString(char *string)
{
	AnsiString_Free(string);
}

HRESULT FilterViewController::GetGroupColumn(ifc_viewcontents *contents, ifc_viewcolumn **column)
{
	const char *name;
	ifc_viewgroupfilter *groupFilter;
	
	if (NULL == contents)
		return E_INVALIDARG;

	name = NULL;

	if (SUCCEEDED(filter->QueryInterface(IFC_ViewGroupFilter, (void**)&groupFilter)))
	{
		ifc_groupprovider *groupProvider;
		if (SUCCEEDED(groupFilter->GetGroupProvider(&groupProvider)))
		{
			name = groupProvider->GetName();
			groupProvider->Release();
		}
		groupFilter->Release();
	}

	if (NULL == name)
		name = filter->GetName();

	return contents->FindColumn(name, column);
}

HRESULT FilterViewController::GetPrimaryColumn(ifc_viewcontents *contents, char **columnName)
{
	ifc_viewcolumn *column;
	
	if (S_OK == GetGroupColumn(contents, &column))
	{
		*columnName = AnsiString_Duplicate(Column_GetName(column));
		column->Release();
	}
	else
		*columnName = NULL;

	return S_OK;
}

HRESULT FilterViewController::GetDefaultSort(ifc_viewcontents *contents, char **columnName, BOOL *ascendingOrder)
{
	if (NULL == ascendingOrder)
		return E_POINTER;
	
	*ascendingOrder = TRUE;

	return GetPrimaryColumn(contents, columnName);
	
}

HRESULT FilterViewController::GetDefaultView(ifc_viewcontents *contents, char **viewName)
{
	if (NULL == viewName)
		return E_POINTER;

	*viewName = AnsiString_Duplicate("report");

	return S_OK;
}

HRESULT FilterViewController::GetDefaultColumns(ifc_viewwindow *window, char **columns)
{
	HRESULT hr;
	ifc_viewcolumn *column;
	ifc_viewcontents *contents;
	

	if (NULL == columns)
		return E_POINTER;

	if (NULL == window)
		return E_INVALIDARG;
	
	if (FAILED(window->GetContents(&contents)))
		return E_FAIL;

	if (S_OK == GetGroupColumn(contents, &column))
	{
		size_t columnsSize;

		columnsSize = 256;
		*columns = AnsiString_Malloc(columnsSize);
		if (NULL == *columns)
			return E_OUTOFMEMORY;
		
		hr = StringCchPrintfA(*columns, columnsSize, 
						 "%s,100dlu|NextFilterCount,32dlu|TrackCount,32dlu|Length,34dlu|Size,38dlu", 
						 Column_GetName(column));
		
		column->Release();
	}
	else
		hr = E_FAIL;

	contents->Release();
	
	if (FAILED(hr))
	{
		AnsiString_Free(*columns);
		hr = E_FAIL;
	}
	
	return hr;
}

HRESULT FilterViewController::GetViewController(ifc_viewcontroller **_controller)
{
	if (NULL == _controller)
		return E_POINTER;

	*_controller = controller;
	if (NULL == controller)
		return S_FALSE;

	controller->AddRef();
	return S_OK;
}

void FilterViewController::ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects)
{
}

void FilterViewController::ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
}

void FilterViewController::ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
}

void FilterViewController::ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
}


void FilterViewController::ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{

}

void FilterViewController::ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list)
{
}

void FilterViewController::ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
}

void FilterViewController::ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason)
{
	ifc_viewgroupfilter *groupFilter;
		
	if (SUCCEEDED(filter->QueryInterface(IFC_ViewGroupFilter, (void**)&groupFilter)))
	{
		groupFilter->UpdateSelection(selection, appended, removed);
		groupFilter->Release();
	}
}

void FilterViewController::ContentsEvent_ColumnsChanged(ifc_viewcontents *contents)
{
}

#define CBCLASS FilterViewController
START_MULTIPATCH;
	START_PATCH(MPIID_FC_VIEWCONTROLLER)
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, ADDREF, AddRef);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, RELEASE, Release);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, API_FREESTRING, FreeString);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, API_GETPRIMARYCOLUMN, GetPrimaryColumn);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, API_GETDEFAULTSORT, GetDefaultSort);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, API_GETDEFAULTVIEW, GetDefaultView);
		M_CB(MPIID_FC_VIEWCONTROLLER, ifc_viewcontroller, API_GETDEFAULTCOLUMNS, GetDefaultColumns);
	NEXT_PATCH(MPIID_FC_VIEWCONTENTSEVENT)
		M_CB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, ADDREF, AddRef);
		M_CB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, RELEASE, Release);
		M_CB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTLISTCHANGED, ContentsEvent_ObjectListChanged);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSADDED, ContentsEvent_ObjectsAdded);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSREMOVED, ContentsEvent_ObjectsRemoved);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSREMOVEDALL, ContentsEvent_ObjectsRemovedAll);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSCHANGED, ContentsEvent_ObjectsChanged);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSUPDATESTARTED, ContentsEvent_ObjectsUpdateStarted);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_OBJECTSUPDATEFINISHED, ContentsEvent_ObjectsUpdateFinished);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_SELECTIONCHANGED, ContentsEvent_SelectionChanged);
		M_VCB(MPIID_FC_VIEWCONTENTSEVENT, ifc_viewcontentsevent, API_CONTENTSEVENT_COLUMNSCHANGED, ContentsEvent_ColumnsChanged);
	END_PATCH
END_MULTIPATCH;
	
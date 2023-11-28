#include "main.h"
#include "./viewContents.h"
#include "./columnEnum.h"
#include "./ifc_viewcolumnmanager.h"
#include "./ifc_viewgroupfilter.h"

ViewContents::ViewContents(const char *_name, ifc_dataprovider *_provider, 
						   ifc_viewconfig *_config, ifc_viewcontroller *_controller, 
						   ifc_viewfilter *_filter)
	: ref(1), name(NULL), provider(_provider), config(_config), controller(_controller),
	  primaryColumn(NULL), sortColumn(NULL), sortOrder(SortOrder_Undefined),
	  objectList(NULL), selection(NULL), filter(_filter)
{
	name = AnsiString_Duplicate(_name);

	if (NULL != provider)
	{
		provider->AddRef();
		provider->RegisterEventHandler(this);
	}

	if (NULL != config)
		config->AddRef();

	if (NULL != controller)
		controller->AddRef();

	if (NULL != filter)
		filter->AddRef();

	if (FAILED(Selection::CreateInstance(&selection)))
		selection = NULL;
	else
	{
		selection->RegisterEventHandler(this);
	}
}

ViewContents::~ViewContents()
{
	Destroy();
	AnsiString_Free(name);
}

HRESULT ViewContents::CreateInstance(const char *name, ifc_dataprovider *provider, 
									 ifc_viewconfig *config, ifc_viewcontroller *controller,
									 ifc_viewfilter *filter, ViewContents **instance)
{
	HRESULT hr;
	ViewContents *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;
	
	if (IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	if (NULL == provider)
		return E_INVALIDARG;
	
	if (NULL == controller)
		return E_INVALIDARG;
	
	if (NULL != config)
	{	
		hr = config->QueryGroup(name, &config);
		if (FAILED(hr))
			return hr;
	}
	
	self = new (std::nothrow) ViewContents(name, provider, config, controller, filter);

	SafeRelease(config);

	if (NULL == self)
		return E_OUTOFMEMORY;

	hr = self->Bind();
	if (FAILED(hr))
	{
		self->Release();
		return hr;
	}

	*instance = self;
	return S_OK;
}

size_t ViewContents::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ViewContents::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ViewContents::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewContents))
		*object = static_cast<ifc_viewcontents*>(this);
	else if (IsEqualIID(interface_guid, IFC_DataObjectListEvent))
		*object = static_cast<ifc_dataobjectlistevent*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewSelectionEvent))
		*object = static_cast<ifc_viewselectionevent*>(this);
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

const char *ViewContents::GetName()
{
	return name;
}

HRESULT ViewContents::GetProvider(ifc_dataprovider **_provider)
{
	if (NULL == _provider)
		return E_POINTER;

	(*_provider) = provider;
	
	if (NULL != provider)
		provider->AddRef();

	return S_OK;
}

HRESULT ViewContents::GetConfig(ifc_viewconfig **_config)
{
	if (NULL == _config)
		return E_POINTER;

	(*_config) = config;
	
	if (NULL != config)
		config->AddRef();

	return S_OK;
}

HRESULT ViewContents::GetController(ifc_viewcontroller **_controller)
{
	if (NULL == _controller)
		return E_POINTER;

	(*_controller) = controller;
	
	if (NULL != controller)
		controller->AddRef();

	return S_OK;
}
HRESULT ViewContents::GetSelection(ifc_dataobjectenum **enumerator)
{
	return E_NOTIMPL;
}

HRESULT ViewContents::EnumerateColumns(ifc_viewcolumnenum **enumerator)
{
	return ColumnEnum::CreateInstance(columnList.begin(), columnList.size(), (ColumnEnum**)enumerator);
}

HRESULT ViewContents::FindColumn(const char *name, ifc_viewcolumn **column)
{
	ifc_viewcolumn **cursor;

	if (NULL == column)
		return E_POINTER;

	cursor = Column_SearchByName(name, columnList.begin(), columnList.size());
	if (NULL == cursor)
	{
		*column = NULL;
		return S_FALSE;
	}

	*column = *cursor;
	(*column)->AddRef();

	return S_OK;
}

HRESULT ViewContents::GetPrimaryColumn(ifc_viewcolumn **column)
{
	if (NULL == column)
		return E_POINTER;

	*column = primaryColumn;
	if (NULL == primaryColumn)
		return S_FALSE;

	primaryColumn->AddRef();
	return S_OK;
}

HRESULT ViewContents::GetSortColumn(ifc_viewcolumn **column)
{
	if (NULL == column)
		return E_POINTER;

	*column = sortColumn;
	if (NULL == sortColumn)
		return S_FALSE;

	sortColumn->AddRef();
	return S_OK;
}

HRESULT ViewContents::SetSortColumn(const char *columnName)
{
	HRESULT hr;
	ifc_viewcolumn *column;
	
	if (S_OK != FindColumn(columnName, &column))
		return E_FAIL;

	if (column != sortColumn)
	{
		SafeRelease(sortColumn);

		sortColumn = column;
		sortColumn->AddRef();
		hr = S_OK;
	}
	else
		hr = S_FALSE;
	
	column->Release();
	
	return hr;
}

HRESULT ViewContents::GetSortOrder(SortOrder *order)
{
	if (NULL == order)
		return E_POINTER;

	*order = sortOrder;

	return (SortOrder_Undefined == sortOrder) ? S_FALSE : S_OK;
}

HRESULT ViewContents::SetSortOrder(SortOrder order)
{
	if (SortOrder_Undefined == order)
		return E_INVALIDARG;

	if (order == sortOrder)
		return S_FALSE;

	sortOrder = order;
	return S_OK;
}

HRESULT ViewContents::AttachObjects(ifc_dataobjectlist *list)
{
	ifc_dataobjectlist *prevList;

	prevList = objectList;
	if (NULL != objectList)
		objectList->UnregisterEventHandler(this);
	
	objectList = list;

	if (NULL != objectList)
	{
		objectList->RegisterEventHandler(this);
		objectList->AddRef();
	}

	if (NULL != selection)
		selection->RemoveAll();

	Notify_ObjectListChanged(objectList, prevList);

	SafeRelease(prevList);

	return S_OK;
}

HRESULT ViewContents::Destroy()
{
	size_t index;

	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	if (NULL != selection)
	{
		selection->UnregisterEventHandler(this);
		selection->Release();
		selection = NULL;
	}

	SafeRelease(filter);

	Columns_RemoveAll();

	if (NULL != provider)
	{
		provider->UnregisterEventHandler(this);
		provider->Release();
		provider = NULL;
	}
	
	SafeRelease(controller);

	if (NULL != objectList)
	{
		objectList->UnregisterEventHandler(this);
		objectList->Release();
		objectList = NULL;

	}

	SafeRelease(config);

	return S_OK;
}

HRESULT ViewContents::GetObjects(ifc_dataobjectlist **list)
{
	if (NULL == list)
		return E_POINTER;
	
	if (NULL == objectList)
	{
		*list = NULL;
		return S_FALSE;
	}

	*list = objectList;
	objectList->AddRef();

	return S_OK;
}

HRESULT ViewContents::GetSelectionTracker(ifc_viewselection **tracker)
{
	if (NULL == tracker)
		return E_POINTER;

	*tracker = selection;
	selection->AddRef();
	
	return S_OK;
}

HRESULT ViewContents::GetFilter(ifc_viewfilter **_filter)
{
	if (NULL == _filter)
		return E_POINTER;

	*(_filter) = filter;
	if (NULL == filter)
		return S_FALSE;

	filter->AddRef();
	return S_OK;
}

HRESULT ViewContents::RegisterEventHandler(ifc_viewcontentsevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	eventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT ViewContents::UnregisterEventHandler(ifc_viewcontentsevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = eventHandlerList.size();
	while(index--)
	{
		if (eventHandler == eventHandlerList[index])
		{
			eventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

void ViewContents::Columns_RemoveAll()
{
	size_t index;
	ifc_viewcolumn *column;

	SafeRelease(primaryColumn);
	SafeRelease(sortColumn);

	sortOrder = SortOrder_Undefined;

	index = columnList.size();
	while(index--)
	{
		column = columnList[index];
		SafeRelease(column);
	}

	columnList.clear();
}

HRESULT ViewContents::Bind()
{
	HRESULT hr;
	ifc_viewcolumnmanager *columnManager;
	ifc_viewcolumninfoenum *columnEnum;
	ifc_viewcolumninfo *columnInfo;
	ifc_viewcolumn *column;
	
	Columns_RemoveAll();

	if (NULL == provider)
		return E_UNEXPECTED;
	
	hr = Plugin_GetColumnManager(&columnManager);
	if (FAILED(hr))
		return hr;

	hr = columnManager->Enumerate(&columnEnum);
	if (SUCCEEDED(hr))
	{
		while(S_OK == columnEnum->Next(&columnInfo, 1, NULL))
		{
			if (SUCCEEDED(columnInfo->CreateColumn(provider, &column)))
			{
				columnList.push_back(column);
			}
			columnInfo->Release();
		}
		columnEnum->Release();
	}

	columnManager->Release();

	if (0 == columnList.size())
		return E_FAIL;

	Column_SortByName(columnList.begin(), columnList.size());

	if (NULL != controller)
	{
		char* columnName;
		BOOL ascendingOrder;
		
		if (SUCCEEDED(controller->GetPrimaryColumn(this, &columnName)) && 
			NULL != columnName)
		{			
			if (FAILED(FindColumn(columnName, &primaryColumn)))
				primaryColumn = NULL;
				
			controller->FreeString(columnName);
			
			if (NULL == primaryColumn)
			{
				// if primary column specified and failed to create - binding failed
				Columns_RemoveAll();
				return E_FAIL;
			}
		}

		ascendingOrder = TRUE;
		if (FAILED(controller->GetDefaultSort(this, &columnName, &ascendingOrder)))
		{
			sortColumn = NULL;
			sortOrder = SortOrder_Undefined;
		}
		else
		{
			if (NULL != columnName)
			{			
				if (FAILED(FindColumn(columnName, &sortColumn)))
					sortColumn = NULL;
					
				controller->FreeString(columnName);
			}

			if (NULL != sortColumn)
			{
				if (FALSE != ascendingOrder)
					sortOrder = SortOrder_Ascending;
				else
					sortOrder = SortOrder_Descending;
			}
		}
	}

	return S_OK;
}

void ViewContents::SelectionEvent_Changed(ifc_viewselection *instance, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason)
{
	Notify_SelectionChanged(instance, appended, removed, reason);
}

void ViewContents::ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	selection->Shift(startIndex, (int)count);
	Notify_ObjectsAdded(list, added, count, startIndex);
}

void ViewContents::ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
	selection->Shift(startIndex + count - 1, -((int)count));
	Notify_ObjectsRemoved(list, removed, count, startIndex);
}

void ViewContents::ObjectListEvent_RemovedAll(ifc_dataobjectlist *list)
{
	selection->RemoveAll();
	Notify_ObjectsRemovedAll(list);
}

void ViewContents::ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	Notify_ObjectsChanged(list, changed, count, startIndex);
}

void ViewContents::ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list)
{
	Notify_ObjectsUpdateStarted(list);
}

void ViewContents::ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list)
{
	Notify_ObjectsUpdateFinished(list);
}


void ViewContents::DataProviderEvent_FieldsChanged(ifc_dataprovider *instance)
{
	Bind();
	Notify_ColumnsChanged();
}

void ViewContents::DataProviderEvent_ObjectsChanged(ifc_dataprovider *instance, ifc_dataobject **objects, size_t count)
{
	size_t index;
	LCID localeId;
	ifc_dataobject *object;

	if (NULL == objects)
		return;
	

	localeId = Plugin_GetUserLocaleId();

	while(count--)
	{
		index = objectList->Find(localeId, objects[count]);
		if ((size_t)-1 != index)
		{
			object = objectList->GetItem(index);
			Notify_ObjectsChanged(objectList, &object, 1, index);
		}
	}
}


void ViewContents::Notify_ObjectListChanged(ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectListChanged(this, newObjects, prevObjects);
	}
}

void ViewContents::Notify_ObjectsAdded(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsAdded(this, list, added, count, startIndex);
	}
}

void ViewContents::Notify_ObjectsRemoved(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsRemoved(this, list, removed, count, startIndex);
	}
}

void ViewContents::Notify_ObjectsRemovedAll(ifc_dataobjectlist *list)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsRemovedAll(this, list);
	}
}

void ViewContents::Notify_ObjectsChanged(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsChanged(this, list, changed, count, startIndex);
	}
}

void ViewContents::Notify_ObjectsUpdateStarted(ifc_dataobjectlist *list)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsUpdateStarted(this, list);
	}
}

void ViewContents::Notify_ObjectsUpdateFinished(ifc_dataobjectlist *list)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ObjectsUpdateFinished(this, list);
	}
}

void ViewContents::Notify_SelectionChanged(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason)
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_SelectionChanged(this, selection, appended, removed, reason);
	}
}

void ViewContents::Notify_ColumnsChanged()
{
	size_t index, max;

	max = eventHandlerList.size();
	for(index = 0; index < max; index++)
	{
		eventHandlerList[index]->ContentsEvent_ColumnsChanged(this);
	}

}


#define CBCLASS ViewContents
START_MULTIPATCH;
	START_PATCH(MPIID_CONTENTS_VIEWCONTENTS)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, ADDREF, AddRef)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, RELEASE, Release)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, QUERYINTERFACE, QueryInterface)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETNAME, GetName)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETPROVIDER, GetProvider)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETCONFIG, GetConfig)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETCONTROLLER, GetController)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETSELECTION, GetSelection)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_ENUMERATECOLUMNS, EnumerateColumns)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_FINDCOLUMN, FindColumn)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETPRIMARYCOLUMN, GetPrimaryColumn)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETSORTCOLUMN, GetSortColumn)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_SETSORTCOLUMN, SetSortColumn)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETSORTORDER, GetSortOrder)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_SETSORTORDER, SetSortOrder)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETOBJECTS, GetObjects)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETSELECTIONTRACKER, GetSelectionTracker)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_GETFILTER, GetFilter)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_REGISTEREVENTHANDLER, RegisterEventHandler)
		M_CB(MPIID_CONTENTS_VIEWCONTENTS, ifc_viewcontents, API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
	NEXT_PATCH(MPIID_CONTENTS_DATAOBJECTLISTEVENT)
		M_CB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, ADDREF, AddRef);
		M_CB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, RELEASE, Release);
		M_CB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_ADDDED, ObjectListEvent_Added);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_REMOVED, ObjectListEvent_Removed);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_REMOVEDALL, ObjectListEvent_RemovedAll);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_CHANGED, ObjectListEvent_Changed);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_UPDATESTARTED, ObjectListEvent_UpdateStarted);
		M_VCB(MPIID_CONTENTS_DATAOBJECTLISTEVENT, ifc_dataobjectlistevent, API_OBJECTLISTEVENT_UPDATEFINISHED, ObjectListEvent_UpdateFinished);
	NEXT_PATCH(MPIID_CONTENTS_VIEWSELECTIONEVENT)
		M_CB(MPIID_CONTENTS_VIEWSELECTIONEVENT, ifc_viewselectionevent, ADDREF, AddRef);
		M_CB(MPIID_CONTENTS_VIEWSELECTIONEVENT, ifc_viewselectionevent, RELEASE, Release);
		M_CB(MPIID_CONTENTS_VIEWSELECTIONEVENT, ifc_viewselectionevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_CONTENTS_VIEWSELECTIONEVENT, ifc_viewselectionevent, API_SELECTIONEVENT_CHANGED, SelectionEvent_Changed);
	NEXT_PATCH(MPIID_CONTENTS_DATAPROVIDEREVENT)
		M_CB(MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent, ADDREF, AddRef);
		M_CB(MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent, RELEASE, Release);
		M_CB(MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent, API_DATAPROVIDEREVENT_FIELDSCHANGED, DataProviderEvent_FieldsChanged);
		M_VCB(MPIID_CONTENTS_DATAPROVIDEREVENT, ifc_dataproviderevent, API_DATAPROVIDEREVENT_OBJECTSCHANGED, DataProviderEvent_ObjectsChanged);
	END_PATCH
END_MULTIPATCH;
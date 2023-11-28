#include "main.h"
#include "./filteredObjectListViewFilterHandler.h"


FilteredObjectListViewFilterHandler::FilteredObjectListViewFilterHandler(FilteredObjectList *_objectList)
	: ref(1), objectList(_objectList)
{
	if (NULL != objectList)
		objectList->AddRef();
}

FilteredObjectListViewFilterHandler::~FilteredObjectListViewFilterHandler()
{
	SafeRelease(objectList);
}

HRESULT FilteredObjectListViewFilterHandler::CreateInstance(FilteredObjectList *objectList, 
						  								    FilteredObjectListViewFilterHandler **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == objectList)
		return E_INVALIDARG;

	*instance = new (std::nothrow) FilteredObjectListViewFilterHandler(objectList);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t FilteredObjectListViewFilterHandler::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilteredObjectListViewFilterHandler::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilteredObjectListViewFilterHandler::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewFilterEvent))
		*object = static_cast<ifc_viewfilterevent*>(this);
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


void FilteredObjectListViewFilterHandler::FilterEvent_BeginUpdate(ifc_viewfilter *instance)
{
	objectList->StartUpdate();
}

void FilteredObjectListViewFilterHandler::FilterEvent_EndUpdate(ifc_viewfilter *instance)
{
	objectList->FinishUpdate();
}

void FilteredObjectListViewFilterHandler::FilterEvent_BlockAll(ifc_viewfilter *instance)
{
	objectList->Filter_BlockAll();
}

void FilteredObjectListViewFilterHandler::FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action)
{
	switch(action)
	{
		case ViewFilterAction_Allow:
			objectList->Filter_Allow(objectIndex, count);
			break;

		case ViewFilterAction_Block:
			objectList->Filter_Block(objectIndex, count);
			break;
	}
}

#define CBCLASS FilteredObjectListViewFilterHandler
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_FILTEREVENT_BEGINUPDATE, FilterEvent_BeginUpdate)
VCB(API_FILTEREVENT_ENDUPDATE, FilterEvent_EndUpdate)
VCB(API_FILTEREVENT_BLOCKALL, FilterEvent_BlockAll)
VCB(API_FILTEREVENT_ACTIONCHANGED, FilterEvent_ActionChanged)
END_DISPATCH;
#undef CBCLASS
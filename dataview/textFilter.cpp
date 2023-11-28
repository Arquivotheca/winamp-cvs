#include "main.h"
#include "./textFilter.h"

TextFilter::TextFilter()
	: ref(1), previous(NULL), next(NULL)
{
}

TextFilter::~TextFilter()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	SafeRelease(next);

	if (NULL != previous)
	{
		ifc_viewfilter *filter;
		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->UnregisterEventHandler(this);
			filter->Release();
		}
		previous->Release();
	}
}

HRESULT TextFilter::CreateInstance(TextFilter **instance)
{
	TextFilter *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;
	
	self = new (std::nothrow) TextFilter();
	if (NULL == self)
		return E_OUTOFMEMORY;

	*instance = self;
	return S_OK;
}


size_t TextFilter::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t TextFilter::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int TextFilter::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewFilter))
		*object = static_cast<ifc_viewfilter*>(this);
	else if (IsEqualIID(interface_guid, IFC_DoublyLinkedNode))
		*object = static_cast<ifc_doublylinkednode*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewFilterEvent))
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

const char *TextFilter::GetName()
{
	return "TextFilter";
}

HRESULT TextFilter::Bind(ifc_dataprovider *provider)
{	
	return S_OK;
}


HRESULT TextFilter::Init(ifc_dataobjectlist *objectList)
{	
	return S_OK;
}

HRESULT TextFilter::IsAllowed(size_t objectIndex)
{
	return S_OK;
}

HRESULT TextFilter::RegisterEventHandler(ifc_viewfilterevent *eventHandler)
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

HRESULT TextFilter::UnregisterEventHandler(ifc_viewfilterevent *eventHandler)
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

HRESULT TextFilter::SetPrevious(ifc_doublylinkednode *node)
{
	ifc_viewfilter *filter;

	if (NULL != previous)
	{
		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->UnregisterEventHandler(this);
			filter->Release();
		}

		previous->Release();
	}
	
	previous = node;
	
	if (NULL != previous)
	{
		previous->AddRef();

		if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter)))
		{
			filter->RegisterEventHandler(this);
			filter->Release();
		}
	}
	
	return S_OK;
}

HRESULT TextFilter::SetNext(ifc_doublylinkednode *node)
{
	SafeRelease(next);
	
	next = node;
	
	if (NULL != next)
		next->AddRef();
	
	return S_OK;
}

HRESULT TextFilter::GetPrevious(ifc_doublylinkednode **node)
{
	if (NULL == node)
		return E_POINTER;

	if (NULL == previous)
	{
		*node = NULL;
		return S_FALSE;
	}

	previous->AddRef();
	*node = previous;

	return S_OK;
}

HRESULT TextFilter::GetNext(ifc_doublylinkednode **node)
{
	if (NULL == node)
		return E_POINTER;

	if (NULL == next)
	{
		*node = NULL;
		return S_FALSE;
	}

	next->AddRef();
	*node = next;

	return S_OK;
}


void TextFilter::FilterEvent_BeginUpdate(ifc_viewfilter *instance)
{

}

void TextFilter::FilterEvent_EndUpdate(ifc_viewfilter *instance)
{
	
}

void TextFilter::FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action)
{
	
}

#define CBCLASS TextFilter
START_MULTIPATCH;
	START_PATCH(MPIID_VIEWFILTER)
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, ADDREF, AddRef);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, RELEASE, Release);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_GETNAME, GetName);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_BIND, Bind);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_INIT, Init);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_ISALLOWED, IsAllowed);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_REGISTEREVENTHANDLER, RegisterEventHandler);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_UNREGISTEREVENTHANDLER, UnregisterEventHandler);
	NEXT_PATCH(MPIID_DOUBLYLINKEDNODE)
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, ADDREF, AddRef);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, RELEASE, Release);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_SETPREVIOUS, SetPrevious);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_SETNEXT, SetNext);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_GETPREVIOUS, GetPrevious);
		M_CB(MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode, API_GETNEXT, GetNext);
	NEXT_PATCH(MPIID_VIEWFILTEREVENT)
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, ADDREF, AddRef);
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, RELEASE, Release);
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_BEGINUPDATE, FilterEvent_BeginUpdate);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ENDUPDATE, FilterEvent_EndUpdate);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ACTIONCHANGED, FilterEvent_ActionChanged);
	END_PATCH
END_MULTIPATCH;

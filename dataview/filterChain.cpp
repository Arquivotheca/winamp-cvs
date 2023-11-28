#include "main.h"
#include "./filterChain.h"
#include "./filterChainEnum.h"

FilterChain::FilterChain()
	: ref(1), head(NULL)
{
}

FilterChain::~FilterChain()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	Destroy();
}

HRESULT FilterChain::CreateInstance(FilterChain **instance)
{
	FilterChain *self;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;
	
	self = new (std::nothrow) FilterChain();
	if (NULL == self)
		return E_OUTOFMEMORY;

	*instance = self;
	return S_OK;
}


size_t FilterChain::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilterChain::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilterChain::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewFilter))
		*object = static_cast<ifc_viewfilter*>(this);
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

const char *FilterChain::GetName()
{
	return "FilterChain";
}

HRESULT FilterChain::Bind(ifc_dataprovider *provider)
{
	HRESULT hr;
	ifc_viewfilter *filter;
	ifc_doublylinkednode *cursor;
	
	hr = S_OK;
	cursor = head;
	if (NULL != cursor)
	{
		ifc_doublylinkednode *next;
		cursor->AddRef();
		do
		{
			if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter)))
			{
				if (FAILED(filter->Bind(provider)))
					hr = E_FAIL;

				filter->Release();
			}
			else
				hr = E_FAIL;
								 
			if (FAILED(hr) || FAILED(cursor->GetNext(&next)))
				next = NULL;

			cursor->Release();
			cursor = next;
		}
		while(NULL != cursor);
	}

	return hr;
}

HRESULT FilterChain::Init(ifc_dataobjectlist *objectList)
{
	HRESULT hr;
	ifc_viewfilter *filter;
	ifc_doublylinkednode *cursor;
	
	hr = S_OK;
	cursor = head;
	if (NULL != cursor)
	{
		ifc_doublylinkednode *next;
		cursor->AddRef();
		do
		{
			if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter)))
			{
				if (FAILED(filter->Init(objectList)))
					hr = E_FAIL;

				filter->Release();
			}
			else
				hr = E_FAIL;
								 
			if (FAILED(cursor->GetNext(&next)))
				next = NULL;

			cursor->Release();
			cursor = next;
		}
		while(NULL != cursor);
	}

	return hr;
}

HRESULT FilterChain::IsAllowed(size_t objectIndex)
{
	HRESULT hr;
	ifc_viewfilter *filter;
	ifc_doublylinkednode *cursor;
	
	hr = S_OK;
	cursor = head;
	if (NULL != cursor)
	{
		ifc_doublylinkednode *next;
		cursor->AddRef();
		do
		{
			if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter)))
			{
				if(S_OK != filter->IsAllowed(objectIndex))
					hr = S_FALSE;

				filter->Release();
			}
											 
			if (S_OK != hr || FAILED(cursor->GetNext(&next)))
				next = NULL;

			cursor->Release();
			cursor = next;
		}
		while(NULL != cursor);
	}

	return hr;
}

HRESULT FilterChain::Update()
{
	HRESULT hr;
	ifc_viewfilter *filter;
	ifc_doublylinkednode *cursor;
	
	hr = S_OK;
	cursor = head;
	if (NULL != cursor)
	{
		ifc_doublylinkednode *next;
		cursor->AddRef();
		do
		{
			if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter)))
			{
				if (FAILED(filter->Update()))
					hr = E_FAIL;

				filter->Release();
			}
			else
				hr = E_FAIL;
								 
			if (FAILED(cursor->GetNext(&next)))
				next = NULL;

			cursor->Release();
			cursor = next;
		}
		while(NULL != cursor);
	}
	
	return hr;
}

HRESULT FilterChain::RegisterEventHandler(ifc_viewfilterevent *eventHandler)
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

HRESULT FilterChain::UnregisterEventHandler(ifc_viewfilterevent *eventHandler)
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

void FilterChain::FilterEvent_BeginUpdate(ifc_viewfilter *instance)
{
	size_t index, count;
	count = eventHandlerList.size();

	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->FilterEvent_BeginUpdate(this);
	}

}

void FilterChain::FilterEvent_EndUpdate(ifc_viewfilter *instance)
{
	size_t index, count;
	count = eventHandlerList.size();

	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->FilterEvent_EndUpdate(this);
	}
}

void FilterChain::FilterEvent_BlockAll(ifc_viewfilter *instance)
{
	size_t index, count;
	count = eventHandlerList.size();

	for (index = 0; index < count; index++)
	{
		eventHandlerList[index]->FilterEvent_BlockAll(this);
	}
}

void FilterChain::FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action)
{
	size_t index, handlerCount;
	handlerCount = eventHandlerList.size();

	for (index = 0; index < handlerCount; index++)
	{
		eventHandlerList[index]->FilterEvent_ActionChanged(this, objectIndex, count, action);
	}
}

HRESULT FilterChain::InsertFilter(size_t insertAt, ifc_viewfilter *filter)
{
	ifc_doublylinkednode *cursor, *next, *node;
	
	if (NULL == filter)
		return E_INVALIDARG;

	if (FAILED(filter->QueryInterface(IFC_DoublyLinkedNode, (void**)&node)))
		return E_INVALIDARG;
	

	cursor = (insertAt > 0) ? head : NULL;
	if (NULL != cursor)
	{
		size_t index;

		index = 0;
		cursor->AddRef();
		
		while(insertAt != ++index)
		{														 
			if (FAILED(cursor->GetNext(&next)) || NULL == next)
				break;

			cursor->Release();
			cursor = next;
		}
	}

	if (NULL == cursor)
	{
		node->SetPrevious(NULL);
		node->SetNext(head);
		
		if (NULL != head)
			head->SetPrevious(node);
		else
			filter->RegisterEventHandler(this);

		head = node;
		head->AddRef();
	}
	else
	{	

		if (FAILED(cursor->GetNext(&next)))
			next = NULL;

		if (NULL == next)
		{
			ifc_viewfilter *filter1;
			if(SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter1)))
			{
				filter1->UnregisterEventHandler(this);
				filter1->Release();
			}

			filter->RegisterEventHandler(this);
		}

		node->SetPrevious(cursor);
		node->SetNext(next);
		cursor->SetNext(node);

		if (NULL != next)
		{
			next->SetPrevious(node);
			next->Release();
		}
		
		cursor->Release();
	}
	
	node->Release();
	
	return S_OK;
}

HRESULT FilterChain::RemoveFilter(ifc_viewfilter *filter)
{
	HRESULT hr;
	ifc_viewfilter *filter1;
	ifc_doublylinkednode *cursor, *next, *previous;
	

	if (NULL == filter)
		return E_INVALIDARG;

	if (NULL == head)
		return S_FALSE;

	hr = S_FALSE;
	
	cursor = head;
	cursor->AddRef();
	
	do
	{
		if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter1)))
		{
			if(filter == filter1)
			{
				hr = S_OK;

				if (FAILED(cursor->GetNext(&next)))
					next = NULL;

				if (FAILED(cursor->GetPrevious(&previous)))
					previous = NULL;
				
				if (NULL != next)
					next->SetPrevious(previous);

				if (NULL != previous)
					previous->SetNext(next);

				if (head == cursor)
					head = next;

				if (NULL == next)
				{
					filter1->UnregisterEventHandler(this);
					if (NULL != previous)
					{
						ifc_viewfilter *filter2;
						if (SUCCEEDED(previous->QueryInterface(IFC_ViewFilter, (void**)&filter2)))
						{
							filter2->RegisterEventHandler(this);
							filter2->Release();
						}
					}
				}
				
				SafeRelease(next);
				SafeRelease(previous);				
			}
			filter1->Release();
		}
											 
		if (S_OK == hr || FAILED(cursor->GetNext(&next)))
			next = NULL;

		cursor->Release();
		cursor = next;
	}
	while(NULL != cursor);
	
	return hr;
}

HRESULT FilterChain::EnumerateFilters(ifc_viewfilterenum **enumerator)
{
	return FilterChainEnum::CreateInstance(head, (FilterChainEnum**)enumerator);
}

HRESULT FilterChain::Destroy()
{
	ifc_doublylinkednode *cursor, *next;

	cursor = head;
	head = NULL;

	while (NULL != cursor)
	{
		if (FAILED(cursor->GetNext(&next)))
			next = NULL;
		
		cursor->SetPrevious(NULL);
		cursor->SetNext(NULL);
		
		if (NULL == next)
		{
			ifc_viewfilter *filter;
			if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&filter)))
			{
				filter->UnregisterEventHandler(this);
				filter->Release();
			}
		}

		cursor->Release();
		cursor = next;
	}

	return S_OK;
}

#define CBCLASS FilterChain
START_MULTIPATCH;
	START_PATCH(MPIID_VIEWFILTER)
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, ADDREF, AddRef);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, RELEASE, Release);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_GETNAME, GetName);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_BIND, Bind);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_INIT, Init);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_ISALLOWED, IsAllowed);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_UPDATE, Update);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_REGISTEREVENTHANDLER, RegisterEventHandler);
		M_CB(MPIID_VIEWFILTER, ifc_viewfilter, API_UNREGISTEREVENTHANDLER, UnregisterEventHandler);
	NEXT_PATCH(MPIID_VIEWFILTEREVENT)
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, ADDREF, AddRef);
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, RELEASE, Release);
		M_CB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, QUERYINTERFACE, QueryInterface);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_BEGINUPDATE, FilterEvent_BeginUpdate);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ENDUPDATE, FilterEvent_EndUpdate);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_BLOCKALL, FilterEvent_BlockAll);
		M_VCB(MPIID_VIEWFILTEREVENT, ifc_viewfilterevent, API_FILTEREVENT_ACTIONCHANGED, FilterEvent_ActionChanged);
	END_PATCH
END_MULTIPATCH;

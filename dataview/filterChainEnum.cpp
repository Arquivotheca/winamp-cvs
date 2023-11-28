#include "main.h"
#include "./filterChainEnum.h"


FilterChainEnum::FilterChainEnum(ifc_doublylinkednode *_head)
	: ref(1), head(_head), cursor(_head)
{
	head->AddRef();
	cursor->AddRef();
}

FilterChainEnum::~FilterChainEnum()
{
	SafeRelease(head);
	SafeRelease(cursor);
}

HRESULT FilterChainEnum::CreateInstance(ifc_doublylinkednode *head, FilterChainEnum **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == head)
		return E_INVALIDARG;

	*instance = new (std::nothrow) FilterChainEnum(head);
	if (NULL == *instance)
		return E_POINTER;

	return S_OK;
}


size_t FilterChainEnum::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t FilterChainEnum::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int FilterChainEnum::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewFilterEnum))
		*object = static_cast<ifc_viewfilterenum*>(this);
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

HRESULT FilterChainEnum::Next(ifc_viewfilter **buffer, size_t bufferMax, size_t *fetched)
{
	ifc_doublylinkednode *node;
	size_t copied;

	if (NULL == buffer)
		return E_POINTER;
	
	if (0 == bufferMax) 
		return E_INVALIDARG;

	if (NULL == cursor)
	{
		if (NULL != fetched) 
			*fetched = 0;

		return S_FALSE;
	}

	copied = 0;

	while(bufferMax-- && NULL != cursor)
	{
		if (SUCCEEDED(cursor->QueryInterface(IFC_ViewFilter, (void**)&buffer[copied])))
			copied++;

		if (S_OK != cursor->GetNext(&node))
			node = NULL;
		
		cursor->Release();
		cursor = node;
	}

	if (NULL != fetched) 
		*fetched = copied;

	return ((size_t)-1 == bufferMax) ? S_OK : S_FALSE;
}

HRESULT FilterChainEnum::Reset(void)
{
	if (cursor != head)
	{
		SafeRelease(cursor);
		cursor = head;
		cursor->AddRef();
	}

	return S_OK;
}

HRESULT FilterChainEnum::Skip(size_t count)
{
	HRESULT hr;
	ifc_doublylinkednode *node;

	while(count--)
	{
		hr = cursor->GetNext(&node);
		cursor->Release();

		if (S_OK != hr)
		{
			cursor = NULL;
			break;
		}

		cursor = node;
	}

	return ((size_t)-1 == count) ? S_OK : S_FALSE;
}


#define CBCLASS FilterChainEnum
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS
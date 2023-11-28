#include "main.h"
#include "./testItemAction.h"
#include "./ifc_viewmenuitem.h"
#include "./ifc_viewmenugroup.h"
#include "./ifc_viewactioncontextenum.h"
#include "./menuItem.h"

#include <strsafe.h>

TestItemAction::TestItemAction(const wchar_t *_title, const GUID *_contextId)
	: ref(1)
{
	title = String_Duplicate(_title);
	contextId = *_contextId;
}

TestItemAction::~TestItemAction()
{	
	String_Free(title);
}

HRESULT TestItemAction::CreateInstance(const wchar_t *title, const GUID *contextId, TestItemAction **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (FALSE != IS_STRING_EMPTY(title) || 
		NULL == contextId)
	{
		return E_INVALIDARG;
	}
	
	*instance = new (std::nothrow) TestItemAction(title, contextId);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}


size_t TestItemAction::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t TestItemAction::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int TestItemAction::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewAction))
		*object = static_cast<ifc_viewaction*>(this);
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

HRESULT TestItemAction::GetContextId(GUID *_contextId)
{
	if (NULL == _contextId)
		return E_POINTER;

	*_contextId = contextId;
	return S_OK;
}

HRESULT TestItemAction::Execute(Dispatchable *context, Dispatchable *source, HWND hostWindow)
{
	wchar_t message[2048], contextInfo[256], sourceInfo[256];
	void *iObject;

	
	if (NULL == source)
	{
		StringCchCopy(sourceInfo, ARRAYSIZE(sourceInfo), L"Null");
	}
	else if (SUCCEEDED(source->QueryInterface(IFC_ViewMenuGroup, &iObject)))
	{
		ifc_viewmenugroup  *group;
		ifc_viewmenuitem *item;
		wchar_t displayName[256];

		group = (ifc_viewmenugroup*)iObject;

		if (FAILED(group->QueryInterface(IFC_ViewMenuItem, (void**)&item)))
			item = NULL;

		if (NULL == item ||
			FAILED(item->GetDisplayName(displayName, ARRAYSIZE(displayName))))
		{
			StringCchCopy(displayName, ARRAYSIZE(displayName), L"unknown");
		}
		
		StringCchPrintf(sourceInfo, ARRAYSIZE(sourceInfo),  L"MenuGroup (addr=0x%08X, name=%S, title=%s)", 
			group, ((NULL != item) ? item->GetName() : "unknown"), displayName);

		if (NULL != item)
			item->Release();

		group->Release();
	}
	else if (SUCCEEDED(source->QueryInterface(IFC_ViewMenuItem, &iObject)))
	{
		ifc_viewmenuitem *item;
		wchar_t displayName[256];

		item = (ifc_viewmenuitem*)iObject;

		if (FAILED(item->GetDisplayName(displayName, ARRAYSIZE(displayName))))
				StringCchCopy(displayName, ARRAYSIZE(displayName), L"unknown");
				
		StringCchPrintf(sourceInfo, ARRAYSIZE(sourceInfo),  L"MenuItem (addr=0x%08X, name=%S, title=%s)", 
			item, item->GetName(), displayName);

		item->Release();
	}
	else
	{		
		StringCchPrintf(sourceInfo, ARRAYSIZE(sourceInfo),  L"Unknown (addr=0x%08X)", source);
	}

	if (NULL == context)
	{
		StringCchCopy(contextInfo, ARRAYSIZE(contextInfo), L"Null");
	}
	else if (SUCCEEDED(context->QueryInterface(IFC_ViewWindow, &iObject)))
	{
		ifc_viewwindow *window;
		window = (ifc_viewwindow*)iObject;

		StringCchPrintf(contextInfo, ARRAYSIZE(contextInfo), L"Window (addr=0x%08X, name=%S)", 
						window, window->GetName());

		window->Release();
	}
	else if (SUCCEEDED(context->QueryInterface(IFC_ViewContents, &iObject)))
	{
		ifc_viewcontents *contents;
		contents = (ifc_viewcontents*)iObject;

		StringCchPrintf(contextInfo, ARRAYSIZE(contextInfo), L"Contents (addr=0x%08X, name=%S)", 
						contents, contents->GetName());

		contents->Release();
	}
	else if (SUCCEEDED(context->QueryInterface(IFC_DataObjectEnum, &iObject)))
	{
		size_t count;
		ifc_dataobjectenum *objects;

		objects = (ifc_dataobjectenum*)iObject;

		if (FAILED(objects->GetCount(&count)))
			count = (size_t)-1;

		StringCchPrintf(contextInfo, ARRAYSIZE(contextInfo), L"Selection (addr=0x%08X, count=%u)", 
						objects, count);

		objects->Release();
	}
	else if (SUCCEEDED(context->QueryInterface(IFC_ViewActionContextEnum, &iObject)))
	{
		size_t count;
		ifc_viewactioncontextenum *contextEnum;

		contextEnum = (ifc_viewactioncontextenum*)iObject;

		if (FAILED(contextEnum->GetCount(&count)))
			count = (size_t)-1;

		
		StringCchPrintf(contextInfo, ARRAYSIZE(contextInfo), L"ActionContextEnum (addr=0x%08X, count=%u)", 
						contextEnum, count);

		contextEnum->Release();
	}
	else
	{
		StringCchPrintf(contextInfo, ARRAYSIZE(contextInfo), L"0x%08X", context);
	}

	StringCchPrintf(message, ARRAYSIZE(message), 
		L"Name:\t '%s'\r\nContext:\t %s\r\nSource:\t %s", 
					title, contextInfo, sourceInfo);

	MessageBox(hostWindow, message, L"Test Action Execute", MB_OK | MB_ICONINFORMATION);

	return S_OK;
}

#define CBCLASS TestItemAction
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETCONTEXTID, GetContextId)
CB(API_EXECUTE, Execute)
END_DISPATCH;
#undef CBCLASS
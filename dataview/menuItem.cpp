#include "main.h"
#include "./menuItem.h"

MenuItem::MenuItem(const char *_name, MenuStyle _style, const wchar_t *_displayName, 
				   const wchar_t *_description, ifc_viewaction *_action, MenuState _state)
	: ref(1), name(NULL), style(_style), displayName(NULL), description(NULL), 
	  action(_action), state(_state)
{
	name = AnsiString_Duplicate(_name);
	displayName = ResourceString_Duplicate(_displayName);
	description = ResourceString_Duplicate(_description);

	if (NULL != action)
		action->AddRef();
}


MenuItem::~MenuItem()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();
	
	SafeRelease(action);

	AnsiString_Free(name);
	ResourceString_Free(displayName);
	ResourceString_Free(description);
}

HRESULT MenuItem::CreateInstance(const char *name,
							     MenuStyle style,
							     const wchar_t *displayName,
							     const wchar_t *description,
							     ifc_viewaction *action,
							     MenuState state,
							     MenuItem **instance)
{
	if (NULL == instance)
		return E_POINTER;
	 
	*instance = NULL;

	if (IS_STRING_EMPTY(name) || 
		IS_STRING_EMPTY(displayName))
	{
		return E_INVALIDARG;
	}
	
	*instance = new (std::nothrow) MenuItem(name, style, displayName, description, action, state);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t MenuItem::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t MenuItem::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int MenuItem::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewMenuItem))
		*object = static_cast<ifc_viewmenuitem*>(this);
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

const char *MenuItem::GetName()
{
	return name;
}

HRESULT MenuItem::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == ResourceString_CopyTo(buffer, bufferSize, displayName) &&
		NULL != displayName)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MenuItem::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == ResourceString_CopyTo(buffer, bufferSize, description) &&
		NULL != description)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MenuItem::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return E_NOTIMPL;
}

MenuStyle MenuItem::GetStyle()
{
	return style;
}

MenuState MenuItem::GetState()
{
	return state;
}

HRESULT MenuItem::SetState(MenuState _state, MenuState mask)
{
	MenuState newState;

	newState = state & ~mask;
	newState |= (_state & mask);
	
	if (state == newState)
		return S_FALSE;
	
	state = newState;
	return S_OK;
}

HRESULT MenuItem::GetAction(ifc_viewaction **_action)
{
	if (NULL == _action)
		return E_POINTER;

	*_action = action;
	
	if (NULL == action)
		return S_FALSE;

	action->AddRef();
	return S_OK;
}

HRESULT MenuItem::RegisterEventHandler(ifc_viewmenuitemevent *eventHandler)
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

HRESULT MenuItem::UnregisterEventHandler(ifc_viewmenuitemevent *eventHandler)
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


#define CBCLASS MenuItem
START_DISPATCH;
CB(ADDREF, AddRef);
CB(RELEASE, Release);
CB(QUERYINTERFACE, QueryInterface);
CB(API_GETNAME, GetName);
CB(API_GETDISPLAYNAME, GetDisplayName);
CB(API_GETDESCRIPTION, GetDescription);
CB(API_GETICON, GetIcon);
CB(API_GETSTYLE, GetStyle);
CB(API_GETSTATE, GetState);
CB(API_SETSTATE, SetState);
CB(API_GETACTION, GetAction);
CB(API_REGISTEREVENTHANDLER, RegisterEventHandler);
CB(API_UNREGISTEREVENTHANDLER, UnregisterEventHandler);
END_DISPATCH;

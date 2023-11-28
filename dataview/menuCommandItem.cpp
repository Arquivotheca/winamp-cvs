#include "main.h"
#include "./menuCommandItem.h"

MenuCommandItem::MenuCommandItem(ifc_viewcommand *_command, MenuStyle _style, MenuState _state)
	: ref(1), command(_command), style(_style), state(_state)
{
	if (NULL != command)
		command->AddRef();
}

MenuCommandItem::~MenuCommandItem()
{
	size_t index;
	
	index = eventHandlerList.size();
	while(index--)
	{
		eventHandlerList[index]->Release();
	}
	eventHandlerList.clear();

	SafeRelease(command);
}

HRESULT MenuCommandItem::CreateInstance(ifc_viewcommand *command, 
										MenuStyle style,
										MenuState state,
										MenuCommandItem **instance)
{
	if (NULL == instance)
		return E_POINTER;
	 
	*instance = NULL;

	if (NULL == command)
		return E_INVALIDARG;
	
	*instance = new (std::nothrow) MenuCommandItem(command, style, state);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t MenuCommandItem::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t MenuCommandItem::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int MenuCommandItem::QueryInterface(GUID interface_guid, void **object)
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

const char *MenuCommandItem::GetName()
{
	return command->GetName();
}

HRESULT MenuCommandItem::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return command->GetDisplayName(buffer, bufferSize);
}

HRESULT MenuCommandItem::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	return command->GetDescription(buffer, bufferSize);
}

HRESULT MenuCommandItem::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return command->GetIcon(buffer, bufferSize, width, height);
}

MenuStyle MenuCommandItem::GetStyle()
{
	return style;
}

MenuState MenuCommandItem::GetState()
{
	return state;
}

HRESULT MenuCommandItem::SetState(MenuState _state, MenuState mask)
{
	MenuState newState;

	newState = state & ~mask;
	newState |= (_state & mask);
	
	if (state == newState)
		return S_FALSE;
	
	state = newState;
	return S_OK;
}

HRESULT MenuCommandItem::GetAction(ifc_viewaction **action)
{
	return command->GetAction(action);
}

HRESULT MenuCommandItem::RegisterEventHandler(ifc_viewmenuitemevent *eventHandler)
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

HRESULT MenuCommandItem::UnregisterEventHandler(ifc_viewmenuitemevent *eventHandler)
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


#define CBCLASS MenuCommandItem
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

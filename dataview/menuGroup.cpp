#include "main.h"
#include "./menuGroup.h"

MenuGroup::MenuGroup(const char *_name, const wchar_t *_displayName, const wchar_t *_description, ifc_viewaction *_action)
	: ref(1), name(NULL), displayName(NULL), description(NULL), 
	  state(MenuState_Normal), action(_action)
{
	name = AnsiString_Duplicate(_name);
	displayName = ResourceString_Duplicate(_displayName);
	description = ResourceString_Duplicate(_description);

	if (NULL != action)
		action->AddRef();
}

MenuGroup::~MenuGroup()
{
	size_t index;
	ifc_viewmenuitem *item;

	index = list.size();
	while(index--)
	{
		item = list[index];
		SafeRelease(item);
	}
	list.clear();

	index = itemEventHandlerList.size();
	while(index--)
	{
		itemEventHandlerList[index]->Release();
	}
	itemEventHandlerList.clear();

	index = groupEventHandlerList.size();
	while(index--)
	{
		groupEventHandlerList[index]->Release();
	}
	groupEventHandlerList.clear();

	SafeRelease(action);
	
	AnsiString_Free(name);
	ResourceString_Free(displayName);
	ResourceString_Free(description);
}

HRESULT MenuGroup::CreateInstance(const char *name, const wchar_t *displayName, 
								  const wchar_t *description, ifc_viewaction *action, 
								  MenuGroup **instance)
{
	if (NULL == instance)
		return E_POINTER;
	 
	*instance = NULL;

	if (IS_STRING_EMPTY(name) || 
		IS_STRING_EMPTY(displayName))
	{
		return E_INVALIDARG;
	}

	*instance = new (std::nothrow) MenuGroup(name, displayName, description, action);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t MenuGroup::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t MenuGroup::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int MenuGroup::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewMenuItem))
		*object = static_cast<ifc_viewmenuitem*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewMenuGroup))
		*object = static_cast<ifc_viewmenugroup*>(this);
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

const char *MenuGroup::GetName()
{
	return name;
}

HRESULT MenuGroup::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == ResourceString_CopyTo(buffer, bufferSize, displayName) &&
		NULL != displayName)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MenuGroup::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	if (NULL == ResourceString_CopyTo(buffer, bufferSize, description) &&
		NULL != description)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MenuGroup::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return E_NOTIMPL;
}

MenuStyle MenuGroup::GetStyle()
{
	return MenuStyle_Normal;
}

MenuState MenuGroup::GetState()
{
	return state;
}

HRESULT MenuGroup::SetState(MenuState _state, MenuState mask)
{
	MenuState newState;

	newState = state & ~mask;
	newState |= (_state & mask);
	
	if (state == newState)
		return S_FALSE;
	
	state = newState;
	return S_OK;
}

HRESULT MenuGroup::GetAction(ifc_viewaction **_action)
{
	if (NULL == _action)
		return E_POINTER;

	*_action = action;
	
	if (NULL == action)
		return S_FALSE;

	action->AddRef();
	return S_OK;
}

HRESULT MenuGroup::RegisterItemEventHandler(ifc_viewmenuitemevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = itemEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == itemEventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	itemEventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT MenuGroup::UnregisterItemEventHandler(ifc_viewmenuitemevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = itemEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == itemEventHandlerList[index])
		{
			itemEventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

size_t MenuGroup::GetCount()
{
	return list.size();
}

HRESULT MenuGroup::Get(size_t position, ifc_viewmenuitem **item)
{
	if (NULL == item)
		return E_POINTER;

	if (position >= list.size())
		return E_INVALIDARG;

	*item = list[position];
	(*item)->AddRef();

	return S_OK;
}

size_t MenuGroup::Locate(const char *name, size_t start)
{
	size_t count;

	if (IS_STRING_EMPTY(name))
		return (size_t)-1;

	count = list.size();
	if (start >= count)
		return (size_t)-1;

	for (;start < count; start++)
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, 
										name, -1, 
										list[start]->GetName(), -1))
		{
			return start;
		}
	}

	return (size_t)-1;
}

HRESULT MenuGroup::Insert(size_t position, ifc_viewmenuitem *item)
{
	const char *insertName;
	MenuStyle insertStyle;

	if (NULL == item)
		return E_INVALIDARG;

	insertName = item->GetName();
	if (IS_STRING_EMPTY(insertName))
		return E_INVALIDARG;

	insertStyle = item->GetStyle();
	if (0 == ((MenuStyle_Separator | 
			   MenuStyle_BarBreak | 
			   MenuStyle_Break) & insertStyle))
	{
		if (-1 != Locate(insertName, 0))
			return MENUGROUP_E_DUPLICATE_ITEM;
	}

	if (position >= list.size())
	{
		position = list.size();
		list.push_back(item);
	}
	else
		list.insertBefore(position, item);

	item->AddRef();
	Notify_ItemAdded(item, position);
	return S_OK;
}

HRESULT MenuGroup::Remove(size_t position)
{
	ifc_viewmenuitem *item;

	if (position >= list.size())
		return E_INVALIDARG;

	item = list[position];
	list.eraseindex(position);
	Notify_ItemRemoved(item, position);
	item->Release();

	return S_OK;
}

HRESULT MenuGroup::RemoveAll()
{
	ifc_viewmenuitem *item;
	size_t index;

	index = list.size();
	while(index--)
	{
		item = list[index];
		list.eraseindex(index);
		Notify_ItemRemoved(item, index);
		item->Release();
	}

	return S_OK;
}\

HRESULT MenuGroup::RegisterGroupEventHandler(ifc_viewmenugroupevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = groupEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == groupEventHandlerList[index])
			return S_FALSE;
	}
	
	eventHandler->AddRef();
	groupEventHandlerList.push_back(eventHandler);
	
	return S_OK;
}

HRESULT MenuGroup::UnregisterGroupEventHandler(ifc_viewmenugroupevent *eventHandler)
{
	size_t index;

	if (NULL == eventHandler)
		return E_INVALIDARG;

	index = groupEventHandlerList.size();
	while(index--)
	{
		if (eventHandler == groupEventHandlerList[index])
		{
			groupEventHandlerList.eraseindex(index);
			eventHandler->Release();
			return S_OK;
		}
	}
	
	return S_FALSE;
}

void MenuGroup::Notify_ItemAdded(ifc_viewmenuitem *item, size_t position)
{
	size_t index, count;

	count = groupEventHandlerList.size();
	for (index = 0; index < count; index++)
	{
		groupEventHandlerList[index]->MenuGroupEvent_ItemAdded(this, item, position);
	}
}

void MenuGroup::Notify_ItemRemoved(ifc_viewmenuitem *item, size_t position)
{
	size_t index, count;

	count = groupEventHandlerList.size();
	for (index = 0; index < count; index++)
	{
		groupEventHandlerList[index]->MenuGroupEvent_ItemRemoved(this, item, position);
	}
}

#define CBCLASS MenuGroup
START_MULTIPATCH;
	START_PATCH(MPIID_VIEWMENUITEM)
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, ADDREF, AddRef);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, RELEASE, Release);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETNAME, GetName);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETDISPLAYNAME, GetDisplayName);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETDESCRIPTION, GetDescription);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETICON, GetIcon);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETSTYLE, GetStyle);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETSTATE, GetState);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_SETSTATE, SetState);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, API_GETACTION, GetAction);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, ifc_viewmenuitem::API_REGISTEREVENTHANDLER, RegisterItemEventHandler);
		M_CB(MPIID_VIEWMENUITEM, ifc_viewmenuitem, ifc_viewmenuitem::API_UNREGISTEREVENTHANDLER, UnregisterItemEventHandler);
	NEXT_PATCH(MPIID_VIEWMENUGROUP)
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, ADDREF, AddRef);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, RELEASE, Release);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, QUERYINTERFACE, QueryInterface);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_GETCOUNT, GetCount);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_GET, Get);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_LOCATE, Locate);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_INSERT, Insert);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_REMOVE, Remove);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, API_REMOVEALL, RemoveAll);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, ifc_viewmenugroup::API_REGISTEREVENTHANDLER, RegisterGroupEventHandler);
		M_CB(MPIID_VIEWMENUGROUP, ifc_viewmenugroup, ifc_viewmenugroup::API_UNREGISTEREVENTHANDLER, UnregisterGroupEventHandler);
	END_PATCH
END_MULTIPATCH;

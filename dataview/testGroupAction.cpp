#include "main.h"
#include "./testGroupAction.h"
#include "./ifc_viewmenugroup.h"
#include "./menuItem.h"


TestGroupAction::TestGroupAction(BOOL _extendedMenu)
	: ref(1), extendedMenu(_extendedMenu)
{
}

TestGroupAction::~TestGroupAction()
{	
}

HRESULT TestGroupAction::CreateInstance(BOOL extendedMenu, TestGroupAction **instance)
{
	if (NULL == instance)
		return E_POINTER;
	
	*instance = new (std::nothrow) TestGroupAction(extendedMenu);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}


size_t TestGroupAction::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t TestGroupAction::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int TestGroupAction::QueryInterface(GUID interface_guid, void **object)
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

HRESULT TestGroupAction::GetContextId(GUID *contextId)
{
	if (NULL == contextId)
		return E_POINTER;

	*contextId = IFC_ViewContents;
	return S_OK;
}

HRESULT TestGroupAction::Execute(Dispatchable *context, Dispatchable *source, HWND hostWindow)
{
	ifc_viewmenugroup *group;
	if (SUCCEEDED(source->QueryInterface(IFC_ViewMenuGroup, (void**)&group)))
	{
		group->RemoveAll();
		MenuGroup_InsertItem(group, 0, "item21", L"Item21", MenuStyle_Normal, MenuState_Normal, NULL);
		MenuGroup_InsertItem(group, 1, "item22", L"Item22", MenuStyle_Normal, MenuState_Normal, NULL);
		MenuGroup_InsertItem(group, 2, "item23", L"Item23 (Disabled)", MenuStyle_Normal, MenuState_Disabled, NULL);

		group->Release();
	}

	return S_OK;
}

#define CBCLASS TestGroupAction
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETCONTEXTID, GetContextId)
CB(API_EXECUTE, Execute)
END_DISPATCH;
#undef CBCLASS
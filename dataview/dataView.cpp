#include "main.h"
#include "./dataView.h"
#include "./dataViewHost.h"
#include "./columnManager.h"
#include "./columns/defaultColumns.h"
#include "./groupManager.h"
#include "./groups/defaultGroups.h"

DataView::DataView()
	: ref(1), columnManager(NULL), groupManager(NULL)
{
}

DataView::~DataView()
{
	SafeRelease(columnManager);
	SafeRelease(groupManager);
}

HRESULT DataView::CreateInstance(DataView **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new (std::nothrow) DataView();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DataView::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DataView::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DataView::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, DataViewGUID))
		*object = static_cast<api_dataview*>(this);
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

HWND DataView::CreateWidget(const char *name, ifc_dataprovider *provider, ifc_viewconfig *config,
					  ifc_viewcontroller *controller, HWND parentWindow, int x, int y, int width, 
					  int height, int controlId)
{
	return DataViewHost_CreateWindow(name, provider, config, controller, 
									 parentWindow, x, y, width, height, controlId);
}

HRESULT DataView::GetColumnManager(ifc_viewcolumnmanager **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == columnManager)
	{
		HRESULT hr;
		hr  = ColumnManager::CreateInstance((ColumnManager**)&columnManager);
		if (FAILED(hr))
			return hr;

		RegisterDefaultColumns(columnManager);
	}

	*instance = columnManager;
	columnManager->AddRef();

	return S_OK;
}

HRESULT DataView::GetGroupManager(ifc_groupmanager **instance)
{
	if (NULL == instance)
		return E_POINTER;

	if (NULL == groupManager)
	{
		HRESULT hr;
		hr  = GroupManager::CreateInstance((GroupManager**)&groupManager);
		if (FAILED(hr))
			return hr;

		RegisterDefaultGroups(groupManager);
	}

	*instance = groupManager;
	groupManager->AddRef();

	return S_OK;
}


HRESULT DataView::CreateStringSortKey(LCID localeId, const wchar_t *string, StringSortKeyFlags flags, ifc_sortkey **instance)
{
	return StringSortKey::CreateInstance(localeId, string, flags, (StringSortKey**)instance);
}

#define CBCLASS DataView
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_CREATEWIDGET, CreateWidget)
CB(API_GETCOLUMNMANAGER, GetColumnManager)
CB(API_GETGROUPMANAGER, GetGroupManager)
CB(API_CREATESTRINGSORTKEY, CreateStringSortKey)
END_DISPATCH;
#undef CBCLASS
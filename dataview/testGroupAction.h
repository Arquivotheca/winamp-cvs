#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_TEST_GROUP_ACTION_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_TEST_GROUP_ACTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewaction.h"

class TestGroupAction : public ifc_viewaction
{
protected:
	TestGroupAction(BOOL extendedMenu);
	~TestGroupAction();

public:
	static HRESULT CreateInstance(BOOL extendedMenu, TestGroupAction **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewaction */
	HRESULT GetContextId(GUID *contextId);
	HRESULT Execute(Dispatchable *context, Dispatchable *source, HWND hostWindow);


protected:
	size_t ref;
	BOOL extendedMenu;
	
protected:
	RECVS_DISPATCH;
	
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_TEST_GROUP_ACTION_HEADER
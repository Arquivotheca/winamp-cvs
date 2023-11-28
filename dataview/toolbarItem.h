#ifndef _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewtoolbaritem.h"

class ToolbarItem : public ifc_viewtoolbaritem
{

protected:
	ToolbarItem(const char *name);
	~ToolbarItem();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewtoolbaritem */
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	AppearanceFlags GetAppearance();

protected:
	size_t ref;
	char *name;
	
protected:
	RECVS_DISPATCH;
};

#endif // _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_HEADER
#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {76764A33-B6DA-456c-85CB-708726DFEF54}
static const GUID DataViewGUID = 
{ 0x76764a33, 0xb6da, 0x456c, { 0x85, 0xcb, 0x70, 0x87, 0x26, 0xdf, 0xef, 0x54 } };

#include <bfc/dispatch.h>

#include "./ifc_dataprovider.h"
#include "./ifc_viewconfig.h"
#include "./ifc_viewcontroller.h"
#include "./ifc_viewcolumnmanager.h"
#include "./ifc_groupmanager.h"
#include "./ifc_sortkey.h"

#define USE_DEFAULT_CONFIG	((ifc_viewconfig*)(intptr_t)1)

typedef enum StringSortKeyFlags
{
	StringSortKey_Normal = 0,
	StringSortKey_IgnoreCase = (1 << 0),
	StringSortKey_Trim = (1 << 1),
	StringSortKey_RemoveArticle = (1 << 2),
}StringSortKeyFlags;
DEFINE_ENUM_FLAG_OPERATORS(StringSortKeyFlags);

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) api_dataview : public Dispatchable
{
protected:
	api_dataview() {}
	~api_dataview() {}

public:
	HWND CreateWidget(const char *name,
					  ifc_dataprovider *provider,
					  ifc_viewconfig *config,
					  ifc_viewcontroller *controller,
					  HWND parentWindow, 
					  int x, 
					  int y, 
					  int width, 
					  int height, 
					  int controlId);

	HRESULT GetColumnManager(ifc_viewcolumnmanager **instance);

	HRESULT GetGroupManager(ifc_groupmanager **instance);

	HRESULT CreateStringSortKey(LCID localeId, 
								const wchar_t *string, 
								StringSortKeyFlags flags,
								ifc_sortkey **instance);

public:
	DISPATCH_CODES
	{
		API_CREATEWIDGET = 10,
		API_GETCOLUMNMANAGER = 20,
		API_GETGROUPMANAGER = 30,
		API_CREATESTRINGSORTKEY = 40,
	};
};


inline HWND api_dataview::CreateWidget(const char *name, ifc_dataprovider *provider, 
										  ifc_viewconfig *config, ifc_viewcontroller *controller,
										  HWND parentWindow, int x, int y, int width, int height, int controlId)
{
	return _call(API_CREATEWIDGET, (HWND)NULL, name, provider, config, controller, 
				 parentWindow, x, y, width, height, controlId);
}

inline HRESULT api_dataview::GetColumnManager(ifc_viewcolumnmanager **instance)
{
	return _call(API_GETCOLUMNMANAGER, (HRESULT)E_NOTIMPL, instance);
}

inline HRESULT api_dataview::GetGroupManager(ifc_groupmanager **instance)
{
	return _call(API_GETGROUPMANAGER, (HRESULT)E_NOTIMPL, instance);
}

inline HRESULT api_dataview::CreateStringSortKey(LCID localeId, const wchar_t *string, StringSortKeyFlags flags, ifc_sortkey **instance)
{
	return _call(API_CREATESTRINGSORTKEY, (HRESULT)E_NOTIMPL, localeId, string, flags, instance);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_INTERFACE_HEADER
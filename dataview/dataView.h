#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./api_dataview.h"

class DataView : public api_dataview
{

protected:
	DataView();
	~DataView();

public:
	static HRESULT CreateInstance(DataView **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* api_dataview */
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
	HRESULT CreateStringSortKey(LCID localeId, const wchar_t *string, StringSortKeyFlags flags, ifc_sortkey **instance);

protected:
	size_t ref;
	ifc_viewcolumnmanager *columnManager;
	ifc_groupmanager *groupManager;

protected:
	RECVS_DISPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_HEADER
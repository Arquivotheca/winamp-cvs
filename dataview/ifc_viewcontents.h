#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {E1611ADE-139E-499f-93A3-440C4AFAB791}
static const GUID IFC_ViewContents = 
{ 0xe1611ade, 0x139e, 0x499f, { 0x93, 0xa3, 0x44, 0xc, 0x4a, 0xfa, 0xb7, 0x91 } };

#include <bfc/dispatch.h>

#include "./ifc_dataprovider.h"
#include "./ifc_dataobjectlist.h"
#include "./ifc_viewcolumnenum.h"
#include "./ifc_viewselection.h"
#include "./ifc_viewconfig.h"
#include "./ifc_viewcontentsevent.h"
#include "./ifc_viewfilter.h"

class ifc_viewcontroller;

typedef enum SortOrder
{
	SortOrder_Descending = -1,
	SortOrder_Undefined = 0,
	SortOrder_Ascending = 1,
}SortOrder;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcontents : public Dispatchable
{

protected:
	ifc_viewcontents() {}
	~ifc_viewcontents() {}

public:
	// GetName:
	// returns unique name;
	const char *GetName();
	
	// GetProvider:
	// returns provider associated with contents
	HRESULT GetProvider(ifc_dataprovider **provider);

	// GetConfig:
	// returns config group associated with contents
	HRESULT GetConfig(ifc_viewconfig **config);

	// GetController:
	// returns controller associated with contents
	HRESULT GetController(ifc_viewcontroller **controller);

	// GetSelection:
	// get enumerator of all selected objects
	HRESULT GetSelection(ifc_dataobjectenum **enumerator);

	// EnumerateColumns:
	// enumerates all columns that available with provider
	HRESULT EnumerateColumns(ifc_viewcolumnenum **enumerator);

	// FindColumn:
	// returns column by name. (S_OK  - column found, S_FALSE - column not found, E_XXXX - error).
	HRESULT FindColumn(const char *name, ifc_viewcolumn **column);

	// GetPrimaryColumn:
	// returns primary column if any (if no primary column is set HRESULT will be S_FALSE)
	HRESULT GetPrimaryColumn(ifc_viewcolumn **column);

	// GetSortColumn:
	// returns sort column if any (if no sort column is set HRESULT will be S_FALSE)
	HRESULT GetSortColumn(ifc_viewcolumn **column);

	// SetSortColumn:
	// sets sort column. (function will fail if column is not one of the columns loaded by contents. S_FALSE will be reutrned if column not chnaged)
	HRESULT SetSortColumn(const char *columnName);

	// GetSortOrder:
	// returns sort order (if no sort order is set HRESULT will be S_FALSE)
	HRESULT GetSortOrder(SortOrder *order);

	// SetSortOrder:
	// sets sort order
	HRESULT SetSortOrder(SortOrder order);

	// GetObjects:
	// returns list of currently attached data objects (returns S_FALSE if NULL)
	HRESULT GetObjects(ifc_dataobjectlist **list);
	
	// GetSelectionTracker:
	// returns selection tracker
	HRESULT GetSelectionTracker(ifc_viewselection **selection);

	// GetFilter
	// returns filter object if exist
	HRESULT GetFilter(ifc_viewfilter **filter);

	// RegisterEventHandler:
	//	
	HRESULT RegisterEventHandler(ifc_viewcontentsevent *eventHandler);

	// UnregisterEventHandler:
	//
	HRESULT UnregisterEventHandler(ifc_viewcontentsevent *eventHandler);
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETPROVIDER = 20,
		API_GETCONFIG = 30,
		API_GETCONTROLLER = 40,
		API_GETSELECTION = 50,
		API_ENUMERATECOLUMNS = 60,
		API_FINDCOLUMN = 70,
		API_GETPRIMARYCOLUMN = 80,
		API_GETSORTCOLUMN = 90,
		API_SETSORTCOLUMN = 100,
		API_GETSORTORDER = 110,
		API_SETSORTORDER = 120,
		API_GETOBJECTS	= 130,
		API_GETSELECTIONTRACKER = 140,
		API_GETFILTER = 150,
		API_REGISTEREVENTHANDLER = 160,
		API_UNREGISTEREVENTHANDLER = 170,
	};
};

inline const char *ifc_viewcontents::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewcontents::GetProvider(ifc_dataprovider **provider)
{
	return _call(API_GETPROVIDER, (HRESULT)E_NOTIMPL, provider);
}

inline HRESULT ifc_viewcontents::GetConfig(ifc_viewconfig **config)
{
	return _call(API_GETCONFIG, (HRESULT)E_NOTIMPL, config);
}

inline HRESULT ifc_viewcontents::GetController(ifc_viewcontroller **controller)
{
	return _call(API_GETCONTROLLER, (HRESULT)E_NOTIMPL, controller);
}

inline HRESULT ifc_viewcontents::GetSelection(ifc_dataobjectenum **enumerator)
{
	return _call(API_GETSELECTION, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_viewcontents::EnumerateColumns(ifc_viewcolumnenum **enumerator)
{
	return _call(API_ENUMERATECOLUMNS, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_viewcontents::FindColumn(const char *name, ifc_viewcolumn **column)
{
	return _call(API_FINDCOLUMN, (HRESULT)E_NOTIMPL, name, column);
}

inline HRESULT ifc_viewcontents::GetPrimaryColumn(ifc_viewcolumn **column)
{
	return _call(API_GETPRIMARYCOLUMN, (HRESULT)E_NOTIMPL, column);
}

inline HRESULT ifc_viewcontents::GetSortColumn(ifc_viewcolumn **column)
{
	return _call(API_GETSORTCOLUMN, (HRESULT)E_NOTIMPL, column);
}

inline HRESULT ifc_viewcontents::SetSortColumn(const char *columnName)
{
	return _call(API_SETSORTCOLUMN, (HRESULT)E_NOTIMPL, columnName);
}

inline HRESULT ifc_viewcontents::GetSortOrder(SortOrder *order)
{
	return _call(API_GETSORTORDER, (HRESULT)E_NOTIMPL, order);
}

inline HRESULT ifc_viewcontents::SetSortOrder(SortOrder order)
{
	return _call(API_SETSORTORDER, (HRESULT)E_NOTIMPL, order);
}

inline HRESULT ifc_viewcontents::GetObjects(ifc_dataobjectlist **list)
{
	return _call(API_GETOBJECTS, (HRESULT)E_NOTIMPL, list);
}

inline HRESULT ifc_viewcontents::GetSelectionTracker(ifc_viewselection **selection)
{
	return _call(API_GETSELECTIONTRACKER, (HRESULT)E_NOTIMPL, selection);
}

inline HRESULT ifc_viewcontents::GetFilter(ifc_viewfilter **filter)
{
	return _call(API_GETFILTER, (HRESULT)E_NOTIMPL, filter);
}


inline HRESULT ifc_viewcontents::RegisterEventHandler(ifc_viewcontentsevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewcontents::UnregisterEventHandler(ifc_viewcontentsevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_INTERFACE_HEADER
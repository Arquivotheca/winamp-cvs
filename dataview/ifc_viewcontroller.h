#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTROLLER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTROLLER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {08B6A434-1A6F-4ba0-B6AA-7ED6C5CB9F9D}
static const GUID IFC_ViewController = 
{ 0x8b6a434, 0x1a6f, 0x4ba0, { 0xb6, 0xaa, 0x7e, 0xd6, 0xc5, 0xcb, 0x9f, 0x9d } };


#include <bfc/dispatch.h>
#include "./ifc_viewcontents.h"
#include "./ifc_viewwindow.h"
#include "./ifc_viewcolumnmanager.h"


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcontroller : public Dispatchable
{

protected:
	ifc_viewcontroller() {}
	~ifc_viewcontroller() {}

public:
	// FreeString:
	// frees strings allocated by ifc_viewcontroller
	void FreeString(char *string);

	// GetPrimaryColumn:
	// return name of the primary column (if any). If primary column specified and failed 
	// to bind to data provider - contents binding will fail.
	HRESULT GetPrimaryColumn(ifc_viewcontents *contents, char **columnName);

	// GetDefaultSort:
	// returns default sorting column and order. If column name not specified - primary column used, 
	// if primary column not set - first visible column will be used.
	HRESULT GetDefaultSort(ifc_viewcontents *contents, char **columnName, BOOL *ascendingOrder);

	// GetDefaultView:
	// return name of the default view wiwndow.
	HRESULT GetDefaultView(ifc_viewcontents *contents, char **viewName);

	// GetDefaultColumns:
	// return default columns config for particular view window.
	// columns string format: <name>[,width[unit]],[H|h]
	// name - column name,
	// width - column width
	// unit - column width unit [pt - points, dlu - dialog units, em - em, px or nothing - pixels
	// H (h) - column hidden
	// example: "Artist,100|Album,80em|Length,8dlu|FileSize,8dlu,H", 
	HRESULT GetDefaultColumns(ifc_viewwindow *window, char **columns);

public:
	DISPATCH_CODES
	{
		API_FREESTRING = 10,
		API_GETPRIMARYCOLUMN = 20,
		API_GETDEFAULTSORT = 30,
		API_GETDEFAULTVIEW = 40,
		API_GETDEFAULTCOLUMNS = 50,
	};
};

inline void ifc_viewcontroller::FreeString(char *string)
{
	_voidcall(API_FREESTRING, string);
}

inline HRESULT ifc_viewcontroller::GetPrimaryColumn(ifc_viewcontents *contents, char **columnName)
{
	return _call(API_GETPRIMARYCOLUMN, (HRESULT)E_NOTIMPL, contents, columnName);
}

inline HRESULT ifc_viewcontroller::GetDefaultSort(ifc_viewcontents *contents, char **columnName, BOOL *ascendingOrder)
{
	return _call(API_GETDEFAULTSORT, (HRESULT)E_NOTIMPL, contents, columnName, ascendingOrder);
}

inline HRESULT ifc_viewcontroller::GetDefaultView(ifc_viewcontents *contents, char **viewName)
{
	return _call(API_GETDEFAULTVIEW, (HRESULT)E_NOTIMPL, contents, viewName);
}

inline HRESULT ifc_viewcontroller::GetDefaultColumns(ifc_viewwindow *window, char **columns)
{
	return _call(API_GETDEFAULTCOLUMNS, (HRESULT)E_NOTIMPL, window, columns);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTROLLER_INTERFACE_HEADER
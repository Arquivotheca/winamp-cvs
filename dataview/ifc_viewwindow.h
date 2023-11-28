#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {2DCD1F98-1B16-4dc7-84DE-5AEA21D299DE}
static const GUID IFC_ViewWindow = 
{ 0x2dcd1f98, 0x1b16, 0x4dc7, { 0x84, 0xde, 0x5a, 0xea, 0x21, 0xd2, 0x99, 0xde } };

#define VIEWWINDOW_WM_GET_OBJECT	L"VIEWWINDOW_WM_GET_OBJECT"

#include <bfc/dispatch.h>

#include "./ifc_viewcontents.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewwindow : public Dispatchable
{

protected:
	ifc_viewwindow() {}
	~ifc_viewwindow() {}

public:
	const char *GetName();
	HRESULT GetConfig(ifc_viewconfig **config);
	HRESULT GetContents(ifc_viewcontents **contents);
	HWND GetWindow();
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETCONFIG = 20,
		API_GETCONTENTS = 30,
		API_GETWINDOW = 40,
	};
};

inline const char *ifc_viewwindow::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewwindow::GetConfig(ifc_viewconfig **config)
{
	return _call(API_GETCONFIG, (HRESULT)E_NOTIMPL, config);
}

inline HRESULT ifc_viewwindow::GetContents(ifc_viewcontents **contents)
{
	return _call(API_GETCONTENTS, (HRESULT)E_NOTIMPL, contents);
}

inline HWND ifc_viewwindow::GetWindow()
{
	return _call(API_GETWINDOW, (HWND)NULL);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_INTERFACE_HEADER
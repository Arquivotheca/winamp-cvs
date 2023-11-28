#ifndef _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewtoolbar.h"
#include "./ifc_viewconfig.h"
#include "./ifc_viewcontroller.h"

class Toolbar : public ifc_viewtoolbar
{

protected:
	Toolbar(const char *name, ifc_viewconfig *config, ifc_viewcontroller *controller);
	~Toolbar();

public:
	static HRESULT CreateInstance(const char *name, 
								  ifc_viewconfig *config,
								  ifc_viewcontroller *controller,
								  Toolbar **instance);
public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewtoolbar */
	const char *GetName();
	HRESULT GetConfig(ifc_viewconfig **config);

public:
	HRESULT SetHost(HWND hwnd);
	HWND GetHost();
	void Paint(HDC hdc, const RECT *paintRect, BOOL erase);
	void Layout(BOOL redraw);
	long GetIdealHeight();
	void UpdateColors();

protected:
	size_t ref;
	char *name;
	ifc_viewconfig *config;
	ifc_viewcontroller *controller;
	HWND hwnd;

	COLORREF backColor;
	
protected:
	RECVS_DISPATCH;
};

#endif // _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_WINDOW_HEADER
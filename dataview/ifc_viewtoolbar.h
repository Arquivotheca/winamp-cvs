#ifndef _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {431868EE-6C29-4944-9CAA-6739D143BF06}
static const GUID IFC_ViewToolbar = 
{ 0x431868ee, 0x6c29, 0x4944, { 0x9c, 0xaa, 0x67, 0x39, 0xd1, 0x43, 0xbf, 0x6 } };

#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewtoolbar : public Dispatchable
{

protected:
	ifc_viewtoolbar() {}
	~ifc_viewtoolbar() {}

public:
	const char *GetName();
	HRESULT GetConfig(ifc_viewconfig **config);
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETCONFIG = 20,
	};
};

inline const char *ifc_viewtoolbar::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewtoolbar::GetConfig(ifc_viewconfig **config)
{
	return _call(API_GETCONFIG, (HRESULT)E_NOTIMPL, config);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_INTERFACE_HEADER
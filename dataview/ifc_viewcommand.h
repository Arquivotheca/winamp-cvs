#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COMMAND_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COMMAND_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {102C3E39-DA88-466a-9C86-7F7E9BF9174E}
static const GUID IFC_ViewCommand = 
{ 0x102c3e39, 0xda88, 0x466a, { 0x9c, 0x86, 0x7f, 0x7e, 0x9b, 0xf9, 0x17, 0x4e } };


#include <bfc/dispatch.h>

#include "./ifc_viewaction.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcommand : public Dispatchable
{

protected:
	ifc_viewcommand() {}
	~ifc_viewcommand() {}

public:
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetAction(ifc_viewaction **action);

public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETDISPLAYNAME = 20,
		API_GETDESCRIPTION = 30,
		API_GETICON = 40,
		API_GETACTION = 50,
	};
};

inline const char *ifc_viewcommand::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewcommand::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewcommand::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDESCRIPTION, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewcommand::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, buffer, bufferSize, width, height);
}

inline HRESULT ifc_viewcommand::GetAction(ifc_viewaction **action)
{
	return _call(API_GETACTION, (HRESULT)E_NOTIMPL, action);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COMMAND_INTERFACE_HEADER
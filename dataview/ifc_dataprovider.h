#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {DF029929-0FD6-4307-88FE-339C75A129B4}
static const GUID IFC_DataProvider = 
{ 0xdf029929, 0xfd6, 0x4307, { 0x88, 0xfe, 0x33, 0x9c, 0x75, 0xa1, 0x29, 0xb4 } };


#include <bfc/dispatch.h>
#include "./ifc_dataobject.h"
#include "./ifc_dataobjectenum.h"
#include "./ifc_dataproviderevent.h"

#define	VALUEID_UNKNOWN	(-1)

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataprovider : public Dispatchable
{
protected:
	ifc_dataprovider() {}
	~ifc_dataprovider() {}

public:
	HRESULT ResolveNames(const char **names, size_t count, size_t *valueIds);
	HRESULT Enumerate(ifc_dataobjectenum **enumerator);
	HRESULT GetColumnDisplayName(const char *name, wchar_t *buffer, size_t bufferSize); // chance to override column title
	HRESULT RegisterEventHandler(ifc_dataproviderevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_dataproviderevent *eventHandler);

public:
	DISPATCH_CODES
	{
		API_RESOLVENAMES = 10,
		API_ENUMERATE = 20,
		API_GETCOLUMNDISPLAYNAME = 30,
		API_REGISTEREVENTHANDLER = 40,
		API_UNREGISTEREVENTHANDLER = 50,
	};
};

inline HRESULT ifc_dataprovider::ResolveNames(const char **names, size_t count, size_t *valueIds)
{
	return _call(API_RESOLVENAMES, (HRESULT)E_NOTIMPL, names, count, valueIds);
}

inline HRESULT ifc_dataprovider::Enumerate(ifc_dataobjectenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_dataprovider::GetColumnDisplayName(const char *name, wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETCOLUMNDISPLAYNAME, (HRESULT)E_NOTIMPL, name, buffer, bufferSize);
}

inline HRESULT ifc_dataprovider::RegisterEventHandler(ifc_dataproviderevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_dataprovider::UnregisterEventHandler(ifc_dataproviderevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_INTERFACE_HEADER
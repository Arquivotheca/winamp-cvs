#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {AFF49C44-B01E-45aa-A500-C021292E0803}
static const GUID IFC_GroupManager = 
{ 0xaff49c44, 0xb01e, 0x45aa, { 0xa5, 0x0, 0xc0, 0x21, 0x29, 0x2e, 0x8, 0x3 } };

#include <bfc/dispatch.h>
#include "./ifc_groupprovider.h"
#include "./ifc_groupenum.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_groupmanager : public Dispatchable
{

protected:
	ifc_groupmanager() {}
	~ifc_groupmanager() {}

public:
	size_t Register(ifc_groupprovider **providers, size_t  count);
	HRESULT Unregister(const char *providerName);
	HRESULT Enumerate(ifc_groupenum **enumerator);
	HRESULT Find(const char *providerName, ifc_groupprovider **provider);

public:
	DISPATCH_CODES
	{
		API_REGISTER = 10,
		API_UNREGISTER = 20,
		API_ENUMERATE = 30,
		API_FIND = 40,
	};
};

inline size_t ifc_groupmanager::Register(ifc_groupprovider **providers, size_t  count)
{
	return _call(API_REGISTER, (size_t)0, providers, count);
}

inline HRESULT ifc_groupmanager::Unregister(const char *providerName)
{
	return _call(API_UNREGISTER, (HRESULT)E_NOTIMPL, providerName);
}

inline HRESULT ifc_groupmanager::Enumerate(ifc_groupenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_groupmanager::Find(const char *providerName, ifc_groupprovider **provider)
{
	return _call(API_FIND, (HRESULT)E_NOTIMPL, providerName, provider);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_INTERFACE_HEADER
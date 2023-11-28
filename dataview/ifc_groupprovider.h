#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_GROUP_PROVIDER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_GROUP_PROVIDER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {F0517A2E-9734-4dfe-BF92-D2FBE803B511}
static const GUID IFC_GroupProvider = 
{ 0xf0517a2e, 0x9734, 0x4dfe, { 0xbf, 0x92, 0xd2, 0xfb, 0xe8, 0x3, 0xb5, 0x11 } };

#include <bfc/dispatch.h>
#include "./ifc_dataprovider.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_groupprovider : public Dispatchable
{
protected:
	ifc_groupprovider() {}
	~ifc_groupprovider() {}

public:
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize); // 
	HRESULT Bind(ifc_dataprovider *provider);
	HRESULT ResolveNames(const char **names, size_t count, size_t *valueIds);
	HRESULT CreateGroup(LCID localeId, ifc_dataobject *object, ifc_dataobject **group); // group also implements ifc_groupobject interface
	int CompareObjects(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2);
	HRESULT GetCounterText(wchar_t *buffer, size_t bufferSize); // text used to specify number of group objects (artists)
	HRESULT GetEmptyText(wchar_t *buffer, size_t bufferSize); // text to use when displaying empty string group
	HRESULT CreateSummaryGroup(ifc_dataobject **group); // must also implement ifc_groupobject.

public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETDISPLAYNAME = 20,
		API_BIND = 30,
		API_RESOLVENAMES = 40,
		API_CREATEGROUP = 50,
		API_COMPAREOBJECTS = 60,
		API_GETCOUNTERTEXT = 70,
		API_GETEMPTYTEXT = 80,
		API_CREATESUMMARYGROUP = 90,
	};
};

inline const char *ifc_groupprovider::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_groupprovider::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer,bufferSize);
}


inline HRESULT ifc_groupprovider::Bind(ifc_dataprovider *provider)
{
	return _call(API_BIND, (HRESULT)E_NOTIMPL, provider);
}

inline HRESULT ifc_groupprovider::ResolveNames(const char **names, size_t count, size_t *valueIds)
{
	return _call(API_RESOLVENAMES, (HRESULT)E_NOTIMPL, names, count, valueIds);
}


inline HRESULT ifc_groupprovider::CreateGroup(LCID localeId, ifc_dataobject *object, ifc_dataobject **group)
{
	return _call(API_CREATEGROUP, (HRESULT)E_NOTIMPL, localeId, object, group);
}

inline int ifc_groupprovider::CompareObjects(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2)
{
	return _call(API_COMPAREOBJECTS, (HRESULT)COBJ_ERROR, localeId, object1, object2);
}

inline HRESULT ifc_groupprovider::GetCounterText(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETCOUNTERTEXT, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_groupprovider::GetEmptyText(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETEMPTYTEXT, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_groupprovider::CreateSummaryGroup(ifc_dataobject **group)
{
	return _call(API_CREATESUMMARYGROUP, (HRESULT)E_NOTIMPL, group);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_PROVIDER_INTERFACE_HEADER
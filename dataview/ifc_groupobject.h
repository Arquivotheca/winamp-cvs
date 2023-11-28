#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_GROUP_OBJECT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_GROUP_OBJECT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {86B4767D-CD72-422f-B890-62AE17FB1383}
static const GUID IFC_GroupObject = 
{ 0x86b4767d, 0xcd72, 0x422f, { 0xb8, 0x90, 0x62, 0xae, 0x17, 0xfb, 0x13, 0x83 } };

#include <bfc/dispatch.h>
#include "./ifc_dataobject.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_groupobject : public Dispatchable
{
protected:
	ifc_groupobject() {}
	~ifc_groupobject() {}

public:
	HRESULT Add(ifc_dataobject *object);
	HRESULT Subtract(ifc_dataobject *object);
	HRESULT Reset();
	HRESULT IsUnknown();
	
public:
	DISPATCH_CODES
	{
		API_ADD = 10,
		API_SUBTRACT = 20,
		API_RESET = 30,
		API_ISUNKNOWN = 40,
	};
};

inline HRESULT ifc_groupobject::Add(ifc_dataobject *object)
{
	return _call(API_ADD, (HRESULT)E_NOTIMPL, object);
}

inline HRESULT ifc_groupobject::Subtract(ifc_dataobject *object)
{
	return _call(API_SUBTRACT, (HRESULT)E_NOTIMPL, object);
}

inline HRESULT ifc_groupobject::Reset()
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_groupobject::IsUnknown()
{
	return _call(API_ISUNKNOWN, (HRESULT)E_NOTIMPL);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_GROUP_OBJECT_INTERFACE_HEADER
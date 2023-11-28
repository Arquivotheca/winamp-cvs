#ifndef _NULLSOFT_WINAMP_DATAVIEW_SUMMARY_DATA_GROUP_OBJECT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_SUMMARY_DATA_GROUP_OBJECT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {89D2AC37-1168-41e1-86EF-180DEC11E7C1}
static const GUID IFC_SummaryGroupObject = 
{ 0x89d2ac37, 0x1168, 0x41e1, { 0x86, 0xef, 0x18, 0xd, 0xec, 0x11, 0xe7, 0xc1 } };


#include <bfc/dispatch.h>
#include "./ifc_groupobject.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_summarygroupobject : public Dispatchable
{
protected:
	ifc_summarygroupobject() {}
	~ifc_summarygroupobject() {}

public:
	HRESULT AddGroup(ifc_groupobject *group);
	HRESULT SubtractGroup(ifc_groupobject *group);
	
public:
	DISPATCH_CODES
	{
		API_ADDGROUP = 10,
		API_SUBTRACTGROUP = 20,
	};
};

inline HRESULT ifc_summarygroupobject::AddGroup(ifc_groupobject *group)
{
	return _call(API_ADDGROUP, (HRESULT)E_NOTIMPL, group);
}

inline HRESULT ifc_summarygroupobject::SubtractGroup(ifc_groupobject *group)
{
	return _call(API_SUBTRACTGROUP, (HRESULT)E_NOTIMPL, group);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_SUMMARY_DATA_GROUP_OBJECT_INTERFACE_HEADER
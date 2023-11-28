#ifndef _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_PROVIDER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_PROVIDER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {74973016-DDC7-4212-BC63-3B0534D23AE5}
static const GUID IFC_PerformanceProvider = 
{ 0x74973016, 0xddc7, 0x4212, { 0xbc, 0x63, 0x3b, 0x5, 0x34, 0xd2, 0x3a, 0xe5 } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_performanceprovider : public Dispatchable
{
protected:
	ifc_performanceprovider() {}
	~ifc_performanceprovider() {}

public:
	HRESULT RegisterTimer(ifc_performancetimer *timer);
	

public:
	DISPATCH_CODES
	{
		API_REGISTERTIMER = 10,
	};
};

inline HRESULT ifc_performanceprovider::RegisterTimer(ifc_performancetimer *timer)
{
	return _call(API_REGISTERTIMER, (HRESULT)E_NOTIMPL, timer);
}



#endif //_NULLSOFT_WINAMP_DATAVIEW_PERFORMANCE_PROVIDER_INTERFACE_HEADER
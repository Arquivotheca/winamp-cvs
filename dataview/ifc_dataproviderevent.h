#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {F7DAD1BD-28F8-4644-BB92-91BFCDEBCC50}
static const GUID IFC_DataProviderEvent = 
{ 0xf7dad1bd, 0x28f8, 0x4644, { 0xbb, 0x92, 0x91, 0xbf, 0xcd, 0xeb, 0xcc, 0x50 } };

#include <bfc/dispatch.h>

class ifc_dataprovider;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataproviderevent : public Dispatchable
{
protected:
	ifc_dataproviderevent() {}
	~ifc_dataproviderevent() {}

public:
	void DataProviderEvent_FieldsChanged(ifc_dataprovider *instance);
	void DataProviderEvent_ObjectsChanged(ifc_dataprovider *instance, ifc_dataobject **objects, size_t count);
	
public:
	DISPATCH_CODES
	{
		API_DATAPROVIDEREVENT_FIELDSCHANGED = 10,
		API_DATAPROVIDEREVENT_OBJECTSCHANGED = 20,
	};
};

inline void ifc_dataproviderevent::DataProviderEvent_FieldsChanged(ifc_dataprovider *instance)
{
	_voidcall(API_DATAPROVIDEREVENT_FIELDSCHANGED, instance);
}

inline void ifc_dataproviderevent::DataProviderEvent_ObjectsChanged(ifc_dataprovider *instance, ifc_dataobject **objects, size_t count)
{
	_voidcall(API_DATAPROVIDEREVENT_OBJECTSCHANGED, instance, objects, count);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_PROVIDER_EVENT_INTERFACE_HEADER
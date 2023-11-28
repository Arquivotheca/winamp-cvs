#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {DC1F7112-1013-421d-83FA-02853A31552B}
static const GUID IFC_DataObjectList = 
{ 0xdc1f7112, 0x1013, 0x421d, { 0x83, 0xfa, 0x2, 0x85, 0x3a, 0x31, 0x55, 0x2b } };

#include <bfc/dispatch.h>

#include "./ifc_dataobjectlistevent.h"

class ifc_dataobject;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataobjectlist : public Dispatchable
{
protected:
	ifc_dataobjectlist() {}
	~ifc_dataobjectlist() {}

public:
	size_t GetCount();
	ifc_dataobject *GetItem(size_t index); // do not increment ref count.
	HRESULT Enumerate(ifc_dataobjectenum **enumerator);
	size_t Find(LCID localeId, ifc_dataobject *object);  // return (size_t)-1 - if not found or error

	HRESULT RegisterEventHandler(ifc_dataobjectlistevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_dataobjectlistevent *eventHandler);

public:
	DISPATCH_CODES
	{
		API_GETCOUNT = 10,
		API_GETITEM = 20,
		API_ENUMERATE = 30,
		API_FIND = 40,
		API_REGISTEREVENTHANDLER = 50,
		API_UNREGISTEREVENTHANDLER = 60,
	};
};

inline size_t ifc_dataobjectlist::GetCount()
{
	return _call(API_GETCOUNT, (size_t)0);
}

inline ifc_dataobject *ifc_dataobjectlist::GetItem(size_t index)
{
	return _call(API_GETITEM, (ifc_dataobject*)NULL, index);
}

inline HRESULT ifc_dataobjectlist::Enumerate(ifc_dataobjectenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline size_t ifc_dataobjectlist::Find(LCID localeId, ifc_dataobject *object)
{
	return _call(API_FIND, (size_t)-1, localeId, object);
}


inline HRESULT ifc_dataobjectlist::RegisterEventHandler(ifc_dataobjectlistevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_dataobjectlist::UnregisterEventHandler(ifc_dataobjectlistevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_INTERFACE_HEADER
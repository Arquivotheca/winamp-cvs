#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C068DE4E-A4C9-4305-8548-F85FECE50D4A}
static const GUID IFC_DataObjectListEvent = 
{ 0xc068de4e, 0xa4c9, 0x4305, { 0x85, 0x48, 0xf8, 0x5f, 0xec, 0xe5, 0xd, 0x4a } };

#include <bfc/dispatch.h>

class ifc_dataobjectlist;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataobjectlistevent : public Dispatchable
{
protected:
	ifc_dataobjectlistevent() {}
	~ifc_dataobjectlistevent() {}

public:
	void ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ObjectListEvent_RemovedAll(ifc_dataobjectlist *list);
	void ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list);
	void ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list);

public:
	DISPATCH_CODES
	{
		API_OBJECTLISTEVENT_ADDDED = 10,
		API_OBJECTLISTEVENT_REMOVED = 20,
		API_OBJECTLISTEVENT_REMOVEDALL = 30,
		API_OBJECTLISTEVENT_CHANGED = 40,
		API_OBJECTLISTEVENT_UPDATESTARTED = 50,
		API_OBJECTLISTEVENT_UPDATEFINISHED = 60,
	};
};

inline void ifc_dataobjectlistevent::ObjectListEvent_Added(ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	_voidcall(API_OBJECTLISTEVENT_ADDDED, list, added, count, startIndex);
}

inline void ifc_dataobjectlistevent::ObjectListEvent_Removed(ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
	_voidcall(API_OBJECTLISTEVENT_REMOVED, list, removed, count, startIndex);
}

inline void ifc_dataobjectlistevent::ObjectListEvent_RemovedAll(ifc_dataobjectlist *list)
{
	_voidcall(API_OBJECTLISTEVENT_REMOVEDALL, list);
}

inline void ifc_dataobjectlistevent::ObjectListEvent_Changed(ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	_voidcall(API_OBJECTLISTEVENT_CHANGED, list, changed, count, startIndex);
}

inline void ifc_dataobjectlistevent::ObjectListEvent_UpdateStarted(ifc_dataobjectlist *list)
{
	_voidcall(API_OBJECTLISTEVENT_UPDATESTARTED, list);
}

inline void ifc_dataobjectlistevent::ObjectListEvent_UpdateFinished(ifc_dataobjectlist *list)
{
	_voidcall(API_OBJECTLISTEVENT_UPDATEFINISHED, list);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_LIST_EVENT_INTERFACE_HEADER
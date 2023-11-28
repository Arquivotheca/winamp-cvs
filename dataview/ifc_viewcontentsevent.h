#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {4EC6DF30-0731-465f-B98F-55DEA5BB4497}
static const GUID IFC_ViewContentsEvent = 
{ 0x4ec6df30, 0x731, 0x465f, { 0xb9, 0x8f, 0x55, 0xde, 0xa5, 0xbb, 0x44, 0x97 } };


#include <bfc/dispatch.h>

class ifc_viewcontents;
class ifc_dataobjectlist;
class ifc_viewselection;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcontentsevent : public Dispatchable
{

protected:
	ifc_viewcontentsevent() {}
	~ifc_viewcontentsevent() {}

public:
	void ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects);
	void ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex); // if index 
	void ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason);
	void ContentsEvent_ColumnsChanged(ifc_viewcontents *contents);
	
	
public:
	DISPATCH_CODES
	{
		API_CONTENTSEVENT_OBJECTLISTCHANGED = 10,
		API_CONTENTSEVENT_OBJECTSADDED = 20,
		API_CONTENTSEVENT_OBJECTSREMOVED = 30,
		API_CONTENTSEVENT_OBJECTSREMOVEDALL = 40,
		API_CONTENTSEVENT_OBJECTSCHANGED = 50,
		API_CONTENTSEVENT_OBJECTSUPDATESTARTED = 60,
		API_CONTENTSEVENT_OBJECTSUPDATEFINISHED = 70,
		API_CONTENTSEVENT_SELECTIONCHANGED = 80,
		API_CONTENTSEVENT_COLUMNSCHANGED = 90,
	};
};

inline void ifc_viewcontentsevent::ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects)
{
	_voidcall(API_CONTENTSEVENT_OBJECTLISTCHANGED, contents, newObjects, prevObjects);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSADDED, contents, list, added, count, startIndex);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSREMOVED, contents, list, removed, count, startIndex);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSREMOVEDALL, contents, list);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSCHANGED, contents, list, changed, count, startIndex);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSUPDATESTARTED, contents, list);
}

inline void ifc_viewcontentsevent::ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
	_voidcall(API_CONTENTSEVENT_OBJECTSUPDATEFINISHED, contents, list);
}

inline void ifc_viewcontentsevent::ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason)
{
	_voidcall(API_CONTENTSEVENT_SELECTIONCHANGED, contents, selection, appended, removed, reason);
}

inline void ifc_viewcontentsevent::ContentsEvent_ColumnsChanged(ifc_viewcontents *contents)
{
	_voidcall(API_CONTENTSEVENT_COLUMNSCHANGED, contents);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_CONTENTS_EVENT_INTERFACE_HEADER
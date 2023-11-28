#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C6A6F57D-2CD2-4fd0-A76E-7D8A23D8DFCA}
static const GUID IFC_ViewSelectionEvent = 
{ 0xc6a6f57d, 0x2cd2, 0x4fd0, { 0xa7, 0x6e, 0x7d, 0x8a, 0x23, 0xd8, 0xdf, 0xca } };


#include <bfc/dispatch.h>

class ifc_viewselection;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewselectionevent : public Dispatchable
{
public:
	typedef enum Reason
	{
		Selection_Set = (1 << 0),
		Selection_Add = (1 << 1),
		Selection_Remove = (1 << 2),
		Selection_RemoveAll = (1 << 3),
		Selection_Shift = (1 << 4),
		Selection_Commit = (1 << 5),
	} Reason;

protected:
	ifc_viewselectionevent() {}
	~ifc_viewselectionevent() {}

public:
	void SelectionEvent_Changed(ifc_viewselection *instance, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason);

	
public:
	DISPATCH_CODES
	{
		API_SELECTIONEVENT_CHANGED = 10,
	};
};

DEFINE_ENUM_FLAG_OPERATORS(ifc_viewselectionevent::Reason);

inline void ifc_viewselectionevent::SelectionEvent_Changed(ifc_viewselection *instance, ifc_viewselection *appended, ifc_viewselection *removed, Reason reason)
{
	_voidcall(API_SELECTIONEVENT_CHANGED, instance, appended, removed, reason);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_EVENT_INTERFACE_HEADER
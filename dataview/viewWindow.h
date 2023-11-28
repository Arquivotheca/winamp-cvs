#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include "./ifc_viewwindow.h"
#include "./ifc_viewcontents.h"
#include "./ifc_viewcontentsevent.h"
#include "./ifc_viewcontroller.h"
#include "./ifc_reflectedmessageproc.h"
#include "./ifc_performancetimer.h"
#include "./ifc_performanceprovider.h"
#include "./ifc_viewgroupfilter.h"
#include "./ifc_viewgroupfilterevent.h"

#include <bfc/multipatch.h>

#define MPIID_VIEWWINDOW			10
#define MPIID_REFLECTEDMESSAGEPROC	20
#define MPIID_VIEWCONTENTSEVENT		30
#define MPIID_VIEWGROUPFILTEREVENT	40
#define MPIID_PERFORMANCEPROVIDER	50

class ViewWindow :	public MultiPatch<MPIID_VIEWWINDOW, ifc_viewwindow>,
					public MultiPatch<MPIID_REFLECTEDMESSAGEPROC, ifc_reflectedmessageproc>,
					public MultiPatch<MPIID_VIEWCONTENTSEVENT, ifc_viewcontentsevent>,
					public MultiPatch<MPIID_VIEWGROUPFILTEREVENT, ifc_viewgroupfilterevent>,
					public MultiPatch<MPIID_PERFORMANCEPROVIDER, ifc_performanceprovider>
{
public:
	typedef enum WindowFlags
	{
		WindowFlag_None = 0,
		WindowFlag_UnicodeWindow = (1 << 0),
	} WindowFlags;

protected:
	ViewWindow(HWND hwnd, const char *name, ifc_viewcontents *contents);
	virtual ~ViewWindow();

public:
	static HRESULT GetObject(HWND hwnd, ifc_viewwindow **window);
	static HRESULT RegisterPerformanceTimer(HWND hwnd, ifc_performancetimer *timer);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewwindow */
	const char *GetName();
	HRESULT GetConfig(ifc_viewconfig **config);
	HRESULT GetContents(ifc_viewcontents **contents);
	HWND GetWindow();

	/* ifc_reflectedmessageproc */
	virtual HRESULT ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result);

	/* ifc_performanceprovider */
	HRESULT RegisterTimer(ifc_performancetimer *timer);

protected:
	/* ifc_viewcontentsevent */
	virtual void ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects);
	virtual void ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	virtual void ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	virtual void ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	virtual void ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	virtual void ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list);
	virtual void ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	virtual void ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason);
	virtual void ContentsEvent_ColumnsChanged(ifc_viewcontents *contents);

	/* ifc_viewgroupfilterevent */
	virtual void GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled);

protected:	
	virtual HRESULT AttachWindow();
	virtual void Destroy();
	virtual void UpdateColors();
	virtual void UpdateFont();
	
	HRESULT GetController(ifc_viewcontroller **controller);
	HRESULT GetSelectionTracker(ifc_viewselection **selection);
	HRESULT GetGroupFilter(ifc_viewgroupfilter **groupFilter);
	HRESULT GetSummaryObject(ifc_dataobject **object);

	// window message handers
	virtual void OnRedrawEnabled(BOOL enabled);
	virtual void OnContextMenu(HWND targetWindow, long cursor);
	BOOL OnGetObject(ifc_viewwindow **window);


	virtual LRESULT WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT PreviousWindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam);
		
private:
	friend static LRESULT CALLBACK ViewWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	size_t ref;
	char *name;
	HWND hwnd;
	WindowFlags windowFlags;
	ifc_viewcontents *contents;
	ifc_performancetimer *performanceTimer;
	

private:
	WNDPROC previousWndProc;

protected:
	RECVS_MULTIPATCH;
};

DEFINE_ENUM_FLAG_OPERATORS(ViewWindow::WindowFlags);

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_WINDOW_HEADER
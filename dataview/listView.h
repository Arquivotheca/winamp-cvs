#ifndef _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include "./viewWindow.h"
#include "./viewColumn.h"
#include "./ifc_viewselectiontransaction.h"
#include "../nu/ptrlist.h"
#include "../nu/vector.h"


class ListView : public ViewWindow
{
public:
	typedef enum ListFlags
	{
		ListFlag_None = 0,
		ListFlag_SummaryEnabled = (1 << 0),
		ListFlag_PublishingSelection = (1 << 1),
		ListFlag_UpdatingSelection = (1 << 2),
		ListFlag_UpdateItems = (1 << 4),
		ListFlag_UpdateSort = (1 << 5),
		ListFlag_UpdateSelection = (1 << 6),
		ListFlag_BypassSetRedraw = (1 << 7),
		ListFlags_MarqueeSelection = (1 << 8),
	} ListFlags;

protected:
	ListView(HWND hwnd, const char *name, ifc_viewcontents *contents);
	~ListView();

public:
	static HWND CreateInstance(ifc_viewcontents *contents,
							   unsigned int windowStyleEx,
							   unsigned int windowStyle,
							   HWND parentWindow, 
							   int x, 
							   int y,
							   int width,
							   int height,
							   int controlId);

public:
	ViewColumn *GetColumn(size_t iColumn);
	HRESULT SwapColumns(int iColumn1, int iColumn2);
	HRESULT GetItem(size_t iItem, ifc_dataobject **object);
	
	size_t LockUpdates();
	size_t UnlockUpdates();

protected:
	virtual BOOL ReloadColumns();
	virtual BOOL UpdateHeader();
	virtual BOOL UpdateItems(BOOL immediate);
	virtual void InvalidateSort(size_t validCount);
	virtual HRESULT UpdateSort(BOOL immediate);
	virtual BOOL SyncSortArrow();
	virtual void UpdateSelection(BOOL immediate);
	BOOL FilterSelectionChange(int *iFrom, int *iTo, BOOL selected); // return FALSE if selection processing should be stopped
	void NotifySelectionChange(int iFrom, int iTo, BOOL selected);
	BOOL PublishSelection(int iFrom, int iTo, BOOL selected);
	BOOL PublishEntireSelection();
	HRESULT CommitSelectionTransaction();
	BOOL IsSummaryItem(size_t iItem);

	HRESULT SetItemCount(size_t count, unsigned int flags);
	HRESULT SetItemState(int iItem, unsigned int state, unsigned int stateMask);
	HRESULT SetFocusItem(int iItem, BOOL focused);
	HRESULT SetSelectedItem(int iItem, BOOL selected, BOOL silentMode);
	
	void SetRedrawInternal(BOOL enableRedraw);
	 
	virtual void ShowItemContextMenu(POINT pt, BOOL extendedMode);
	virtual void ShowViewContextMenu(POINT pt, BOOL extendedMode);
	
protected:
	// ViewWindow
	HRESULT AttachWindow();
	HRESULT ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result);
	LRESULT WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam);
	
	
protected:
	// Reflected Messages
	LRESULT OnNotification(NMHDR *notification);

	// Notifications
	virtual int OnFindItem(int iStart, LVFINDINFO *findInfo);
	virtual void OnGetDisplayInfo(LVITEM *item);
	virtual void OnColumnClick(int iColumn);
	virtual unsigned int OnCustomDraw(int iItem, int iSubItem, NMCUSTOMDRAW *drawData, COLORREF backColor, COLORREF textColor);
	virtual void OnGetInfoTip(int iItem, int iSubItem, wchar_t *buffer, size_t bufferMax, BOOL unfolded);
	virtual void OnKeyDown(unsigned short vKey);
	virtual void OnReturnPressed();
	virtual void OnDoubleClick(POINT pt);
	virtual void OnStateChange(int iFrom, int iTo, unsigned int newState, unsigned int oldState);
	virtual void OnFocusChanged(BOOL focusReceived);
	virtual BOOL OnMarqueeBegin();
	virtual void OnReleasedCapture();

	/* Windows Messages */
	void OnRedrawEnabled(BOOL enabled);
	void OnContextMenu(HWND targetWindow, long cursor);

	/* ifc_viewcontentsevent */
	void ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects);
	void ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex);
	void ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list);
	void ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list);
	void ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason);
	void ContentsEvent_ColumnsChanged(ifc_viewcontents *contents);
	
	/* ifc_viewgroupfilterevent */
	void GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled);

protected:
	typedef nu::PtrList<ViewColumn> ColumnList;
	typedef Vector<size_t, 8> IndexMap;

protected:
	LCID localeId;
	size_t updateLock;
	ListFlags listFlags;
	ColumnList columnList;
	IndexMap headerMap;
	ifc_viewcolumn *sortColumn;
	SortOrder sortOrder;
	IndexMap itemMap;	
	IndexMap indexMap;	// opposite to sort map
	ifc_viewselectiontransaction *selectionTransaction;
	size_t sortedCount;
	size_t listCount;
	BOOL filterBypassTransaction;
};

DEFINE_ENUM_FLAG_OPERATORS(ListView::ListFlags);

#endif // _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_WINDOW_HEADER
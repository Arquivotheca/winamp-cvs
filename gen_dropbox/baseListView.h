#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_BASELISTVIEW_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_BASELISTVIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include <windows.h>
#include <commctrl.h>

#include "./itemView.h"
#include "./document.h"
#include "./dropWindowInternal.h"
#include "./frameCache.h"
#include "../gen_ml/ml.h"
#include "../nu/Vector.h"

#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

interface IClipboardFormatProcessor;
class ListViewDragDropData;

class BaseListView : public DropboxView 
{

public:
	typedef enum
	{
		NotifyReadSelection = 0, 
		NotifyUpdateMetrics,
		NotifyCacheModified,
		NotifyItemShifted,
		NotifyRangeReversed,
		NotifyRangeReordered,
		NotifyRangeRemoved,
	} NotifyCode;
	

protected:
	BaseListView(HWND hView);
	virtual ~BaseListView();

public:
	STDMETHOD(SetDocument)(Document *pDoc);
	STDMETHOD(ProcessNotification)(NMHDR *pnmh, LRESULT *pResult);
	STDMETHOD(DrawItem)(DRAWITEMSTRUCT *pdis);
	STDMETHOD(MeasureItem)(MEASUREITEMSTRUCT *pmis);
	STDMETHOD(GetSelectionMetrics)(Document::METRICS *pMetrics);
	STDMETHOD(GetSelectionCount)(size_t *pCount);
	STDMETHOD(EnableSelectionRead)(BOOL bEnable); 


	STDMETHOD (DragEnter)(IDataObject *, DWORD, POINTL, DWORD*);
    STDMETHOD (DragOver)(DWORD, POINTL, DWORD*);
    STDMETHOD (DragLeave)(void);
    STDMETHOD (Drop)(IDataObject*, DWORD, POINTL, DWORD*);


	HRESULT MakeDataObject(INT iFirst, POINT *pDropPoint, BOOL bNonClient, IDataObject **pDataObject, UINT useFormats, size_t **ppHdropItems, size_t *pcHdropItems);
	
	// drag&drop
	HRESULT PerformDragDrop(INT iItem);

	HRESULT EnqueueSelection(INT iFirst, BOOL bPlay);

	STDMETHOD(SetSkinned)(BOOL bSkinned);
	STDMETHOD(ProcessCommand)(INT commandId);
	
protected:
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	// message handlers
	
	void OnUpdateUiState(WORD action, WORD state);
	LRESULT OnGetDlgCode(UINT vKey, MSG *pMsg);
	virtual void OnWindowPosChanged(WINDOWPOS *pwp);
	virtual LRESULT OnSetItemCount(INT cItems, DWORD dwFlags);
	virtual void OnVScroll(UINT scrollCode, UINT position, HWND hCtrl);
	virtual void OnContextMenu(HWND hTarget, POINTS pts);
	virtual void OnSetFont(HFONT hFont, BOOL bRedraw);
	
	// supported notifications
	virtual void OnCacheHint(NMLVCACHEHINT *pch);
	virtual void OnKeyDown(NMLVKEYDOWN *pkd);
	virtual void OnBeginDrag(INT iItem);
	virtual void OnStateChanged(NMLVODSTATECHANGE *pstateChanged);
	virtual void OnItemChanged(NMLISTVIEW *plv);
	void OnItemDblClick(NMITEMACTIVATE *pnma);
	void OnEnterKey(NMHDR *pnmh);
		
	// supported commands
	virtual void OnCommand(INT ctrlId, INT eventId, HWND hwndCtrl);

	virtual HRESULT OnSelectAll(BOOL bSelect);
	virtual HRESULT OnDeleteSelection();
	virtual HRESULT OnCopySelection();
	virtual HRESULT OnPasteItems();
	virtual HRESULT OnCutSelection();
	virtual HRESULT OnInvertSelection();
	virtual HRESULT OnShowFileInfo();
	virtual HRESULT OnPlay();
	virtual HRESULT OnEnqueue();
	virtual HRESULT OnAction(); // gets resolved to play or enqueue
	virtual HRESULT OnReverseOrder();
	virtual HRESULT OnExploreFolder();
	
	// drag core
	HRESULT ProcessorDragEnter(IClipboardFormatProcessor *processor, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	HRESULT ProcessorDragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	HRESULT ProcessorDragLeave();
	HRESULT ProcessorDrop(IClipboardFormatProcessor *processor, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

	void OnMediaLibraryDragDrop(INT code, mlDropItemStruct *pdis);
		
	virtual void OnDocumentNotify(UINT eventId, LONG_PTR param);
	void OnDocumentCountChanged();
	void OnDocumentReadScheduled(Document::ITEMREAD *readData);
	void OnDocumentReadCompleted(Document::ITEMREAD *readData);
	void OnDocumentItemShifted(Document::ITEMSHIFT *pShiftData);
	void OnDocumentReadQueueEmpty();
	void OnDocumentAsyncFinished();
	void OnDocumentItemInvalidated(INT index);
	void OnDocumentRangeReversed(size_t first, size_t last);
	void OnDocumentRangeReordered(size_t first, size_t last);
	void OnDocumentRangeRemoved(size_t first, size_t last);

	virtual void OnSkinChanged();

	void RedrawCachedItems();
	BOOL FetchItems(INT first, INT last);
	BOOL CanPlaySelection();
	BOOL CanCopySelection();

	
	void OnSelectionRead(BOOL bRestart);
	void OnUpdateMetrics(INT index, Document::ITEMREAD *readData);
	void OnCacheModified();
	void OnItemShifted(Document::ITEMSHIFT *pShiftData);
	void OnRangeReversed(INT first, INT last);
	void OnRangeReordered(INT first, INT last);
	void OnRangeRemoved(INT first, INT last);
	

	BOOL ProcessNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT SendNotification(UINT notifyCode, WPARAM wParam, LPARAM lParam);
	BOOL SendNotificationMT(UINT notifyCode, WPARAM wParam, LPARAM lParam);
	BOOL PostNotification(UINT notifyCode, WPARAM wParam, LPARAM lParam);

private:
	
	void ScheduleSelectionRead();

private:
	friend static void CALLBACK BaseListView_RedrawTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	friend static void CALLBACK BaseListView_ReadSelectionTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	friend static BOOL CALLBACK BaseListView_OnFetchItems(INT first, INT last, ULONG_PTR param);
	friend static BOOL CALLBACK BaseListView_OnRedrawFetchedItems(INT first, INT last, ULONG_PTR param);
	friend static void CALLBACK BaseListView_OnCacheModified(BOOL cacheModified, ULONG_PTR param);
	friend LRESULT BaseListView_SendNotification(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam);
	friend BOOL BaseListView_SendNotificationMT(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam);
	friend BOOL BaseListView_PostNotification(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam);

protected:
	typedef Vector<METAKEY, 16> METAKEYLIST;

protected:
	WORD		uiState;
	QUERYTHEMECOLOR GetThemeColor;
	QUERYTHEMEBRUSH GetThemeBrush;
	METAKEYLIST metaKeyList;
	INT selectionCursor;
	Document::METRICS selectionMetrics;

private:
	FrameCache	frameCache;
	BOOL		enableSelectionRead;
	BOOL		enableCachedRedraw;
	ListViewDragDropData	 *pDragData;
};





#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_BASELISTVIEW_HEADER
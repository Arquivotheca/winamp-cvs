#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/Vector.h"
#include "./document.h"


__interface IDropTargetHelper;

class DropboxView;
class DropboxViewMeta;

typedef void (CALLBACK *VIEWPROC)(DropboxView* /*pView*/, UINT /*uCode*/, LONG_PTR /*param*/, UINT_PTR /*user*/);

class DropboxView : public IDropTarget
{
protected:
	DropboxView(HWND hView);
	virtual ~DropboxView();

public:
	typedef enum
	{
		EventSelectionChanged = 0,
		EventSelectionLengthChanged = 1,
		
	} VIEWNOTIFICATION;

public:
	/*** IUnknown ***/
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD (QueryInterface)(REFIID, LPVOID*);
	
	/*** IDropTarget ***/
	STDMETHOD (DragEnter)(IDataObject *, DWORD, POINTL, DWORD*);
    STDMETHOD (DragOver)(DWORD, POINTL, DWORD*);
    STDMETHOD (DragLeave)(void);
    STDMETHOD (Drop)(IDataObject*, DWORD, POINTL, DWORD*);

	virtual DropboxViewMeta* STDMETHODCALLTYPE GetMeta() = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDocument(Document *pDoc);
	virtual HRESULT STDMETHODCALLTYPE ProcessNotification(NMHDR *pnmh, LRESULT *pResult) = 0; // return S_OK if you support this feature
	virtual HRESULT STDMETHODCALLTYPE DrawItem(DRAWITEMSTRUCT *pdis) = 0; // return S_OK if you processes this message.
	virtual HRESULT STDMETHODCALLTYPE MeasureItem(MEASUREITEMSTRUCT *pmis) = 0; // return S_OK if you processes this message.
	virtual HRESULT STDMETHODCALLTYPE SetSkinned(BOOL bSkinned);
	virtual HRESULT STDMETHODCALLTYPE ProcessCommand(INT commandId) = 0; // return S_OK if processed
	virtual HRESULT STDMETHODCALLTYPE ConfigChanged(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE Save(Profile *profile) = 0; // at this time you neeed to save your data (if needed).

	HWND GetHwnd(void) { return hwnd; }
	HRESULT RegisterDragDrop(void) { return ::RegisterDragDrop(hwnd, this); } 
	HRESULT RevokeDragDrop(void) { return ::RevokeDragDrop(hwnd); }
	BOOL GetSkinned() { return skinned;}
	

	virtual HRESULT STDMETHODCALLTYPE GetSelectionMetrics(Document::METRICS *pMetrics) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSelectionCount(size_t *pCount) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnableSelectionRead(BOOL bEnable) = 0; // if enabled - will read missing metadata for selection
	

	void RegisterCallback(VIEWPROC callback, ULONG_PTR user);
	void UnregisterCallback(VIEWPROC callback, ULONG_PTR user);


protected:
	BOOL IsAttached();
	LRESULT CallPrevWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CallDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnDocumentNotify(UINT eventId, LONG_PTR param) {}
	void Notify(UINT nCode, LONG_PTR param);

private:
	friend static LRESULT WINAPI DropBoxItemView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend static void CALLBACK DropBoxItemView_DocumentNotify(Document *pDocument, UINT eventId, LONG_PTR param, UINT_PTR user);
	
	typedef struct __SUBSCRIBER
	{
		VIEWPROC		callback;
		UINT_PTR	user;
	} SUBSCRIBER;

	typedef Vector<SUBSCRIBER> SUBSCRIPTION;
	

protected:		
	HWND hwnd;
	BOOL skinned;
	Document *pActiveDocument;

private:
	ULONG ref;
	WNDPROC	fnWndProc;
	IDropTargetHelper *pDropTargetHerlper;
	SUBSCRIPTION subscription;
};

DropboxView *DropBox_GetItemView(HWND hwnd);


#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_HEADER
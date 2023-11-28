#include "./main.h"
#include "./plugin.h"
#include "./itemView.h"
//#include "./itemViewManager.h"
#include "./skinWindow.h"
#include "./document.h"
#include <shlobj.h>
#include <windows.h>

static ATOM	DBITEMVIEW = 0;

static void CALLBACK UninitializeItemView(void)
{
	if (0 != DBITEMVIEW)
	{
		GlobalDeleteAtom(DBITEMVIEW);
		DBITEMVIEW = 0;
	}
}

DropboxView *DropBox_GetItemView(HWND hwnd)
{
	return (DropboxView*)GetProp(hwnd, MAKEINTATOM(DBITEMVIEW));
}

DropboxView::DropboxView(HWND hView) : 
	ref(1), hwnd(NULL), fnWndProc(NULL), pDropTargetHerlper(NULL),
	skinned(FALSE), pActiveDocument(NULL)
{
	if (0 == DBITEMVIEW)
	{
		 DBITEMVIEW = GlobalAddAtom(TEXT("waDropboxView"));
		 if (0 == DBITEMVIEW) return;
		 Plugin_RegisterUnloadCallback(UninitializeItemView);
	}

	if (NULL != hView && NULL == DropBox_GetItemView(hView))
	{		
		hwnd  = hView;
		fnWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hView, GWLP_WNDPROC, (LONGX86)(LONG_PTR)DropBoxItemView_WindowProc);
		if (NULL != fnWndProc && !SetProp(hView, MAKEINTATOM(DBITEMVIEW), this))
		{
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)fnWndProc);
			fnWndProc = NULL;
			hwnd = NULL;
		}
	}
}

DropboxView::~DropboxView()
{
	if (NULL == hwnd || !IsWindow(hwnd))
		return;
	
	if (NULL != pActiveDocument)
		pActiveDocument->UnregisterCallback(DropBoxItemView_DocumentNotify, (ULONG_PTR)this);

	RemoveProp(hwnd, MAKEINTATOM(DBITEMVIEW));
	if (NULL != fnWndProc)
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)fnWndProc);

	if (NULL != pDropTargetHerlper)
		pDropTargetHerlper->Release();
}

STDMETHODIMP_(ULONG) DropboxView::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) DropboxView::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP DropboxView::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDropTarget))
		*ppvObject = (IDropTarget*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else if (IsEqualIID(riid, IID_IDropTargetHelper))
		*ppvObject = (IDropTargetHelper*)pDropTargetHerlper;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP DropboxView::SetDocument(Document *pDoc)
{
	pActiveDocument = pDoc;

	if (NULL != pActiveDocument)
		pActiveDocument->RegisterCallback(DropBoxItemView_DocumentNotify, (ULONG_PTR)this);

	return S_OK;
}


STDMETHODIMP DropboxView::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	if (NULL == pDropTargetHerlper)
	{
		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
	                 IID_IDropTargetHelper, (PVOID*)&pDropTargetHerlper);
		if (FAILED(hr))
			pDropTargetHerlper = NULL;
	}

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		pDropTargetHerlper->DragEnter(hwnd, pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}

STDMETHODIMP DropboxView::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{	
	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		pDropTargetHerlper->DragOver(&pt, *pdwEffect);
	}
 	return S_OK;
}

STDMETHODIMP DropboxView::DragLeave(void)
{
	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragLeave();

	return S_OK;
}

STDMETHODIMP DropboxView::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y};
		pDropTargetHerlper->Drop(pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}



BOOL DropboxView::IsAttached()
{ 
	return ((NULL != hwnd) && this == DropBox_GetItemView(hwnd)); 
}

LRESULT DropboxView::CallPrevWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(fnWndProc, hwnd, uMsg, wParam, lParam);
}

LRESULT DropboxView::CallDefWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

STDMETHODIMP DropboxView::SetSkinned(BOOL bSkinned)
{
	if (bSkinned == skinned)
		return S_OK;
	HRESULT hr;
	if (bSkinned)
	{
		hr = MlSkinWindow(hwnd, SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);
		if (SUCCEEDED(hr))
			skinned = TRUE;
	}
	else
	{
		hr = MlUnskinWindow(hwnd);
		if (SUCCEEDED(hr))
			skinned = FALSE;
	}

	return hr;
}


static LRESULT WINAPI DropBoxItemView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DropboxView *pView = DropBox_GetItemView(hwnd);

	if (NULL == pView)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	
	switch(uMsg)
	{
		case WM_DESTROY:
			{
				pView->WindowProc(uMsg, wParam, lParam);
				WNDPROC fnWndProc;
				fnWndProc = pView->fnWndProc;
				delete(pView);
				return CallWindowProc(fnWndProc, hwnd, uMsg, wParam, lParam);
			}
			break;
	}
	return pView->WindowProc(uMsg, wParam, lParam);
}

static void CALLBACK DropBoxItemView_DocumentNotify(Document *pDocument, UINT eventId, LONG_PTR param, UINT_PTR user)
{
	DropboxView *instance = (DropboxView*)user;
	if (NULL != instance)
		instance->OnDocumentNotify(eventId, param);
}

void DropboxView::RegisterCallback(VIEWPROC callback, ULONG_PTR user)
{
	if (NULL == callback)
		return;

	size_t index = subscription.size();
	while (index--)
	{
		if (subscription[index].callback == callback &&
			subscription[index].user == user)
		return;
	}
	SUBSCRIBER s;
	s.callback = callback;
	s.user = user;
	subscription.push_back(s);
}	

void DropboxView::UnregisterCallback(VIEWPROC callback, ULONG_PTR user)
{
	size_t index = subscription.size();
	while (index--)
	{
		if (subscription[index].callback == callback &&
			subscription[index].user == user)
		{
			subscription.eraseAt(index);
			return;
		}
	}
}

void DropboxView::Notify(UINT nCode, LONG_PTR param)
{
	size_t index = subscription.size();
	while (index--)
	{
		subscription[index].callback(this, nCode, param, subscription[index].user);
	}
}
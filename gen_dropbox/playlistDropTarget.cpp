#include "./main.h"
#include "./playlistDropTarget.h"
#include "./feHDrop.h"
#include "./dropWindow.h"
#include "./mediaLibraryDrop.h"

#include "./skinWindow.h"
#include "./guiObjects.h"
#include <shlobj.h>


PlaylistDropTarget::PlaylistDropTarget(HWND hwndTarget, HWND hwndDropBox) : 
	ref(1), pDropTargetHerlper(NULL), acceptable(FALSE), callback(NULL)
{
	hTarget = hwndTarget;
	hDropBox = hwndDropBox;
}

PlaylistDropTarget::~PlaylistDropTarget(void)
{
	if (NULL != callback)
		callback->OnDestroy(this);

	if (NULL != pDropTargetHerlper)
		pDropTargetHerlper->Release();
}

PlaylistDropTarget *PlaylistDropTarget::RegisterWindow(HWND hwndTarget, HWND hwndDropBox)
{
	if (!IsWindow(hwndTarget))
		return NULL;

	PlaylistDropTarget *pTarget = new PlaylistDropTarget(hwndTarget, hwndDropBox);
	if (NULL == pTarget)
		return NULL;
	
	HRESULT hr = RegisterDragDrop(hwndTarget, pTarget);
	return SUCCEEDED(hr) ? pTarget : NULL;
}

STDMETHODIMP_(ULONG) PlaylistDropTarget::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) PlaylistDropTarget::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP PlaylistDropTarget::QueryInterface(REFIID riid, PVOID *ppvObject)
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

HWND PlaylistDropTarget::GetHostHwnd(void)
{
	return hTarget;
}

HRESULT PlaylistDropTarget::CanProcessDataObject(IDataObject *pDataObject)
{
	if (NULL == pDataObject)
		return E_POINTER;

	if (NULL == hDropBox || !IsWindow(hDropBox))
		return E_INVALIDARG;

	HRESULT hr;

	FORMATETC ffetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	hr = pDataObject->QueryGetData(&ffetc);
	if (SUCCEEDED(hr))
	{
		IFileEnumerator *pfe;
		hr = HDropFileEnumerator::CreateEnumerator(pDataObject, &pfe);
		if (SUCCEEDED(hr))
			pfe->Release();
	}
	return hr;
}

STDMETHODIMP PlaylistDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	acceptable = FALSE;
	POINT pt = { ptl.x, ptl.y };
	BOOL enabled = (NULL != callback) ? callback->DragEnter(this, pt, grfKeyState) : TRUE;

	if (enabled && 0 != (DROPEFFECT_COPY & *pdwEffect))
	{
		HRESULT hr = CanProcessDataObject(pDataObject);
		acceptable = SUCCEEDED(hr);
	}
			
	*pdwEffect = (acceptable && enabled) ? DROPEFFECT_COPY : (0x00FFFFFF & ~(*pdwEffect));

	if (NULL == pDropTargetHerlper)
	{
		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
	                 IID_IDropTargetHelper, (PVOID*)&pDropTargetHerlper);
		if (FAILED(hr))
			pDropTargetHerlper = NULL;
	}

	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragEnter(hTarget, pDataObject, &pt, *pdwEffect);
	
	return S_OK;
}

STDMETHODIMP PlaylistDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	POINT pt = { ptl.x, ptl.y };
	BOOL enabled = (NULL != callback) ? callback->DragOver(this, pt, grfKeyState) : TRUE;

	if (enabled && acceptable && 0 != (DROPEFFECT_COPY & *pdwEffect))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));
	
	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragOver(&pt, *pdwEffect);
		
	return S_OK;
}

STDMETHODIMP PlaylistDropTarget::DragLeave(void)
{
	if (NULL != callback)
		callback->DragLeave(this);

	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragLeave();

	acceptable = FALSE;
	return S_OK;
}

STDMETHODIMP PlaylistDropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	POINT pt = { ptl.x, ptl.y };
	BOOL enabled = (NULL != callback) ? callback->Drop(this, pt, grfKeyState) : TRUE;

	if (enabled && acceptable && 0 != (DROPEFFECT_COPY & *pdwEffect))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->Drop(pDataObject, &pt, *pdwEffect);
	
	
	if (acceptable && enabled)
	{
		IFileEnumerator *pfe;
		HRESULT hr = HDropFileEnumerator::CreateEnumerator(pDataObject, &pfe);
		if (SUCCEEDED(hr))
		{
			DropboxWindow_DocumentFromEnumerator(hDropBox, pfe);
			pfe->Release();
		}
	}
	acceptable = FALSE;
	return S_OK;
}

void PlaylistDropTarget::MediaLibraryDragDrop(INT code, mlDropItemStruct *pdis)
{
	DWORD dwEffect;
	BOOL enabled = TRUE;
		
	POINT pt;
	if (NULL != pdis)
	{
		pt.x = pdis->p.x;
		pt.y = pdis->p.y;
	}
	else
		GetCursorPos(&pt);
	
	DWORD keyState = 0;
	if (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)))
		keyState |= MK_SHIFT;
	if (0 != (0x8000 & GetAsyncKeyState(VK_MENU)))
		keyState |= MK_ALT;
	if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
		keyState |= MK_CONTROL;
	if (0 != (0x8000 & GetAsyncKeyState(VK_LBUTTON)))
		keyState |= ((0 == GetSystemMetrics(SM_SWAPBUTTON)) ? MK_LBUTTON : MK_RBUTTON);
	if (0 != (0x8000 & GetAsyncKeyState(VK_RBUTTON)))
		keyState |= ((0 == GetSystemMetrics(SM_SWAPBUTTON)) ? MK_RBUTTON : MK_LBUTTON);
	if (0 != (0x8000 & GetAsyncKeyState(VK_MBUTTON)))
		keyState |= MK_MBUTTON;

	switch(code)
	{
		case DRAGDROP_DRAGENTER:
			acceptable = FALSE;
			if (NULL != callback)
				enabled = callback->DragEnter(this, pt, keyState);
			if (enabled && MlDropItemProcessor::CanProcess(pdis))
			{
				acceptable = TRUE;
				dwEffect = DROPEFFECT_COPY;
				pdis->result = 1;
				SetCursor(GetOleCursorFromDropEffect(dwEffect));
				pdis->flags |= ML_HANDLEDRAG_FLAG_NOCURSOR;
			}
			break;
		case DRAGDROP_DRAGLEAVE:
			if (NULL != callback) 
				callback->DragLeave(this);
			acceptable = FALSE;
			break;
		case DRAGDROP_DRAGOVER:
			if (NULL != callback)
				enabled = callback->DragOver(this, pt, keyState);
			if (enabled && acceptable)
			{
				dwEffect = DROPEFFECT_COPY;
				pdis->result = 1;
				SetCursor(GetOleCursorFromDropEffect(dwEffect));
				pdis->flags |= ML_HANDLEDRAG_FLAG_NOCURSOR;
			}
			break;
		case DRAGDROP_DROP:
			if (NULL != callback)
				enabled = callback->Drop(this, pt, keyState);
			if (enabled && acceptable)
			{
				IFileEnumerator *pfe;
				HRESULT hr = MlDropItemProcessor::GetFileEnumerator(pdis, &pfe);
				if (SUCCEEDED(hr))
				{
					DropboxWindow_DocumentFromEnumerator(hDropBox, pfe);
					pfe->Release();
				}
			}
			acceptable = FALSE;
			break;
	}
}
#include "./main.h"
#include "./dropTarget.h"
#include <shlobj.h>


DropTarget::DropTarget(HWND hwndTarget) : 
	ref(1), pDropTargetHerlper(NULL)
{
	hTarget = hwndTarget;
}

DropTarget::~DropTarget(void)
{
	if (NULL != pDropTargetHerlper)
		pDropTargetHerlper->Release();
}

BOOL DropTarget::RegisterWindow(HWND hwndTarget)
{
	if (!IsWindow(hwndTarget))
		return FALSE;
	DropTarget *pTarget = new DropTarget(hwndTarget);
	if (NULL == pTarget)
		return FALSE;
	
	HRESULT hr = RegisterDragDrop(hwndTarget, pTarget);
	pTarget->Release();

	return SUCCEEDED(hr);
}

STDMETHODIMP_(ULONG) DropTarget::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) DropTarget::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP DropTarget::QueryInterface(REFIID riid, PVOID *ppvObject)
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

HWND DropTarget::GetHostHwnd(void)
{
	return hTarget;
}

void DropTarget::TraceDataObject(IDataObject *pObject)
{
#ifdef _DEBUG
	TRACE_LINE(TEXT("DataObject dump:"));
	if (!pObject)
		TRACE_LINE(TEXT("  Error: Null pointer."));
	else
	{
		TRACE_FMT(TEXT("  Pointer: 0x%08X\n"), pObject);
		HRESULT hr;
		IEnumFORMATETC *pEnum = NULL;
		hr = pObject->EnumFormatEtc(DATADIR_GET, &pEnum);
		if (S_OK != hr)
			TRACE_LINE(TEXT("  Error: Unable to get format enumerator."));
		else
		{
			pEnum->Reset();
			FORMATETC ffetc;
			while(S_OK == pEnum->Next(1, &ffetc, NULL))
			{
				TRACE_FMT(TEXT(" Clipboard Format: %d, Type of Medium: %d\n"), ffetc.cfFormat, ffetc.tymed);
			}
			pEnum->Release();
		}
	}
	TRACE_LINE(TEXT("DataObject dump completed."));
#endif // _DEBUG
}


STDMETHODIMP DropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL == pDropTargetHerlper)
	{
		HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
	                 IID_IDropTargetHelper, (PVOID*)&pDropTargetHerlper);
		if (FAILED(hr))
			pDropTargetHerlper = NULL;
	}

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y };
		pDropTargetHerlper->DragEnter(hTarget, pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}

STDMETHODIMP DropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y };
		pDropTargetHerlper->DragOver(&pt, *pdwEffect);
	}	
	return S_OK;
}

STDMETHODIMP DropTarget::DragLeave(void)
{
	if (NULL != pDropTargetHerlper) 
		pDropTargetHerlper->DragLeave();

	return S_OK;
}

STDMETHODIMP DropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	*pdwEffect = (0x00FFFFFF & ~(*pdwEffect));

	if (NULL != pDropTargetHerlper) 
	{
		POINT pt = { ptl.x, ptl.y };
		pDropTargetHerlper->Drop(pDataObject, &pt, *pdwEffect);
	}
	return S_OK;
}


 
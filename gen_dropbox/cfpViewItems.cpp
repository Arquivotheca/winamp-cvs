#include "./main.h"
#include "./cfpViewItems.h"
#include "./dropWindow.h"
#include "./dataObject.h"
#include "./document.h"


static UINT dbcfViewItems = 0; 

CfViewItemsProcessor::CfViewItemsProcessor(IDataObject *pDataObject, HWND hwndDropBox) : ref(1)
{	
	pObject = pDataObject;
	hDropBox = hwndDropBox;
	if (NULL != pObject)
		pObject->AddRef();
}

CfViewItemsProcessor::~CfViewItemsProcessor()
{	
	if (NULL != pObject)
		pObject->Release();
}


HRESULT CfViewItemsProcessor::CanProcess(IDataObject *pDataObject, HWND hwndDropBox)
{
	if (NULL == pDataObject)
		return E_POINTER;

	if (0 == dbcfViewItems) 
	{
		dbcfViewItems = RegisterClipboardFormat(DBCF_VIEWITEMS); 
		if (0 == dbcfViewItems)
			return E_UNEXPECTED;
	}
	
	FORMATETC ffetc = { dbcfViewItems, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	HRESULT hr = pDataObject->GetData(&ffetc, &stgmed);
	if (FAILED(hr))
		return S_FALSE;

	DROPVIEWITEMS *pHeader = (DROPVIEWITEMS*)GlobalLock(stgmed.hGlobal);
	if (NULL != pHeader)
	{
		hr = (IsChild(hwndDropBox, pHeader->hwndList) ||
				hwndDropBox == pHeader->hwndList) ? S_OK : S_FALSE;
		GlobalUnlock(pHeader);
		ReleaseStgMedium(&stgmed);
	}
	else 
		hr = E_FAIL;
	
	return hr;
}

STDMETHODIMP_(ULONG) CfViewItemsProcessor::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) CfViewItemsProcessor::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

STDMETHODIMP CfViewItemsProcessor::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IClipboardFormatProcessor))
		*ppvObject = (IClipboardFormatProcessor*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP CfViewItemsProcessor::Process(INT iInsert)
{
	FORMATETC ffetc = { dbcfViewItems, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	HRESULT hr = pObject->GetData(&ffetc, &stgmed);
	if (SUCCEEDED(hr))
	{
		DROPVIEWITEMS *pHeader = (DROPVIEWITEMS*)GlobalLock(stgmed.hGlobal);
		if (NULL != pHeader)
		{
			if (IsChild(hDropBox, pHeader->hwndList) || hDropBox == pHeader->hwndList)
			{
				Document *pDoc = DropboxWindow_GetActiveDocument(hDropBox);
                if (NULL != pDoc)
				{
					if (pHeader->nCount > 0)
						hr = pDoc->ShiftItems((size_t*)(((BYTE*)pHeader) + pHeader->pItems), pHeader->nCount, iInsert);
					else 
						hr = S_OK;
				}
				else 
					hr = E_POINTER;
			}
			GlobalUnlock(pHeader);
			ReleaseStgMedium(&stgmed);
		}
		else 
			hr = E_FAIL;
	}

	return hr;
}
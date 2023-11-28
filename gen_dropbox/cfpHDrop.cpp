#include "./main.h"
#include "./cfpHDrop.h"
#include "./feHDrop.h"
#include "./dropWindow.h"
#include "./document.h"

CfHDropProcessor::CfHDropProcessor(IDataObject *pDataObject, HWND hwndDropBox) : ref(1)
{	
	pObject = pDataObject;
	hDropBox = hwndDropBox;
	if (NULL != pObject)
		pObject->AddRef();
}

CfHDropProcessor::~CfHDropProcessor()
{	
	if (NULL != pObject)
		pObject->Release();
}


HRESULT CfHDropProcessor::CanProcess(IDataObject *pDataObject, HWND hwndDropBox)
{
	if (NULL == pDataObject)
		return E_POINTER;
	FORMATETC ffetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	HRESULT hr = pDataObject->QueryGetData(&ffetc);
	return (SUCCEEDED(hr)) ? S_OK : S_FALSE;
}

STDMETHODIMP_(ULONG) CfHDropProcessor::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) CfHDropProcessor::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

STDMETHODIMP CfHDropProcessor::QueryInterface(REFIID riid, PVOID *ppvObject)
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

STDMETHODIMP CfHDropProcessor::Process(INT iInsert)
{
	IFileEnumerator *enumerator;
	HRESULT hr = HDropFileEnumerator::CreateEnumerator(pObject, &enumerator);
	if (SUCCEEDED(hr))
	{
		if (!DropboxWindow_InsertEnumerator(hDropBox, iInsert, enumerator))
			hr = E_FAIL;
		enumerator->Release();
	}
	return hr;
}
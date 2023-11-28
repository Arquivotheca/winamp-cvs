#include "./main.h"
#include "./plugin.h"
#include "./feHDrop.h"
#include "../nu/AutoWideFn.h"


HDropFileEnumerator::HDropFileEnumerator(STGMEDIUM *pStorageMedium) :
	ref(1), pDropFiles(NULL), cursor(NULL)
{	
	stgmed = *pStorageMedium;
	pDropFiles = (DROPFILES*)GlobalLock(stgmed.hGlobal);
}


HDropFileEnumerator::~HDropFileEnumerator()
{
	if (NULL != pDropFiles)
			GlobalUnlock(stgmed.hGlobal);

	ReleaseStgMedium(&stgmed);
}

HRESULT HDropFileEnumerator::CreateEnumerator(IDataObject *pDataObject, IFileEnumerator **pEnumerator)
{
	HRESULT hr;

	if (NULL == pEnumerator)
		return E_INVALIDARG;

	*pEnumerator = NULL;

	if (NULL == pDataObject)
		return E_POINTER;

	
	FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmed;
	
	hr = pDataObject->GetData(&fmtetc, &stgmed);
	if (S_OK == hr)
	{
		HDropFileEnumerator *pEnum = new HDropFileEnumerator(&stgmed);
		*pEnumerator = pEnum;
	}

	return hr;
}

STDMETHODIMP_(ULONG) HDropFileEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) HDropFileEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP HDropFileEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileEnumerator))
		*ppvObject = (IFileEnumerator*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP HDropFileEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{
	if (NULL == pDropFiles || 0 == celt || NULL == pfiBuffer) 
		return S_FALSE;

	ULONG counter = 0;
	
	if (NULL == cursor)
		cursor = (LPCTSTR)(((BYTE*)pDropFiles) + pDropFiles->pFiles);
	
	HRESULT hr = S_OK;
	if (pDropFiles->fWide)
	{
		LPCTSTR start = cursor;
		while (celt)
		{
			if (TEXT('\0') == *cursor) cursor++;
				
			start = cursor;
			while (TEXT('\0') != *cursor) cursor++;
					
			if (start == cursor) break;

			hr = PLUGIN_REGTYPES->CreateItem(start, NULL, &pfiBuffer[counter]);
			if (S_OK != hr) 
			{
				pfiBuffer[counter] = NULL;
				hr = E_FILEENUM_CREATEINFO_FAILED;
				break;
			}

			celt--;
			counter++;
		}
	}
	else
	{
		LPCSTR cursorA = (LPCSTR)cursor;
		LPCSTR startA = cursorA;
		while (celt)
		{			
            if ('\0' == *cursorA) cursorA++;

			startA = cursorA;
			while ('\0' != *cursorA) cursorA++;
					
			if (startA == cursorA) break;
			
			hr = PLUGIN_REGTYPES->CreateItem(AutoWideFn(startA), NULL, &pfiBuffer[counter]);
			if (S_OK != hr) 
			{
				pfiBuffer[counter] = NULL;
				hr = E_FILEENUM_CREATEINFO_FAILED;
				break;
			}

			celt--;
			counter++;
		}
		cursor = (LPCTSTR)cursorA;
	}
	if (pceltFetched) *pceltFetched = counter;

	if (S_OK != hr)
		return hr;
	return (counter > 0) ? S_OK : S_FALSE;
}

STDMETHODIMP HDropFileEnumerator::Skip(ULONG celt)
{
	return E_NOTIMPL;
}
STDMETHODIMP HDropFileEnumerator::Reset(void)
{
	cursor = NULL;
	return S_OK;
}

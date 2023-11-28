#include "./dataObject.h"
#include "./enumFormatEtc.h"
#include "./fileInfoInterface.h"
#include "../nu/trace.h"
#include <shlobj.h>
#include <strsafe.h>

static HGLOBAL GlobalClone(HGLOBAL hglobIn)
{
    HGLOBAL hglobOut = NULL;
    LPVOID pvIn = GlobalLock(hglobIn);
    if (pvIn) 
	{
		SIZE_T cb = GlobalSize(hglobIn);
		HGLOBAL hglobOut = GlobalAlloc(GMEM_FIXED, cb);
		if (hglobOut)
		{
			CopyMemory(hglobOut, pvIn, cb);
		}
		GlobalUnlock(hglobIn);
	}
    return hglobOut;
}

static IUnknown *GetCanonicalIUnknown(IUnknown *punk)
{
	IUnknown *punkCanonical;
	if (punk && SUCCEEDED(punk->QueryInterface(IID_IUnknown, (LPVOID*)&punkCanonical))) 
		punkCanonical->Release();
	else
		punkCanonical = punk;
	return punkCanonical;
}

DataObject::DataObject() : ref(1)
{
}

DataObject::~DataObject()
{
	for(size_t i = 0; i < dataList.size(); i++)
	{
		CoTaskMemFree(dataList[i].format.ptd);
		ReleaseStgMedium(&dataList[i].storage);
	}
}

STDMETHODIMP_(ULONG) DataObject::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) DataObject::Release(void)
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP DataObject::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDataObject))
		*ppvObject = (IDataObject*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}


STDMETHODIMP DataObject::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	if (NULL == pFormatEtc || NULL == pMedium)
		return E_INVALIDARG;

	ENTRY *pEntry;
    HRESULT hr = FindFormatEtc(pFormatEtc, &pEntry, FALSE);
	if (SUCCEEDED(hr)) 
		hr = AddRefStgMedium(&pEntry->storage, pMedium, FALSE);
    return hr;
}


STDMETHODIMP DataObject::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
	return E_NOTIMPL;
}

STDMETHODIMP DataObject::QueryGetData(FORMATETC *pFormatEtc)
{
	ENTRY *pEntry;
    return FindFormatEtc(pFormatEtc, &pEntry, FALSE);
}

STDMETHODIMP DataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP DataObject::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease)
{
	ENTRY *pEntry;
    HRESULT hr = FindFormatEtc(pFormatEtc, &pEntry, TRUE);
	if (SUCCEEDED(hr))
	{
		if (pEntry->storage.tymed)
		{
			ReleaseStgMedium(&pEntry->storage);
			ZeroMemory(&pEntry->storage, sizeof(STGMEDIUM));
		}
		if (fRelease) 
		{
            pEntry->storage = *pMedium;
			hr = S_OK;
        } 
		else
		{
            hr = AddRefStgMedium(pMedium, &pEntry->storage, TRUE);
		}
        pEntry->format.tymed = pEntry->storage.tymed;

        if (GetCanonicalIUnknown(pEntry->storage.pUnkForRelease) == GetCanonicalIUnknown(static_cast<IDataObject*>(this))) 
		{
			pEntry->storage.pUnkForRelease->Release();
			pEntry->storage.pUnkForRelease = NULL;
		}
	}
	return hr;
}

STDMETHODIMP DataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
	*ppEnumFormatEtc = NULL;
	switch(dwDirection)
	{
		case DATADIR_GET:
			{
				FORMATETC *pEnumFormats = (FORMATETC*)CoTaskMemAlloc(dataList.size() * sizeof(FORMATETC));
				for (size_t i = 0;  i < dataList.size(); i++)
					pEnumFormats[i] = dataList.at(i).format;
				
				*ppEnumFormatEtc = new FormatEtcEnumerator(pEnumFormats, dataList.size(), TRUE);
				if(NULL != *ppEnumFormatEtc) 
					return S_OK;
				else 
					CoTaskMemFree(pEnumFormats);
			}
			break;
		default:
			return OLE_S_USEREG;
	}
	return E_OUTOFMEMORY;
}

STDMETHODIMP DataObject::DAdvise(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	return E_NOTIMPL;
}
STDMETHODIMP DataObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}
STDMETHODIMP DataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DataObject::FindFormatEtc(FORMATETC *pfe, ENTRY **ppEntry, BOOL fAdd)
{
    *ppEntry = NULL;
    if (pfe->ptd != NULL) return DV_E_DVTARGETDEVICE;
	
	FORMATETC *pfMine;
	size_t index = dataList.size();
	while (index--)
	{
		pfMine = &dataList[index].format;
		if (pfMine->cfFormat == pfe->cfFormat && 
			pfMine->dwAspect == pfe->dwAspect &&
			pfMine->lindex == pfe->lindex)
		{
			if (fAdd || (pfMine->tymed & pfe->tymed)) 
			{
                *ppEntry = &dataList[index];
                return S_OK;
			}
			else 
				return DV_E_TYMED;
		}
	}

	if (!fAdd) return DV_E_FORMATETC;
	
	ENTRY newEntry;
	newEntry.format = *pfe;
	ZeroMemory(&newEntry.storage, sizeof(STGMEDIUM));
	dataList.push_back(newEntry);
	*ppEntry = &dataList.at(dataList.size() - 1);
    return S_OK;
}

HRESULT DataObject::AddRefStgMedium(STGMEDIUM *pstgmIn, STGMEDIUM *pstgmOut, BOOL fCopyIn)
{
    HRESULT hr = S_OK;
    STGMEDIUM stgmOut = *pstgmIn;

    if (pstgmIn->pUnkForRelease == NULL &&
        !(pstgmIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE))) 
	{
		if (fCopyIn) 
		{
			/* Object needs to be cloned */
			if (pstgmIn->tymed == TYMED_HGLOBAL) 
			{
				stgmOut.hGlobal = GlobalClone(pstgmIn->hGlobal);
				if (!stgmOut.hGlobal) 
					hr = E_OUTOFMEMORY;
                
            } 
			else 
                hr = DV_E_TYMED;      /* Don't know how to clone GDI objects */
        } 
		else
			stgmOut.pUnkForRelease = static_cast<IDataObject*>(this);
    }
    if (SUCCEEDED(hr)) 
	{
        switch (stgmOut.tymed) 
		{
			case TYMED_ISTREAM:
				stgmOut.pstm->AddRef();
				break;
			case TYMED_ISTORAGE:
				stgmOut.pstg->AddRef();
				break;
        }
        if (stgmOut.pUnkForRelease) 
			stgmOut.pUnkForRelease->AddRef();
        *pstgmOut = stgmOut;
    }
    return hr;
}



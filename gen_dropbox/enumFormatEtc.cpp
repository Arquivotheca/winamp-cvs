#include "./enumFormatEtc.h"
#include <shlobj.h>

FormatEtcEnumerator::FormatEtcEnumerator(FORMATETC *pEnumFormats, size_t numberItems, BOOL bRelease) :
	ref(1), cursor(0), pFormats(NULL), count(0)
{
	if (!bRelease)
	{
		pFormats = (FORMATETC*)CoTaskMemAlloc(sizeof(FORMATETC)*numberItems);
		if (pFormats)
		{
			CopyMemory(pFormats, pEnumFormats, sizeof(FORMATETC) * numberItems);
			count = numberItems;
		}
	}
	else
	{
		pFormats = pEnumFormats;
		count = numberItems;
	}
}

FormatEtcEnumerator::~FormatEtcEnumerator()
{
	if (NULL != pFormats)
		CoTaskMemFree(pFormats);

}

STDMETHODIMP_(ULONG) FormatEtcEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) FormatEtcEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP FormatEtcEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IEnumFORMATETC))
		*ppvObject = (IEnumFORMATETC*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP FormatEtcEnumerator::Next(ULONG celt, LPFORMATETC pFE, ULONG *puFetched)
{
	ULONG cReturn = 0L;
	if(celt <= 0 || pFE == NULL || cursor >= count)
		return S_FALSE;

	if(puFetched == NULL && celt != 1)
		return S_FALSE;

	if(puFetched != NULL)
		*puFetched = 0;

	while(cursor < count && celt > 0)
	{
		*pFE++ = pFormats[cursor++];
		cReturn++;
		celt--;
	}

	if(NULL != puFetched)
		*puFetched = (cReturn - celt);

	return S_OK;
}

STDMETHODIMP FormatEtcEnumerator::Skip(ULONG celt)
{
	if((cursor + celt) >= count)
		return S_FALSE;
	cursor += celt;
	return S_OK;
}

STDMETHODIMP FormatEtcEnumerator::Reset(void)
{
	cursor = 0;
	return S_OK;
}


STDMETHODIMP FormatEtcEnumerator::Clone(IEnumFORMATETC **ppCloneEnumFormatEtc)
{
	FormatEtcEnumerator *pClone;
	if(NULL == ppCloneEnumFormatEtc)
		return S_FALSE;

	pClone = new FormatEtcEnumerator(pFormats, count, FALSE);
	if(NULL == pClone)
		return E_OUTOFMEMORY;

	pClone->AddRef();
	pClone->cursor = cursor;

	*ppCloneEnumFormatEtc = pClone;
	return S_OK;
}
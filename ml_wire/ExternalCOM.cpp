#include "main.h"
#include "./externalCOM.h"
#include "./util.h"
#include "./rssCOM.h"

#define DISPTABLE_CLASS	 ExternalCOM

DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPATCH_PODCAST, L"Podcast", OnPodcast)
DISPTABLE_END

#undef DISPTABLE_CLASS

ExternalCOM::ExternalCOM() 
	: ref(1)
{
}

ExternalCOM::~ExternalCOM()
{
}

HRESULT ExternalCOM::CreateInstance(ExternalCOM **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new ExternalCOM();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

STDMETHODIMP_(ULONG) ExternalCOM::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ExternalCOM::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ExternalCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;

	if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


HRESULT ExternalCOM::OnPodcast(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (NULL != pvarResult)
	{
		VariantInit(pvarResult);

		RssCOM *rss;
		if (SUCCEEDED(RssCOM::CreateInstance(&rss)))
		{
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = rss;
		}
		else
		{
			V_VT(pvarResult) = VT_NULL;
		}

	}
	return S_OK;
}
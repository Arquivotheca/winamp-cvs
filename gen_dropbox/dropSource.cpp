#include "./dropSource.h"

DropSource::DropSource(HWND hwndSource) : 
	ref(1), rearrangeCursor(NULL), flags(DROPSOURCE_NORMALMODE), hSource(hwndSource)
{
}

DropSource::~DropSource()
{

}


STDMETHODIMP_(ULONG) DropSource::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) DropSource::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP DropSource::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDropSource))
		*ppvObject = (IDropSource*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP DropSource::GiveFeedback(DWORD dwEffect)
{
	if (0 != (DROPSOURCE_REARRANGEMODE & flags) && 
		DROPEFFECT_LINK == dwEffect)
	{
		if (NULL == rearrangeCursor)
		{
			rearrangeCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_HAND), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE);
			if (NULL == rearrangeCursor)
				return DRAGDROP_S_USEDEFAULTCURSORS;
		}
		SetCursor(rearrangeCursor);
		return S_OK;
	}
	return DRAGDROP_S_USEDEFAULTCURSORS;
}


STDMETHODIMP DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if(fEscapePressed)
		return DRAGDROP_S_CANCEL;

	if(!(grfKeyState & MK_LBUTTON))
		return DRAGDROP_S_DROP;

	return S_OK;
}

UINT DropSource::GetFlags(UINT flagsMask)
{
	return (flags & flagsMask);
}

void DropSource::SetFlags(UINT newFlags, UINT flagsMask)
{
	flags = ((flags & ~flagsMask) | newFlags);
}

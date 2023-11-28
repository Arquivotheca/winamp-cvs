#ifndef NULLSOFT_DROPBOX_PLUGIN_CANCELOPERATION_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_CANCELOPERATION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

EXTERN_C const IID IID_ICancelOperation;

MIDL_INTERFACE("52370A59-454F-4694-AB67-CDD47EE3A09B")
ICancelOperation : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetSignal(BOOL *pSignal) = 0;
};


#endif //NULLSOFT_DROPBOX_PLUGIN_CANCELOPERATION_INTERFACE_HEADER
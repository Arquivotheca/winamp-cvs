#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./fileInfoInterface.h"

EXTERN_C const IID IID_IFileEnumerator;

MIDL_INTERFACE("45B99086-C19D-43a3-90DE-2E95081462F9")
IFileEnumerator : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched) = 0;
	virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt) = 0;
	virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;
};


#define E_FILEENUM_CREATEINFO_FAILED		MAKE_HRESULT(1, FACILITY_ITF, 0x0201)

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_INTERFACE_HEADER
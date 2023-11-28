#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_HDROP_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_HDROP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"
#include <shlobj.h>


class TypeCollection;


class HDropFileEnumerator : public IFileEnumerator
{
protected:
	HDropFileEnumerator(STGMEDIUM *pStorageMedium);
	~HDropFileEnumerator();

public:
	static HRESULT CreateEnumerator(IDataObject *pDataObject, IFileEnumerator **pEnumerator);

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);

protected:
	ULONG ref;
	STGMEDIUM stgmed;
	DROPFILES *pDropFiles;
	LPCTSTR cursor;
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_HDROP_HEADER
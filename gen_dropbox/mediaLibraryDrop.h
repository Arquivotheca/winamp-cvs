#ifndef NULLOSFT_DROPBOX_PLUGIN_MEDIALIBRARY_DROP_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_MEDIALIBRARY_DROP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "../gen_ml/ml.h"
#include "./cfpInterface.h"

__interface IFileEnumerator;

class MlDropItemProcessor : public IClipboardFormatProcessor
{
public:
	MlDropItemProcessor(mlDropItemStruct *pDropItem, HWND hwndDropBox);
	~MlDropItemProcessor();

public:
	static BOOL CanProcess(mlDropItemStruct *pDropItem);
	static HRESULT GetFileEnumerator(mlDropItemStruct *pDropItem, IFileEnumerator **ppfe);
public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IClipboardFormatProcessor ***/
	STDMETHOD(Process)(INT iInsert);

	void SetDropItem(mlDropItemStruct *pDropItem);
private:
	ULONG ref;
	HWND hDropBox;
	mlDropItemStruct *pdis;
};


#endif // NULLOSFT_DROPBOX_PLUGIN_MEDIALIBRARY_DROP_HEADER
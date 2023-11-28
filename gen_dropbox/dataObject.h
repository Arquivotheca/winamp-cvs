#ifndef NULLOSFT_DROPBOX_PLUGIN_DATAOBJECT_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DATAOBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "../nu/vector.h"

interface IFileInfo;

#define DBCF_VIEWITEMS		TEXT("WADBCFVIEWITEMS")

typedef struct __DROPVIEWITEMS
{
	DWORD pItems;	//offset
	HWND hwndList;
	INT nCount;
	INT iTop;
} DROPVIEWITEMS;

class DataObject : public IDataObject
{
protected:
	typedef struct 
	{
		FORMATETC format;
		STGMEDIUM storage;
	} ENTRY;

public:
	DataObject();
	~DataObject();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IDataObject ***/
	STDMETHOD(GetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
    STDMETHOD(GetDataHere)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
	STDMETHOD(QueryGetData)(FORMATETC *pFormatEtc);
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
    STDMETHOD(SetData)(FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
	STDMETHOD(DAdvise)(FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppEnumAdvise);

	
protected:
	HRESULT FindFormatEtc(FORMATETC *pfe, ENTRY  **ppEntry, BOOL fAdd);
	HRESULT AddRefStgMedium(STGMEDIUM *pstgmIn, STGMEDIUM *pstgmOut, BOOL fCopyIn);

protected:
	typedef Vector<ENTRY, 8> DATALIST;

protected:
	ULONG ref;
	DATALIST	dataList;
};

#endif //NULLOSFT_DROPBOX_PLUGIN_DATAOBJECT_HEADER
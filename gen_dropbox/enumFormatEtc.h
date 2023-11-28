#ifndef NULLOSFT_DROPBOX_PLUGIN_ENUM_FORMATETC_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_ENUM_FORMATETC_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>


class FormatEtcEnumerator : public IEnumFORMATETC
{
public:
	FormatEtcEnumerator(FORMATETC *pEnumFormats, size_t numberItems, BOOL bRelease); // if bRelease == TRUE data will be freed using CoTaskMemFree
	~FormatEtcEnumerator();

public:

	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IEnumFORMATETC ***/
	STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG*);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC**);

protected:
	ULONG ref;
	FORMATETC *pFormats;
	size_t count;
	size_t cursor;

};

#endif //NULLOSFT_DROPBOX_PLUGIN_ENUM_FORMATETC_HEADER
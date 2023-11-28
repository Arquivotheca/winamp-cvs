#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_LINK_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_LINK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"
#include <shobjidl.h>

class TypeCollection;


class LinkFileEnumerator : public IFileEnumerator
{
public:
	LinkFileEnumerator(LPCTSTR pszLinkFile);
	~LinkFileEnumerator();
	
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
	LPTSTR pszLink;
	IShellLinkW *pLink;
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_PLAYLIST_HEADER
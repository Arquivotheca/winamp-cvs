#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FOLDER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FOLDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"

class TypeCollection;


class FolderFileEnumerator : public IFileEnumerator
{
public:
	FolderFileEnumerator(LPCTSTR  pszFolder);
	~FolderFileEnumerator();

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
	TCHAR szPath[MAX_PATH];
	HANDLE hFind;
	WIN32_FIND_DATA fData;

};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FOLDER_HEADER
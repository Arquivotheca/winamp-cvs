#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FILENAMES_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FILENAMES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"

class TypeCollection;


class FileNamesEnumeratorA : public IFileEnumerator
{
public:
	FileNamesEnumeratorA(LPCSTR pszFileNames);
	~FileNamesEnumeratorA();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, PVOID *);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG, IFileInfo **, ULONG *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);

protected:
	ULONG ref;
	LPSTR pszFiles;
	LPCSTR pszCursor;	
};

class FileNamesEnumeratorW : public IFileEnumerator
{
public:
	FileNamesEnumeratorW(LPCWSTR pszFileNames);
	~FileNamesEnumeratorW();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, PVOID *);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG, IFileInfo **, ULONG *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);

protected:
	ULONG ref;
	LPWSTR pszFiles;
	LPCWSTR pszCursor;	
};
#endif // NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_FILENAMES_HEADER
#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_ITEMRECORDLIST_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_ITEMRECORDLIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"
#include "./fileMetaInterface.h"
#include "../gen_ml/ml.h"

class TypeCollection;

class ItemRecordListEnumerator : public IFileEnumerator
{
public:
	ItemRecordListEnumerator(itemRecordList *pItemRecordList, BOOL bUnicode);
	~ItemRecordListEnumerator();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, PVOID *);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG, IFileInfo **, ULONG *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);

public:
	void SetKeyFilter(METAKEY *pMetaKey, INT keyCount); // if set will read only specified keys (pMetaKey = NULL to remove

private:
	HRESULT CreateFileInfoA(itemRecord *pRecord, IFileInfo **ppFileInfo);
	HRESULT CreateFileInfoW(itemRecordW *pRecord, IFileInfo **ppFileInfo);
	LPCTSTR ConvertToUnicode(UINT codePage, LPCSTR pszStringIn);

protected:
	ULONG ref;
	itemRecord *pRecords;
	INT			recordCount;
	BOOL unicode;
	INT cursor;	
	LPTSTR  pszBuffer;
	INT		cchBufferMax;
	METAKEY *pMetaFilter;
	INT filterCount;
};

#endif // NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_ITEMRECORDLIST_HEADER
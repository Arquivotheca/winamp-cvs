#ifndef NULLSOFT_DROPBOX_PLUGIN_GENERICINFOREADER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_GENERICINFOREADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./fileMetaInterface.h"
#include "./genericItemMeta.h"

#define MAX_KEYCOUNT  64

class GenericMetaReader : public IFileMetaReader
{
public:
	typedef Vector<FILEMETARECORD> FILEMETARECORDLIST;
	typedef enum
	{
		READERSOURCE_FILEINFO = 0x0001,
		READERSOURCE_MLDB = 0x0002,
	} READERSOURCE;

public:
	GenericMetaReader(GenericItemMeta *pMetaObject, const METAKEY *pKeysToRead, size_t keysToReadCount, INT readMode);
	virtual ~GenericMetaReader();

public:
	static BOOL CanRead(METAKEY metaKey);
public:

	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileMetaReader ***/
	STDMETHOD(Read)(void);
	STDMETHOD(SetCookie)(DWORD cookie);
	STDMETHOD(GetCookie)(DWORD *pCookie);

	void EnableSource(INT readerSource, BOOL bEnable);
	INT GetSource(void);

protected:
	void ReadDataFromMldb(LPCWSTR pszPath, FILEMETARECORDLIST *pRecords, METAKEY *pReadKeys, size_t readKeyCount);
	void ReadDataFromExtenedInfo(LPCWSTR pszPath, FILEMETARECORDLIST *pRecords, METAKEY *pReadKeys, size_t readKeyCount);
	BOOL BeginCdRead(LPCWSTR pszPath);
	void EndCdRead(LPCWSTR pszPath);

protected:
	ULONG ref;
	GenericItemMeta *pObject;
	METAKEY szKeys[MAX_KEYCOUNT];
	size_t keyCount;
	INT mode;
	INT source;
	DWORD cookie;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_GENERICINFOREADER_HEADER
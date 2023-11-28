#ifndef NULLSOFT_DROPBOX_PLUGIN_GENERIC_ITEMMETA_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_GENERIC_ITEMMETA_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./fileMetaInterface.h"
#include "../nu/vector.h"

class GenericItemMeta : public IFileMeta
{
public:
	
#pragma pack(push, 1) 

	typedef struct __DIRECTRECORD
	{	
		WORD			state;
		METAVALUE	value;
	} DIRECTRECORD;

	typedef struct __EXRECORD
	{		
		METAKEY		key;
		WORD			state;
		METAVALUE	value;
	} EXRECORD;

#pragma pack(pop) 

	typedef enum
	{
		ADDMETAFLAG_MAKECOPY	 = 0x0001,
		ADDMETAFLAG_MARKMODIFIED = 0x0002,
		ADDMETAFLAG_MARKREAD = 0x0004,
	} ADDMETAFLAGS;

	typedef Vector<EXRECORD> EXRECORDLIST;

public:
	GenericItemMeta();
	virtual ~GenericItemMeta();

	/*** IFileMeta ***/
	STDMETHOD(GetState)(METAKEY, INT *);
	STDMETHOD(QueryValue)(METAKEY, METAVALUE *);
	STDMETHOD(QueryValueHere)(METAKEY, METAVALUE *, VOID *, size_t);
	STDMETHOD(SetValue)(METAKEY, METAVALUE *, BOOL);
	STDMETHOD(SetRecords)(FILEMETARECORD *, size_t, INT, BOOL, BOOL);
	STDMETHOD(RemoveValue)(METAKEY);
	STDMETHOD(GetReader)(METAKEY *, INT, INT, IFileMetaReader **);
	STDMETHOD(CanRead)(METAKEY);
	STDMETHOD(FilterKeys)(METAKEY *, INT *, INT); 
	STDMETHOD(Clear)();

	HRESULT AddMetaRecords(FILEMETARECORD *pRecords, size_t count, INT readMode, INT addFlags);
	BOOL NeedRead(METAKEY *, INT);
	void SetState(METAKEY *keyList, size_t keyCount, UINT state, BOOL bSet, BOOL bForceAdd);
protected:

	HRESULT QueryValueReal(METAKEY metaKey, METAVALUE *pMetaValue, VOID *pBuffer, size_t cbBuffer, BOOL bStrict);
	
private:
	friend static int __cdecl GenericItemMeta_SearchCompare(const void *key, const void *elem);
	friend static int __cdecl GenericItemMeta_SortCompare(const void *elem1, const void *elem2);


protected:
	DIRECTRECORD szDirectRecords[17];
	EXRECORDLIST exRecords;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_GENERIC_ITEMMETA_HEADER
#include "./main.h"
#include "./metaKeyStore.h"
#include "./fileMetaInterface.h"
#include "./plugin.h"
#include "../nu/ptrList.h"

typedef struct __METAKERRECORD
{
	LPSTR	name;
	METAKEY	id;
} METAKEYRECORD;

typedef nu::PtrList<METAKEYRECORD> METAKEYSTORE;

static METAKEYSTORE *metaKeyStore = NULL;
static CRITICAL_SECTION	storeLock;

static void CALLBACK UninitializeMetaKeyStore(void)
{
	EnterCriticalSection(&storeLock);

	if (NULL != metaKeyStore)
	{
		size_t index = metaKeyStore->size();
        while(index--)
		{
			if (NULL != metaKeyStore->at(index)->name)
				lfh_free(metaKeyStore->at(index)->name);

			lfh_free(metaKeyStore->at(index));
		}
		metaKeyStore->clear();
		delete (metaKeyStore);
		metaKeyStore = NULL;
	}

	LeaveCriticalSection(&storeLock);

	DeleteCriticalSection(&storeLock);
}

static void InitializeMetaKeyStore()
{
	InitializeCriticalSection(&storeLock);
	Plugin_RegisterUnloadCallback(UninitializeMetaKeyStore);
	metaKeyStore = new METAKEYSTORE();
}

static int __cdecl MetaKeyStore_SearchComparer(const void *key, const void *elem)
{
	METAKEYRECORD* rec;
	rec = (METAKEYRECORD*)elem;
	rec = *((METAKEYRECORD**)elem);
	
	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (LPCSTR)key, -1, (*((METAKEYRECORD**)elem))->name, -1) - 2;
}

static int __cdecl MetaKeyStore_SortComparer(const void *elem1, const void *elem2)
{
	return CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (*((METAKEYRECORD**)elem1))->name, -1, (*((METAKEYRECORD**)elem2))->name, -1) - 2;
}

static INT GetNextFreeId()
{
	static int id = 0;
	return ++id;
}

static HRESULT AddMetaKey(LPCSTR pszKeyName, METAKEY *pMetaKeyOut)
{
	METAKEYRECORD *prec = (METAKEYRECORD*)lfh_malloc(sizeof(METAKEYRECORD));
	if (NULL == prec)
		return E_OUTOFMEMORY;

	INT cbLen = (lstrlenA(pszKeyName) + 1) * sizeof(CHAR);
	prec->name = (LPSTR)lfh_malloc(cbLen);
	if (NULL == prec->name)
	{
		lfh_free(prec);
		return E_OUTOFMEMORY;
	}
	CopyMemory(prec->name, pszKeyName, cbLen);

	prec->id = GetNextFreeId();
	metaKeyStore->push_back(prec);
	
	if (NULL != pMetaKeyOut)
		*pMetaKeyOut = prec->id;

	return S_OK;
}

static void AddDefaultKeys()
{
	EnterCriticalSection(&storeLock);
	AddMetaKey(METAKEY_TRACKARTIST_STRING, NULL);
	AddMetaKey(METAKEY_TRACKALBUM_STRING, NULL);
	AddMetaKey(METAKEY_TRACKTITLE_STRING, NULL);
	AddMetaKey(METAKEY_TRACKGENRE_STRING, NULL);
	AddMetaKey(METAKEY_TRACKCOMMENT_STRING, NULL);
	AddMetaKey(METAKEY_TRACKLENGTH_STRING, NULL);
	AddMetaKey(METAKEY_TRACKBITRATE_STRING, NULL);
	AddMetaKey(METAKEY_TRACKNUMBER_STRING, NULL);
	AddMetaKey(METAKEY_TRACKCOUNT_STRING, NULL);
	AddMetaKey(METAKEY_DISCNUMBER_STRING, NULL);
	AddMetaKey(METAKEY_DISCCOUNT_STRING, NULL);
	AddMetaKey(METAKEY_TRACKYEAR_STRING, NULL);
	AddMetaKey(METAKEY_TRACKPUBLISHER_STRING, NULL);
	AddMetaKey(METAKEY_TRACKCOMPOSER_STRING, NULL);
	AddMetaKey(METAKEY_ALBUMARTIST_STRING, NULL);
	AddMetaKey(METAKEY_FORMATTEDTITLE_STRING, NULL);
	AddMetaKey(METAKEY_TRACKBPM_STRING, NULL);
	qsort(metaKeyStore->begin(), metaKeyStore->size(), sizeof(METAKEYRECORD*), MetaKeyStore_SortComparer);
	LeaveCriticalSection(&storeLock);
}

HRESULT RegisterMetaKey(LPCSTR pszKeyName, METAKEY *pMetaKeyOut)
{		
	if (NULL == metaKeyStore)
	{
		InitializeMetaKeyStore();
		if (NULL == metaKeyStore)
			return E_OUTOFMEMORY;

		AddDefaultKeys();
	}	
	
	EnterCriticalSection(&storeLock);
	HRESULT hr = GetMetaKeyByName(pszKeyName, pMetaKeyOut);
	if (E_METAKEY_UNKNOWN == hr)
	{
		hr = AddMetaKey(pszKeyName, pMetaKeyOut);
		if (SUCCEEDED(hr))
			qsort(metaKeyStore->begin(), metaKeyStore->size(), sizeof(METAKEYRECORD*), MetaKeyStore_SortComparer);
	}
	
	LeaveCriticalSection(&storeLock);
	return hr;
}

HRESULT GetMetaKeyByName(LPCSTR pszKeyName, METAKEY *pMetaKeyOut)
{
	if (NULL == pMetaKeyOut)
		return E_POINTER;

	*pMetaKeyOut = METAKEY_INVALID;

	if (NULL == pszKeyName || '\0' == pszKeyName)
		return E_INVALIDARG;
	
	if (NULL == metaKeyStore)
	{
		InitializeMetaKeyStore();
		if (NULL == metaKeyStore)
			return E_OUTOFMEMORY;
		AddDefaultKeys();
	}

	EnterCriticalSection(&storeLock);

	METAKEYRECORD *pRecord = *((METAKEYRECORD**)bsearch(pszKeyName, metaKeyStore->begin(), 
														metaKeyStore->size(), sizeof(METAKEYRECORD*), 
														MetaKeyStore_SearchComparer));
	if (NULL != pRecord)
		*pMetaKeyOut = pRecord->id;

	LeaveCriticalSection(&storeLock);
	return (0 != *pMetaKeyOut) ? S_OK : E_METAKEY_UNKNOWN;
}

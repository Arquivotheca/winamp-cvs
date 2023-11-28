#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEMETA_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEMETA_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./metaKeyStore.h"

typedef enum __METATYPE 
{
	METATYPE_EMPTY,
	METATYPE_INT32,
	METATYPE_WSTR,
	METATYPE_BSTR,
} METATYPE;

#pragma pack(push, 1) 

typedef struct __METAVALUE
{
	BYTE type;
	union
	{
		INT iVal;			// METATYPE_I32
		LPWSTR pwVal;		// METATYPE_WSTR
		BSTR  bstrVal;
	};
} METAVALUE;

typedef struct __FILEMETARECORD
{
	METAKEY		key;
	METAVALUE	value;
} FILEMETARECORD;

#pragma pack(pop) 

typedef enum 
{
	METAREADMODE_NORMAL = 0,
	METAREADMODE_REPLACE = 1,
} METAREADMODE;

typedef enum
{
	METAOPERATION_NONE = 0x0000,
	METAOPERATION_READ = 0x0001,
	METAOPERATION_WRITE = 0x0002,
} METAOPERATION;

typedef enum
{
	METARECORD_SET			= 0x0001, // internal use
    METARECORD_MODIFIED		= 0x0002,
	METARECORD_READ			= 0x0004, // 
	METARECORD_READING		= 0x0010,
	METARECORD_WRITING		= 0x0020,
} METARECORDSTATE;

void ReleaseMetaValue(METAVALUE *pValue);
HRESULT DuplicateMetaValue(METAVALUE *pValueOut, const METAVALUE *pValueIn);
HRESULT MetaValueWStrW(METAVALUE *pValue, LPCWSTR pszStringIn);
HRESULT MetaValueInt32(METAVALUE *pValue, INT nValueIn);
HRESULT MetaValueEmpty(METAVALUE *pValue);

interface IFileMeta;
interface IFileMetaReader;

// {275C467E-2F84-4665-A76B-ECA055D7830D}
EXTERN_C const IID IID_IFileMeta;

// {5DA12CED-41BA-4583-9CCC-F89C8B0EC825}
EXTERN_C const IID IID_IFileMetaReader;


#define E_METAKEY_UNKNOWN		MAKE_HRESULT(1, FACILITY_ITF, 0x0201)
#define E_METAVALUE_BADFORMAT	MAKE_HRESULT(1, FACILITY_ITF, 0x0202)
#define E_METAVALUE_NOTSET		MAKE_HRESULT(1, FACILITY_ITF, 0x0203)
#define E_META_NOTHINGTOREAD		MAKE_HRESULT(1, FACILITY_ITF, 0x0204)


MIDL_INTERFACE("275C467E-2F84-4665-A76B-ECA055D7830D")
IFileMeta : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetState(METAKEY metaKey, INT *pState) = 0;
	/*
		Parameters:
			metaKey  -	[in]	  meta key to use. 
			pValue	-	[in][out] on input type must contain requested type, on out meta value if key located and type matches.
			
		Return:
			S_OK					- success
			E_OUTOFMEMORY			- standard COM return value
			E_INVALIDARG			- standard COM return value
			E_METAKEY_UNKNOWN		- there is no value with such key
			E_METAVALUE_BADFORMAT	- 
			E_METAVALUE_NOTSET		- there is no value with such key, but data was never readed and\or set
		Remarks:
			In case of E_METAVALUE_NOTSET returned back you can try to request IFileMetaReader interface and call IFileMetaReader::Read()
	 */
	virtual HRESULT STDMETHODCALLTYPE QueryValue(METAKEY metaKey, METAVALUE *pValue) = 0;

	/*
		Parameters:
			metaKey  -	[in]	  meta key to use. 
			pValue	-	[in][out] on input type must contain requested type, on out meta value if key located and type matches.
			
		Return:
			S_OK					- success
			E_OUTOFMEMORY			- standard COM return value
			E_INVALIDARG			- standard COM return value
			E_METAKEY_UNKNOWN		- there is no value with such key
			E_METAVALUE_BADFORMAT	- 
			E_METAVALUE_NOTSET		- there is no value with such key, but data was never readed and\or set
		Remarks:
			In case of E_VALUE_NOTSET returned back you can try to request IFileMetaReader interface and call IFileMetaReader::Read()
	 */
	virtual HRESULT STDMETHODCALLTYPE QueryValueHere(METAKEY metaKey, METAVALUE *pMetaValue, VOID *pBuffer, size_t cbBuffer) = 0;

	/*
		Return:
			S_OK				- success
			E_OUTOFMEMORY		- standard COM return value
			E_INVALIDARG		- standard COM return value
			E_NOTIMPL			- standard COM return value
	 */
	virtual HRESULT STDMETHODCALLTYPE SetValue(METAKEY metaKey, METAVALUE *pValue, BOOL bMakeCopy) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetRecords(FILEMETARECORD *pRecords, size_t count, INT readMode, BOOL bMakeCopy, BOOL bMarkModified) = 0;
	/*
		Return:
			S_OK				- success
			E_METAKEY_UNKNOWN	- there is no such key
			E_NOTIMPL			- if you not support removing
	 */
	virtual HRESULT STDMETHODCALLTYPE RemoveValue(METAKEY metaKey) = 0;

	/*
		Return:
			S_OK				- success
			E_OUTOFMEMORY		- standard COM return value
			E_INVALIDARG		- standard COM return value
			E_NOTIMPL			- if you not support reading
			E_METAKEY_UNKNOWN	- there is no value with such key
	 */
	virtual HRESULT STDMETHODCALLTYPE GetReader(METAKEY *metaKey, INT keyCount, INT readMode, IFileMetaReader **ppMetaReader) = 0;


	/*
		Return:
			S_OK				- success
			E_NOTIMPL			- if you not support reading
			E_METAKEY_UNKNOWN	- no reader can read this 
	 */
	virtual HRESULT STDMETHODCALLTYPE CanRead(METAKEY metaKey) = 0;
	
	/*
		filterMask - METARECORDSTATE values that you interesting in
		keyCount - on input: count of keys in metaKey, on output count of keys that passed filter.
		metaKey - will be modified.!!!
	*/
	virtual HRESULT STDMETHODCALLTYPE FilterKeys(METAKEY *metaKey, INT *keyCount, INT stateFilter) = 0; 


	virtual HRESULT STDMETHODCALLTYPE Clear() = 0;

};

MIDL_INTERFACE("5DA12CED-41BA-4583-9CCC-F89C8B0EC825")
IFileMetaReader : public IUnknown
{
	/*
		Return:
			S_OK				- success
			E_OUTOFMEMORY		- standard COM return value
			E_UNEXPECTED		- standard COM return value
			E_FAIL				- standard COM return value
	 */
	virtual HRESULT STDMETHODCALLTYPE Read(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCookie(DWORD /*cookie*/) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCookie(DWORD* /*pCookie*/) = 0;

};

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEMETA_INTERFACE_HEADER
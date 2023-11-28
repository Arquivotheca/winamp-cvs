#include "./main.h"
#include "./plugin.h"
#include "./feItemRecordList.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "../nu/AutoWideFn.h"

static void CopyItemRecordA(itemRecord *dst, const itemRecord *src)
{
	if (NULL == src)
		ZeroMemory(dst, sizeof(itemRecord));
	else
	{		
		dst->album		= lfh_strdupA(src->album);
		dst->artist		= lfh_strdupA(src->artist);
		dst->comment		= lfh_strdupA(src->comment);
		dst->filename	= lfh_strdupA(src->filename);
		dst->genre		= lfh_strdupA(src->genre);
		dst->title		= lfh_strdupA(src->title);
		dst->year		= src->year;
		dst->track		= src->track;
		dst->length		= src->length;
		dst->extended_info = NULL;
	}
}

static void FreeItemRecordA(itemRecord *record)
{			
	if (NULL != record)
	{
		if (NULL != record->album)
			lfh_free(record->album);
		if (NULL != record->album)
			lfh_free(record->artist);
		if (NULL != record->album)
			lfh_free(record->comment);
		if (NULL != record->album)
			lfh_free(record->filename);
		if (NULL != record->album)
			lfh_free(record->genre);
		if (NULL != record->album)
			lfh_free(record->title);
	}
}

static void CopyItemRecordW(itemRecordW *dst, const itemRecordW *src)
{
	if (NULL == src)
		ZeroMemory(dst, sizeof(itemRecord));
	else
	{		
		dst->album		= lfh_strdupW(src->album);
		dst->artist		= lfh_strdupW(src->artist);
		dst->comment		= lfh_strdupW(src->comment);
		dst->filename	= lfh_strdupW(src->filename);
		dst->genre		= lfh_strdupW(src->genre);
		dst->title		= lfh_strdupW(src->title);
		dst->albumartist	= lfh_strdupW(src->albumartist);
		dst->replaygain_album_gain = lfh_strdupW(src->replaygain_album_gain);
		dst->replaygain_track_gain = lfh_strdupW(src->replaygain_track_gain);
		dst->publisher	= lfh_strdupW(src->publisher);
		dst->composer	= lfh_strdupW(src->composer);
		dst->year		= src->year;
		dst->track		= src->track;
		dst->tracks		= src->tracks;
		dst->length		= src->length;
		dst->rating		= src->rating;
		dst->playcount	= src->playcount;
		dst->lastplay	= src->lastplay;
		dst->lastupd	= src->lastupd;
		dst->filetime	= src->filetime;
		dst->filesize	= src->filesize;
		dst->bitrate	= src->bitrate;
		dst->type		= src->type;
		dst->disc		= src->disc;
		dst->discs		= src->discs;
		dst->bpm			= src->bpm;
		dst->extended_info = NULL;
	}
}

static void FreeItemRecordW(itemRecordW *record)
{			
	if (NULL != record)
	{
		if (NULL != record->album)
			lfh_free(record->album);
		if (NULL != record->artist)
			lfh_free(record->artist);
		if (NULL != record->comment)
			lfh_free(record->comment);
		if (NULL != record->filename)
			lfh_free(record->filename);
		if (NULL != record->genre)
			lfh_free(record->genre);
		if (NULL != record->title)
			lfh_free(record->title);
		if (NULL != record->albumartist)
			lfh_free(record->albumartist);
		if (NULL != record->replaygain_album_gain)
			lfh_free(record->replaygain_album_gain);
		if (NULL != record->replaygain_track_gain)
			lfh_free(record->replaygain_track_gain);
		if (NULL != record->publisher)
			lfh_free(record->publisher);
		if (NULL != record->composer)
			lfh_free(record->composer);
	}
}

ItemRecordListEnumerator::ItemRecordListEnumerator(itemRecordList *pItemRecordList, BOOL bUnicode) 
	: ref(1), pRecords(NULL), recordCount(0), cursor(0), unicode(bUnicode), pszBuffer(NULL), cchBufferMax(0), 
		pMetaFilter(NULL), filterCount(0)
{	
	if (pItemRecordList->Size > 0)
	{
		if (unicode)
		{
			pRecords = (itemRecord*)malloc(sizeof(itemRecordW) * pItemRecordList->Size);
		}
		else
		{
			pRecords = (itemRecord*)malloc(sizeof(itemRecord) * pItemRecordList->Size);
		}

	}

	if (NULL != pRecords)
		recordCount = pItemRecordList->Size;
	
	if (unicode)
	{
		for (int i =0; i < recordCount; i++)
			CopyItemRecordW(&((itemRecordW*)pRecords)[i], &((itemRecordListW*)pItemRecordList)->Items[i]);
	}
	else
	{
		for (int i =0; i < recordCount; i++)
			CopyItemRecordA(&((itemRecord*)pRecords)[i], &pItemRecordList->Items[i]);
	}

	
}


ItemRecordListEnumerator::~ItemRecordListEnumerator()
{
	if (NULL != pszBuffer)
		free(pszBuffer);
	if (NULL != pMetaFilter)
		free(pMetaFilter);

	if (NULL != pRecords)
	{
		if (unicode)
		{
			for (int i =0; i < recordCount; i++)
				FreeItemRecordW(&((itemRecordW*)pRecords)[i]);
		}
		else
		{
			for (int i =0; i < recordCount; i++)
				FreeItemRecordA(&((itemRecord*)pRecords)[i]);
		}
		free(pRecords);
	}
		
}

STDMETHODIMP_(ULONG) ItemRecordListEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ItemRecordListEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ItemRecordListEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileEnumerator))
		*ppvObject = (IFileEnumerator*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

LPCWSTR ItemRecordListEnumerator::ConvertToUnicode(UINT codePage, LPCSTR pszStringIn)
{
	if (NULL == pszStringIn)
		return NULL;

	if ('\0' == pszStringIn)
		return L'\0';

	INT cchLen = lstrlenA(pszStringIn);
	INT count = MultiByteToWideChar(codePage, 0, pszStringIn, cchLen, NULL, 0);
	if (0 == count)
		return L'\0';
	if ((count + 1) >= (cchBufferMax - 1))
	{
		cchBufferMax = ((count +1)  * 2);
		pszBuffer = (LPWSTR)realloc(pszBuffer, sizeof(WCHAR)*cchBufferMax);
		if (NULL == pszBuffer)
		{
			cchBufferMax = 0;
			return L'\0';
		}
	}

	count = MultiByteToWideChar(codePage, 0, pszStringIn, cchLen, pszBuffer, cchBufferMax);
	pszBuffer[count] = L'\0';

	return pszBuffer;
}


STDMETHODIMP ItemRecordListEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{
	if (NULL == pRecords || 0 == celt || NULL == pfiBuffer) 
		return S_FALSE;

	ULONG counter = 0;
	HRESULT hr = S_OK;
	
	while (celt)
	{
		if (cursor >= recordCount)
			break;
		
		if(unicode)
			hr = CreateFileInfoW(&((itemRecordW*)pRecords)[cursor], &pfiBuffer[counter]);
		else
			hr = CreateFileInfoA(&((itemRecord*)pRecords)[cursor], &pfiBuffer[counter]);
		
		if (FAILED(hr))
		{
			pfiBuffer[counter] = NULL;
			hr = E_FILEENUM_CREATEINFO_FAILED;
			break;
		}
		
		if (S_OK == hr)	
			counter++;
		
		celt--;
		cursor++;
	}
	if (pceltFetched) *pceltFetched = counter;
	
	if (S_OK != hr)
		return hr;
	return (counter > 0) ? S_OK : S_FALSE;
}

STDMETHODIMP ItemRecordListEnumerator::Skip(ULONG celt)
{
	if (NULL == pRecords)
		return E_POINTER;

	if((cursor + celt) >= (UINT)recordCount)
		return S_FALSE;
	cursor += celt;
	return E_NOTIMPL;
}
STDMETHODIMP ItemRecordListEnumerator::Reset(void)
{
	cursor = 0;
	return S_OK;
}

HRESULT ItemRecordListEnumerator::CreateFileInfoA(itemRecord *pRecord, IFileInfo **ppFileInfo)
{
	HRESULT hr;
	AutoWideFn fileName(pRecord->filename);
	LPCWSTR pszFile = fileName;
	if (NULL == pszFile || L'\0' == pszFile)
		return S_FALSE;
		
	hr = PLUGIN_REGTYPES->CreateItem(pszFile, NULL, ppFileInfo);

	IFileMeta *pMeta;
	if (SUCCEEDED(hr) &&
		SUCCEEDED((*ppFileInfo)->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{
		FILEMETARECORD szMetaRecords[17];
		size_t metaCount = 0;

		#define SETMETASTR_A(__metaKey, __recordField) if (NULL != pRecord->##__recordField){\
			szMetaRecords[metaCount].key = (METAKEY_##__metaKey);\
			MetaValueWStrW(&szMetaRecords[metaCount].value, ConvertToUnicode(CP_ACP, pRecord->##__recordField));\
			metaCount++;	}
		#define SETMETAINT_A(__metaKey, __recordField){\
			szMetaRecords[metaCount].key = (METAKEY_##__metaKey);\
			MetaValueInt32(&szMetaRecords[metaCount].value, pRecord->##__recordField);\
			metaCount++;	}

		#define CASE_SETMETASTR_A(__metaKey, __recordField)\
			 case (METAKEY_##__metaKey): SETMETASTR_A(##__metaKey, ##__recordField); break;
		#define CASE_SETMETAINT_A(__metaKey, __recordField)\
			 case (METAKEY_##__metaKey): SETMETAINT_A(##__metaKey, ##__recordField); break;

		if (NULL == pMetaFilter)
		{
			SETMETASTR_A(TRACKTITLE, title);
			SETMETASTR_A(TRACKALBUM, album);
			SETMETASTR_A(TRACKARTIST, artist);
			SETMETASTR_A(TRACKCOMMENT, comment);
			SETMETASTR_A(TRACKGENRE, genre);
			SETMETAINT_A(TRACKYEAR, year);
			SETMETAINT_A(TRACKNUMBER, track);
			SETMETAINT_A(TRACKLENGTH, length);
		}
		else
		{
			for(int i = 0; i < filterCount; i++)
			{
				switch(pMetaFilter[i])
				{
					CASE_SETMETASTR_A(TRACKTITLE, title);
					CASE_SETMETASTR_A(TRACKALBUM, album);
					CASE_SETMETASTR_A(TRACKARTIST, artist);
					CASE_SETMETASTR_A(TRACKCOMMENT, comment);
					CASE_SETMETASTR_A(TRACKGENRE, genre);
					CASE_SETMETAINT_A(TRACKYEAR, year);
					CASE_SETMETAINT_A(TRACKNUMBER, track);
					CASE_SETMETAINT_A(TRACKLENGTH, length);
				}
			}
		}

		if (0 != metaCount)
		{
			if (FAILED(pMeta->SetRecords(szMetaRecords, metaCount, METAREADMODE_NORMAL, FALSE, FALSE)))
			{
				while(metaCount--)
					ReleaseMetaValue(&szMetaRecords[metaCount].value);
			}
		}
        pMeta->Release();
	}

	return hr;
}

HRESULT ItemRecordListEnumerator::CreateFileInfoW(itemRecordW *pRecord, IFileInfo **ppFileInfo)
{
	HRESULT hr;
	IFileMeta *pMeta;
	if (NULL == pRecord->filename || L'\0' == pRecord->filename)
		return S_FALSE;

	hr = PLUGIN_REGTYPES->CreateItem(pRecord->filename, NULL, ppFileInfo);

	if (SUCCEEDED(hr) &&
		SUCCEEDED((*ppFileInfo)->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{
		FILEMETARECORD szMetaRecords[17];
		size_t metaCount = 0;

		#define SETMETASTR(__metaKey, __recordField) if (NULL != pRecord->##__recordField){\
			szMetaRecords[metaCount].key = (METAKEY_##__metaKey);\
			MetaValueWStrW(&szMetaRecords[metaCount].value, pRecord->##__recordField);\
			metaCount++;	}
		#define SETMETAINT(__metaKey, __recordField){\
			szMetaRecords[metaCount].key = (METAKEY_##__metaKey);\
			MetaValueInt32(&szMetaRecords[metaCount].value, pRecord->##__recordField);\
			metaCount++;	}

		#define CASE_SETMETASTR(__metaKey, __recordField)\
			 case (METAKEY_##__metaKey): SETMETASTR(##__metaKey, ##__recordField); break;
		#define CASE_SETMETAINT(__metaKey, __recordField)\
			 case (METAKEY_##__metaKey): SETMETAINT(##__metaKey, ##__recordField); break;

		if (NULL == pMetaFilter)
		{
			SETMETASTR(TRACKTITLE, title);
			SETMETASTR(TRACKALBUM, album);
			SETMETASTR(TRACKARTIST, artist);
			SETMETASTR(TRACKCOMMENT, comment);
			SETMETASTR(TRACKGENRE, genre);
			SETMETAINT(TRACKLENGTH, length);
			SETMETAINT(TRACKBITRATE, bitrate);
			SETMETAINT(TRACKNUMBER, track);
			SETMETAINT(TRACKCOUNT, tracks);
			SETMETAINT(DISCNUMBER, disc);
			SETMETAINT(DISCCOUNT, discs);
			SETMETAINT(TRACKYEAR, year);
			SETMETASTR(TRACKPUBLISHER, publisher);
			SETMETASTR(TRACKCOMPOSER, composer);
			SETMETASTR(ALBUMARTIST, albumartist);
			SETMETAINT(TRACKBPM, bpm);
		}
		else
		{
			for(int i = 0; i < filterCount; i++)
			{
				switch(pMetaFilter[i])
				{
					CASE_SETMETASTR(TRACKTITLE, title);
					CASE_SETMETASTR(TRACKALBUM, album);
					CASE_SETMETASTR(TRACKARTIST, artist);
					CASE_SETMETASTR(TRACKCOMMENT, comment);
					CASE_SETMETASTR(TRACKGENRE, genre);
					CASE_SETMETAINT(TRACKLENGTH, length);
					CASE_SETMETAINT(TRACKBITRATE, bitrate);
					CASE_SETMETAINT(TRACKNUMBER, track);
					CASE_SETMETAINT(TRACKCOUNT, tracks);
					CASE_SETMETAINT(DISCNUMBER, disc);
					CASE_SETMETAINT(DISCCOUNT, discs);
					CASE_SETMETAINT(TRACKYEAR, year);
					CASE_SETMETASTR(TRACKPUBLISHER, publisher);
					CASE_SETMETASTR(TRACKCOMPOSER, composer);
					CASE_SETMETASTR(ALBUMARTIST, albumartist);
					CASE_SETMETAINT(TRACKBPM, bpm);
				}
			}
		}

		if (0 != metaCount)
		{
			if (FAILED(pMeta->SetRecords(szMetaRecords, metaCount, METAREADMODE_NORMAL, FALSE, FALSE)))
			{
				while(metaCount--)
					ReleaseMetaValue(&szMetaRecords[metaCount].value);
			}
		}
        pMeta->Release();
	}
	return hr;
}

void ItemRecordListEnumerator::SetKeyFilter(METAKEY *pMetaKey, INT keyCount)
{
	if (NULL != pMetaFilter)
	{
		free(pMetaFilter);
		pMetaFilter = 0;
	}
	filterCount = 0;
	if (NULL == pMetaKey || keyCount < 1)
		return;
	pMetaFilter = (METAKEY*)malloc(sizeof(METAKEY) * keyCount);
	if (NULL == pMetaFilter)
		return;
	CopyMemory(pMetaFilter, pMetaKey, sizeof(METAKEY) * keyCount);
	filterCount = keyCount;

}

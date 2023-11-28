#include "./genericMetaReader.h"
#include "./wasabiAPi.h"
#include "./fileInfoInterface.h"
#include "./formattedTitleReader.h"
#include "./formattedTitleReader.cpp"
#include <strsafe.h>


#define READFILEINFO(__fileName, __tag, __pszBuffer, __cchBuffer)\
	(AGAVE_API_METADATA->GetExtendedFileInfo((__fileName), (__tag), (__pszBuffer), (__cchBuffer)))

// sets part and parts to -1 or 0 on fail/missing (e.g. parts will be -1 on "1", but 0 on "1/")
static void ParseIntSlashInt(wchar_t *string, int *part, int *parts)
{
	*part = -1;
	*parts = -1;

	if (string && string[0])
	{
		*part = _wtoi(string);
		while (*string && *string != L'/')
		{
			string++;
		}
		if (*string == L'/')
		{
			string++;
			*parts = _wtoi(string);
		}
	}
}


GenericMetaReader::GenericMetaReader(GenericItemMeta *pMetaObject, const METAKEY *pKeysToRead, size_t keysToReadCount, INT readMode) 
	: ref(1), pObject(pMetaObject), keyCount(keysToReadCount), mode(readMode), source(READERSOURCE_FILEINFO | READERSOURCE_MLDB)
{
	if (keyCount > 0)
	{
		if (keyCount > ARRAYSIZE(szKeys))
			keyCount = ARRAYSIZE(szKeys);
		CopyMemory(szKeys, pKeysToRead, sizeof(METAKEY) * keyCount);
	}
	
	if (NULL != pObject)
	{
		pObject->SetState(szKeys, keyCount, METARECORD_READING, TRUE, TRUE);
		pObject->AddRef();
	}
}

GenericMetaReader::~GenericMetaReader()
{
	if (NULL != pObject)
	{
		pObject->SetState(szKeys, keyCount, METARECORD_READING, FALSE, TRUE);
		pObject->Release();
	}
}

STDMETHODIMP_(ULONG) GenericMetaReader::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) GenericMetaReader::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP GenericMetaReader::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileMetaReader))
	{
		*ppvObject = (IFileMetaReader*)this;
		((IUnknown*)*ppvObject)->AddRef();
		return S_OK;
	}
	return pObject->QueryInterface(riid, ppvObject);
}


void GenericMetaReader::ReadDataFromMldb(LPCWSTR pszPath, FILEMETARECORDLIST *pRecords, METAKEY *pReadKeys, size_t readKeyCount)
{

	#define MLDB2METASTR(__metaKeyId, __itemRecordField)\
	case (METAKEY_##__metaKeyId):{\
		metaRec.key = (METAKEY_##__metaKeyId);\
		MetaValueWStrW(&metaRec.value, record->##__itemRecordField);\
		pReadKeys[index] = 0;}\
		break;\

	#define MLDB2METAINT(__metaKeyId, __itemRecordField)\
	case (METAKEY_##__metaKeyId):{\
		metaRec.key = (METAKEY_##__metaKeyId);\
		MetaValueInt32(&metaRec.value, record->##__itemRecordField);\
		pReadKeys[index] = 0;}\
		break;\


	size_t index = 0;
	BOOL knownKeyFound = FALSE;
	for (index; index < readKeyCount && !knownKeyFound; index++)
	{
		switch(pReadKeys[index])
		{
			case METAKEY_TRACKTITLE:
			case METAKEY_TRACKALBUM:
			case METAKEY_TRACKARTIST:
			case METAKEY_TRACKCOMMENT:
			case METAKEY_TRACKGENRE:
			case METAKEY_TRACKLENGTH:
			case METAKEY_TRACKBITRATE:
			case METAKEY_TRACKNUMBER:
			case METAKEY_TRACKCOUNT:
			case METAKEY_DISCNUMBER:
			case METAKEY_DISCCOUNT:
			case METAKEY_TRACKYEAR:
			case METAKEY_TRACKPUBLISHER:
			case METAKEY_TRACKCOMPOSER:
			case METAKEY_ALBUMARTIST:
			case METAKEY_TRACKBPM:
				knownKeyFound = TRUE;
				break;
		}
	}
	if (!knownKeyFound)
		return; // we not supoort this key read

	if (NULL == AGAVE_API_MLDB &&  
		NULL == (AGAVE_API_MLDB = QueryWasabiInterface(api_mldb, mldbApiGuid)))
		return;

	itemRecordW *record = AGAVE_API_MLDB->GetFile(pszPath);
	if (NULL == record)
		return;

	FILEMETARECORD metaRec;

	for (index = 0; index < readKeyCount; index++)
	{
		metaRec.key = -1;
		switch(pReadKeys[index])
		{		
			MLDB2METASTR(TRACKTITLE, title);
			MLDB2METASTR(TRACKALBUM, album);
			MLDB2METASTR(TRACKARTIST, artist);
			MLDB2METASTR(TRACKCOMMENT, comment);
			MLDB2METASTR(TRACKGENRE, genre);
			MLDB2METAINT(TRACKLENGTH, length);
			MLDB2METAINT(TRACKBITRATE, bitrate);
			MLDB2METAINT(TRACKNUMBER, track);
			MLDB2METAINT(TRACKCOUNT, tracks);
			MLDB2METAINT(DISCNUMBER, disc);
			MLDB2METAINT(DISCCOUNT, discs);
			MLDB2METAINT(TRACKYEAR, year);
			MLDB2METASTR(TRACKPUBLISHER, publisher);
			MLDB2METASTR(TRACKCOMPOSER, composer);
			MLDB2METASTR(ALBUMARTIST, albumartist);
			MLDB2METAINT(TRACKBPM, bpm);
		}
		if (((WORD)-1) != metaRec.key)
			pRecords->push_back(metaRec);
	}

	AGAVE_API_MLDB->FreeRecord(record);
}


void GenericMetaReader::ReadDataFromExtenedInfo(LPCWSTR pszPath, FILEMETARECORDLIST *pRecords, METAKEY *pReadKeys, size_t readKeyCount)
{	

	#define FILEINFO2METASTR(__metaKeyId, __tag)\
	case (METAKEY_##__metaKeyId):{\
		metaRec.key = (METAKEY_##__metaKeyId);\
		READFILEINFO(pszPath, __tag, szBuffer, ARRAYSIZE(szBuffer));\
		if (L'\0' != szBuffer[0]) MetaValueWStrW(&metaRec.value, szBuffer);\
		else MetaValueEmpty(&metaRec.value);\
		pReadKeys[index] = 0;}\
		break;

	#define FILEINFO2METAINT(__metaKeyId, __tag)\
	case (METAKEY_##__metaKeyId):{\
		metaRec.key = (METAKEY_##__metaKeyId);\
		READFILEINFO(pszPath, __tag, szBuffer, ARRAYSIZE(szBuffer));\
		if (L'\0' != szBuffer[0]) MetaValueInt32(&metaRec.value, _wtoi(szBuffer));\
		else MetaValueEmpty(&metaRec.value);\
		pReadKeys[index] = 0;}\
		break;


	size_t index = 0;
	for (index; index < readKeyCount && ((((SHORT)pReadKeys[index]) < 1) || METAKEY_FORMATTEDTITLE == pReadKeys[index]); index++);
	if (index == readKeyCount)
		return; // nothing to read
	
	if (NULL == AGAVE_API_METADATA && 
		NULL == (AGAVE_API_METADATA = QueryWasabiInterface(api_metadata, api_metadataGUID)))
		return;

	WCHAR szBuffer[8192];
	INT trackNum, trackCount, discNum, discCount;
	INT trackState = 0, discState = 0;
	FILEMETARECORD metaRec;

	for (index; index < readKeyCount; index++)
	{
		szBuffer[0] = L'\0';
		metaRec.key = -1;
		switch(pReadKeys[index])
		{	
			FILEINFO2METASTR(TRACKTITLE,  L"title");
			FILEINFO2METASTR(TRACKALBUM,  L"album");
			FILEINFO2METASTR(TRACKARTIST, L"artist");
			FILEINFO2METASTR(TRACKCOMMENT, L"comment");
			FILEINFO2METASTR(TRACKGENRE, L"genre");
			FILEINFO2METAINT(TRACKBITRATE, L"bitrate");
			FILEINFO2METAINT(TRACKYEAR,  L"year");
			FILEINFO2METASTR(TRACKPUBLISHER, L"publisher");
			FILEINFO2METASTR(TRACKCOMPOSER, L"composer");
			FILEINFO2METASTR(ALBUMARTIST, L"albumartist");
			FILEINFO2METAINT(TRACKBPM, L"bpm");

			case METAKEY_TRACKNUMBER:
			case METAKEY_TRACKCOUNT:
				if (0 == trackState)
				{					
					trackState = 1;
					READFILEINFO(pszPath, L"track", szBuffer, ARRAYSIZE(szBuffer));
					if (L'\0' != szBuffer[0])
					{
						ParseIntSlashInt(szBuffer, &trackNum, &trackCount); 
						trackState = 2;
					}
				}
				metaRec.key = pReadKeys[index];
				pReadKeys[index] = 0;
				if (2 == trackState) 
					MetaValueInt32(&metaRec.value, (METAKEY_TRACKCOUNT == metaRec.key) ? trackCount : trackNum);
				else if (1 == trackState)
					MetaValueEmpty(&metaRec.value);
				break;
			case METAKEY_DISCNUMBER:
			case METAKEY_DISCCOUNT:
				if (0 == discState)
				{
					szBuffer[0] = L'\0';
					discState = 1;
					READFILEINFO(pszPath, L"disc", szBuffer, ARRAYSIZE(szBuffer));
					if (L'\0' != szBuffer[0])
					{
						ParseIntSlashInt(szBuffer, &discNum, &discCount); 
						discState = 2;
					}
				}
				metaRec.key = pReadKeys[index];
				pReadKeys[index] = 0;
				if (2 == discState)
					MetaValueInt32(&metaRec.value, (METAKEY_DISCCOUNT == metaRec.key) ? discCount : discNum);
				else if (1 == discState)
					MetaValueEmpty(&metaRec.value);
                break;
			case METAKEY_TRACKLENGTH:
				metaRec.key = METAKEY_TRACKLENGTH;
				READFILEINFO(pszPath, L"length", szBuffer, ARRAYSIZE(szBuffer));
				if (L'\0' != szBuffer[0]) MetaValueInt32(&metaRec.value, _wtoi(szBuffer)/1000);
				else MetaValueEmpty(&metaRec.value);
				pReadKeys[index] = 0;
				
				break;
		}
		if (((WORD)-1) != metaRec.key)
			pRecords->push_back(metaRec);

	}
}

BOOL GenericMetaReader::BeginCdRead(LPCWSTR pszPath)
{
	TCHAR szBuffer[32];
	szBuffer[0] = L'\0';

	if (NULL == AGAVE_API_METADATA && 
		NULL == (AGAVE_API_METADATA = QueryWasabiInterface(api_metadata, api_metadataGUID)))
		return FALSE;

	if (!READFILEINFO(pszPath, L"<begin>", szBuffer, ARRAYSIZE(szBuffer)) ||
		(L'\0' != szBuffer[0] && (L'0' == szBuffer[0] &&  L'\0' == szBuffer[1])))
		return FALSE;

	return TRUE;
}

void GenericMetaReader::EndCdRead(LPCWSTR pszPath)
{
	TCHAR szBuffer[32];
	szBuffer[0] = L'\0';
	READFILEINFO(pszPath, L"<end>", szBuffer, ARRAYSIZE(szBuffer));
}

STDMETHODIMP GenericMetaReader::Read(void)
{	
	IFileInfo *pfi;
	TCHAR szPath[MAX_PATH];

	METAKEY szReadKeys[ARRAYSIZE(szKeys)];
	size_t readKeyCount = keyCount;
	
	if (NULL == pObject) 
		return E_POINTER;

	if (FAILED(pObject->QueryInterface(IID_IFileInfo, (void**)&pfi)))
		return E_FAIL;
	
	LPCTSTR pszPathHolder;
	if (FAILED(pfi->GetPath(&pszPathHolder)) || 
		FAILED(StringCchCopyEx(szPath, ARRAYSIZE(szPath), pszPathHolder, NULL, NULL, STRSAFE_IGNORE_NULLS)))
		szPath[0] = L'\0';


	DWORD itemType;
	if (FAILED(pfi->GetType(&itemType)))
		itemType = IItemType::itemTypeUnknown;

	
	if (L'\0' == szPath)
	{
		pfi->Release();
		return E_FAIL;
	}

	CopyMemory(szReadKeys, szKeys, sizeof(METAKEY) * readKeyCount);
	
	FILEMETARECORDLIST metaRecords;

	if (0 != (READERSOURCE_MLDB & source))
	{		
		switch(itemType)
		{
			case IItemType::itemTypeAudioFile:
			case IItemType::itemTypeVideoFile:
			case IItemType::itemTypeUnknown:
			case IItemType::itemTypeMissingFile:
				ReadDataFromMldb(szPath, &metaRecords, szReadKeys, readKeyCount);
				break;
		}
	}
		
	if (0 != (READERSOURCE_FILEINFO & source))
	{
		switch(itemType)
		{
			case IItemType::itemTypeAudioFile:
			case IItemType::itemTypeVideoFile:
			case IItemType::itemTypeHttpStream:
				ReadDataFromExtenedInfo(szPath, &metaRecords, szReadKeys, readKeyCount);
				break;
			case IItemType::itemTypeAudioCdTrack:
				if (BeginCdRead(szPath))
				{
					ReadDataFromExtenedInfo(szPath, &metaRecords, szReadKeys, readKeyCount);
					EndCdRead(szPath);
				}
				else
				{
					FILEMETARECORD metaRec;
					MetaValueEmpty(&metaRec.value);
					for (size_t index = 0; index < readKeyCount; index++)
					{
						metaRec.key = szReadKeys[index];
						metaRecords.push_back(metaRec);
					}
				}
				break;
		}

	}

	HRESULT hr = S_OK;

	if (0 != metaRecords.size())
	{
		INT addFlags = GenericItemMeta::ADDMETAFLAG_MARKREAD;
		hr = pObject->AddMetaRecords(metaRecords.begin(), metaRecords.size(), mode, addFlags); 
		if (FAILED(hr))
		{	
			size_t index = metaRecords.size();
			while(index--)
				ReleaseMetaValue(&metaRecords[index].value);
		}

		for (size_t index = 0; index < readKeyCount; index++)
	{
		if (METAKEY_FORMATTEDTITLE == szReadKeys[index])
		{			
			FormattedTitleReader<4096, 2048> titleReader(pfi);
			FILEMETARECORD metaRecord;
			metaRecord.key = METAKEY_FORMATTEDTITLE;
			MetaValueWStrW(&metaRecord.value, titleReader.GetTitle());
			//MetaValueWStrW(&metaRecord.value, L"--- test ---");
			INT addFlags = GenericItemMeta::ADDMETAFLAG_MARKREAD;
			if (FAILED(pObject->AddMetaRecords(&metaRecord, 1, mode, addFlags)))
				ReleaseMetaValue(&metaRecord.value);
			break;
		}
	}

	}
	else
	{
		aTRACE_LINE("empty thing");
	}
	
	
	
	pfi->Release();

	return hr;
}

BOOL GenericMetaReader::CanRead(METAKEY metaKey)
{
	switch(metaKey)
	{
		case METAKEY_TRACKARTIST:
		case METAKEY_TRACKTITLE:
		case METAKEY_TRACKALBUM:
		case METAKEY_TRACKGENRE:
		case METAKEY_TRACKCOMMENT:
		case METAKEY_TRACKPUBLISHER:
		case METAKEY_TRACKCOMPOSER:
		case METAKEY_TRACKLENGTH:
		case METAKEY_TRACKBITRATE:
		case METAKEY_TRACKYEAR:
		case METAKEY_TRACKNUMBER:
		case METAKEY_TRACKCOUNT:
		case METAKEY_DISCNUMBER:
		case METAKEY_DISCCOUNT:
		case METAKEY_ALBUMARTIST:
		case METAKEY_TRACKBPM:
			return TRUE;
	}
	return FALSE; 
}

void GenericMetaReader::EnableSource(INT readerSource, BOOL bEnable)
{
	source = (source & ~readerSource);
	if (bEnable) source |= readerSource;
}

INT GenericMetaReader::GetSource(void)
{
	return source;
}

STDMETHODIMP GenericMetaReader::SetCookie(DWORD cookie)
{
	this->cookie = cookie;
	return S_OK;
}
STDMETHODIMP GenericMetaReader::GetCookie(DWORD *pCookie)
{
	if (NULL == pCookie)
		return E_POINTER;
	*pCookie = cookie;
	return S_OK;
}
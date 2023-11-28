#include "./main.h"
#include "./plugin.h"
#include "./document.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./fileEnumInterface.h"
#include "./antiLoop.h"
#include "./extensionFilterList.h"
#include "./wasabiApi.h"
#include "./documentSaver.h"
#include "../playlist/api_playlists.h"
#include "./documentCallback.h"
#include "./documentListener.h"
#include "./fileMetaScheduler.h"
#include "./documentFilter.h"
#include "./filterPolicy.h"
#include "../nu/sort.h"
#include "./resource.h"

#include <shlwapi.h>
#include <strsafe.h>

#define DOCUMENTNAME_DEFAULTPREFIX		TEXT("Playlist")

#define LENGTH_UNKNOWN		((ULONG_PTR)-1)
#define LENGTH_NOTSUPPORTED ((ULONG_PTR)-2)

static LONG lastDocumentIndex = 0;

typedef struct __INSERTENUMPARAM
{
	Document *document;
	size_t insertBefore;
	IFileEnumerator *enumerator;
	BOOL markModified;
	IDocumentFilter *filter;
}INSERTENUMPARAM;

typedef struct 
{
	Document		*document;
	METAKEY		*metaKeys;
	INT			keysCount;
	Document::CSORTPROC	primaryCompare;
	Document::CSORTPROC	secondaryCompare;
}ORDERTHREADPARAM;

typedef struct __ORDERPARAM
{
	Document::CSORTPROC primary;
	Document::CSORTPROC secondary;
} ORDERPARAM;

Document::Document(LPCTSTR pszDocName) 
	: ref(1), pszPath(NULL), pszTitle(NULL), bModified(FALSE), sysCallback(NULL), 
	schedulerLimit(0), scheduler(NULL), scheduleListener(NULL), closing(FALSE)
{
	InitializeCriticalSection(&lockItems); 
	InitializeCriticalSection(&lockScheduler);

	SetTitle(pszDocName);
	SetModified(FALSE);
	
	sysCallback = new DocumentCallback(this);
	sysCallback->Register();
	GetSystemTimeAsFileTime(&fileModified);

	ZeroMemory(&metrics, sizeof(METRICS));
	ZeroMemory(&asyncOperation, sizeof(ASYNCOPERATION));
}

Document::~Document()
{
	if (NULL != sysCallback)
	{
		sysCallback->Unregister();
		delete(sysCallback);
	}

	if (NULL != pszTitle)
		lfh_free(pszTitle);
	if (NULL != pszPath)
		lfh_free(pszPath);

	size_t index = itemList.size();
	size_t totalCount = index;
	
	CloseScheduler(FALSE);

#ifdef _DEBUG
	size_t releasedCount = 0;
#endif //_DEBUG

	while(index--)
	{
		#ifdef _DEBUG
		if (0 == itemList[index]->Release())
			releasedCount++;
		#else
		itemList[index]->Release();
		#endif // _DEBUG
	}

#ifdef _DEBUG
	if (totalCount != releasedCount)
	{
		aTRACE_LINE("Document: not all items released");
		DebugBreak();
	}
#endif //_DEBUG

	DeleteCriticalSection(&lockItems); 
	DeleteCriticalSection(&lockScheduler); 
	lfh_compact();
}


ULONG Document::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG Document::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

BOOL Document::IsClosing()
{
	return (closing || 0 == ref);
}
void Document::Close()
{
	SignalAsyncAbort();
	closing = TRUE;
	Release();
}

LPCTSTR Document::GenerateUniqueName(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pszPrefix)
{
	LPTSTR p;
	WCHAR szPrefix[64];

	if (NULL == pszPrefix)
	{		
		if (NULL != WASABI_API_LNGSTRINGW_BUF(IDS_DOCUMENTNAME_PREFIX, szPrefix, ARRAYSIZE(szPrefix)) &&
			L'\0' != szPrefix[0])
		{
			pszPrefix = szPrefix;
		}
		else
		{
			pszPrefix = DOCUMENTNAME_DEFAULTPREFIX;
		}
	}

    lastDocumentIndex++;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, pszPrefix, &p, &cchBufferMax, 0);
	if (SUCCEEDED(hr)) 
		StringCchPrintf(p, cchBufferMax, TEXT("%d"), lastDocumentIndex);

	if (FAILED(hr))
	{
		pszBuffer = TEXT('\0');
		return NULL;
	}
	return pszBuffer;
}

HRESULT Document::Create(LPCTSTR pszDocName, Document **ppDocument)
{
	if (NULL == ppDocument)
		return E_INVALIDARG;

	TCHAR szDocName[128];
	if (NULL == pszDocName) 
		pszDocName = GenerateUniqueName(szDocName, ARRAYSIZE(szDocName), NULL);

	if (NULL == pszDocName)
		return E_POINTER;
	
	*ppDocument = new Document(pszDocName);
	if (NULL == *ppDocument)
		return E_OUTOFMEMORY;
	
	return S_OK;
}

void Document::ReadPlaylistName(IFileInfo *pfi)
{
	IFileMeta *pMeta;
	if (SUCCEEDED(pfi->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{
		METAKEY key = METAKEY_TRACKTITLE;
		IFileMetaReader *pReader;
		if (SUCCEEDED(pMeta->GetReader(&key, 1, METAREADMODE_NORMAL, &pReader)))
		{
			pReader->Read();
			pReader->Release();
		}
		METAVALUE value;
		value.type = METATYPE_WSTR;
		if (SUCCEEDED(pMeta->QueryValue(METAKEY_TRACKTITLE, &value)))
		{
			SetTitle(value.pwVal);
			ReleaseMetaValue(&value);
		}
		pMeta->Release();
	}
}

static void FlushPlaylistsChanges(LPCTSTR pszPath)
{
	if (NULL == WASABI_API_SYSCB) return;
	api_playlists *playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
	if (NULL == playlistsApi) return;

	playlistsApi->Lock();
	size_t count = playlistsApi->GetCount();
	while (count--)
	{
		LPCWSTR fileName = playlistsApi->GetFilename(count);
		if (NULL != fileName || L'\0' != fileName)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, fileName, -1, pszPath, -1))
			{					
				WASABI_API_SYSCB->syscb_issueCallback(api_playlists::SYSCALLBACK, 
						api_playlists::PLAYLIST_FLUSH_REQUEST, count, GetPluginUID());
				break;
			}
		}
	}
    playlistsApi->Unlock();
	ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);
}

HRESULT Document::OpenFile(LPCTSTR pszPath, Document **ppDocument)
{
	if (NULL == ppDocument)
		return E_INVALIDARG;

	WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
	if (!GetFileAttributesEx(pszPath, GetFileExInfoStandard, &fileAttributes))
	{
		DWORD error = GetLastError();
		return E_FAIL;
	}

	if (FILE_ATTRIBUTE_DIRECTORY & fileAttributes.dwFileAttributes)
		return E_UNEXPECTED;
	
	// flush api_playlist
	FlushPlaylistsChanges(pszPath);

	HRESULT hr = E_FAIL;

	Document *pDoc = new Document(NULL);
	if (NULL == pDoc || FAILED(pDoc->SetPath(pszPath)))
	{
		if (NULL != pDoc) delete(pDoc);
		return E_OUTOFMEMORY;
	}

	hr = pDoc->Reload();

	if (SUCCEEDED(hr))
		*ppDocument = pDoc;
	else 
		delete(pDoc);

	
	return hr;
}

HRESULT Document::GetFilterList(LPTSTR pszFilter, size_t cchFilterMax, BOOL bSaveDialog, DWORD *pIndex)
{
	return (FALSE == bSaveDialog) ? 
			GetPlaylistOpenFilters(pszFilter, cchFilterMax, pIndex) : 
			GetPlaylistSaveFilters(pszFilter, cchFilterMax, pIndex);
}

static TCHAR PathValidateChar(TCHAR cVal, BOOL bAllowBackSlash)
{
	switch(cVal)
	{
		case TEXT('\\'):	if (!bAllowBackSlash) return TEXT('-');
							break;
		case TEXT('/'):		if (!bAllowBackSlash) return TEXT('-'); 
							return TEXT('\\');
		case TEXT(':'):		return TEXT('-'); 
		case TEXT('*'):		return TEXT('_'); 
		case TEXT('?'):		return TEXT('_'); 
		case TEXT('\"'):	return TEXT('\'');
		case TEXT('<'):		return TEXT('('); 
		case TEXT('>'):		return TEXT(')'); 
		case TEXT('|'):		return TEXT('_'); 
	}
	return cVal;
}

HRESULT Document::FileNameFromTitle(LPTSTR pszBuffer, INT cchBufferMax, LPCTSTR pszTitle)
{
	INT cchLen = 0;
	if (NULL != pszTitle && TEXT('\0') != *pszTitle)
	{
		while (TEXT(' ') == *pszTitle) pszTitle++;
		cchLen = lstrlen(pszTitle);
		while (cchLen > 0 && TEXT(' ') == pszTitle[cchLen - 1]) cchLen--;
	}
	
	if (cchLen < 1)
	{
		LPCTSTR p = GenerateUniqueName(pszBuffer, cchBufferMax, NULL);
		return (NULL != p) ? S_OK : E_OUTOFMEMORY;
	}
	
	if (cchLen > (MAX_PATH - 32))
		cchLen = MAX_PATH - 32;

	if (cchBufferMax < cchLen)
		return E_OUTOFMEMORY;

	for (INT i = 0; i < cchLen; i++)
		pszBuffer[i] = PathValidateChar(pszTitle[i], FALSE);
	
	pszBuffer[cchLen] = 0;
	return S_OK;
}

HRESULT Document::Reload()
{
	IFileInfo *pfi;
	DWORD type;

	HRESULT hr = E_FAIL;

	RemoveAll();

	if (NULL != PLUGIN_REGTYPES && SUCCEEDED(PLUGIN_REGTYPES->CreateItem(pszPath, NULL, &pfi)) && 
		SUCCEEDED(pfi->GetType(&type)) && IItemType::itemTypePlaylistFile == type)
	{
		ReadPlaylistName(pfi);
		pfi->GetModifiedTime(&fileModified);
		IFileEnumerator *pe;
		if (SUCCEEDED(pfi->EnumItems(&pe)))
		{
			ReloadFilter *filter = ReloadFilter::CreateInstance();
			hr = InsertItems(0, pe, filter, FALSE);
			if (NULL != filter)
				filter->Release();
			pe->Release();
		}
	}	
	if (NULL != pfi)
		pfi->Release();
	
	SetModified(FALSE);

	return hr;
}
HRESULT Document::Save(BOOL registerPL)
{
	DocumentSaver *saver = new DocumentSaver(pszPath, pszTitle, itemList.begin(), itemList.size(), registerPL);
	if (NULL == saver)
		return E_OUTOFMEMORY;
	
	HRESULT hr = saver->Save();
	
	if (SUCCEEDED(hr))
	{
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(pszPath, &fd);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			CopyMemory(&fileModified, &fd.ftLastWriteTime, sizeof(FILETIME));
			FindClose(hFind);
		}
		SetModified(FALSE);
	}

	delete(saver);
	return hr;
}


HRESULT Document::GetPath(LPTSTR pszBuffer, INT cchBufferMax)
{	
	return StringCchCopyEx(pszBuffer, cchBufferMax, pszPath, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT Document::SetPath(LPCTSTR pszBuffer)
{
	HRESULT hr;
	hr = ValidateName(pszBuffer);
    if (FAILED(hr))
		return hr;

	if (NULL != pszPath)
		lfh_free(pszPath);
	
	SetModified(TRUE);

	INT cbLen = (lstrlen(pszBuffer) + 1) * sizeof(TCHAR);
    pszPath = (LPTSTR)lfh_malloc(cbLen);
	if(NULL == pszPath)
		return E_OUTOFMEMORY;
	CopyMemory(pszPath, pszBuffer, cbLen);
	
	return S_OK;
}

BOOL Document::GetModified(void)
{
	return bModified;
}
void Document::SetModified(BOOL bMarkModified)
{
	bModified = bMarkModified;
}

HRESULT Document::GetTitle(LPTSTR pszBuffer, INT cchBufferMax)
{	
	return StringCchCopyEx(pszBuffer, cchBufferMax, pszTitle, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT Document::SetTitle(LPCTSTR pszBuffer)
{
	HRESULT hr;

	if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, pszBuffer, -1, pszTitle, -1))
		return S_OK;

	hr = ValidateName(pszBuffer);
    if (FAILED(hr))
		return hr;

	if (NULL != pszTitle)
		lfh_free(pszTitle);
	
	SetModified(TRUE);

	INT cbLen = (lstrlen(pszBuffer) + 1) * sizeof(TCHAR);
    pszTitle = (LPTSTR)lfh_malloc(cbLen);
	if(NULL == pszTitle)
		return E_OUTOFMEMORY;
	CopyMemory(pszTitle, pszBuffer, cbLen);

	Notify(EventTitleChanged, 0);

	return S_OK;
}

HRESULT Document::FlushTitle(void)
{	
	api_playlists *playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
	if (NULL == playlistsApi) return E_NOINTERFACE;

	playlistsApi->Lock();
	size_t count = playlistsApi->GetCount();
	while (count--)
	{
		LPCWSTR fileName = playlistsApi->GetFilename(count);
		if (NULL != fileName || L'\0' != fileName)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, fileName, -1, pszPath, -1))
			{
				playlistsApi->RenamePlaylist(count, pszTitle);
				break;
			}
		}
	}
    playlistsApi->Unlock();
	ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);
	return S_OK;
}

HRESULT Document::GetFileModified(BOOL *pModified)
{
	if (NULL == pModified)
		return E_INVALIDARG;
	
	*pModified = FALSE;
	if (NULL == pszPath || TEXT('\0') == pszPath)
		return E_POINTER;

	WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
	if (!GetFileAttributesEx(pszPath, GetFileExInfoStandard, &fileAttributes))
		return E_FAIL;
	
	if (0 != CompareFileTime(&fileAttributes.ftLastWriteTime, &fileModified))
		*pModified = TRUE;
	
	return S_OK;
}
HRESULT Document::ValidateName(LPCTSTR pszBuffer)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	while (TEXT('\0') != *pszBuffer && TEXT(' ') == *pszBuffer) pszBuffer++;
	if (TEXT('\0') == *pszBuffer)
		return E_INVALIDARG;

	return S_OK;
}

static int __fastcall ItemList_CompareProc(const void *elem1, const void *elem2, const void *context)
{
	INT result = ((ORDERPARAM*)context)->primary(elem1, elem2);
	if (0 == result && NULL != ((ORDERPARAM*)context)->secondary)
		result = ((ORDERPARAM*)context)->secondary(elem1, elem2);
	return result;
}

HRESULT Document::Order(CSORTPROC primaryCompare, CSORTPROC secondaryCompare)
{
	size_t first, last;
	first = 0;
	last = 0;
		
	EnterCriticalSection(&lockItems);
	
	if (itemList.size() > 1)
	{		
		last = itemList.size() - 1;
		ORDERPARAM param;
		param.primary = primaryCompare;
		param.secondary = secondaryCompare;

		nu::qsort(itemList.begin() + first, (last - first) + 1, 
					sizeof(IFileInfo*), 	&param, ItemList_CompareProc);
	}
	LeaveCriticalSection(&lockItems);

	ITEMRANGE range;
	if (first < last)
	{		
		range.first = first;
		range.last = last;
		Notify(EventRangeReordered, (LONG_PTR)&range);
		SetModified(TRUE);
	}
	return S_OK;
}
HRESULT Document::ReadAndOrder(CSORTPROC primaryCompare, CSORTPROC secondaryCompare, const METAKEY *pMetaKey, INT metaKeyCount)
{
	DWORD threadId;

	ORDERTHREADPARAM *param = (ORDERTHREADPARAM*)malloc(sizeof(ORDERTHREADPARAM)); // thread proc will free it
	ZeroMemory(param, sizeof(ORDERTHREADPARAM));
	param->document = this;
	param->primaryCompare = primaryCompare;
	param->secondaryCompare = secondaryCompare;

	if (metaKeyCount > 0)
	{
		param->metaKeys = (METAKEY*)malloc(sizeof(METAKEY) * metaKeyCount);
		CopyMemory(param->metaKeys, pMetaKey, sizeof(METAKEY) * metaKeyCount);
		param->keysCount = metaKeyCount;
	}
	
	param->document->AddRef();	
	
	HANDLE hThread = CreateThread(NULL, 0, Document::OrderThreadProc, param, 0, &threadId);
	if (NULL == hThread)
	{
		if (NULL != param->metaKeys)
			free(param->metaKeys);
		free(param);
		return E_UNEXPECTED;
	}
	
	CloseHandle(hThread);
	return S_OK;
}

DWORD CALLBACK Document::OrderThreadProc(void *param)
{
	ORDERTHREADPARAM *orderParam = (ORDERTHREADPARAM*)param;
	if (NULL == orderParam || NULL == orderParam->document)
		return -1;
	
	HRESULT hr = S_OK;

	size_t count = orderParam->document->GetItemCount();
	orderParam->document->AsyncOpStart(AsyncOrder, count, TRUE);

	BOOL cancelMode = 0;

	if (NULL != orderParam->metaKeys && 
		orderParam->keysCount > 0)
	{
		IFileInfo *pItem;
		IFileMeta *pMeta;
		IFileMetaReader *pReader;
		ITEMREAD readData;
		
		size_t index = 0;
		
		while(NULL != (pItem = orderParam->document->GetItemSafe(index)))
		{
			if (orderParam->document->IsClosing() ||
				0 == orderParam->document->AsyncOpStep(1))
			{
				cancelMode = (orderParam->document->IsClosing()) ? 2 : 1;
				pItem->Release();
				break;
			}

			hr = pItem->QueryInterface(IID_IFileMeta, (void**)&pMeta);

			if (SUCCEEDED(hr))
			{				
				hr = pMeta->GetReader(orderParam->metaKeys, orderParam->keysCount, METAREADMODE_NORMAL, &pReader);
				if (SUCCEEDED(hr))
				{
					readData.result = S_OK;
					readData.index = index;
					readData.pItem = pItem;
					orderParam->document->Notify(EventItemReadScheduled, (LONG_PTR)&readData);

					pReader->SetCookie((DWORD)index);
					orderParam->document->ExecuteRead(FALSE, &pReader, 1);
				}
				pMeta->Release();
			}

			pItem->Release();
			index++;
		}
	}
		
	if (2 != cancelMode)
	{
		orderParam->document->Order(orderParam->primaryCompare, orderParam->secondaryCompare);
		hr = S_OK;
	}
	else 
	{
		hr = E_ABORT;
	}

	orderParam->document->AsyncOpFinish(hr);

	orderParam->document->Release();

	if (NULL != orderParam->metaKeys)
		free(orderParam->metaKeys);
		
	free(orderParam);

	return 0;
}

static void AddItemMetrics(IFileInfo *pItem, Document::METRICS *pMetrics)
{
	HRESULT hr;
	
	IFileMeta *pMeta;
	ULONGLONG itemSize;
	ULONG_PTR itemLength;

	if (SUCCEEDED(pItem->GetSize(&itemSize)))
		pMetrics->size += itemSize;
		
	if (SUCCEEDED(pItem->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{				
		METAVALUE metaValue;
		metaValue.type = METATYPE_INT32;
		hr = pMeta->QueryValueHere(METAKEY_TRACKLENGTH, &metaValue, &itemLength, sizeof(itemLength));
			
		if (FAILED(hr))
		{
			itemLength = 0;
			if (E_METAKEY_UNKNOWN == hr && S_OK == pMeta->CanRead(METAKEY_TRACKLENGTH))
			{
				pMetrics->unknownData++;
				pItem->SetExtraInfo(LENGTH_UNKNOWN);
			}
			else
				pItem->SetExtraInfo(LENGTH_NOTSUPPORTED);
		}
		else
		{
			if (itemLength < 0) itemLength = 0;
			if (SUCCEEDED(pItem->SetExtraInfo(itemLength)))
				pMetrics->length += itemLength;
		}

		pMeta->Release();
	}
	else
	{
		pItem->SetExtraInfo(LENGTH_NOTSUPPORTED);
	}
}

static void RemoveItemMetrics(IFileInfo *pItem, Document::METRICS *pMetrics)
{
	// update metrics
	ULONGLONG itemSize;
	ULONG_PTR itemLength;
		
	if (SUCCEEDED(pItem->GetSize(&itemSize)))
		pMetrics->size -= itemSize;
			
	if (SUCCEEDED(pItem->GetExtraInfo(&itemLength)))
	{
		switch(itemLength)
		{
			case LENGTH_UNKNOWN: pMetrics->unknownData--; break;
			case LENGTH_NOTSUPPORTED:break;
			default: pMetrics->length -= itemLength; break;
		}
		pItem->SetExtraInfo(LENGTH_UNKNOWN);
	}

	
		
	
}


static void EnumerateItems(IFileEnumerator *pEnumerator, Document::ENUMINFO *info)
{		
	IFileInfo *pfi;
	HRESULT hr;
	for(;;)
	{
		hr = pEnumerator->Next(1, &pfi, NULL);
		if (S_OK != hr)
		{
			if (E_FILEENUM_CREATEINFO_FAILED == hr)
				continue;
			break;
		}
		
		if (info->document->IsClosing())
		{
			pfi->Release();
			return;
		}

		BYTE rule = info->filter->GetRule(pfi);
		switch(rule)
		{
			case FilterPolicy::entryRuleEnumerate:
				{
					IFileEnumerator *pe2;
					hr = pfi->EnumItems(&pe2);
					if (SUCCEEDED(hr))
					{			
						if (info->antiLoop->Check(pfi))
							EnumerateItems(pe2, info);
						pe2->Release();
					}
				}
				break;
			case FilterPolicy::entryRuleAdd:
				{
					size_t sz = info->list->size();
					if (sz == (info->list->capacity() -1))
					{
						info->list->reserve(sz * 2);
					}
					info->list->push_back(pfi);
					pfi->AddRef();
				}
				break;
		}
		
		pfi->Release();

		if (0 == info->document->AsyncOpStep(1))
			return;
	}
}


HRESULT Document::InsertItemsReal(size_t insertBefore, IFileInfo **ppItems, size_t itemCount, BOOL markModified)
{
	if (0 == itemCount) 
		return S_OK;
	
	// update metrics

	for(size_t i = 0; i < itemCount; i++)
	{
		AddItemMetrics(ppItems[i], &metrics);
	}

	EnterCriticalSection(&lockItems);
	
	size_t before = itemList.size();
	itemList.insertBefore(insertBefore, ppItems, itemCount);
		
	if (markModified)
		SetModified(TRUE);

	Notify(EventCountChanged, 0);

	if (before > 0 && insertBefore < before)
	{
		ITEMSHIFT itemShift;
		itemShift.first = insertBefore;
		itemShift.last = before - 1;
		itemShift.delta = (INT)((itemList.size() - before));
		Notify(EventItemShifted, (LONG_PTR)&itemShift);
	}
	
	LeaveCriticalSection(&lockItems);
	return S_OK;
}


DWORD CALLBACK Document::InsertEnumThreadProc(void *param)
{
	INSERTENUMPARAM *insertParam = (INSERTENUMPARAM*)param;
	if (NULL == insertParam || NULL == insertParam->document)
		return -1;
	
	insertParam->document->AsyncOpStart(AsyncInsert, ((size_t)-1), TRUE);

	ITEMLIST tempList;
	tempList.reserve(4096);
	AntiLoop antiloop;
	
	ENUMINFO enumInfo;
	enumInfo.list = &tempList;
	enumInfo.antiLoop = &antiloop;
	enumInfo.document = insertParam->document;
	enumInfo.filter = insertParam->filter;
	
	EnumerateItems(insertParam->enumerator, &enumInfo);

	HRESULT hr;
	if (!insertParam->document->IsClosing())
	{
		hr = insertParam->document->InsertItemsReal(insertParam->insertBefore, tempList.begin(), 
													tempList.size(), insertParam->markModified);
	}
	else
		hr = E_ABORT;

	insertParam->document->AsyncOpFinish(hr);

	insertParam->document->Release();
	insertParam->enumerator->Release();
	insertParam->filter->Release();
	
	free(insertParam);

	return 0;
}

HRESULT Document::InsertItems(size_t insertBefore, IFileEnumerator *pEnumerator, IDocumentFilter *pFilter, BOOL bMarkModified)
{
	DWORD threadId;

	if (NULL == pEnumerator)
		return S_OK;

	INSERTENUMPARAM *param = (INSERTENUMPARAM*)malloc(sizeof(INSERTENUMPARAM)); // thread proc will free it
	ZeroMemory(param, sizeof(INSERTENUMPARAM));
	param->document = this;
	param->enumerator = pEnumerator;
	param->insertBefore = insertBefore;
	param->markModified = bMarkModified;
	param->filter = pFilter;

	param->document->AddRef();	
	param->enumerator->AddRef();
	param->filter->AddRef();
	
	HANDLE hThread = CreateThread(NULL, 0, Document::InsertEnumThreadProc, param, 0, &threadId);
	if (NULL == hThread)
	{
		Document::InsertEnumThreadProc(param);
	}
	else
		CloseHandle(hThread);
	
	return S_OK;
}


HRESULT Document::RemoveItems(size_t *indices, size_t count)
{
	if (NULL == indices || 0 == count)
		return E_INVALIDARG;

	BOOL modified = FALSE;
	size_t itemIndex, first, last;

	EnterCriticalSection(&lockItems);

	first = ((size_t)-1);
	last = 0;

	ITEMRANGE itemRange;
	for (size_t i = count; i-- > 0;)
	{        
		itemIndex = indices[i];

		RemoveItemMetrics(itemList[itemIndex], &metrics);
		itemList[itemIndex]->Release();

		if (itemIndex == (first - 1))
			 first--;
		 else
		 {
			if (first <= last)
			{
				modified = TRUE;
				itemList.eraseRange(first, last);
				itemRange.first = first;
				itemRange.last = last;
				Notify(EventRangeRemoved, (LONG_PTR)&itemRange);
			}

			last = itemIndex;
			first = last;
		 }
	}

	if (first <= last)
	{
		modified = TRUE;
		itemList.eraseRange(first, last);
		itemRange.first = first;
		itemRange.last = last;
		Notify(EventRangeRemoved, (LONG_PTR)&itemRange);
	}

	LeaveCriticalSection(&lockItems);

	if (modified)
	{
		SetModified(TRUE);
		Notify(EventCountChanged, 0);
	}

	return S_OK;	
}

HRESULT Document::RemoveRange(size_t first, size_t last)
{
	if (0 == itemList.size())
		return S_OK;

	if (last >= itemList.size())
		last = itemList.size() - 1;
		
	if (first > last) 
		return S_OK;
	
	EnterCriticalSection(&lockItems);

	for(size_t i = first; i <= last; i++)
	{
		RemoveItemMetrics(itemList[i], &metrics);
		itemList[i]->Release();
	}

	itemList.eraseRange(first, last);
	LeaveCriticalSection(&lockItems);

	SetModified(TRUE);

	ITEMRANGE itemRange;
	itemRange.first = first;
	itemRange.last = last;
	Notify(EventRangeRemoved, (LONG_PTR)&itemRange);
	Notify(EventCountChanged, 0);

	return S_OK;
}

HRESULT Document::RemoveAll()
{
	size_t index = itemList.size();
	if (index > 0)
	{
		EnterCriticalSection(&lockItems);
		while(index--)
			itemList[index]->Release();
		itemList.clear();

		LeaveCriticalSection(&lockItems);
		ZeroMemory(&metrics, sizeof(METRICS));
		SetModified(TRUE);
		Notify(EventCountChanged, 0);
	}
	return S_OK;
}

IFileInfo *Document::GetItemDirect(size_t index)
{
	return itemList[index];
}

IFileInfo *Document::GetItemSafe(size_t index)
{
	IFileInfo *pItem = NULL;
	
	EnterCriticalSection(&lockItems);
	
	if (index < itemList.size())
	{
		pItem = itemList[index];
		if (NULL != pItem)
			pItem->AddRef();
	}
	
	LeaveCriticalSection(&lockItems);

	return pItem;
}

HRESULT Document::Reverse(size_t first, size_t last)
{
	
	HRESULT hr = E_INVALIDARG; 
	EnterCriticalSection(&lockItems);
	
	if (itemList.size() > 0)
	{
		if(last >= itemList.size())
			last = itemList.size() - 1;
		
		if (first == last)
			hr = S_OK;
		else if (first < last)
		{
			IFileInfo *t;
			IFileInfo **l, **r;
			l = (itemList.begin() + first);
			r = (itemList.begin() + last);
			for (; l < r; l++, r--)
			{
				t = *l; 
				*l = *r; 
				*r = t;
			}
			hr = S_OK;
		}
	}
	LeaveCriticalSection(&lockItems);
	
	if (SUCCEEDED(hr) && first < last)
	{
		ITEMRANGE range;
		range.first  = first;
		range.last = last;
		Notify(EventRangeReversed, (LONG_PTR)&range);
		SetModified(TRUE);
	}
	return S_OK;
}

size_t Document::GetItemCount(void)
{
	return itemList.size();
}


size_t Document::ItemToIndex(IFileInfo *pItem, size_t firstIndex, size_t lastIndex)
{	
	if (0 == itemList.size())
		return ((size_t)-1);

	if (lastIndex > itemList.size())
		lastIndex = itemList.size() - 1;

	size_t index;
	EnterCriticalSection(&lockItems);
	for (index = firstIndex; index <= lastIndex; index++)
	{
		if (itemList[index] == pItem)
			break;
	}
	LeaveCriticalSection(&lockItems);

	return (index > lastIndex) ? -1 : index;
		
}

static int __cdecl CompareSizeT(const void *elem1, const void *elem2)
{
	return (int)(*((size_t*)elem1) - *((size_t*)elem2));
}


HRESULT Document::ShiftItems(size_t *indices, size_t count, INT moveTo)
{
	if (moveTo < 0)
		moveTo = 0;

	if (moveTo > (INT)itemList.size())
		moveTo = (INT)itemList.size();

	qsort(indices, count, sizeof(size_t), CompareSizeT);

	INT delta = delta = moveTo - (INT)indices[0];
	if (delta > 0) 
	{
		for (size_t i = 0; i < count && delta > 0; i++)
		if ((INT)indices[i] < moveTo) delta--;
		else break;
	}
			
	void *buffer = NULL;
	size_t alloc = 0;
	size_t l, i, moveLen, cbLen;
		
	EnterCriticalSection(&lockItems);
	if (delta < 0)
	{
		for (i = 0; i < count;)
		{						
			for ( l = i + 1; l < count && ((indices[l] - indices[l-1]) == 1); l++);
			if (l == i) continue;
			moveLen = l - i;

			if (alloc < moveLen)
			{
				free(buffer);
				alloc = ((moveLen*2/512) + 1) * 512;
				buffer = malloc(sizeof(IFileInfo*)*alloc);
			}
			
			IFileInfo **ppFileInfo = itemList.begin();
			cbLen = sizeof(IFileInfo*)*moveLen;
			CopyMemory(buffer, ppFileInfo + indices[i], cbLen);
			BYTE *top = (BYTE*)(ppFileInfo + (indices[i] + delta));
			MoveMemory(top + cbLen, top, sizeof(IFileInfo*) * abs(delta));
			CopyMemory(top, buffer, cbLen);
			ITEMSHIFT itemShift;
			itemShift.first = indices[i];
			itemShift.last = itemShift.first + moveLen - 1;
			itemShift.delta = delta;
			Notify(EventItemShifted, (LONG_PTR)&itemShift);
			//aTRACE_FMT("moving(%d, %d) on %d lines\n" , indices[i] + 1, (l - i), delta);
			i = l;
		}
	}
	else if (delta > 0)
	{	
		INT realDelta;
		size_t maxMove = (itemList.size() > 0) ? (itemList.size() - 1) : 0;

		for (l = count; l > 0;)
		{						
			for (i = l - 1;;i--)
			{
				if (i != (l - 1) && ((indices[i + 1] - indices[i]) != 1))
				{
					i++;
					break;
				}
				if (i == 0)
					break;
				
			}
			if (l == i) continue;

			moveLen = l - i;
			if (alloc < moveLen)
			{
				free(buffer);
				alloc = ((moveLen*2/512) + 1) * 512;
				buffer = malloc(sizeof(IFileInfo*)*alloc);
			}
			
			IFileInfo **ppFileInfo = itemList.begin();
			cbLen = sizeof(IFileInfo*)*moveLen;
			CopyMemory(buffer, ppFileInfo + indices[i], cbLen);
			BYTE *top = (BYTE*)(ppFileInfo + indices[i]);

			if (0 != maxMove)
			{
				realDelta = (INT)(((delta + indices[i] + (moveLen - 1)) > maxMove) ? 
							(maxMove - (indices[i] + (moveLen - 1))) : delta);
				if (realDelta  > 0)
				{
					MoveMemory(top, top + cbLen, sizeof(IFileInfo*) * abs(realDelta));
					CopyMemory(top + sizeof(IFileInfo*) * abs(realDelta), buffer, cbLen);
					ITEMSHIFT itemShift;
					itemShift.first = indices[i];
					itemShift.last = itemShift.first + moveLen - 1;
					itemShift.delta = realDelta;
					Notify(EventItemShifted, (LONG_PTR)&itemShift);
				
					//aTRACE_FMT("moving(%d, %d) on %d lines\n" , indices[i] + 1, (l - i), realDelta);
				}
				maxMove--;
			}
			l = i;
		}
	}

	LeaveCriticalSection(&lockItems);

	if (NULL != buffer) free(buffer);
	SetModified(TRUE);
	return S_OK;
}

void Document::RegisterCallback(DOCPROC callback, ULONG_PTR user)
{
	if (NULL == callback)
		return;

	size_t index = subscription.size();
	while (index--)
	{
		if (subscription[index].callback == callback &&
			subscription[index].user == user)
		return;
	}
	SUBSCRIBER s;
	s.callback = callback;
	s.user = user;
	subscription.push_back(s);
}	

void Document::UnregisterCallback(DOCPROC callback, ULONG_PTR user)
{
	size_t index = subscription.size();
	while (index--)
	{
		if (subscription[index].callback == callback &&
			subscription[index].user == user)
		{
			subscription.eraseAt(index);
			return;
		}
	}
}

void Document::Notify(UINT nCode, LONG_PTR param)
{
	if (IsClosing())
		return;

	size_t index = subscription.size();
	while (index--)
	{
		subscription[index].callback(this, nCode, param, subscription[index].user);
	}
}


HRESULT Document::EnqueueItems(size_t *indices, size_t count, DWORD enqueueFlags, size_t *enqueuedCount)
{
	LPCTSTR pszPath;
	TCHAR szTitle[2048];
	METAVALUE mVal;
	IFileMeta *pMeta;
	enqueueFileWithMetaStructW efs;
	size_t itemListCount = itemList.size();
	size_t enqueued = 0;
	size_t index;

	if (0 != (EF_ENQUEUEALL & enqueueFlags))
		count = itemListCount;
	else 
	{
		if (NULL == indices)
		{
			if (NULL != enqueuedCount) *enqueuedCount = 0;
			return E_INVALIDARG;
		}
	}
		
	for (size_t i = 0; i < count; i++)
	{		
		if (0 == (EF_ENQUEUEALL & enqueueFlags))
		{
			index = indices[i];
			if (index >= itemListCount)
				continue;
		}
		else
			index = i;

		if (0 == (EF_ENQUEUEALL & enqueueFlags) && 
			S_OK != itemList[index]->CanPlay())
			continue;

		if (SUCCEEDED(itemList[index]->GetPath(&pszPath)))
		{					
			efs.filename = pszPath;
			efs.length	= -1;
			efs.title	= NULL;
			if (SUCCEEDED(itemList[index]->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
			{					
				mVal.type = METATYPE_WSTR;
				if(SUCCEEDED(pMeta->QueryValueHere(METAKEY_FORMATTEDTITLE, &mVal, szTitle, ARRAYSIZE(szTitle))))
					efs.title = mVal.pwVal;

				mVal.type = METATYPE_INT32;
				if(SUCCEEDED(pMeta->QueryValue(METAKEY_TRACKLENGTH, &mVal)))
					efs.length = mVal.iVal;	
				
				pMeta->Release();
			}

			if (SENDWAIPC(plugin.hwndParent, IPC_ENQUEUEFILEW, (WPARAM)&efs))
				enqueued++;
		}
	}

	if (NULL != enqueuedCount)
		*enqueuedCount = enqueued;

	return S_OK;
}
HRESULT Document::PlayItems(size_t *indices, size_t count, DWORD enqueueFlags, size_t *enqueuedCount, size_t startItem)
{
	SENDWAIPC(plugin.hwndParent, IPC_DELETE, 0);
	
	size_t processed = 0;
	HRESULT hr = EnqueueItems(indices, count, enqueueFlags, &processed);
	
	if (SUCCEEDED(hr) && processed > 0)
	{
		if (startItem > processed)
			startItem = 0;
		SENDWAIPC(plugin.hwndParent, IPC_SETPLAYLISTPOS, startItem);
		SENDWAIPC(plugin.hwndParent, IPC_STARTPLAY, 0);
	}

	if (NULL != enqueuedCount)
		*enqueuedCount = processed;

	return hr;
}

void Document::ExecuteRead(BOOL useScheduler, IFileMetaReader **readers, INT readersCount)
{

	if (useScheduler)
	{
		EnterCriticalSection(&lockScheduler);
		StartScheduler();
	}

	if (FALSE == useScheduler ||
		NULL == scheduler || 
		FAILED(scheduler->ScheduleArrayRead(readers, readersCount, FALSE)))
	{		
		Document::ITEMREAD readData;
		DWORD cookie;

		for(int i = 0; i < readersCount; i++)
		{			
			readData.result = readers[i]->Read();

			if (SUCCEEDED(readers[i]->GetCookie(&cookie)))
				readData.index = (size_t)cookie;
			else
				readData.index = -1;

			if (FAILED(readers[i]->QueryInterface(IID_IFileInfo, (void**)&readData.pItem)))
				readData.pItem = NULL;
			ItemReadCompleted(&readData);
			if (NULL != readData.pItem)
				readData.pItem->Release();
		}
	}

	if (useScheduler)
		LeaveCriticalSection(&lockScheduler);

	for(int i = 0; i < readersCount; i++) 
		readers[i]->Release();
	
}

INT Document::ReadItems(size_t from, size_t to, METAKEY *metaKeys, INT metaKeysCount, BOOL readNow)
{
	if (0 == itemList.size())
		return 0;
	
	if (to >= itemList.size())
		to = itemList.size() - 1;
	
	if (from > to)
		return 0;

	if (AsyncInvalid != asyncOperation.nCode)
		return 0;
	
	IFileMeta *pMeta;
	IFileMetaReader *szReaders[256];  // readers chunk
	INT prepCount = 0;
	HRESULT hr;
	ITEMREAD readData;
	readData.result = S_OK;
	do
	{	
		readData.index = from;
		readData.pItem = itemList[from];
		Notify(EventItemReadScheduled, (LONG_PTR)&readData);
		

		hr = itemList[from]->QueryInterface(IID_IFileMeta, (void**)&pMeta);
		if (SUCCEEDED(hr))
		{				
			hr = pMeta->GetReader(metaKeys, metaKeysCount, METAREADMODE_NORMAL, &szReaders[prepCount]);
			if (SUCCEEDED(hr))
			{
				szReaders[prepCount]->SetCookie((DWORD)from);
				prepCount++;
				
				if (ARRAYSIZE(szReaders) == prepCount)
				{
					ExecuteRead(!readNow, szReaders, prepCount);
					prepCount = 0;
				}
			}
			pMeta->Release();
		}
		else
		{
			hr = E_NOTIMPL;
		}
		
		if (FAILED(hr))
		{
			readData.result = hr;
			ItemReadCompleted(&readData);
		}

	}while (from++ != to);

	if (prepCount > 0)
		ExecuteRead(!readNow, szReaders, prepCount);

	return prepCount;
}

void Document::SetQueueLimit(size_t newLimit)
{
	EnterCriticalSection(&lockScheduler);
	
	schedulerLimit = newLimit;
	if (NULL != scheduler)
	{
		scheduler->SetQueueLimit(schedulerLimit);
	}
	
	LeaveCriticalSection(&lockScheduler);
}

size_t Document::GetQueueSize()
{
	size_t size = 0;
	EnterCriticalSection(&lockScheduler);
	if (NULL != scheduler)
		size = scheduler->GetQueueSize();
	LeaveCriticalSection(&lockScheduler);
	return size;
}

void Document::ItemReadCompleted(Document::ITEMREAD *readData)
{	
	EnterCriticalSection(&lockItems);
	
	if (((readData->index >= itemList.size()) && NULL != readData->pItem) ||
		(readData->index < itemList.size() && readData->pItem != itemList[readData->index]))
	{			
		readData->index = ItemToIndex(readData->pItem, 0, itemList.size());
	}
	
	if (-1 != readData->index)
	{
		readData->newLength = 0;
		readData->oldLength = 0;
		readData->newUnknown = FALSE;
		readData->oldUnknown = FALSE;

		if (NULL == readData->pItem)
		{
			readData->pItem = itemList[readData->index];
		}

		if (SUCCEEDED(readData->result) && NULL != readData->pItem) // update Metrics
		{
			ULONG_PTR cached;
			HRESULT hr = readData->pItem->GetExtraInfo(&cached);
			if (SUCCEEDED(hr))
			{
				IFileMeta *pMeta;
				if (SUCCEEDED(readData->pItem->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
				{
					INT length;
					METAVALUE metaValue;
					metaValue.type = METATYPE_INT32;
					hr = pMeta->QueryValueHere(METAKEY_TRACKLENGTH, &metaValue, &length, sizeof(length));
					pMeta->Release();

					if (SUCCEEDED(hr))
					{						
						if (length < 0) length = 0;
						if (length != cached)
						{
							readData->oldLength = 0;
							readData->newLength = 0;
							readData->oldUnknown = FALSE;
							readData->newUnknown = FALSE;

							if (LENGTH_UNKNOWN == cached)
							{
								metrics.unknownData--;
								readData->oldUnknown = TRUE;
							}
							else if (LENGTH_NOTSUPPORTED != cached)
							{
								metrics.length -= cached;
								readData->oldLength = (INT)cached;
							}
							
							if(SUCCEEDED(readData->pItem->SetExtraInfo(length)))
							{
								metrics.length += length;
								readData->newLength = length;
							}
						}
						else
						{
							if (LENGTH_UNKNOWN == cached)
							{
								readData->oldUnknown = TRUE;
								readData->newUnknown = TRUE;
							}
							else if (LENGTH_NOTSUPPORTED != cached)
							{
								readData->oldLength = (INT)cached;
								readData->newLength = (INT)cached;
							}
						}
					}
					else
					{
						if (LENGTH_UNKNOWN != cached && LENGTH_NOTSUPPORTED != cached)
						{
							metrics.length -= cached;
							readData->oldLength = (INT)cached;
						}
						else
							readData->oldLength = 0;

						if (LENGTH_UNKNOWN == cached && E_METAKEY_UNKNOWN != hr)
						{
							metrics.unknownData--;
							readData->oldUnknown = TRUE;
							readData->newUnknown = FALSE;

						}

						if (LENGTH_UNKNOWN != cached && E_METAKEY_UNKNOWN == hr)
						{
							metrics.unknownData++;
							readData->oldUnknown = FALSE;
							readData->newUnknown = TRUE;
						}
						
						readData->newLength = 0;
						readData->pItem->SetExtraInfo((E_METAKEY_UNKNOWN == hr) ? LENGTH_UNKNOWN : LENGTH_NOTSUPPORTED);
					}
					
				}
			}
			else
			{
				readData->newLength = 0;
				readData->oldLength = 0;
				readData->newUnknown = FALSE;
				readData->oldUnknown = FALSE;
			}
		}
		
	}

	LeaveCriticalSection(&lockItems);
	Notify(EventItemReadCompleted, (LONG_PTR)readData);

	
}

BOOL Document::GetMetrics(size_t first, size_t last, Document::METRICS *pMetrics)
{
	if (NULL == pMetrics)
		return FALSE;

	if (0 == ((FlagMetricSize | FlagMetricLength) & pMetrics->flags))
		return FALSE;

	if (first > last)
		first = last;

	if ((((size_t)-1) == last || last >= itemList.size()) && 
		(last == first || 0 == first))
	{
		if (FlagMetricSize & pMetrics->flags)
			pMetrics->size = metrics.size;
		
		if (FlagMetricLength & pMetrics->flags)
		{
			pMetrics->length = metrics.length;
			pMetrics->unknownData = metrics.unknownData;
		}
		return TRUE;
	}

	if (FlagMetricSize & pMetrics->flags)
		pMetrics->size = 0;
		
	if (FlagMetricLength & pMetrics->flags)
	{
		pMetrics->length = 0;
		pMetrics->unknownData = 0;
	}
	
	if (0 == itemList.size())
		return TRUE;

	if (last >= itemList.size())
		last = itemList.size();

	ULONGLONG itemSize;
	ULONG_PTR itemExtra;
	for (;first <= last; first++)
	{
		if (0 != (FlagMetricSize & pMetrics->flags) &&
			SUCCEEDED(itemList[first]->GetSize(&itemSize)))
			pMetrics->size += itemSize;
	
		if (0 != (FlagMetricLength & pMetrics->flags) &&
			SUCCEEDED(itemList[first]->GetExtraInfo(&itemExtra)))
		{
			switch(itemExtra)
			{
				case LENGTH_UNKNOWN:
					pMetrics->unknownData++;
					break;
				case LENGTH_NOTSUPPORTED:
					break;
				default:
					pMetrics->length += itemExtra;
					break;
			}
		}
	}

	return TRUE;
}

BOOL Document::StartScheduler()
{
	EnterCriticalSection(&lockScheduler);
	
	schedulerIdle = FALSE;
	
	if (NULL == scheduler)
	{
		scheduler = new FileMetaScheduler();
		if (NULL != scheduler)
			scheduler->SetQueueLimit(schedulerLimit);
	}

	if (NULL == scheduleListener && NULL != scheduler)
		scheduleListener = new DocumentSchedulerListener(this, scheduler);

	LeaveCriticalSection(&lockScheduler);

	return (NULL != scheduler && NULL != scheduleListener);
}

BOOL Document::CloseScheduler(BOOL onlyIdle)
{	
	EnterCriticalSection(&lockScheduler);
	schedulerIdle = FALSE;

	if (onlyIdle)
	{
		if ((NULL != scheduler && !scheduler->IsIdle()) ||
			(NULL != scheduleListener && !scheduleListener->IsIdle()))
		{
			LeaveCriticalSection(&lockScheduler);
			return FALSE;
		}
	}

	if (NULL != scheduleListener)
		delete(scheduleListener);

	if (NULL != scheduler)
		delete(scheduler);

	scheduler = NULL;
	scheduleListener = NULL;
	schedulerIdle = FALSE;

	LeaveCriticalSection(&lockScheduler);

	return TRUE;
}

void Document::SetSchedulerIdle()
{
	if (schedulerIdle)
		return;

	EnterCriticalSection(&lockScheduler);
		
	schedulerIdle = ((NULL == scheduler || scheduler->IsIdle()) &&
					 (NULL == scheduleListener || scheduleListener->IsIdle()));
	
	if (schedulerIdle)
	{
		HANDLE winampThread = (NULL != WASABI_API_APP) ? 
				WASABI_API_APP->main_getMainThreadHandle() : NULL;

		if (NULL != winampThread)
		{
			AddRef();		
			if (!QueueUserAPC(StopSchedulerAPC, winampThread, (DWORD_PTR)this))
				Release();
		}
	}

	LeaveCriticalSection(&lockScheduler);
}

void Document::StopSchedulerAPC(DWORD_PTR param)
{
	Document *pDoc = (Document*)param;
	if (NULL == pDoc)
		return;
	if (pDoc->CloseScheduler(TRUE))
		pDoc->schedulerIdle = FALSE;
	pDoc->Release();
}

BOOL Document::QueryAsyncOpInfo(ASYNCOPERATION *pOperation)
{
	if (AsyncInvalid == asyncOperation.nCode)
		return FALSE;

	if (NULL != pOperation)
	{
		CopyMemory(pOperation, &asyncOperation, sizeof(ASYNCOPERATION));
	}

	return TRUE;
}

void Document::SignalAsyncAbort()
{
	if (AsyncInvalid != asyncOperation.nCode)
	{
        signalAbortAsync = TRUE;
		asyncOperation.bCancelable = FALSE;
	}
}

void Document::AsyncOpStart(INT operationCode, size_t total, BOOL bCancelable)
{
	signalAbortAsync = FALSE;
	ZeroMemory(&asyncOperation, sizeof(ASYNCOPERATION));
	
	asyncOperation.nCode =operationCode;
	asyncOperation.bCancelable = bCancelable;
	asyncOperation.total = total;
	asyncOperation.result = E_PENDING;
	
	Notify(Document::EventAsyncStarted, (LONG_PTR)&asyncOperation);

}

void Document::AsyncOpFinish(HRESULT result)
{
	signalAbortAsync = FALSE;
	if (AsyncInvalid != asyncOperation.nCode)
	{
		ASYNCOPERATION completed;
		CopyMemory(&completed, &asyncOperation, sizeof(ASYNCOPERATION));
		completed.result = result;
		ZeroMemory(&asyncOperation, sizeof(ASYNCOPERATION));

		Notify(Document::EventAsyncFinished, (LONG_PTR)&completed);
	}
	else
	{
		ZeroMemory(&asyncOperation, sizeof(ASYNCOPERATION));
	}
}

BOOL Document::AsyncOpStep(LONG step)
{
	if (AsyncInvalid == asyncOperation.nCode)
		return FALSE;
	
	asyncOperation.processed += step;
	Notify(Document::EventAsyncStep, (LONG_PTR)&asyncOperation);
	return !signalAbortAsync;
}

BOOL Document::InvalidateItem(IFileInfo *pItem)
{
	if (NULL == pItem)
		return FALSE;

	size_t index = ItemToIndex(pItem, 0, itemList.size());
	
	if (-1 != index)
		RemoveItemMetrics(pItem, &metrics);
		
	pItem->ResetCache();
	
	IFileMeta *pMeta;
	if (SUCCEEDED(pItem->QueryInterface(IID_IFileMeta, (void**)&pMeta)))
	{
		pMeta->Clear();
		pMeta->Release();
	}
	
	if (-1 != index)
	{
		AddItemMetrics(pItem, &metrics);
		Notify(EventItemInvalidated, index);
	}
	return TRUE;
}
BYTE Document::GetFilterRule(IFileInfo *pItem)
{
	if (NULL == pItem)
		return FilterPolicy::entryRuleError;

	return FilterPolicy::entryRuleIgnore;
}
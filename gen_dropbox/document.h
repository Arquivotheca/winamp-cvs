#ifndef NULLOSFT_DROPBOX_DOCUMENT_HEADER
#define NULLOSFT_DROPBOX_DOCUMENT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/PtrList.h"
#include "../nu/Vector.h"

interface IFileInfo;
interface IFileEnumerator;
interface IFileMetaReader;

class AntiLoop;
class FileMetaScheduler;
class Document;
interface IDocumentFilter;


typedef WORD METAKEY;	


typedef void (CALLBACK *DOCPROC)(Document* /*pDocument*/, UINT /*uCode*/, LONG_PTR /*param*/, UINT_PTR /*user*/);



class Document
{
public:

	typedef enum
	{
		EventTitleChanged		= 0,
		EventCheckFileModified	= 1,
		EventCountChanged		= 2,
		EventItemReadScheduled	= 3,
		EventItemReadCompleted	= 4,
		EventItemShifted			= 5,
		EventReadQueueEmpty		= 6, // 
		EventAsyncStarted		= 7,
		EventAsyncFinished		= 8,
		EventAsyncStep			= 9,
		EventItemInvalidated		= 10, // param - item index
		EventRangeReversed		= 11, // param - ITEMRANGE
		EventRangeReordered		= 12, // param - ITEMRANGE
		EventRangeRemoved		= 13, // param - ITEMRANGE
	} DOCUMENTNOTIFICATION;

	typedef enum
	{
		EF_ENQUEUEALL = 0x0001,
		EF_FORCEUNPLAYABLE = 0x0002,
	} ENQUEUEFLAGS;
	
	typedef struct
	{
		IFileInfo	*pItem;
		size_t		index;
		HRESULT		result;		// if E_ABORT - read was not executed
		INT			newLength;
		INT			oldLength;
		BOOL		newUnknown;
		BOOL		oldUnknown;
	} ITEMREAD;

	typedef struct
	{
		size_t	first;
		size_t	last;
		int		delta;
	} ITEMSHIFT;

	typedef struct
	{
		size_t	first;
		size_t	last;
	} ITEMRANGE;

	typedef enum
	{
		FlagMetricLength = 0x0001,
		FlagMetricSize = 0x0002,
	} METRICFLAGS;

	typedef struct __METRICS
	{
		size_t		cbSize; // sizeof(METRICS)
		UINT		flags;
		ULONGLONG	size;
		ULONGLONG	length;
		size_t		unknownData;  // number of items with unknown length 
	} METRICS;

	typedef struct
	{
		INT		nCode;
		BOOL	bCancelable;
		size_t	total;
		size_t	processed;
		HRESULT	result;
	} ASYNCOPERATION;

	typedef enum
	{
		AsyncInvalid = 0,
		AsyncInsert,
		AsyncOrder,
	} ASYNCCODE;

	typedef int (__cdecl *CSORTPROC)(const void *, const void *);

protected:
	typedef struct __SUBSCRIBER
	{
		DOCPROC		callback;
		UINT_PTR	user;
	} SUBSCRIBER;

	
	typedef nu::PtrList<IFileInfo> ITEMLIST;
	typedef Vector<SUBSCRIBER> SUBSCRIPTION;

	typedef struct ___ENUMINFO
	{
		ITEMLIST		*list;
		AntiLoop	*antiLoop;
		Document		*document;
		IDocumentFilter *filter;
	} ENUMINFO;

protected:
	Document(LPCTSTR pszDocName);

protected:
	virtual ~Document();

public:
	static LPCTSTR GenerateUniqueName(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pszPrefix);
	static HRESULT Create(LPCTSTR pszDocName, Document **ppDocument); // if NULL == pszDocName then Document::GenerateUniqueName will be used with "Playlist" prefix
	static HRESULT OpenFile(LPCTSTR pszPath, Document **ppDocument);
	static HRESULT GetFilterList(LPTSTR pszFilter, size_t cchFilterMax, BOOL bSaveDialog, DWORD *pIndex);
	static HRESULT FileNameFromTitle(LPTSTR pszBuffer, INT cchBufferMax, LPCTSTR pszTitle);
	
public:
	ULONG AddRef();
	ULONG Release();

	void Close();

	HRESULT Save(BOOL registerPL);
	HRESULT GetTitle(LPTSTR pszBuffer, INT cchBufferMax);
	HRESULT SetTitle(LPCTSTR pszBuffer);
	HRESULT ValidateName(LPCTSTR pszBuffer);
	HRESULT GetPath(LPTSTR pszBuffer, INT cchBufferMax);
	HRESULT SetPath(LPCTSTR pszBuffer);
	HRESULT FlushTitle(void);
	HRESULT GetFileModified(BOOL *pModified);

	HRESULT Reload();


	HRESULT Order(CSORTPROC primaryCompare, CSORTPROC secondaryCompare);
	HRESULT ReadAndOrder(CSORTPROC primaryCompare, CSORTPROC secondaryCompare, const METAKEY *pMetaKey, INT metaKeyCount);
	HRESULT Reverse(size_t first, size_t last);

	HRESULT InsertItems(size_t insertBefore, IFileEnumerator *pEnumerator, IDocumentFilter *pFilter, BOOL bMarkModified);
	
	HRESULT RemoveAll();
	HRESULT RemoveRange(size_t first, size_t last);
	HRESULT RemoveItems(size_t *indices, size_t count);

	HRESULT ShiftItems(size_t *indices, size_t count, INT moveTo);
	IFileInfo *GetItemDirect(size_t index);
	IFileInfo *GetItemSafe(size_t index); // checks bounds and increases ref count
	size_t GetItemCount(void);
    
	size_t ItemToIndex(IFileInfo *pItem, size_t firstIndex, size_t lastIndex);
		
	HRESULT EnqueueItems(size_t *indices, size_t count, DWORD enqueueFlags, size_t *enqueuedCount);
	HRESULT PlayItems(size_t *indices, size_t count, DWORD enqueueFlags, size_t *enqueuedCount, size_t startItem);

	BOOL GetModified(void);
	void SetModified(BOOL bMarkModified);

	void RegisterCallback(DOCPROC callback, ULONG_PTR user);
	void UnregisterCallback(DOCPROC callback, ULONG_PTR user);
	
	INT ReadItems(size_t from, size_t to, METAKEY *metaKeys, INT metaKeysCount, BOOL readNow);
	void SetQueueLimit(size_t newLimit);
	size_t GetQueueSize();

	BOOL IsClosing();

	BOOL GetMetrics(size_t first, size_t last, Document::METRICS *pMetrics);
	ULONGLONG GetTotalSize();
	ULONGLONG GetTotalLength();

	BOOL QueryAsyncOpInfo(ASYNCOPERATION *pOperation); // pOperation can be NULL to query if any operation is active
	void SignalAsyncAbort();

	BOOL InvalidateItem(IFileInfo *pItem);

	BYTE GetFilterRule(IFileInfo *pItem);
	
	
protected:
	void ReadPlaylistName(IFileInfo *pfi);
	void Notify(UINT nCode, LONG_PTR param);
	void ItemReadCompleted(Document::ITEMREAD *readData);

private:

	HRESULT InsertItemsReal(size_t insertBefore, IFileInfo **ppItems, size_t itemCount, BOOL markModified);
	void ExecuteRead(BOOL useScheduler, IFileMetaReader **readers, INT readersCount);
	
	void InitScheduler();
	BOOL StartScheduler();
	BOOL CloseScheduler(BOOL onlyIdle);
	void SetSchedulerIdle();
	static void CALLBACK StopSchedulerAPC(DWORD_PTR param);

	void AsyncOpStart(INT operationCode, size_t total, BOOL bCancelable); // set total = ((size_t)-1) if unknown
	void AsyncOpFinish(HRESULT hr);
	BOOL AsyncOpStep(LONG step); // return 0 to abort async operation


	static DWORD CALLBACK InsertEnumThreadProc(void *param);
	static DWORD CALLBACK OrderThreadProc(void *param);


	friend class DocumentCallback;
	friend class DocumentSchedulerListener;
	friend static void EnumerateItems(IFileEnumerator *pEnumerator, Document::ENUMINFO *info);

protected:
	ULONG ref;
	LPTSTR pszTitle;
    LPTSTR pszPath;
	ITEMLIST itemList;
	SUBSCRIPTION subscription;
	BOOL bModified;
	FILETIME	 fileModified;
	BOOL	closing;

	DocumentCallback *sysCallback;

	size_t	schedulerLimit;
	BOOL	schedulerIdle;
	CRITICAL_SECTION lockScheduler;
	FileMetaScheduler *scheduler;
	DocumentSchedulerListener *scheduleListener;

	CRITICAL_SECTION lockItems;

	METRICS metrics;
	BOOL signalAbortAsync;
	ASYNCOPERATION asyncOperation;
};


#endif // NULLOSFT_DROPBOX_DOCUMENT_HEADER
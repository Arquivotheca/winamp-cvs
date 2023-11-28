#include "DownloadManager.h"
#include "api.h"
#include "../nu/threadname.h"
#include <api/service/waservicefactory.h>
#include <string.h>
#include "../nu/AutoChar.h"
#include "../nu/threadpool/timerhandle.h"
#include <strsafe.h>

static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};

#define DOWNLOAD_TIMEOUT_MS 60000 // 60 second timeout
#define DOWNLOAD_SLEEP_MS 50
#define DOWNLOAD_BUFFER_SIZE 131072
// gives a maximum download rate of 2.5 mb/sec per file

DownloadManager::DownloadManager()
{
	download_thread = NULL;
	killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&downloadsCS);
}

void DownloadManager::Kill()
{
	SetEvent(killswitch);
	if (download_thread)
	{
		WaitForSingleObject(download_thread, 3000);
		CloseHandle(download_thread);
	}

	DeleteCriticalSection(&downloadsCS);
	CloseHandle(killswitch);
}

int DownloadManager::DownloadTickThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id)
{
	DownloadManager *dlmgr = (DownloadManager *)user_data;
	if (dlmgr->DownloadThreadTick())
	{
		TimerHandle t(handle);
		t.Wait(DOWNLOAD_SLEEP_MS);
		return 0;
	}
	else
	{
		WASABI_API_THREADPOOL->RemoveHandle(0, handle);
		SetEvent(dlmgr->download_thread);
		return 0;
	}
	return 0;
}

bool DownloadManager::InitDownloadThread()
{
	if (download_thread == NULL)
	{
		download_thread = CreateEvent(0, FALSE, FALSE, 0);
		TimerHandle t;
		WASABI_API_THREADPOOL->AddHandle(0, t, DownloadTickThreadPoolFunc, this, 0, api_threadpool::FLAG_LONG_EXECUTION);
		t.Wait(DOWNLOAD_SLEEP_MS);
	}

	return (download_thread != NULL);
}

int DownloadManager::Tick(DownloadData *thisDownload, void *buffer, int bufferSize)
{
	int state = thisDownload->http->run();
	if (state == HTTPRECEIVER_RUN_ERROR)
		return api_downloadManager::TICK_FAILURE;

	int downloaded = thisDownload->http->get_bytes(buffer, bufferSize);
	if (downloaded)
	{
		switch(thisDownload->flags & DOWNLOADEX_MASK_DOWNLOADMETHOD)
		{
		case api_downloadManager::DOWNLOADEX_BUFFER:
			{
				thisDownload->buffer.reserve(thisDownload->http->content_length());
				thisDownload->buffer.add(buffer, downloaded);
			}
			break;
		case api_downloadManager::DOWNLOADEX_TEMPFILE:
			{
				DWORD written;
				WriteFile(thisDownload->hFile, buffer, downloaded, &written, NULL);
				if (written != downloaded)
					return api_downloadManager::TICK_WRITE_ERROR;
			}
			break;
		case api_downloadManager::DOWNLOADEX_CALLBACK:
			{
				if (thisDownload->flags & api_downloadManager::DOWNLOADEX_UI)
				{
					for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
					{
						(*itr)->OnData(thisDownload, buffer, downloaded);
					}
				}

				if (thisDownload->callback)
					thisDownload->callback->OnData(thisDownload, buffer, downloaded);
				
			}
		}
		thisDownload->lastDownloadTick = GetTickCount();
		thisDownload->bytesDownloaded+=downloaded;

		return api_downloadManager::TICK_SUCCESS;
	}
	else // nothing in the buffer
	{
		if (state == HTTPRECEIVER_RUN_CONNECTION_CLOSED) // see if the connection is closed
		{
			return api_downloadManager::TICK_FINISHED; // yay we're done
		}
		if (GetTickCount() - thisDownload->lastDownloadTick > DOWNLOAD_TIMEOUT_MS) // check for timeout
			return api_downloadManager::TICK_TIMEOUT;

		switch(thisDownload->http->get_status())
		{
		case HTTPRECEIVER_STATUS_CONNECTING:
			if (thisDownload->last_status != HTTPRECEIVER_STATUS_CONNECTING)
			{
				thisDownload->last_status=HTTPRECEIVER_STATUS_CONNECTING;
				return api_downloadManager::TICK_CONNECTING;
			}
			else
			{
				return api_downloadManager::TICK_NODATA;
			}
		case HTTPRECEIVER_STATUS_READING_HEADERS:
			if (thisDownload->last_status != HTTPRECEIVER_STATUS_READING_HEADERS)
			{
				thisDownload->last_status=HTTPRECEIVER_STATUS_READING_HEADERS;
				return api_downloadManager::TICK_CONNECTED;
			}
			else
			{
				return api_downloadManager::TICK_NODATA;
			}
		}
		if (!thisDownload->replyCode)
			thisDownload->replyCode = thisDownload->http->getreplycode();

		switch (thisDownload->replyCode)
		{
		case 0:
		case 100:
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
			return api_downloadManager::TICK_NODATA;
		default:
			return api_downloadManager::TICK_CANT_CONNECT;
		}
	}
}

static void SetUserAgent(api_httpreceiver *http)
{
	char agent[256];
	StringCchPrintfA(agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString());
	http->addheader(agent);
}

DownloadToken DownloadManager::Download(const char *url, ifc_downloadManagerCallback *callback)
{
	return DownloadEx(url, callback, api_downloadManager::DOWNLOADEX_TEMPFILE);
}

DownloadToken DownloadManager::DownloadEx(const char *url, ifc_downloadManagerCallback *callback, int flags)
{
	if (InitDownloadThread())
	{
		api_httpreceiver *http = 0;
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
		if (sf) http = (api_httpreceiver *)sf->getInterface();

		if (http)
		{
			DownloadData *downloadData = new DownloadData(http, url, flags, callback);
			int use_proxy = 1;
			bool proxy80 = AGAVE_API_CONFIG->GetBool(internetConfigGroupGUID, L"proxy80", false);
			if (proxy80 && strstr(url, ":") && (!strstr(url, ":80/") && strstr(url, ":80") != (url + strlen(url) - 3)))
				use_proxy = 0;

			const wchar_t *proxy = use_proxy?AGAVE_API_CONFIG->GetString(internetConfigGroupGUID, L"proxy", 0):0;

			//		http->AllowCompression();
			http->open(API_DNS_AUTODNS, DOWNLOAD_BUFFER_SIZE, (proxy && proxy[0]) ? (const char *)AutoChar(proxy) : NULL);
			/*if (startPosition > 0)
			{
			char temp[128];
			StringCchPrintfA(temp, 128, "Range: bytes=%d-", startPosition);
			http->addheader(temp);
			}*/
			SetUserAgent(http);
			if (callback)
				callback->OnInit(downloadData);
			if (downloadData->flags & api_downloadManager::DOWNLOADEX_UI)
			{
				for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
				{
					(*itr)->OnInit(downloadData);
				}
			}
			//only call http->connect when it is not pending download request
			if ( !(flags & DOWNLOADEX_PENDING) ) 
				http->connect(url);
			//http->run(); // let's get this party started
			EnterCriticalSection(&downloadsCS);
			downloads.push_back(downloadData);
			LeaveCriticalSection(&downloadsCS);
			return downloadData;
		}
	}
	return 0;
}

void DownloadManager::FinishDownload(DownloadData *data, int code)
{
	if (NULL == data)
		return;

	ifc_downloadManagerCallback *callback;

	EnterCriticalSection(&downloadsCS);
	data->Close(&callback);
	LeaveCriticalSection(&downloadsCS);

	if (NULL != callback)
	{
		if (TICK_FINISHED == code)
		{
			callback->OnFinish(data);
			if (data->flags & api_downloadManager::DOWNLOADEX_UI)
			{
				for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
				{
					(*itr)->OnFinish(data);
				}
			}
		}
		else
		{
			callback->OnError(data, code);
			if (data->flags & api_downloadManager::DOWNLOADEX_UI)
			{
				for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
				{
					(*itr)->OnError(data, code);
				}
			}
		}

		callback->Release();
	}

	EnterCriticalSection(&downloadsCS);
	downloads.erase(data);
	LeaveCriticalSection(&downloadsCS);
	data->Release();
}

bool DownloadManager::DownloadThreadTick()
{
	size_t i=0;
	char downloadBuffer[DOWNLOAD_BUFFER_SIZE];

	while(WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
	{
		EnterCriticalSection(&downloadsCS);
		if (downloads.empty())
		{
			// TODO: might be nice to dynamically increase the sleep time if this happens
			// (maybe to INFINITE and have Download() wake us?)
			LeaveCriticalSection(&downloadsCS);
			return true;
		}
		if (i >= downloads.size())
		{
			LeaveCriticalSection(&downloadsCS);
			return true;
		}

		DownloadData *thisDownload = downloads[i];
		if (thisDownload->pending)
		{
			LeaveCriticalSection(&downloadsCS);
		}
		else
		{
			thisDownload->Retain();
			LeaveCriticalSection(&downloadsCS);

			INT tick = Tick(thisDownload, downloadBuffer, DOWNLOAD_BUFFER_SIZE);
			switch (tick)
			{
			case TICK_NODATA:
				// do nothing
				break;

			case TICK_CONNECTING:
				break;

			case TICK_CONNECTED:
				if (thisDownload->callback)
					thisDownload->callback->OnConnect(thisDownload);
				if (thisDownload->flags & api_downloadManager::DOWNLOADEX_UI)
				{
					for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
					{
						(*itr)->OnConnect(thisDownload);
					}
				}
				break;

			case TICK_SUCCESS:
				if (thisDownload->callback)
					thisDownload->callback->OnTick(thisDownload);
				if (thisDownload->flags & api_downloadManager::DOWNLOADEX_UI)
				{
					for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
					{
						(*itr)->OnTick(thisDownload);
					}
				}

				// TODO: send out update callback
				break;

			case TICK_FINISHED:
			case TICK_FAILURE:
			case TICK_TIMEOUT:
			case TICK_CANT_CONNECT:
			case TICK_WRITE_ERROR:
				FinishDownload(thisDownload, tick);
				break;
			}
			thisDownload->Release();
		}
		i++;
	}
	return false; // we only get here when killswitch is set
}

DownloadData::DownloadData(api_httpreceiver *_http, const char *url, int _flags, ifc_downloadManagerCallback *_callback)
{
	flags = _flags;
	http = _http;
	callback = _callback;
	if (callback)
		callback->AddRef();

	hFile = INVALID_HANDLE_VALUE;
	filepath[0]=0;
		
	int download_method = (api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & flags);
	switch(download_method)
	{
		case api_downloadManager::DOWNLOADEX_TEMPFILE:
			{
				wchar_t temppath[MAX_PATH-14]; // MAX_PATH-14 'cause MSDN said so
				GetTempPathW(MAX_PATH-14, temppath);
				GetTempFileNameW(temppath, L"wdl", 0, filepath);
				hFile = CreateFileW(filepath, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
			}
			break;
		case api_downloadManager::DOWNLOADEX_CALLBACK:
			if (callback)
				callback->GetLocation(filepath, MAX_PATH);
			break;
	}

	strcpy_s(this->url, 1024, url);
	source[0]=0;
	title[0]=0;
	if (flags & api_downloadManager::DOWNLOADEX_CALLBACK)
	{
		if (callback)
		{
			callback->GetSource(source, 1024);
			callback->GetTitle(title, 1024);
		}
	}
	
	connectionStart=lastDownloadTick=GetTickCount();
	replyCode=0;
	bytesDownloaded=0;
	refCount=1;
	last_status=HTTPRECEIVER_STATUS_ERROR;
	pending=(flags & api_downloadManager::DOWNLOADEX_PENDING)>0;
}

void DownloadData::Retain()
{
	InterlockedIncrement(&refCount);
}

void DownloadData::Release()
{
	if (InterlockedDecrement(&refCount) == 0)
		delete this;
}

void DownloadData::Close(ifc_downloadManagerCallback **callbackCopy)
{
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	hFile = INVALID_HANDLE_VALUE;

	if (NULL != callbackCopy)
	{
		*callbackCopy = callback;
		if (NULL != callback) callback->AddRef();
	}

	if (NULL != callback)
	{
		callback->Release();
		callback = NULL;
	}

	// don't want to close http here, because someone might still want to get the headers out of it
}

DownloadData::~DownloadData()
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(httpreceiverGUID);
	if (sf) sf->releaseInterface(http);
	http=0;

	int download_method = (api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & flags);
	if (download_method == api_downloadManager::DOWNLOADEX_TEMPFILE && filepath[0])
	{
		DeleteFileW(filepath);
	}
	if (callback)
		callback->Release();
	callback=0;
}

void DownloadManager::ResumePendingDownload(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data->pending)
	{
		data->pending=false;
		data->connectionStart=data->lastDownloadTick=GetTickCount();
		data->http->connect(data->url);
	}
}

void DownloadManager::CancelDownload(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	EnterCriticalSection(&downloadsCS);
	if (downloads.contains(data))
	{
		ifc_downloadManagerCallback *callback;
		data->Close(&callback);
		downloads.erase(data);
		LeaveCriticalSection(&downloadsCS);

		if (callback)
		{
			callback->OnCancel(token);
			if (data->flags & api_downloadManager::DOWNLOADEX_UI)
			{
				for (StatusList::iterator itr=status_callbacks.begin();itr!=status_callbacks.end();itr++)
				{
					(*itr)->OnCancel(token);
				}
			}
			callback->Release();
		}
		data->Release();
	}
	else
		LeaveCriticalSection(&downloadsCS);	
}

void DownloadManager::RetainDownload(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data)
		data->Retain();
}

void DownloadManager::ReleaseDownload(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data)
		data->Release();
}

void DownloadManager::RegisterStatusCallback(ifc_downloadManagerCallback *callback)
{
	EnterCriticalSection(&downloadsCS);
	status_callbacks.push_back(callback);
	LeaveCriticalSection(&downloadsCS);	
}

void DownloadManager::UnregisterStatusCallback(ifc_downloadManagerCallback *callback)
{
	EnterCriticalSection(&downloadsCS);
	status_callbacks.erase(callback);
	LeaveCriticalSection(&downloadsCS);	
}

const wchar_t *DownloadManager::GetSource(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data)
		return data->source;
	else
		return 0;
}

const wchar_t *DownloadManager::GetTitle(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data)
		return data->title;
	else
		return 0;
}

bool DownloadManager::IsPending(DownloadToken token)
{
	DownloadData *data = (DownloadData *)token;
	if (data)
		return data->pending;
	else
		return 0;
}

#define CBCLASS DownloadManager
START_DISPATCH;
CB(API_DOWNLOADMANAGER_DOWNLOAD, Download)
CB(API_DOWNLOADMANAGER_DOWNLOADEX, DownloadEx)
CB(API_DOWNLOADMANAGER_GETRECEIVER, GetReceiver)
CB(API_DOWNLOADMANAGER_GETLOCATION, GetLocation)
CB(API_DOWNLOADMANAGER_GETBYTESDOWNLOADED, GetBytesDownloaded);
CB(API_DOWNLOADMANAGER_GETBUFFER, GetBuffer);
VCB(API_DOWNLOADMANAGER_RESUMEPENDINGDOWNLOAD, ResumePendingDownload);
VCB(API_DOWNLOADMANAGER_CANCELDOWNLOAD, CancelDownload);
VCB(API_DOWNLOADMANAGER_RETAINDOWNLOAD, RetainDownload);
VCB(API_DOWNLOADMANAGER_RELEASEDOWNLOAD, ReleaseDownload);
VCB(API_DOWNLOADMANAGER_REGISTERSTATUSCALLBACK, RegisterStatusCallback);
VCB(API_DOWNLOADMANAGER_UNREGISTERSTATUSCALLBACK, UnregisterStatusCallback);
CB(API_DOWNLOADMANAGER_GETSOURCE, GetSource);
CB(API_DOWNLOADMANAGER_GETTITLE, GetTitle);
CB(API_DOWNLOADMANAGER_ISPENDING, IsPending);
END_DISPATCH;
#undef CBCLASS

Buffer_GrowBuf::Buffer_GrowBuf() 
{ 
	m_alloc=m_used=0;
	m_s=NULL; 
}

Buffer_GrowBuf::~Buffer_GrowBuf() 
{
	if (m_s) free(m_s);
	m_s=0; 
}

void Buffer_GrowBuf::reserve(size_t len)
{
	if (len > m_alloc)
	{
		void *ne;
		m_alloc = len;
		ne = realloc(m_s, m_alloc);
		if (!ne)
		{
			ne=malloc(m_alloc);
			memcpy(ne,m_s,m_used);
			free(m_s);
		}
		m_s=ne;
	}
}

size_t Buffer_GrowBuf::add(void *data, size_t len) 
{ 
	if (len<=0) return 0;
	resize(m_used+len); 
	memcpy((char*)get()+m_used-len,data,len);
	return m_used-len;
}

void Buffer_GrowBuf::set(void *data, size_t len)
{
	resize(len);
	memcpy((char*)get(),data,len);
}

void Buffer_GrowBuf::resize(size_t newlen)
{
	m_used=newlen;
	if (newlen > m_alloc)
	{
		void *ne;
		m_alloc = newlen*2;
		if (m_alloc < 1024) m_alloc =1024;
		ne = realloc(m_s, m_alloc);
		if (!ne)
		{
			ne=malloc(m_alloc);
			if (!ne) *((char*)ne)=NULL;
			memcpy(ne,m_s,m_used);
			free(m_s);
		}
		m_s=ne;
	}
}

size_t Buffer_GrowBuf::getlen() 
{ 
	return m_used;
}

void *Buffer_GrowBuf::get() 
{
	return m_s; 
}
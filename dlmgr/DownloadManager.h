#pragma once
#include "../nu/ptrlist.h"
#include "../jnetlib/api_httpget.h"
#include <windows.h>
#include <bfc/dispatch.h>
#include "api_downloadManager.h"

class Buffer_GrowBuf
{
  public:
    Buffer_GrowBuf();
    ~Buffer_GrowBuf();

		void reserve(size_t len);

    size_t add(void *data, size_t len);

    void set(void *data, size_t len);

    void resize(size_t newlen);
    size_t getlen();
    void *get();

private:
    void *m_s;
    size_t m_alloc;
    size_t m_used;
};

struct DownloadData
{
	DownloadData(api_httpreceiver *http, const char *url, int flags, ifc_downloadManagerCallback *callback);
	~DownloadData();
	void Retain();
	void Release();
	void Close(ifc_downloadManagerCallback **callbackCopy);
	api_httpreceiver *http;
	wchar_t filepath[MAX_PATH]; // where file is downloaded to. probably a temp path
	char url[1024];
	wchar_t source[1024];
	wchar_t title[1024];
	HANDLE hFile; // handle to the open file where data is written to
	DWORD lastDownloadTick; // last time we got data out of the connection. used for timeout
	DWORD connectionStart; // used for when the connection starts, to help us calculate a k/s download rate
	int replyCode; // HTTP 200, 404, etc.
	uint64_t bytesDownloaded;
	int last_status; // from http->get_status()
	bool pending;

	ifc_downloadManagerCallback *callback;
	LONG refCount;
	int flags;
	Buffer_GrowBuf buffer;
};

/* method ideas

Download(url, owner_token, callback, user_data)
Lock()  - call before enumerating and doing anything
Unlock()
CancelDownload()
Enum(owner_token)

*/

class DownloadManager : public api_downloadManager
{
public:
	static const char *getServiceName()	{	return "Download Manager"; }
	static GUID getServiceGuid() { return DownloadManagerGUID; }
public:
	DownloadManager();
	void Kill();
	DownloadToken Download(const char *url, ifc_downloadManagerCallback *callback);
	DownloadToken DownloadEx(const char *url, ifc_downloadManagerCallback *callback, int flags);
	void ResumePendingDownload(DownloadToken token);
	void CancelDownload(DownloadToken token);
	void RetainDownload(DownloadToken token);
	void ReleaseDownload(DownloadToken token);

	void RegisterStatusCallback(ifc_downloadManagerCallback *callback);
	void UnregisterStatusCallback(ifc_downloadManagerCallback *callback);
	const wchar_t *GetSource(DownloadToken token);
	const wchar_t *GetTitle(DownloadToken token);
	bool IsPending(DownloadToken token);

	/*
		only call these during a callback!
	*/
	api_httpreceiver *GetReceiver(DownloadToken token)
	{
		if (token)
			return ((DownloadData *)token)->http;
		else
			return 0;
	}

	const wchar_t *GetLocation(DownloadToken token)
	{
		if (token)
			return ((DownloadData *)token)->filepath;
		else
			return 0;
	}

		int GetBuffer(DownloadToken token, void **buffer, size_t *bufferlen)
	{
		if (token)
		{
			*buffer = ((DownloadData *)token)->buffer.get();
			*bufferlen = ((DownloadData *)token)->buffer.getlen();
			return 0;
		}
		else
			return 1;
	}

	uint64_t GetBytesDownloaded(DownloadToken token)
	{
		if (token)
			return ((DownloadData *)token)->bytesDownloaded;
		else
			return 0;
	}

private:
	CRITICAL_SECTION downloadsCS;
	bool DownloadThreadTick();
	static int DownloadTickThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id);
	bool InitDownloadThread();
	void FinishDownload(DownloadData *data, int code);
int Tick(DownloadData *thisDownload, void *buffer, int bufferSize);	
	HANDLE download_thread;
	HANDLE killswitch;
	nu::PtrList<DownloadData> downloads;
	typedef nu::PtrList<ifc_downloadManagerCallback> StatusList;
	StatusList status_callbacks;
	
protected:
	RECVS_DISPATCH;
};

#include "Main.h"
#include "Downloaded.h"
#include "BackgroundDownloader.h"

#include "Feeds.h"
#include "DownloadStatus.h"
#include "DownloadsDialog.h"
#include "api.h"
#include <api/service/waServiceFactory.h>
#include "../jnetlib/api_httpget.h"
using namespace Nullsoft::Utility;

#define SIMULTANEOUS_DOWNLOADS 2
Vector<DownloadToken> downloadsQueue;
LockGuard downloadsQueueLock;

class DownloaderCallback : public ifc_downloadManagerCallback
{
public:
	DownloaderCallback(const wchar_t *url, const wchar_t *destination_filepath, const wchar_t *channel, const wchar_t *item, __time64_t publishDate)
	{
		this->hFile = INVALID_HANDLE_VALUE;
		this->url = _wcsdup(url);
		this->destination_filepath = _wcsdup(destination_filepath);
		this->channel = _wcsdup(channel);
		this->item = _wcsdup(item);
		this->publishDate = publishDate;
		this->totalSize = 0;
		this->downloaded = 0;
		ref_count=1;
	}
	
	void OnInit(DownloadToken token)
	{
		// ---- Inform the download status service of our presence----
		downloadStatus.AddDownloadThread(token, this->channel, this->item, this->destination_filepath);
	}

	void OnConnect(DownloadToken token)
	{
		// ---- retrieve total size
		api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
		if (http) 
		{
			this->totalSize = http->content_length();
		}

		// ---- create file handle
		hFile = CreateFile(destination_filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( hFile == INVALID_HANDLE_VALUE ) 
		{
			AGAVE_API_DOWNLOADMANAGER->CancelDownload(token);
		}		
	}

	void OnData(DownloadToken token, void *data, size_t datalen)
	{
		// ---- OnConnect copied here due to dlmgr OnData called first
		// ---- retrieve total size
		api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
		if ( !this->totalSize && http ) 
		{
			this->totalSize = http->content_length();
		}

		if ( hFile == INVALID_HANDLE_VALUE )
		{
			// ---- create file handle
			hFile = CreateFile(destination_filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if ( hFile == INVALID_HANDLE_VALUE ) 
			{
				AGAVE_API_DOWNLOADMANAGER->CancelDownload(token);
				return;
			}		
		}
		// ---- OnConnect to be removed once dlmgr is fixed

		// ---- OnData
		// ---- if file handle is invalid, then cancel download
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			AGAVE_API_DOWNLOADMANAGER->CancelDownload(token);
			return;
		}

		this->downloaded = AGAVE_API_DOWNLOADMANAGER->GetBytesDownloaded(token);

		if ( datalen > 0 ) 
		{
			// ---- hFile is valid handle, and write to disk
			DWORD numWritten;
			WriteFile(hFile, data, datalen, &numWritten, FALSE);
			
			// ---- failed writing the number of datalen characters, cancel download
			if (numWritten != datalen) 
			{
				AGAVE_API_DOWNLOADMANAGER->CancelDownload(token);
				return;
			}
		}
	
		// if killswitch is turned on, then cancel download
		if ( downloadStatus.UpdateStatus(token, this->downloaded, this->totalSize) )
		{
			AGAVE_API_DOWNLOADMANAGER->CancelDownload(token);
		}
	}

	void OnCancel(DownloadToken token)
	{
		if ( hFile != INVALID_HANDLE_VALUE ) 
		{
			CloseHandle(hFile);
			DeleteFile(destination_filepath);
		}
		DownloadsUpdated(token,NULL);
		downloadStatus.DownloadThreadDone(token);

		{
			AutoLock lock(downloadsQueueLock);
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if (downloadsQueue.at(index) == token)
				{
					downloadsQueue.eraseAt(index);
					break;
				}
			}
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if(AGAVE_API_DOWNLOADMANAGER->IsPending(downloadsQueue.at(index)))
				{
					AGAVE_API_DOWNLOADMANAGER->ResumePendingDownload(downloadsQueue.at(index));
					break;
				}
			}
		}
		this->Release();
	}	

	void OnError(DownloadToken token, int error)
	{
		if ( hFile != INVALID_HANDLE_VALUE ) 
		{
			CloseHandle(hFile);
			DeleteFile(destination_filepath);
		}
		DownloadsUpdated(token,NULL);
		downloadStatus.DownloadThreadDone(token);

		{
			AutoLock lock(downloadsQueueLock);
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if (downloadsQueue.at(index) == token)
				{
					downloadsQueue.eraseAt(index);
					break;
				}
			}
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if(AGAVE_API_DOWNLOADMANAGER->IsPending(downloadsQueue.at(index)))
				{
					AGAVE_API_DOWNLOADMANAGER->ResumePendingDownload(downloadsQueue.at(index));
					break;
				}
			}
		}
		this->Release();
	}

	void OnFinish(DownloadToken token)
	{
		if ( hFile != INVALID_HANDLE_VALUE ) 
		{
			CloseHandle(hFile);

			DownloadedFile *data = new DownloadedFile(this->url, this->destination_filepath, this->channel, this->item, this->publishDate);
			data->bytesDownloaded = this->downloaded;
			data->totalSize = this->totalSize;
			{
				AutoLock lock(downloadedFiles);
				downloadedFiles.downloadList.push_back(*data);
				addToLibrary_thread(*data);

				AddPodcastData(*data);

				DownloadsUpdated(token,&downloadedFiles.downloadList[downloadedFiles.downloadList.size()-1]);
			}
			downloadStatus.DownloadThreadDone(token);
			delete data;
		}
		else {
			DownloadsUpdated(token,NULL);
			downloadStatus.DownloadThreadDone(token);
		}

		{
			AutoLock lock(downloadsQueueLock);
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if (downloadsQueue.at(index) == token)
				{
					downloadsQueue.eraseAt(index);
					break;
				}
			}
			for (size_t index = 0; index < downloadsQueue.size(); index++)
			{
				if(AGAVE_API_DOWNLOADMANAGER->IsPending(downloadsQueue.at(index)))
				{
					AGAVE_API_DOWNLOADMANAGER->ResumePendingDownload(downloadsQueue.at(index));
					break;
				}
			}
		}
		this->Release();
	}

	int GetSource(wchar_t *source, size_t source_cch)
	{
		return wcscpy_s(source, source_cch, this->channel);
	}

	int GetTitle(wchar_t *title, size_t title_cch)
	{
		return wcscpy_s(title, title_cch, this->item);
	}

	int GetLocation(wchar_t *location, size_t location_cch)
	{
		return wcscpy_s(location, location_cch, this->destination_filepath);
	}

	size_t AddRef()
	{
		return InterlockedIncrement((LONG*)&ref_count);
	}

	size_t Release()
	{
		if (0 == ref_count)
			return ref_count;

		LONG r = InterlockedDecrement((LONG*)&ref_count);
		if (0 == r)
			delete(this);

		return r;
	}

private: // private destructor so no one accidentally calls delete directly on this reference counted object
	~DownloaderCallback() {
		if(url) free(url);
		if(destination_filepath) free(destination_filepath);
		if(channel) free(channel);
		if(item) free(item);
	}

protected:
	RECVS_DISPATCH;

private:
	HANDLE hFile;
	wchar_t *url;
	wchar_t *destination_filepath;
	wchar_t *channel;
	wchar_t *item;
	__time64_t publishDate;
	size_t totalSize;
	size_t downloaded;
	LONG ref_count;
};

void BackgroundDownloader::Download(const wchar_t *url, const wchar_t *savePath,
									const wchar_t *channel, const wchar_t *item, __time64_t publishDate)
{
	DownloaderCallback *callback = new DownloaderCallback(url, savePath, channel, item, publishDate);
	{
		Nullsoft::Utility::AutoLock lock(downloadsQueueLock);
		if (downloadsQueue.size() < SIMULTANEOUS_DOWNLOADS) 
		{
			DownloadToken dt = AGAVE_API_DOWNLOADMANAGER->DownloadEx(AutoChar(url), callback, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_UI);
			downloadsQueue.push_back(dt);
		}
		else
		{
			DownloadToken dt = AGAVE_API_DOWNLOADMANAGER->DownloadEx(AutoChar(url), callback, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_PENDING | api_downloadManager::DOWNLOADEX_UI);
			downloadsQueue.push_back(dt);
		}
	}
}

BackgroundDownloader downloader;

#define CBCLASS DownloaderCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnError)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONDATA, OnData)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
CB(IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE, GetSource)
CB(IFC_DOWNLOADMANAGERCALLBACK_GETTITLE, GetTitle)
CB(IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION, GetLocation)
CB(ADDREF, AddRef)
CB(RELEASE, Release)
END_DISPATCH;
#undef CBCLASS
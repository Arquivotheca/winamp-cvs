#include "Main.h"
#include "Downloaded.h"

#include "DownloadStatus.h"
#include "DownloadsDialog.h"
#include "api.h"
#include <api/service/waServiceFactory.h>
#include "../jnetlib/api_httpget.h"
#include "../dlmgr/DownloadManager.h"
using namespace Nullsoft::Utility;

DownloadViewCallback::DownloadViewCallback()
{
	ref_count=1;
}
	
void DownloadViewCallback::OnInit(DownloadToken token)
{
	// ---- Inform the download status service of our presence----
	downloadStatus.AddDownloadThread(token, AGAVE_API_DOWNLOADMANAGER->GetSource(token), AGAVE_API_DOWNLOADMANAGER->GetTitle(token), AGAVE_API_DOWNLOADMANAGER->GetLocation(token));
}

void DownloadViewCallback::OnConnect(DownloadToken token)
{
}

void DownloadViewCallback::OnData(DownloadToken token, void *data, size_t datalen)
{
	api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
	size_t totalSize = http->content_length();
	size_t downloaded = (size_t)AGAVE_API_DOWNLOADMANAGER->GetBytesDownloaded(token);

	// if killswitch is turned on, then cancel download
	downloadStatus.UpdateStatus(token, downloaded, totalSize);
}

void DownloadViewCallback::OnCancel(DownloadToken token)
{
	DownloadedFile *data = new DownloadedFile(AutoWide(AGAVE_API_DOWNLOADMANAGER->GetReceiver(token)->get_url()), AGAVE_API_DOWNLOADMANAGER->GetLocation(token), AGAVE_API_DOWNLOADMANAGER->GetSource(token), AGAVE_API_DOWNLOADMANAGER->GetTitle(token), DownloadedFile::DOWNLOAD_CANCELED, _time64(0));

	api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
	data->totalSize = http->content_length();
	data->bytesDownloaded = (size_t)AGAVE_API_DOWNLOADMANAGER->GetBytesDownloaded(token);
	{
		AutoLock lock(downloadedFiles);
		downloadedFiles.downloadList.push_back(*data);
		DownloadsUpdated(token,&downloadedFiles.downloadList[downloadedFiles.downloadList.size()-1]);
	}
	downloadStatus.DownloadThreadDone(token);
	delete data;
}	

void DownloadViewCallback::OnError(DownloadToken token, int error)
{
	DownloadedFile *data = new DownloadedFile(AutoWide(AGAVE_API_DOWNLOADMANAGER->GetReceiver(token)->get_url()), AGAVE_API_DOWNLOADMANAGER->GetLocation(token), AGAVE_API_DOWNLOADMANAGER->GetSource(token), AGAVE_API_DOWNLOADMANAGER->GetTitle(token), DownloadedFile::DOWNLOAD_FAILURE, _time64(0));

	api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
	data->totalSize = http->content_length();
	data->bytesDownloaded = (size_t)AGAVE_API_DOWNLOADMANAGER->GetBytesDownloaded(token);
	{
		AutoLock lock(downloadedFiles);
		downloadedFiles.downloadList.push_back(*data);
		DownloadsUpdated(token,&downloadedFiles.downloadList[downloadedFiles.downloadList.size()-1]);
	}
	downloadStatus.DownloadThreadDone(token);
	delete data;
}

void DownloadViewCallback::OnFinish(DownloadToken token)
{
	DownloadedFile *data = new DownloadedFile(AutoWide(AGAVE_API_DOWNLOADMANAGER->GetReceiver(token)->get_url()), AGAVE_API_DOWNLOADMANAGER->GetLocation(token), AGAVE_API_DOWNLOADMANAGER->GetSource(token), AGAVE_API_DOWNLOADMANAGER->GetTitle(token), DownloadedFile::DOWNLOAD_SUCCESS, _time64(0));

	api_httpreceiver *http = AGAVE_API_DOWNLOADMANAGER->GetReceiver(token);
	data->totalSize = http->content_length();
	data->bytesDownloaded = (size_t)AGAVE_API_DOWNLOADMANAGER->GetBytesDownloaded(token);
	{
		AutoLock lock(downloadedFiles);
		downloadedFiles.downloadList.push_back(*data);

		//AddDownloadData(*data);

		DownloadsUpdated(token,&downloadedFiles.downloadList[downloadedFiles.downloadList.size()-1]);
	}
	downloadStatus.DownloadThreadDone(token);
	delete data;
}


size_t DownloadViewCallback::AddRef()
{
	return InterlockedIncrement((LONG*)&ref_count);
}

size_t DownloadViewCallback::Release()
{
	if (0 == ref_count)
		return ref_count;

	LONG r = InterlockedDecrement((LONG*)&ref_count);
	if (0 == r)
		delete(this);

	return r;
}

DownloadViewCallback::~DownloadViewCallback() 
{
}

#define CBCLASS DownloadViewCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnError)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONDATA, OnData)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
CB(ADDREF, AddRef)
CB(RELEASE, Release)
END_DISPATCH;
#undef CBCLASS
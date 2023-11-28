#include "api.h"
#include "SongDownloader.h"
#include "main.h"
#include "../jnetlib/api_httpget.h"
#include <strsafe.h>

SongDownloader::SongDownloader(const wchar_t *filename, HANDLE done_event, void (*callback)(void *callbackContext, wchar_t *status), void *context) 
: done_event(done_event), callback(callback), context(context)
{
	hFile = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	content_length=0;
	bytes_downloaded=0;
}

SongDownloader::~SongDownloader()
{
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);	
}

void SongDownloader::OnInit(DownloadToken token)
{
	api_httpreceiver *jnet = WASABI_API_DLMGR->GetReceiver(token);
	if (jnet)
	{
		jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
		jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
	}
}

void SongDownloader::OnData(DownloadToken token, void *data, size_t datalen)
{
	if (!content_length)
	{
		api_httpreceiver *jnet = WASABI_API_DLMGR->GetReceiver(token);
		if (jnet)
		{
			const char *header = jnet->getheader("content-length");
			if (header)
				content_length = _strtoui64(header, 0, 10);
		}
	}
	DWORD written;
	WriteFile(hFile, data, datalen, &written, 0);
	bytes_downloaded+=written;
	wchar_t status[128];
	if (content_length && callback)
	{
		// TODO: lang pack
		StringCbPrintf(status, sizeof(status), L"Transferring (%d%%)", (int)(100ULL * bytes_downloaded / content_length));
		callback(context,status);
	}
}

void SongDownloader::OnCancel(DownloadToken token)
{
	wchar_t status[128];

	// TODO: lang pack
	StringCbCopy(status, sizeof(status), L"Cancelled");
	callback(context,status);

	SetEvent(done_event);
	this->Release();
}	

void SongDownloader::OnError(DownloadToken token, int error)
{
	wchar_t status[128];

	// TODO: lang pack
	StringCbCopy(status, sizeof(status), L"Failed");
	callback(context,status);

	SetEvent(done_event);
	this->Release();
}

void SongDownloader::OnFinish(DownloadToken token)
{
	wchar_t status[128];

	// TODO: lang pack
	StringCbCopy(status, sizeof(status), L"Done");
	callback(context,status);

	SetEvent(done_event);
	this->Release();
}

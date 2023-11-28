#include "main.h"
#include "api.h"
#include "DownloadStatus.h"
#include "DownloadsDialog.h"

#include <strsafe.h>

extern HWND downloads_window;
DownloadStatus downloadStatus;

using namespace Nullsoft::Utility;

DownloadStatus::Status::Status() 
{
	Init();
}

DownloadStatus::Status::Status(size_t _downloaded, size_t _maxSize, const wchar_t *_source, const  wchar_t *_title, const wchar_t *_path)
: downloaded(_downloaded), maxSize(_maxSize), source(source), title(title), path(path),killswitch(0)
{
	Init();
	downloaded = _downloaded;
	maxSize = _maxSize;
	source = _wcsdup(_source);
	title = _wcsdup(_title);
	path = _wcsdup(_path);
}

const DownloadStatus::Status &DownloadStatus::Status::operator =(const DownloadStatus::Status &copy)
{
	Reset();
	Init();
	downloaded = copy.downloaded;
	maxSize = copy.maxSize;
	source = _wcsdup(copy.source);
	title = _wcsdup(copy.title);
	path = _wcsdup(copy.path);
	killswitch=copy.killswitch;
	return *this;
}

DownloadStatus::Status::~Status()
{
	Reset();
}
void DownloadStatus::Status::Init()
{
	downloaded = 0;
	maxSize = 0;
	killswitch = 0;
	source=0;
	title=0;
	path=0;
}
void DownloadStatus::Status::Reset()
{
	free(source);
	free(title);
	free(path);
}

void DownloadStatus::AddDownloadThread(DownloadToken token, const wchar_t *source, const wchar_t *title, const wchar_t *path)
{
	{
		AutoLock lock(statusLock);
		downloads[token] = Status(0,0,source,title,path);
		DownloadsUpdated(downloads[token],token);
	}
	PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, DOWNLOADS_UPDATE);
}

void DownloadStatus::DownloadThreadDone(DownloadToken token)
{
	{
		AutoLock lock(statusLock);
		downloads.erase(token);
	}
		PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, DOWNLOADS_UPDATE);
}

bool DownloadStatus::UpdateStatus(DownloadToken token, size_t downloaded, size_t maxSize)
{
	AutoLock lock(statusLock);
	downloads[token].downloaded = downloaded;
	downloads[token].maxSize = maxSize;
	return !!downloads[token].killswitch;
}

bool DownloadStatus::CurrentlyDownloading()
{
	AutoLock lock(statusLock);
	return !downloads.empty();
}

void DownloadStatus::GetStatusString(wchar_t *status, size_t len)
{
	AutoLock lock(statusLock);
	Downloads::iterator itr;
	size_t bytesDownloaded = 0, bytesTotal = 0, numDownloads= 0;
	bool unknownTotal = false;
	for (itr=downloads.begin();itr!=downloads.end();itr++)
	{	
		Status &dlstatus = itr->second;
		if (dlstatus.maxSize)
		{
			numDownloads++;
			bytesDownloaded+=dlstatus.downloaded;
			bytesTotal += dlstatus.maxSize;
		}
		else // don't have a max size
		{
			if (dlstatus .downloaded) // if we've downloaded some then we just don't know the total
			{
				unknownTotal = true;
				numDownloads++;
				bytesDownloaded+=dlstatus.downloaded;
			}
		}
	}
	if (0 == numDownloads)
	{
		status[0] = L'\0';
	}
	else
	{
		if (unknownTotal || bytesTotal == 0)
			StringCchPrintf(status, len, WASABI_API_LNGSTRINGW(IDS_DOWNLOADING_KB_COMPLETE),
			numDownloads, bytesDownloaded/1024);
		else
			StringCchPrintf(status, len, WASABI_API_LNGSTRINGW(IDS_DOWNLOADING_KB_PROGRESS),
			numDownloads, bytesDownloaded/1024,bytesTotal/1024, MulDiv(bytesDownloaded, 100, bytesTotal));
	}
}

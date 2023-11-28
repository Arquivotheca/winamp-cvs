#include "main.h"
#include "Downloaded.h"

DownloadList downloadedFiles;
using namespace Nullsoft::Utility;
Nullsoft::Utility::LockGuard downloadedLock;

DownloadedFile::DownloadedFile()
{
	Init();
}

DownloadedFile::~DownloadedFile()
{
	Reset();
}

DownloadedFile::DownloadedFile(const wchar_t *_url, const wchar_t *_path, const wchar_t *_source, const wchar_t *_title, int downloadStatus, __time64_t downloadDate)
{
	Init();
	this->downloadStatus = downloadStatus;
	this->downloadDate = downloadDate;
	SetSource(_source);
	SetTitle(_title);
	SetPath(_path);
	SetURL(_url);
}

void DownloadedFile::Init()
{
	url=0;
	path=0;
	source=0;
	title=0;
	bytesDownloaded = 0;
	totalSize = 0;
	downloadStatus = 0;
	downloadDate = 0;
}

void DownloadedFile::Reset()
{
	free(url);
	free(path);
	free(source);
	free(title);
}

void DownloadedFile::SetPath(const wchar_t *_path)
{
	free(path);
	path = _wcsdup(_path);
}

void DownloadedFile::SetURL(const wchar_t *_url)
{
	free(url);
	url = _wcsdup(_url);
}

void DownloadedFile::SetTitle(const wchar_t *_title)
{
	free(title);
	title = _wcsdup(_title);
}

void DownloadedFile::SetSource(const wchar_t *_source)
{
	free(source);
	source = _wcsdup(_source);
}

DownloadedFile::DownloadedFile(const DownloadedFile &copy)
{
	Init();
	operator =(copy);
}

const DownloadedFile &DownloadedFile::operator =(const DownloadedFile &copy)
{
	Reset();
	Init();
	SetSource(copy.source);
	SetTitle(copy.title);
	bytesDownloaded = copy.bytesDownloaded;
	totalSize = copy.totalSize;
	downloadStatus = copy.downloadStatus;
	downloadDate = copy.downloadDate;
	SetPath(copy.path);
	SetURL(copy.url);
	return *this;
}

wchar_t *GetDownloadStatus(int downloadStatus)
{
	switch(downloadStatus)
	{
		case DownloadedFile::DOWNLOAD_SUCCESS:
			return WASABI_API_LNGSTRINGW(IDS_DOWNLOAD_SUCCESS);
		case DownloadedFile::DOWNLOAD_FAILURE:
			return WASABI_API_LNGSTRINGW(IDS_DOWNLOAD_FAILURE);
		case DownloadedFile::DOWNLOAD_CANCELED:
			return WASABI_API_LNGSTRINGW(IDS_DOWNLOAD_CANCELED);
		default:
			return WASABI_API_LNGSTRINGW(IDS_DOWNLOAD_FAILURE);
	}
}
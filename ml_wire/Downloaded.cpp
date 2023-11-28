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

DownloadedFile::DownloadedFile(const wchar_t *_url, const wchar_t *_path, const wchar_t *_channel, const wchar_t *_item, __time64_t publishDate)
{
	Init();
	this->publishDate=publishDate;
	SetChannel(_channel);
	SetItem(_item);
	SetPath(_path);
	SetURL(_url);
}

void DownloadedFile::Init()
{
	url=0;
	path=0;
	channel=0;
	item=0;
	bytesDownloaded = 0;
	totalSize = 0;
	publishDate = 0;
}

void DownloadedFile::Reset()
{
	free(url);
	free(path);
	free(channel);
	free(item);
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

void DownloadedFile::SetItem(const wchar_t *_item)
{
	free(item);
	item = _wcsdup(_item);
}

void DownloadedFile::SetChannel(const wchar_t *_channel)
{
	free(channel);
	channel = _wcsdup(_channel);
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
	SetChannel(copy.channel);
	SetItem(copy.item);
	bytesDownloaded = copy.bytesDownloaded;
	totalSize = copy.totalSize;
	publishDate = copy.publishDate;
	SetPath(copy.path);
	SetURL(copy.url);
	return *this;
}
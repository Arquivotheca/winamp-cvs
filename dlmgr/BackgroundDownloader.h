#pragma once
#include <bfc/platform/types.h>

class Downloader
{ 
public:
	typedef int (*DownloadCallback)(void *userdata, void *buffer, size_t bufferSize);
	bool Download(const char *url, DownloadCallback callback, void *userdata, uint64_t startPosition = 0);

};
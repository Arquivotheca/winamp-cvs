#ifndef NULLSOFT_DOWNLOADSDIALOGH
#define NULLSOFT_DOWNLOADSDIALOGH
#include "DownloadStatus.h"

void DownloadsUpdated();
void DownloadsUpdated(DownloadToken token, const DownloadedFile * f);
void DownloadsUpdated(const DownloadStatus::Status& s, DownloadToken token);
#endif
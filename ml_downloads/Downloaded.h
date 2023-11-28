#ifndef NULLSOFT_DOWNLOADEDH
#define NULLSOFT_DOWNLOADEDH
#include "../nu/AutoLock.h"
#include "../nu/AutoCharFn.h"
#include "../gen_ml/ml.h"
#include "../nu/Vector.h"
#include "../nu/MediaLibraryInterface.h"

class DownloadedFile
{
public:
	DownloadedFile();
	~DownloadedFile();
	DownloadedFile(const wchar_t *_url, const wchar_t *_path,
		const wchar_t *_source, const wchar_t *_title, int downloadStatus, __time64_t downloadDate);
	DownloadedFile(const DownloadedFile &copy);
	const DownloadedFile &operator =(const DownloadedFile &copy);

	void SetPath(const wchar_t *_path);
	void SetURL(const wchar_t *_url);
	void SetTitle(const wchar_t *_title);
	void SetSource(const wchar_t *_source);
	
	enum
	{
		DOWNLOAD_FAILURE = 0,
		DOWNLOAD_SUCCESS = 1,
		DOWNLOAD_CANCELED = 2,
	};

	size_t bytesDownloaded;
	size_t totalSize;
	int downloadStatus;
	__time64_t downloadDate;
	wchar_t *path, *url, *source, *title;
private:
	
	void Init();
	void Reset();
};

class DownloadList
{
public:
	typedef Vector<DownloadedFile> DownloadedFileList;
	typedef DownloadedFileList::iterator iterator;
	typedef DownloadedFileList::const_iterator const_iterator;
	operator Nullsoft::Utility::LockGuard &()
	{
		return downloadedLock;
	}
	void Remove(size_t index)
	{
		downloadList.eraseAt(index);
	}

	bool RemoveAndDelete(int index)
	{
		SendMessage(mediaLibrary.library,WM_ML_IPC,(WPARAM)downloadList[index].path,ML_IPC_DB_REMOVEITEMW);
		if(!DeleteFile(downloadList[index].path) && GetLastError() != ERROR_FILE_NOT_FOUND) return false;
		downloadList.eraseAt(index);
		return true;
	}

	DownloadedFileList downloadList;
	Nullsoft::Utility::LockGuard downloadedLock;

	iterator begin() { return downloadList.begin(); }
	iterator end() { return downloadList.end(); }
};

extern DownloadList downloadedFiles;
extern int downloadsItemSort;
extern bool downloadsSortAscending;
void CleanupDownloads();
wchar_t *GetDownloadStatus(int downloadStatus);
#endif
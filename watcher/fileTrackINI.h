#ifndef NULLSOFT_FILETRACKING_INI_HEADER
#define NULLSOFT_FILETRACKING_INI_HEADER

#include <windows.h>
#include ".\ini.h"

#include ".\watchScanner.h"

class FileTrackINI : protected BaseINI
{

public:
	FileTrackINI(void);
	FileTrackINI(const wchar_t *file, const wchar_t *watcherID);
	virtual ~FileTrackINI(void);

public:
	HRESULT SetTracker(const wchar_t *file, const wchar_t *watcherID);
	HRESULT WriteFileInfo(FILEINFO *fileInfo);
	HRESULT GetFullFileInfo(FILEINFO *fileInfo);
	unsigned int GetCount(void);
	HRESULT SetCount(unsigned int count);
	HRESULT GetFilesHashList(unsigned __int32 *hashList, unsigned int size);
	HRESULT DeleteFileRec(unsigned __int32 fileHash);
	HRESULT DeleteSection(void);
	void DeleteEmptyFile(void);

protected:
	MLString record;
};

#endif // NULLSOFT_FILETRACKING_INI_HEADER
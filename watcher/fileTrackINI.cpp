#include ".\fileTrackINI.h"
#include <strsafe.h>

FileTrackINI::FileTrackINI(void)
{
	record.Allocate(512);
}

FileTrackINI::~FileTrackINI(void)
{
	DeleteFile(TRUE);
}

FileTrackINI::FileTrackINI(const wchar_t *file, const wchar_t *watcherID) : BaseINI(file, watcherID)
{
	if (!IsFileExist()) 
	{
		CreateUnicode(TRUE);
		SetCount(0);
	}
}

HRESULT FileTrackINI::SetTracker(const wchar_t *file, const wchar_t *watcherID)
{
	SetWrokingINI(file);
	SetWorkingSection(watcherID);
	if (!IsFileExist()) 
	{
		CreateUnicode(TRUE);
		SetCount(0);
	}
	return S_OK;
}
HRESULT FileTrackINI::WriteFileInfo(FILEINFO *fileInfo)
{	
	wchar_t hash[9];
	if (S_OK != StringCchPrintfW(hash, 9, L"%08I32X", fileInfo->hash)) return S_FALSE;				
	record.Format(L"%08I32X%08I32X:%08I32X%08I32X:%s", fileInfo->sizeHigh, fileInfo->sizeLow,
														fileInfo->wTime.dwHighDateTime, fileInfo->wTime.dwLowDateTime,
														fileInfo->name);
	SetValue(hash, record);
	return S_OK;
}

HRESULT FileTrackINI::GetFullFileInfo(FILEINFO *fileInfo)
{	
	wchar_t hash[9];
	StringCchPrintfW(hash, 9, L"%08I32X", fileInfo->hash);				
	GetStringValue(hash, &record, L"");
	if(record.GetLength() == 0) return S_FALSE;	
	fileInfo->name.Set(record.GetBuffer() + 34, record.GetLength() - 34);
	swscanf(record.GetBuffer(), L"%08I32X", &fileInfo->sizeHigh);
	swscanf(record.GetBuffer() + 8, L"%08I32X", &fileInfo->sizeLow);
	swscanf(record.GetBuffer() + 17, L"%08I32X", &fileInfo->wTime.dwHighDateTime);
	swscanf(record.GetBuffer() + 25, L"%08I32X", &fileInfo->wTime.dwLowDateTime);
	return S_OK;
}

unsigned int FileTrackINI::GetCount(void)
{
	return GetIntValue(L"COUNT", MAXDWORD); 
}

HRESULT FileTrackINI::SetCount(unsigned int count)
{
	SetValue(L"COUNT", count);
	return S_OK;
}

HRESULT FileTrackINI::GetFilesHashList(unsigned __int32 *hashList, unsigned int size)
{
	MLString buffer((size + 1)*9);
	GetKeys(&buffer);
	if (buffer.GetLength() == 0) return S_FALSE;
	wchar_t *keys = buffer.GetBuffer();
	if (0 == lstrcmpW(keys, L"COUNT")) keys += lstrlenW(L"COUNT") + 1;
	unsigned int index = 0;
    while(*keys != 0 && index < size)
	{
		swscanf(keys, L"%08I32X", &hashList[index]);
		keys += 9;
		index++;
	}
	return S_OK;
}

HRESULT FileTrackINI::DeleteFileRec(unsigned __int32 fileHash)
{
	HRESULT retCode = S_FALSE;
	wchar_t hash[9];
	if (S_OK == StringCchPrintfW(hash, 9, L"%08I32X", fileHash))
	{
		retCode = DeleteKey(hash);
	}
	return retCode;
}

HRESULT FileTrackINI::DeleteSection(void)
{
	return BaseINI::DeleteSection();
}

void FileTrackINI::DeleteEmptyFile(void)
{
	DeleteFile(TRUE);
}

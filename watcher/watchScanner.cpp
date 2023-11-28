#include <windows.h>
#include <strsafe.h>
#include ".\watchScanner.h"
#include ".\watcher.h"

#include "..\nu\threadname.h"

MLWatchScanner::MLWatchScanner(void) :  watch(NULL), priority(SCANPRIORITY_NORMAL), thread(NULL), 
										force(FALSE), searchLevel(0), status(STATUS_SCANER_STOPPED)
{
	heap = GetProcessHeap();
	evntStopScan = CreateEvent(NULL, TRUE, FALSE, NULL);
	evntThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	Reset();
}

MLWatchScanner::~MLWatchScanner(void)
{
	DestroyScanner();
	CloseHandle(evntStopScan);
	CloseHandle(evntThreadExit);
}

int MLWatchScanner::CreateScanner(MLWatcher *watch, int force)
{
	DestroyScanner();
    if (!watch)  return FALSE;
	this->watch = watch;
	this->force = force;
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	id = (st.wSecond + (60 + st.wMinute) + (60 + st.wHour)) |
			((((unsigned __int32)(st.wDay + (40 + st.wMonth))) << 8) & 0x0000FF00) |
			((((unsigned __int32)st.wYear) << 16)  & 0xFFFF0000);
	return TRUE;
}

void MLWatchScanner::DestroyScanner(void)
{
	if (!watch) return;
	
	if (searchLevel) Stop(TRUE);
	if (thread) CloseHandle(thread);
	status = STATUS_SCANER_STOPPED;
	Reset();
	priority = SCANPRIORITY_NORMAL;
	force = FALSE;
	watch = NULL;
	id = 0;
}

void MLWatchScanner::Reset(void)
{
	counterFile		= 0;
	counterFolder	= 0;

	ResetEvent(evntStopScan);
	ResetEvent(evntThreadExit);

}

int MLWatchScanner::Start(void)
{	
	if (!watch || thread) return FALSE;
	
	Reset();
	
	unsigned long tid;
	thread = CreateThread(NULL, 0, ScanThread, this, CREATE_SUSPENDED, &tid);
	if(thread)
	{	
		SetPriority(priority);
		ResumeThread(thread);
	}
	return (NULL != thread);
}

void MLWatchScanner::Stop(int block)
{
	if (!thread) return;
	SetEvent(evntStopScan);
	if (block)
	{
		MSG msg;
		while (WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &evntThreadExit, FALSE, 10, QS_ALLEVENTS))
		{
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessageW(&msg); 
		}
		FireStatusEvent(STATUS_SCANER_STOPPED);

	}
}

void MLWatchScanner::SetPriority(int priority) 
{ 
	this->priority = priority;
	int p;
	switch(priority)
	{
		case SCANPRIORITY_IDLE:		p = THREAD_PRIORITY_IDLE; break;
		case SCANPRIORITY_LOWEST:	p = THREAD_PRIORITY_LOWEST; break;
		case SCANPRIORITY_LOW:		p = THREAD_PRIORITY_BELOW_NORMAL; break;
		case SCANPRIORITY_HIGH:		p = THREAD_PRIORITY_ABOVE_NORMAL; break;
		case SCANPRIORITY_HIGHEST:	p = THREAD_PRIORITY_HIGHEST; break;
		default: p = THREAD_PRIORITY_NORMAL; break;
	}
	if (thread) SetThreadPriority(thread, p);
} 
__inline void MLWatchScanner::IncreaseLevel(void) 
{ 
	searchLevel++;
	if (searchLevel == 1) 
	{
		FireStatusEvent(STATUS_SCANER_STARTING);
		FireStatusEvent(STATUS_SCANER_ACTIVE);
	}
}

__inline int MLWatchScanner::DecreaseLevel(HANDLE search, int rCode)  
{
	if (searchLevel > 0) searchLevel--;
	FindClose(search);
	return rCode;
}

__inline void MLWatchScanner::FireStatusEvent(int newStatus)
{
	status = newStatus;
	if (!watch) return;
	watch->Notify(WATCHER_MSG_STATUS, status);
}

#ifdef _DEBUG
#include "../nu/AutoChar.h"
#endif  _DEBUG 

unsigned long _stdcall MLWatchScanner::ScanThread(void* param)
{
	BOOL scanResult;
	MLWatchScanner* scanner = (MLWatchScanner*)param;

#ifdef _DEBUG
	char scanner_id[128];
	StringCchPrintfA(scanner_id, 128,"SID:%08I32X WID:%s", scanner->id, (const char*)AutoChar(scanner->watch->id));
	SetThreadName(-1, scanner_id);
#endif //_DEBUG

	PATHINFO info;
	ZeroMemory(&info, sizeof(PATHINFO));
	info.path.Allocate(1024);
	info.path.Format(L"%s\\*", scanner->watch->path);
	
	Vector<unsigned __int32> folderList;		
	scanResult = scanner->ScanFolder(&info, &folderList);
	if (STATUS_SCANER_FINISHED == scanResult)
	{
		scanner->watch->ProcessFolderList(&folderList);
	}
	CloseHandle(scanner->thread);
	scanner->thread = NULL;
	ResetEvent(scanner->evntStopScan);
	scanner->FireStatusEvent(scanResult);
	SetEvent(scanner->evntThreadExit);
	return scanResult;
}

int MLWatchScanner::ScanFolder(PATHINFO *info, Vector<unsigned __int32> *folderList)
{
	IncreaseLevel();
	void *search;
	
	search = FindFirstFileW(info->path, &ffData);
	if (INVALID_HANDLE_VALUE == search) return DecreaseLevel(search, STATUS_ERROR);
	
	info->path[info->path.GetLength() - 2] = 0x0000;
	info->path.UpdateBuffer();
	info->hPath = watch->GetHash(info->path, info->path.GetLength());
	folderList->push_back(info->hPath);
	
	do
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(evntStopScan, 0)) 	return DecreaseLevel(search, STATUS_SCANER_STOPPING); 

		if (FILE_ATTRIBUTE_DIRECTORY == (ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			if (!lstrcmpW(ffData.cFileName, L".") || !lstrcmpW(ffData.cFileName, L"..")) continue;
			if (watch->recurse)
			{
				PATHINFO infoNext;
				infoNext.path.Allocate(info->path.GetLength() + 128);
				infoNext.path.Format(L"%s\\%s\\*", (wchar_t*)info->path, ffData.cFileName); 
				counterFolder++;
				int retVal =  ScanFolder(&infoNext, folderList);
				if (STATUS_SCANER_FINISHED != retVal)  return DecreaseLevel(search, retVal);
			}
		}
		else
		{
			counterFile++;
			FILEINFO *file = (FILEINFO*)HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(FILEINFO));
			file->name = ffData.cFileName;
			file->sizeHigh = ffData.nFileSizeHigh;
			file->sizeLow = ffData.nFileSizeLow;
			file->wTime = ffData.ftLastWriteTime;
			file->hash = watch->GetHash(file->name, file->name.GetLength());
			info->fileInfo.push_back(file);
		}
		
	}while(FindNextFileW(search, &ffData));

	int ret = (GetLastError() == ERROR_NO_MORE_FILES) ? STATUS_SCANER_FINISHED :STATUS_ERROR;
	if (STATUS_SCANER_FINISHED == ret && info->fileInfo.size() > 0)
	{
		watch->ProcessFolder(info, force);
		size_t len = info->fileInfo.size();
		for (size_t i = 0; i < len; i++) HeapFree(heap, NULL, info->fileInfo.at(i));
		info->fileInfo.clear();
	}
	return DecreaseLevel(search, ret);
}





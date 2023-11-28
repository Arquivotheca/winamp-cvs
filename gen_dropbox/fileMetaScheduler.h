#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEMETA_SCHEDULER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEMETA_SCHEDULER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/PtrDeque.h"
#include "../nu/PtrList.h"

interface IFileInfo;
interface IFileMetaReader;

class api_metadata;
class FileMetaScheduler;


class __declspec(novtable) FileMetaSchedulerListener
{
public:
	virtual void __cdecl ReadCompleted(FileMetaScheduler *pScheduler, IFileMetaReader *pReader, HRESULT readResult) = 0; // if readResult = E_ABORT scheduler kicked it out
	virtual void __cdecl QueueSizeChanged(FileMetaScheduler *pScheduler) = 0;
	virtual void __cdecl Idle(FileMetaScheduler *pScheduler) = 0;
};

class FileMetaScheduler
{
protected:

public:
	FileMetaScheduler();
	virtual ~FileMetaScheduler();

public:
	HRESULT ScheduleRead(IFileMetaReader *pReader);
	HRESULT ScheduleArrayRead(IFileMetaReader **pReaders, size_t readerCount, BOOL bReversePush);
	void TruncateQueue(size_t nMax);
	size_t GetQueueSize(void);
	void SetQueueLimit(size_t newLimit); // set to 0 to disable;
	void RegisterListener(FileMetaSchedulerListener *pListener);
	BOOL IsIdle();

protected:
	BOOL InitializeThread();
	void KillThread();
	DWORD DiscoveryThread();
	BOOL PerformRead();
	void OnReadCompleted(IFileMetaReader *pReader, HRESULT hr);
	void OnQueueSizeChanged();
	void OnIdle();

protected:
	typedef nu::PtrDeque<IFileMetaReader> SCHEDULEDEQUE;
	friend static DWORD CALLBACK Discovery_ThreadProc(LPVOID param);

protected:
	CRITICAL_SECTION	lockQueue;
	CRITICAL_SECTION	lockListeners;
	HANDLE				hThread;
	HANDLE				hWakeUp;
	HANDLE				hKill;
	SCHEDULEDEQUE		schedule;
	FileMetaSchedulerListener *listener;
	size_t				queueLimit;
	BOOL				idleState;

};

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEMETA_SCHEDULER_HEADER
#include "main.h"

#include "./documentListener.h"
#include "./Document.h"
#include "../nu/threadname.h"

#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"


DocumentSchedulerListener::DocumentSchedulerListener(Document *pDocument, FileMetaScheduler *pScheduler) :
	document(pDocument), scheduler(pScheduler), hThread(NULL), hKill(NULL), hWake(NULL), idle(TRUE)
{	
	InitializeCriticalSection(&lockResults);

	if (NULL != scheduler)
		scheduler->RegisterListener(this);

	DWORD threadId;
	hWake = CreateEvent(0, FALSE, FALSE, 0);
	hKill = CreateEvent(0, TRUE, FALSE, 0);
	if (NULL != hWake && NULL != hKill)
	{
		hThread = CreateThread(NULL, 0, Queue_ThreadProc, this, 0, &threadId);
	}
	
	if (NULL == hThread)
	{
		if (NULL != hWake) 
		{ 
			CloseHandle(hWake);
			hWake = NULL;
		}
		if (NULL != hKill)
		{
			CloseHandle(hKill);
			hKill = NULL;
		}
	}
		
}

DocumentSchedulerListener::~DocumentSchedulerListener(void)
{
	if (NULL != scheduler)
		scheduler->RegisterListener(NULL);

	if (NULL != hThread)
	{
		if (NULL != hKill)
		{
			SetEvent(hKill);
			WaitForSingleObject(hThread, INFINITE);
		}
		CloseHandle(hThread);
	}
	
	if (NULL != hKill)
		CloseHandle(hKill);
	if (NULL != hWake)
		CloseHandle(hWake);

	
	for (size_t i = results.size(); i-- > 0;)
	{
		if (NULL != results[i].pItem)
			results[i].pItem->Release();
	}

	DeleteCriticalSection(&lockResults);
}


void __cdecl DocumentSchedulerListener::ReadCompleted(FileMetaScheduler *pScheduler, IFileMetaReader *pReader, HRESULT readResult)
{
	if (NULL == document || NULL == pReader)
		return;

	DWORD cookie;
	Document::ITEMREAD readData;
	
	readData.result = readResult;
	readData.index = (size_t)((SUCCEEDED(pReader->GetCookie(&cookie))) ? cookie : -1);
	
	if (FAILED(pReader->QueryInterface(IID_IFileInfo, (void**)&readData.pItem)))
		readData.pItem = NULL;
			

	EnterCriticalSection(&lockResults);
	idle = FALSE;
	results.push_back(readData);
	LeaveCriticalSection(&lockResults);

	SetEvent(hWake);
}

void __cdecl DocumentSchedulerListener::QueueSizeChanged(FileMetaScheduler *pScheduler)
{	
}


void __cdecl DocumentSchedulerListener::Idle(FileMetaScheduler *pScheduler)
{
	if (NULL != document)
		document->SetSchedulerIdle();
}

BOOL DocumentSchedulerListener::ProcessResults()
{
	for(;;)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(hKill, 0))
			return FALSE;

		EnterCriticalSection(&lockResults);
		if (results.empty()) 
		{
			idle = TRUE;
			LeaveCriticalSection(&lockResults);
			return TRUE;
		}

		Document::ITEMREAD readData = results.back();
		results.pop_back();
		LeaveCriticalSection(&lockResults);
		
		IFileInfo *pfi = readData.pItem;
		document->ItemReadCompleted(&readData);
		if (NULL != pfi)
			pfi->Release();
	}

	return TRUE;
}

DWORD DocumentSchedulerListener::QueueThread()
{
	HANDLE hEvents[] = { hKill, hWake };
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
	
	for(;;)
	{
		switch (WaitForMultipleObjectsEx(2, hEvents, FALSE, INFINITE, TRUE))
		{
			case WAIT_OBJECT_0: // kill switch
				return 0;

			case WAIT_OBJECT_0 + 1: // job
				if (ProcessResults() && 
					0 == document->GetQueueSize())
				{
					document->Notify(Document::EventReadQueueEmpty, 0);
				}
				break;
		}
	}
}

BOOL DocumentSchedulerListener::IsIdle()
{
	return idle;
}

static DWORD CALLBACK Queue_ThreadProc(LPVOID param)
{
	SetThreadName(-1, "DocumentListenerThread");
	DocumentSchedulerListener* listener = (DocumentSchedulerListener*)param;
	return (NULL != listener) ? listener->QueueThread() : 1;
}


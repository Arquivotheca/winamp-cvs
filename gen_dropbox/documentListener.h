#ifndef NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_LISTENER_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_LISTENER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "./fileMetaScheduler.h"

#include "./document.h"



class DocumentSchedulerListener : FileMetaSchedulerListener
{
public:
	DocumentSchedulerListener(Document *pDocument, FileMetaScheduler *pScheduler);
	~DocumentSchedulerListener(void);

public:
	virtual void __cdecl ReadCompleted(FileMetaScheduler *pScheduler, IFileMetaReader *pReader, HRESULT readResult);
	virtual void __cdecl QueueSizeChanged(FileMetaScheduler *pScheduler);
	virtual void __cdecl Idle(FileMetaScheduler *pScheduler);

	BOOL IsIdle();

protected:
	DWORD QueueThread();
	BOOL ProcessResults();

private:
	friend static DWORD CALLBACK Queue_ThreadProc(LPVOID param);

protected:
	typedef Vector<Document::ITEMREAD, 16> RESULTLIST; 

protected:
	Document *document;
	FileMetaScheduler *scheduler;

	CRITICAL_SECTION lockResults;
	RESULTLIST	results;
	
	HANDLE hThread;
	HANDLE hWake;
	HANDLE hKill;
	BOOL idle;
	

	
};

#endif //NULLOSFT_DROPBOX_PLUGIN_DROPWINDOW_LISTENER_HEADER
#include "main.h"
#include "collect.h"
#include "../nu/LockFreeItem.h"
// use queue to share collected metadata among threads for set AV Track
static LockFreeItem<CollectedData> avItem;

HANDLE hThread=0;
HANDLE killswitch=0,avwake=0,wake=0;

static void SetAvTrack()
{
	CollectedData *data = avItem.GetItem();
	if (data)
	{
		// fire and forget, do not check status
		SubmitAvTrack(data);

		// free memory and run destructor
		delete data;
	}
}

static DWORD WINAPI MUDThread(LPVOID lpParameter)
{
	HANDLE handles[3] = {killswitch, avwake, wake};
	
	DWORD retVal;
	while ((retVal = WaitForMultipleObjects(3, handles, FALSE, INFINITE)) != WAIT_OBJECT_0)
	{
		// TODO: critical section around session_key
		int status = GetLoginStatus();
		if (status != LOGIN_LOGGEDIN && status != LOGIN_EXPIRING) // not logged in
			continue;

		switch(retVal) {
			// avwake
			case WAIT_OBJECT_0+1:
				{
					SetAvTrack();
				}
				break;

			// wake
			case WAIT_OBJECT_0+2:
				{
					int num_submitted=0;
					CollectedData data;
					while (GetCollectedData(data))
					{

						int submission_status = Submit(data);
						if (submission_status == SUBMIT_TRY_AGAIN)
						{
							// if we failed to submit, put it back in the DB
							AddCollectedData(data);
							break; // so we don't loop forever
						}

						data.Reset();
						num_submitted++;
					}

					// if too many items were in the database
					// should compact it
					// since 'normal' usage will only have 1 item in DB at a time
					if (num_submitted > 5) 
					{
						Log(L"[%s] Compacting Database", MakeDateString(_time64(0)));
						CompactDatabase();
					}
				}
				break;

			// other, should not happen
			default:
				break;
		}
		
	}

	// if killswitch is signaled, then resetting av track if avItem is available
	SetAvTrack();

	return 0;
}


static void StartThread()
{
	if (!hThread)
	{
		avwake = CreateEvent(NULL, FALSE, FALSE, NULL);
		wake = CreateEvent(NULL, FALSE, FALSE, NULL);
		killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD threadId=0;
		hThread = CreateThread(0, 0, MUDThread, 0, 0, &threadId);
	}
}

void CloseQueue()
{
	if (killswitch)
	{
		SetEvent(killswitch);
		if (hThread)
			WaitForSingleObject(hThread, INFINITE);
		CloseHandle(killswitch);	
		killswitch=0;
	}

	if (avwake)
		CloseHandle(avwake);
	avwake=0;

	if (wake)
		CloseHandle(wake);
	wake=0;
	
	if (hThread)
		CloseHandle(hThread);
	hThread=0;
}

void AwakenQueue()
{
	StartThread();
	if (wake)
		SetEvent(wake);
}

void AwakenAvQueue()
{
	StartThread();
	if (avwake)
		SetEvent(avwake);
}

void AddCollectedDataToAvQueue(const CollectedData &data)
{
	// allocate memory and copy collected data
	CollectedData *qdata = new CollectedData(data);

	CollectedData *old_data = avItem.SetItem(qdata);
	delete old_data;
}

void FlushAvQueue()
{
	CollectedData *old_data = avItem.GetItem();
	delete old_data;
}
#include "main.h"
#include "api.h"
#include "config.h"
#include "collect.h"
#include "../Winamp/wa_ipc.h"
#include <shlwapi.h>
#include <strsafe.h>

const DWORD_PTR still_listening_timer_id = 52345234U;
const DWORD_PTR collect_timer_id = 52345235U;
const DWORD_PTR avtrack_timer_id = 52345236U;
static wchar_t listening_filename[1040]; // large enough to hold a stream name

static FILETIME startTime={0};
static FILETIME pauseStartTime={0};
static uint64_t lengthPaused=0;
static uint64_t minimumTime=0;
static bool collecting=false;
static bool paused=false;
static bool collected=false;
static CollectedData data;
static bool sent_av = false; //indicate whether need to reset av track
/* things to think about
* verify that state variables (collecting, collected, etc) stay in sync.   Don't want to accidentally scrobble
the wrong track if we hit "next" right as the CollectProc() is running, e.g.
* how to handle radio collecting (if we want to do that) - can't use IPC_PLAYING_FILEW
*/

void ListenReset()
{
	KillTimer(plugin.hwndParent, still_listening_timer_id);
	KillTimer(plugin.hwndParent, collect_timer_id);
	KillTimer(plugin.hwndParent, avtrack_timer_id);
	collecting=false;
	collected=false;
	paused=false;
	lengthPaused=0;
	minimumTime=0;
	data.Reset();
}
static bool ValidMetadata(const CollectedData &data)
{
	return (data.filename && data.filename[0] && data.artist && data.artist[0] && data.title && data.title[0]);
}

static bool IsPlaying()
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 1;
}

static bool IsStopped()
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 0;
}

static bool IsPaused()
{
	return SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 3;
}

static time_t FILETIME_to_time_t(const FILETIME &ft)
{
	ULARGE_INTEGER end;
	memcpy(&end,&ft,sizeof(end));
	end.QuadPart -= 116444736000000000ULL; // adjust epoch
	end.QuadPart /= 10000000ULL; // 100ns -> seconds
	return (time_t)end.QuadPart;
}

static VOID CALLBACK CollectProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	if (collecting && !IsStopped())
	{
		const wchar_t *filename = (const wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
		if (!wcscmp(filename, listening_filename)) // make sure we're still playing the same song!
		{
			// go ahead and collect metadata now, since it's likely to be cached while playing
			// and for URLs there's no way to get metadata after playback has stopped
			if (config_collect && data.Populate(listening_filename))
			{
				data.timestamp = FILETIME_to_time_t(startTime);
				collected=true;
				Log(L"[%s] Collected metadata for %s", MakeDateString(_time64(0)), PathFindFileName(listening_filename));
			}
		}
	}
}

static VOID CALLBACK AvTrackProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	if (collecting && !IsStopped())
	{
		const wchar_t *filename = (const wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
		if (!wcscmp(filename, listening_filename)) // make sure we're still playing the same song!
		{
			// go ahead and collect metadata now, since it's likely to be cached while playing
			// and for URLs there's no way to get metadata after playback has stopped
			// for avtrack set, if metadata is invalid then do not submit to av track
			if (config_avtrack && data.Populate(listening_filename) && ValidMetadata(data))
			{
				data.timestamp = FILETIME_to_time_t(startTime);
				AddCollectedDataToAvQueue(data);
				AwakenAvQueue(); /* Signal av queue	*/
				sent_av = true;
			}
		}
	}
}

int SecondsToWait(int songlength)
{
	// if the user has overridden the song length prefs
	if (config_listening_length >= 50 && config_listening_length <= 90) // sanity check values
	{
		return MulDiv(config_listening_length, songlength, 100);
	}
	else // default values
	{
		// wait at least 2 minutes or half the song length
		return min(songlength/2, 120);
	}
}

static uint64_t FileTimeDelta(const FILETIME &start, const FILETIME &end)
{
	const ULARGE_INTEGER *start_uint = (const ULARGE_INTEGER *)&start;
	const ULARGE_INTEGER *end_uint = (const ULARGE_INTEGER *)&end;
	return end_uint->QuadPart - start_uint->QuadPart;
}

static VOID CALLBACK StillListeningProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, collect_timer_id);
	KillTimer(hwnd, avtrack_timer_id);
	KillTimer(hwnd, idEvent);
	if (collecting && !IsStopped())
	{
		const wchar_t *filename = (const wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);

		// copy to local buffer so we can remember the filename later
		// also, the lifetime of the returned buffer can't be guaranteed if we make
		// another WM_WA_IPC call
		StringCbCopyW(listening_filename, sizeof(listening_filename), filename);

		// get song length (in seconds)
		// most reliable way to get song length is IPC_GET_BASIC_FILE_INFOW
		basicFileInfoStructW info;
		info.filename = listening_filename;
		info.quickCheck = 0;
		info.title = 0;
		info.titlelen = 0;
		info.length = 0;
		SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&info, IPC_GET_BASIC_FILE_INFOW);
		if (info.length > 0)
		{
			if (config_avtrack) 
			{
				// set timer for AvTrackProc with 5 seconds
				SetTimer(hwnd, avtrack_timer_id, 5000, AvTrackProc);
			}

			if (config_collect && AllowedFilename(listening_filename))
			{
				collecting = true;
        
				// calculate timer time and set timer for CollectProc
				int seconds_to_wait = SecondsToWait(info.length);

				// minimumTime is in 100nanosecond units
				minimumTime = (uint64_t)seconds_to_wait * 1000ULL /*ms*/ * 1000ULL /* us */ * 10ULL /* 100 ns */;

				// calculate how long it's been since the original start time
				FILETIME curTime;
				GetSystemTimeAsFileTime(&curTime);
				uint64_t time_elapsed = FileTimeDelta(startTime, curTime);

				// calculate how long to set our timer for
				uint64_t timer_time = minimumTime;
				if (timer_time > time_elapsed)
					timer_time -= time_elapsed;
				else
					timer_time = 0;

				Log(L"[%s] Listening to %s, will collect metadata in %u seconds", MakeDateString(_time64(0)), PathFindFileName(listening_filename), timer_time/(1000ULL /*ms*/ * 1000ULL /* us */ * 10ULL /* 100 ns */));
				SetTimer(hwnd, collect_timer_id, timer_time/(1000ULL /* us */ * 10ULL /* 100 ns */), CollectProc);
			}
		}
	}	
}

void OnStop(bool quick)
{
	if (collected && ValidMetadata(data))
	{
		/* calculate total time spent in playback */
		FILETIME stoppedTime;
		GetSystemTimeAsFileTime(&stoppedTime);
		uint64_t timePlaying = FileTimeDelta(startTime, stoppedTime);
		timePlaying -= lengthPaused; // adjust for any amount of time paused

		/* verify that timePlaying is long enough.
		collect=true will be set on a timer that doesn't account for playback pausing */
		if (timePlaying >= minimumTime)
		{
			data.playLength = (int32_t)(timePlaying/10000000ULL); // 100ns -> seconds
			if (config_collect && AddCollectedData(data))
			{
				Log(L"[%s] Added %s to the local usage database", MakeDateString(_time64(0)), PathFindFileName(listening_filename));
				if (!quick)
				{
					AwakenQueue(); /* Signal queue thread	*/
				}
				else
				{
					config_awaken_on_load=1;
				}
			}
		}
	}			
	
	ListenReset();
	
	// always reset av track when we previousely set a valid av track
	if ( sent_av ) {
		// reset av track to empty values	
		AddCollectedDataToAvQueue(data);
		AwakenAvQueue(); /* Signal av queue */
		sent_av=false;
	}
	
}

static WNDPROC old_wa_proc = 0;
static LRESULT WINAPI SongChangeListenProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/* on song changes, Winamp sends itself the following:
	SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)filename, IPC_PLAYING_FILEW);
	so we'll going to listen for it
	*/
	if (msg == WM_WA_IPC)
	{
		if (lParam == IPC_PLAYING_FILEW) 
		{
			OnStop(); // we might receive this before we get the IPC_CB_MISC_STATUS about the stop, so go ahead and try here
			if (config_collect || config_avtrack)
			{
				const wchar_t *filename = (const wchar_t *)wParam;
				GetSystemTimeAsFileTime(&startTime);
				// we're going to give ourselves 5 seconds before to start to gather information
				// this should prevent some performance issues, hopefully
				if (!PathIsURLW(filename))
				{
					collecting=true;
					SetTimer(hwnd, still_listening_timer_id, 5000, StillListeningProc);
				}
			}
		}
		else if (wParam == IPC_CB_MISC_STATUS && lParam == IPC_CB_MISC)
		{
			// status change (play/stop/pause)
			if (collecting && IsStopped()) // we were collecting and now we've stopped playing
			{
				OnStop();
			}
			else if (collecting && !paused && IsPaused()) // user hit pause
			{
				// start counting paused time
				paused = true;
				GetSystemTimeAsFileTime(&pauseStartTime);
			}
			else if (collecting && paused && IsPlaying()) // unpaused
			{
				paused=false;
				FILETIME unpausedTime;
				GetSystemTimeAsFileTime(&unpausedTime);
				lengthPaused += FileTimeDelta(pauseStartTime, unpausedTime);
			}
		}
	}
	else if (msg == WM_COMMAND)
	{
		WORD cmd = LOWORD(wParam);
		if (cmd == enable_menu_id)
		{
			config_collect=!config_collect;
			Config_SyncEnabled();
			/* TODO: if we're already playing a track, should we scrobble?
			* might have to move if (config_collect) during IPC_PLAYING_FILEW
			* and call SetTimer(StillListeningProc) here
			*/
		}
		else if (cmd == options_menu_id)
		{
			PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&g_prefsItem, IPC_OPENPREFSTOPAGE);
		}
		else if (cmd == login_menu_id)
		{
			if (GetLoginStatus() == LOGIN_LOGGEDIN)
				Logout();
			else
			{
				Login(hwnd, session_key, 512, token_a, 512);
				EnableOrgling_AfterLogin();
			}
			Config_SyncLogin();
		}

	}
	return CallWindowProcW(old_wa_proc, hwnd, msg, wParam, lParam);
}

void HookWinampProc(HWND hwnd)
{
	/** we need to subclass the Winamp main window to get song change notifications
	** subclassing is the "classic" way to get notifications and do fancy stuff in plugins
	** it's ugly, and can cause potential problems, but it works
	**/	
	old_wa_proc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)SongChangeListenProc);
}


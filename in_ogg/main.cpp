#include "../Winamp/in2.h"
#include "main.h"
#include "api.h"
#include "../winamp/wa_ipc.h"
#include <api/service/waservicefactory.h>
#include "resource.h"
#include <strsafe.h>

#define OGG_PLUGIN_VERSION "v0.1"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

void SetFileExtensions(void)
{
	static char fileExtensionsString[256] = {0};	// "OGV\0Ogg Video (OGV)\0"
	char* end = 0;
	size_t remaining;
	StringCchCopyExA(fileExtensionsString, 256, "OGG", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_OGG_DESC), &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, "OGV", &end, &remaining, 0);
	StringCchCopyExA(end+1, remaining-1, WASABI_API_LNGSTRING(IDS_OGV_DESC), &end, &remaining, 0);
	plugin.FileExtensions = fileExtensionsString;
}

HANDLE killswitch = 0;
int g_duration = -1;
int paused = 0;
static HANDLE play_thread = 0;
char pluginName[256] = {0};

static int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMS msgbx = {sizeof(MSGBOXPARAMS),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCE(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirect(&msgbx);
}

void About(HWND hwndParent)
{
	wchar_t message[1024], text[1024];
	StringCchPrintf(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_OGG,text,1024),
					TEXT(OGG_PLUGIN_VERSION),
					TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

void Init()
{
	api_service *service= (api_service *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!service || service == (api_service *)1)
	{
		// no wasabi service manager, wtf?!?!?!
		service=0;
		plugin.FileExtensions = "\0\0"; // get Winamp to ignore us
		plugin.description = "<disabled>";
		return;
	}

	WasabiInit(service);

	StringCchPrintfA(pluginName,256,"%s "OGG_PLUGIN_VERSION,WASABI_API_LNGSTRING(IDS_NULLSOFT_OGG));
	SetFileExtensions();
}

void Quit()
{
	WasabiQuit();
}

void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	if (title)
		*title=0;
	if (length_in_ms)
	{
		*length_in_ms = -1000; // fallback if anything fails
	}

}

int InfoBox(const wchar_t *file, HWND hwndParent)
{
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t *fn)
{
	return 0;
}

int Play(const wchar_t *fn)		// return zero on success, -1 on file-not-found, some other value on other (stopping winamp) error
{
	g_duration = -1;
		paused=0;
	if (!killswitch)
		killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);

	ResetEvent(killswitch);

	play_thread = CreateThread(0, 0, OggPlayThread, _wcsdup(fn), 0, 0);
	SetThreadPriority(play_thread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));
	return 0; // success
}


void Pause()
{
	paused = 1;

		plugin.outMod->Pause(1);
}

void UnPause()
{
	paused = 0;

		plugin.outMod->Pause(0);
}

int IsPaused()
{
	return paused;
}

void Stop()
{
	 if (play_thread)
	{
		SetEvent(killswitch);
		WaitForSingleObject(play_thread, INFINITE);
		ResetEvent(killswitch);
		play_thread=0;
	}
}

// time stuff
int GetLength()
{
	return g_duration;
}

int GetOutputTime()
{
	if (plugin.outMod)
		return plugin.outMod->GetOutputTime();
	else
		return 0;
}

void SetOutputTime(int time_in_ms)
{
	//if (seek_event)
	//{
//		InterlockedExchange(&seek_position, time_in_ms);
		//SetEvent(seek_event);
	//}
}

void SetVolume(int volume)
{
		plugin.outMod->SetVolume(volume);
}

void SetPan(int pan)
{	
		plugin.outMod->SetPan(pan);
}

void EQSet(int on, char data[10], int preamp)
{
}


In_Module plugin = 
{
	IN_VER,
	pluginName,
	NULL, // hMainWindow
	NULL, // hDllInstance
	0, // "Ogv\0Audio/Xiph File Format (Ogg)\0"
	1, // is seekable
	IN_MODULE_FLAG_USES_OUTPUT_PLUGIN, //UsesOutputPlug
	About,
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoBox,
	IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,
	GetLength,
	GetOutputTime,
	SetOutputTime,
	SetVolume,
	SetPan,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	EQSet,
	0,
	0
};

extern "C"	__declspec(dllexport) In_Module * winampGetInModule2()
{
	return &plugin;
}

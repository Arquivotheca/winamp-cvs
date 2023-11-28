#define PLUGIN_NAME "Nullsoft CD plug-in (MusicID 2.6)"
#define PLUGIN_VERSION L"3.94"

#include "main.h"
#include "cddb.h"
#include "windac/dac32.h"
#include "PlayStatus.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "unicode.h"
#include <shlwapi.h>
#include "api.h"
#include "workorder.h"
#include <strsafe.h>
using namespace Nullsoft::Utility;
Nullsoft::Utility::LockGuard *playDeviceGuard = 0;
char * s_last_error = NULL;

static char playDriveLetter;
extern int config_maxextractspeed;
api_config *AGAVE_API_CONFIG=0;
#ifndef WINAMPX

#include "../winamp/wa_ipc.h"

static void code(long* v, long* k)
{
	unsigned long y = v[0], z = v[1], sum = 0,         /* set up */
		delta = 0x9e3779b9, n = 32 ;  /* key schedule constant*/

	while (n-- > 0)
	{                                       /* basic cycle start */
		sum += delta;
		y += ((z << 4) + k[0]) ^(z + sum) ^((z >> 5) + k[1]);
		z += ((y << 4) + k[2]) ^(y + sum) ^((y >> 5) + k[3]); /* end cycle */
	}
	v[0] = y; v[1] = z;

}

int getRegVer()
{
	int *x = (int*)malloc(32);
	long s[3];
	long ss[2] = {GetTickCount(), (int)x + (int)s}; // fucko: better challenge, too
	long tealike_key[4] = { 31337, 0xf00d, 0xdead, 0xbeef}; //fucko: hide this a bit
	free(x);
	s[0] = ss[0];
	s[1] = ss[1];
	s[2] = 0;

	SendMessage(line.hMainWindow, WM_WA_IPC, (WPARAM)s, IPC_GETREGISTEREDVERSION);

	code(ss, tealike_key);

	if (memcmp(s, ss, 8)) return 0;

	return s[2];
}
#else
int getRegVer()
{
	return 1;
}
#endif//WINAMPX

static DINFO g_ps;
int g_ps_inuse = 0;
int g_playtrack, g_playlength;
char lastfn[1024];
int paused;
void _setvolume();
DWORD MainThreadId;

extern char INI_FILE[];
extern char app_name[];

int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
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

void about(HWND hwndParent)
{
	wchar_t message[1024], text[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_CD_PLUGIN_OLD,text,1024);
	StringCchPrintf(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					line.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}


obj_primo *m_veritas_handle = 0;

void quit();
void init();

int isourfile(const char *fn)
{
	if (!_strnicmp(fn, "cda://", 6)) return 1;
	return 0;
}

const char *scanstr_back(const char *str, char *toscan, const char *defval)
{
	const char *s = str + strlen(str) - 1;
	if (strlen(str) < 1) return defval;
	if (strlen(toscan) < 1) return defval;
	while (1)
	{
		char *t = toscan;
		while (*t)
			if (*t++ == *s) return s;
		t = CharPrevA(str, s);
		if (t == s) return defval;
		s = t;
	}
}


const char *extension(const char *fn)
{
	const char *s = scanstr_back(fn, ".\\", fn - 1);
	if (s < fn) return "";
	if (*s == '\\') return "";
	return (s + 1);
}

const char *filename(const char *fn)
{
	const char *s = scanstr_back(fn, "\\/", 0);
	if (!s) return fn;
	return (s + 1);
}


volatile int done;

#include "CDPlay.h"

int g_lastpos;
C_CDPlay *g_cdplay=0;
static int a_v, a_p;

#include "MCIPlay.h"
#include "WindacPlay.h"
#include "LineInPlay.h"
#include "VeritasPlay.h"

bool ParseName(const char *fn, char &device, int &trackNum)
{
	char track[16] = "1";
	if (!_strnicmp(fn, "cda://", 6))
	{
		fn += 6;
		char d[16] = {0, };
		char *p = d;
		while (*fn && *fn != ',' && (p - d < 15)) *p++ = *fn++;
		*p = 0;
		device = toupper(d[0]);
		if (*fn == ',') fn++;
		lstrcpynA(track, fn, sizeof(track));
		trackNum = atoi(track);
		return true;
	}
	else if (!_stricmp(extension(fn), "cda"))
	{
		const char *f = filename(fn);
		if (!_strnicmp(f, "track", 5)) f += 5;
		char t[16] = {0, };
		char *p = t;
		while (*f && *f != '.' && (p - t < 15)) *p++ = *f++;
		lstrcpynA(track, t, sizeof(track));
		device = toupper(fn[0]);
		trackNum = atoi(track);
		return true;
	}
	return false;
}

VeritasPlay *veritasPlayer = 0;
WindacPlay *windacPlayer = 0;
MciPlay *mciPlayer = 0;

int play(const char *fn)
{
	done = 0;
	lstrcpynA(lastfn, fn, 1024);
	line.is_seekable = 0;
	g_lastpos = 0;

	int track;
	if (!ParseName(fn, playDriveLetter, track))
		return 1;

	if (playStatus[playDriveLetter].IsRipping() || (g_cdplay && g_cdplay->IsPlaying(playDriveLetter)))
	{
		char title[32];
		MessageBoxA(NULL, WASABI_API_LNGSTRING(IDS_CD_CURRENTLY_IN_USE),
			WASABI_API_LNGSTRING_BUF(IDS_DRIVE_IN_USE,title,32), MB_OK);
		return 1;
	}
	g_cdplay = NULL;
	int ret = 1;
	//first, try Veritas
	if (!veritasPlayer)	veritasPlayer = new VeritasPlay;
	g_cdplay = veritasPlayer;
	ret = g_cdplay->play(playDriveLetter, track);
	if (ret != 0)
	{
		//second, try Windac
		if (!windacPlayer) windacPlayer = new WindacPlay;
		g_cdplay = windacPlayer;
		ret = g_cdplay->play(playDriveLetter, track);
		if (ret != 0)
		{
			//try MCI
			if (!mciPlayer) mciPlayer = new MciPlay;
			g_cdplay = mciPlayer;
			int ret = g_cdplay->play(playDriveLetter, track);
			if (ret != 0)
			{
				//no luck
				g_cdplay = NULL;
				return ret;
			}
		}
	}
	g_cdplay->setvolume(a_v, a_p);
	paused = 0;

	return 0;
}


void pause()
{
	if (g_cdplay) g_cdplay->pause();
	paused = 1;
}
void unpause()
{
	if (g_cdplay) g_cdplay->unpause();
	paused = 0;
}
int ispaused()
{
	return paused;
}

void stop()
{
	if (g_cdplay)
	{
		g_cdplay->stop();
		g_cdplay = NULL;
	}

	done = 0;
	line.SAVSADeInit();
}

int getlength()
{
	if (g_cdplay) return g_cdplay->getlength();
	return -1000;
}

int getoutputtime()
{
	if (g_cdplay) return g_cdplay->getoutputtime();
	return 0;
	//return audioGetPos();
}
void setoutputtime(int time_in_ms)
{
	if (g_cdplay) g_cdplay->setoutputtime(time_in_ms);
}

void setvolume(int volume)
{
	if (volume != -666)
	{
		a_v = volume;
	}
	if (g_cdplay) g_cdplay->setvolume(a_v, a_p);
}

void setpan(int pan)
{
	a_p = pan;
	if (g_cdplay) g_cdplay->setvolume(a_v, a_p);
}

int infoDlg(const char *fn, HWND hwnd)
{
	return 0;
#if 0 // switched to unified file info dialog in 5.53
	if (!_stricmp(extension(fn), "cda") || !_strnicmp(fn, "cda://", 6))
	{
		if (!g_ps_inuse)
		{
			char device;
			int res=1;
			MCIDEVICEID d = 0;
			g_ps_inuse = 1;
			device = fn[_strnicmp(fn, "cda://", 6) ? 0 : 6];
			if (device >= 'a' && device <= 'z') device += 'A' -'a';

			CDOpen(&d, device, L"infoDlg");
			memset(&g_ps, 0, sizeof(g_ps));
			res = GetDiscID(d, &g_ps);
			CDClose(&d);
			if (!res)
				res = GetCDDBInfo(&g_ps, device);

			//if (!res) DBEdit(&g_ps, hwnd, 0, device);
			//if (!res) 
			{
				if (CDEdit(device, &g_ps, hwnd))
				{
					g_ps_inuse = 0;
					return INFOBOX_EDITED;
				}
			}
			g_ps_inuse = 0;
		}
	}
	return INFOBOX_UNCHANGED;
#endif
}


void getfileinfo(const char *filename, char *title, int *length_in_ms)
{
	int track;
	char device;
	MCIDEVICEID dev2 = 0;
	if (length_in_ms) *length_in_ms = -1000;

	if (!filename || !*filename) // currently playing
	{
		if (!_stricmp(extension(lastfn), "cda") || !_strnicmp(lastfn, "cda://", 6))
		{
			if (title)
			{
				lstrcpynA(title, "CD Track", GETFILEINFO_TITLE_LENGTH);
				if (!g_ps_inuse)
				{
					wchar_t wtitle[256] = L"";//titlestr[256], artist[256], album[256], tuid[32];
					g_ps_inuse = 1;
					memset(&g_ps, 0, sizeof(g_ps));

					if (CDOpen(&dev2, playDriveLetter, L"getfileinfo"))
					{
						int ret = GetDiscID(dev2, &g_ps);
						CDClose(&dev2);
						if (!ret && GetCDDBInfo(&g_ps, 0)) // TODO: get device letter
							PostMessage(line.hMainWindow, WM_USER, (WPARAM) L"cda://", 247/*IPC_REFRESHPLCACHE*/);
						if (wtitle[0])
							lstrcpynA(title, AutoChar(wtitle), GETFILEINFO_TITLE_LENGTH);
						
					}
					g_ps_inuse = 0;
				}
			}
			if (length_in_ms) *length_in_ms = g_playlength;
		}

		return ;
	}

	if (title)
	{
		const char *p = filename + strlen(filename);
		while (p >= filename && *p != '\\') p--;
		lstrcpynA(title, ++p, GETFILEINFO_TITLE_LENGTH);
	}
	track = 0;

	if (!_strnicmp(filename, "cda://", 6)) // determine length of cd track via MCI
	{
		track = atoi(filename + 8);
		device = filename[6];
		if (device >= 'a' && device <= 'z') device += 'A' -'a';

		if (length_in_ms)
		{
			if (CDOpen(&dev2, device, L"getfileinfo"))
			{
				MCI_SET_PARMS sMCISet;
				sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
				MCISendCommand(dev2, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);
				*length_in_ms = CDGetTrackLength(dev2, track);
				sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
				MCISendCommand(dev2, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &sMCISet);
			}
		}
	}
	else // determine from RIFF structure of .CDA
	{
		HMMIO hmmio;
		hmmio = mmioOpenA((char *)filename, NULL, MMIO_READ | MMIO_ALLOCBUF);
		device = filename[0];
		if (device >= 'a' && device <= 'z') device += 'A' -'a';
		if (hmmio)
		{
			MMCKINFO mmckinfoParent;     // parent chunk information
			mmckinfoParent.fccType = mmioFOURCC('C', 'D', 'D', 'A');
			if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF) == MMSYSERR_NOERROR)
			{
				MMCKINFO mmckinfoSubchunk;   // subchunk information structure
				mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
				if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK) == MMSYSERR_NOERROR)
				{
					char *format;
					DWORD dwFmtSize;          // size of "FMT" chunk
					dwFmtSize = mmckinfoSubchunk.cksize;
					format = (char *) GlobalAlloc(GMEM_FIXED, dwFmtSize);
					if (mmioRead(hmmio, (HPSTR) format, dwFmtSize) == (int)dwFmtSize)
					{
						mmioAscend(hmmio, &mmckinfoSubchunk, 0);
						track = *((short int *)format + 1);
						if (length_in_ms)
						{
							int length = *((int *)format + 3);
							int l = length % 75;
							length /= 75;
							length *= 1000;
							length += (l * 1000) / 75;
							*length_in_ms = length;
						}
					}
					GlobalFree(format);
				}
			}
			mmioClose(hmmio, 0);
		}
	}

	if (title && track)
	{
		if (0 && !g_ps_inuse)
		{
			g_ps_inuse = 1;
			memset(&g_ps, 0, sizeof(g_ps));
			if (!dev2)
			{
				CDOpen(&dev2, device, L"getfileinfo");
			}
			if (dev2)
			{
				StringCchPrintfA(title, GETFILEINFO_TITLE_LENGTH, "CD Track %d", track);
				wchar_t wtitle[256] = L"";
				int ret = GetDiscID(dev2, &g_ps);
				CDClose(&dev2);
				dev2=0;
				if (!ret && GetCDDBInfo(&g_ps, device))
					PostMessage(line.hMainWindow, WM_USER, (WPARAM) L"cda://", 247 /*IPC_REFRESHPLCACHE*/);
				if (wtitle[0])
					lstrcpynA(title, AutoChar(wtitle), GETFILEINFO_TITLE_LENGTH);
			}
			g_ps_inuse = 0;
		}
	}
	if (dev2) CDClose(&dev2);
}

void eq_set(int on, char data[10], int preamp)
{}

In_Module line =
{
	IN_VER_U,
	"nullsoft(in_cdda.dll)",
	0,		// hMainWindow
	0,		// hDllInstance
	0,
	0,		// is_seekable
	1,		// uses output plugins
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	getlength,
	getoutputtime,
	setoutputtime,
	setvolume,
	setpan,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,    // dsp shit
	eq_set,
	NULL,   		// setinfo
	NULL // out_mod
};

int m_nblock = 0;

extern "C"
{
	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		s_last_error = NULL;
		return &line;
	}




#if 0 // TODO?

	__declspec(dllexport) int winampWriteExtendedFileInfo()
	{
		s_last_error = NULL;
		// write it out
		if (m_eiw_lastdrive)
		{
			AddToDatabase(&setInfo);
			m_eiw_lastdrive = 0;
			return 1;
		}
		return 0;
	}
#endif
};

api_service *WASABI_API_SVC = 0;
// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_gracenote *AGAVE_API_GRACENOTE = 0;
api_application *WASABI_API_APP = 0;

void SetFileExtensions(void)
{
	static char fileExtensionsString[1200] = {0};	// "CDA\0CDDA Audio Tracks (*.CDA)\0"
	char* end = 0;
	StringCchCopyExA(fileExtensionsString, 1200, "CDA", &end, 0, 0);
	StringCchCopyExA(end+1, 1200, WASABI_API_LNGSTRING(IDS_CDDA_AUDIO_TRACKS), 0, 0, 0);
	line.FileExtensions = fileExtensionsString;
}

const char *inidir = 0;
void init()
{
	CoInitialize(0);
	Cddb_Initialize();

	InitializeCddbCache();

	inidir = (const char *)SendMessage(line.hMainWindow, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
	WASABI_API_SVC = (api_service *)SendMessage(line.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);

	// loader so that we can get the localisation service api for use
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
	if (sf) AGAVE_API_CONFIG = reinterpret_cast<api_config*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID);
	if (sf) AGAVE_API_GRACENOTE = reinterpret_cast<api_gracenote*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(line.hDllInstance,InCDDALangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_CD_PLUGIN),PLUGIN_VERSION);
	line.description = (char*)szDescription;

	SetFileExtensions();

	playDeviceGuard = new Nullsoft::Utility::LockGuard;
	playStatusGuard = new Nullsoft::Utility::LockGuard;

	MainThreadId = GetCurrentThreadId();
	config_read();
}

void quit()
{
	ShutdownMusicIDWorkOrder();

	if (playStatusGuard)
		delete playStatusGuard;
	playStatusGuard = 0;
	if (m_veritas_handle)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) sf->releaseInterface(m_veritas_handle);
		m_veritas_handle = 0;
	}

	delete veritasPlayer;
	delete windacPlayer;
	delete mciPlayer;

	ShutDownCDDB();

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) sf->releaseInterface(WASABI_API_LNG);

	sf = WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
	if (sf) sf->releaseInterface(AGAVE_API_CONFIG);

	sf = WASABI_API_SVC->service_getServiceByGuid(gracenoteApiGUID);
	if (sf) sf->releaseInterface(AGAVE_API_GRACENOTE);

	Cddb_Uninitialize();
	UninitializeCddbCache();

	CloseTables();
	CoUninitialize();
}
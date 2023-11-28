//#define PLUGIN_NAME "Nullsoft MPEG Audio Decoder"

#include "main.h"
#include "../mp3/mp3dec/mp3ssc.h"
#include <time.h>
#include "DecodeThread.h"
#include "api.h"
#include "../Winamp/wa_ipc.h"
#include "MP4Factory.h"
#include "config.h"
#include "AlbumArt.h"
#include "MetadataFactory.h"
#include "NSVFactory.h"
#include "flv_mp3_decoder.h"
#include "../nu/Singleton.h"
#include "mkv_mp3_decoder.h"
#include "avi_mp3_decoder.h"
#include "RawMediaReader.h"

char lastfn_status[256];
int lastfn_status_err;
CRITICAL_SECTION g_lfnscs;
CRITICAL_SECTION streamInfoLock;

int lastfn_data_ready;

int config_fastvis=0;
unsigned char config_miscopts=0;
unsigned char allow_sctitles=1;
unsigned char sctitle_format=1;
unsigned char config_eqmode=4,config_http_proxynonport80=1;
unsigned int winampVersion=0x00005010; // default version # to use if winamp version is 5.1 or less (and therefore doesn't have a valid HWND during Init)
char config_http_save_dir[MAX_PATH] = "C:\\";
int config_http_buffersize=64, config_http_prebuffer=40, config_http_prebuffer_underrun=10;
unsigned char config_downmix=0, config_downsample=0, allow_scartwork=1;

int config_max_bufsize_k=128;
int config_gapless=1;
char INI_FILE[MAX_PATH];

wchar_t lastfn[8192];	// currently playing file (used for getting info on the current file)

// Used for correcting DSP plug-in pitch changes
int paused;				// are we paused?

int m_is_stream;
bool m_is_stream_seekable;

volatile int killDecodeThread=0;			// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread

DWORD WINAPI DecodeThread(LPVOID b); // the decode thread procedure

extern char *getfileextensions();

#include <api/service/waservicefactory.h>

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
}

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_application *WASABI_API_APP = 0;
api_service *serviceManager = 0;
api_config *AGAVE_API_CONFIG=0;
api_memmgr *WASABI_API_MEMMGR = 0;
MPEG4Factory mp3_in_mp4_factory;
AlbumArtFactory albumArtFactory;
MetadataFactory metadataFactory;

static AVIDecoder avi_decoder;
static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static MKVDecoder mkv_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static FLVDecoderCreator flvCreator;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
static NSVFactory nsvFactory;
static SingletonServiceFactory<svc_nsvFactory, NSVFactory> factory;
static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;

void init()
{
	if (mod.hMainWindow)
	{
		winampVersion = SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GETVERSION);
		WASABI_API_SVC = (api_service *)SendMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
		if (WASABI_API_SVC == (api_service *)1)
			WASABI_API_SVC=0;
		WASABI_API_SVC->service_register(&metadataFactory);
		WASABI_API_SVC->service_register(&mp3_in_mp4_factory);
		WASABI_API_SVC->service_register(&albumArtFactory);
		factory.Register(WASABI_API_SVC, &nsvFactory);
		flvFactory.Register(WASABI_API_SVC, &flvCreator);
		mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
		avi_factory.Register(WASABI_API_SVC, &avi_decoder);
		raw_factory.Register(WASABI_API_SVC, &raw_media_reader_service);
	}
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(mod.hDllInstance,InMp3LangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription,256,WASABI_API_LNGSTRINGW(IDS_NULLSOFT_MPEG_AUDIO_DECODER),PLUGIN_VERSION);
	mod.description = (char*)szDescription;

	InitializeCriticalSection(&g_lfnscs);
	InitializeCriticalSection(&streamInfoLock);
	mod.UsesOutputPlug|=2;
	config_read();
	mod.FileExtensions=getfileextensions();
}

void quit()
{
	DeleteCriticalSection(&g_lfnscs);
	DeleteCriticalSection(&streamInfoLock);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	WASABI_API_SVC->service_deregister(&mp3_in_mp4_factory);
	WASABI_API_SVC->service_deregister(&albumArtFactory);
	factory.Deregister(WASABI_API_SVC);
	flvFactory.Deregister(WASABI_API_SVC);
	mkv_factory.Deregister(WASABI_API_SVC);
	avi_factory.Deregister(WASABI_API_SVC);
	raw_factory.Deregister(WASABI_API_SVC);
}

int g_eq_ok;

int isourfile(const wchar_t *fn)
{
	if (!_wcsnicmp(fn,L"uvox://",7)) return 1;
	if (!_wcsnicmp(fn,L"icy://",6)) return 1;
	if (!_wcsnicmp(fn,L"sc://",5)) return 1;
	if (!_wcsnicmp(fn,L"shoutcast://",12)) return 1;
	return 0;
}


int m_force_seek=-1;

// called when winamp wants to play a file
int play(const in_char *fn)
{
	DWORD thread_id;
	lastfn_status_err=0;
	paused=0;
	g_length=-1000;
	decode_pos_ms=0;
	seek_needed=m_force_seek;
	m_force_seek=-1;
	m_is_stream = 0;
	m_is_stream_seekable = false;
	killDecodeThread=0;
	g_sndopened=0;
	lastfn_data_ready=0;
	lastfn_status[0]=0;
	g_bufferstat=0;
	g_closeaudio=0;
	lstrcpynW(lastfn,fn, 8192);
	mod.is_seekable = 0;
	mod.SetInfo(0,0,0,0);

	g_ds=config_downsample;

	g_eq_ok=1;
	// launch decode thread
	thread_handle = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) DecodeThread,NULL,0,&thread_id);
	SetThreadPriority(thread_handle, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	return 0;
}

// standard pause implementation
void pause()
{
	paused=1;
	if (g_sndopened)
		mod.outMod->Pause(1);
}

void unpause()
{
	paused=0;
	if (g_sndopened)
		mod.outMod->Pause(0);
}

int ispaused()
{
	return paused;
}

// stop playing.
void stop()
{
	killDecodeThread=1;
	WaitForSingleObject(thread_handle,INFINITE);
	CloseHandle(thread_handle);
	g_eq_ok=0;
	thread_handle = INVALID_HANDLE_VALUE;
	g_length=-1000;
	lastfn[0]=0;
	if (g_closeaudio)
	{
		g_closeaudio=0;
		mod.outMod->Close();
		mod.SAVSADeInit();
	}
	g_sndopened=0;
	m_force_seek=-1;
}


// returns length of playing track
int getlength()
{
	return g_length;
}


// returns current output position, in ms.
// you could just use return mod.outMod->GetOutputTime(),
// but the dsp plug-ins that do tempo changing tend to make
// that wrong.
int getoutputtime()
{
	if (g_bufferstat)
		return g_bufferstat;

	if (!lastfn_data_ready||!g_sndopened)
		return 0;

	if (seek_needed!=-1)
		return seek_needed;

	return decode_pos_ms +
	       (mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime());
}


// called when the user releases the seek scroll bar.
// usually we use it to set seek_needed to the seek
// point (seek_needed is -1 when no seek is needed)
// and the decode thread checks seek_needed.
void setoutputtime(int time_in_ms)
{


	if (m_is_stream == 0 || (m_is_stream !=0 && m_is_stream_seekable))
	{
		seek_needed=time_in_ms;
		m_force_seek=-1;
	}
}


// standard volume/pan functions
void setvolume(int volume)
{
	mod.outMod->SetVolume(volume);
}
void setpan(int pan)
{
	mod.outMod->SetPan(pan);
}


// this is an odd function. it is used to get the title and/or
// length of a track.
// if filename is either NULL or of length 0, it means you should
// return the info of lastfn. Otherwise, return the information
// for the file in filename.
// if title is NULL, no title is copied into it.
// if length_in_ms is NULL, no length is copied into it.

static int memcmpv(char *d, char v, int l)
{
	while (l--)
		if (*d++ != v) return 1;
	return 0;
}

void eq_set(int on, char data[10], int preamp)
{
	int x;
	eq_preamp = preamp;
	eq_enabled = on;
	for (x = 0; x < 10; x ++)
		eq_tab[x] = data[x];

	// if eq zeroed out, dont use eq
	if (eq_enabled && preamp==31 && !memcmpv(data,31,10))
		eq_enabled=0;
}



// render 576 samples into buf.
// this function is only used by DecodeThread.

// note that if you adjust the size of sample_buffer, for say, 1024
// sample blocks, it will still work, but some of the visualization
// might not look as good as it could. Stick with 576 sample blocks
// if you can, and have an additional auxiliary (overflow) buffer if
// necessary..


// module definition.

extern In_Module mod =
{
	IN_VER_U,	// defined in IN2.H
	"nullsoft(in_mp3.dll)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	0,
	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	0,	// is_seekable
	1,	// uses output plug-in system
	config,
	about,
	init,
	quit,
	getfileinfo,
	id3Dlg,
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

	0,0,0,0,0,0,0,0,0, // visualization calls filled in by winamp

	0,0, // dsp calls filled in by winamp

	eq_set,

	NULL,		// setinfo call filled in by winamp

	0, // out_mod filled in by winamp
};

// exported symbol. Returns output module.
extern "C"
{
	__declspec(dllexport) In_Module * winampGetInModule2()
	{
		return &mod;
	}
}
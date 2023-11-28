#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH
#include <windows.h>
#include <stdio.h>
#include "../Winamp/in2.h"
#include "audio.h"
#include "resource.h"
#include "PlayStatus.h"
#include "../nu/AutoLock.h"
#include <mmsystem.h>

#define WM_WA_MPEG_EOF WM_USER+2

extern In_Module line;
extern int m_veritas_iter;

extern volatile int done;
extern int paused;
extern int g_lastpos;
extern int m_nblock;

extern DWORD MainThreadId;

void config(HWND hwndParent);
void config_read();
extern int config_sample, config_use_dae2, config_use_veritas, config_rip_veritas;

extern int config_rip_buffersize; // number of sectors to read at once when ripping
extern int config_rip_buffers; // number of buffers to use when ripping
extern int config_play_buffersize; // number of sectors to read at once when playing 
extern int config_play_buffers; // number of buffers to use when playing

extern int config_maxextractspeed;

extern int config_offset; // number of samples of offset when ripping (like EAC)
extern int config_read_leadin;

extern int g_playlength;

extern int g_playtrack;
int getRegVer();
const char *extension(const char *fn);
const char *filename(const char *fn);
void WaitForEvent(HANDLE hEvent, DWORD msMaxWaitTime);
MCIERROR MCISendCommand(MCIDEVICEID IDDevice, UINT uMsg, DWORD fdwCommand,      DWORD_PTR dwParam);
int isMediaPresent(MCIDEVICEID wDeviceID);
void CDClose(MCIDEVICEID* lpDeviceID);
BOOL CDOpen(MCIDEVICEID* lpDeviceID, int device, const wchar_t *alias = 0);
BOOL CDID(MCIDEVICEID wDeviceID, wchar_t *id, size_t len);
void CDClose(MCIDEVICEID* lpDeviceID);
unsigned int CDGetTracks(MCIDEVICEID wDeviceID);
unsigned int CDGetCurrTrack(MCIDEVICEID wDeviceID);
int CDPlay(MCIDEVICEID wDeviceID, unsigned int nTrack, BOOL bResume, unsigned int nMin, unsigned int nSec, unsigned int endms);
void CDStop(MCIDEVICEID wDeviceID);
void CDEject(MCIDEVICEID wDeviceID);
void CDPause(MCIDEVICEID wDeviceID);
unsigned int CDGetTrackLength(MCIDEVICEID wDeviceID,
                              unsigned int nTrack);

int getSCSIIDFromDrive(char driveletter, int *host, int *id, int *lun);
extern char * s_last_error;
extern const char *inidir;
void CloseTables();

const char *ReadLine(const char *input, char *output, size_t size, int codepage);

bool ParseName(const char *fn, char &device, int &trackNum);

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
    { 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

class C_CDPlay;
class VeritasPlay;
class WindacPlay;
class MciPlay;

extern C_CDPlay *g_cdplay;
extern VeritasPlay *veritasPlayer;
extern WindacPlay *windacPlayer;
extern MciPlay *mciPlayer;

#include "../primo/obj_primo.h"
extern obj_primo *m_veritas_handle;
#endif
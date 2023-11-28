#ifndef NULLSOFT_WAVECONFIGH
#define NULLSOFT_WAVECONFIGH

//#include "../nu/Config.h"

//extern Nullsoft::Utility::Config waveConfig;
#include <windows.h>
namespace out_wave
{
	extern unsigned int cfg_dev;
	extern int cfg_buf_ms,cfg_prebuf,cfg_trackhack;
	extern bool cfg_volume,cfg_altvol,cfg_resetvol;
}
void SetDefaults();
void SaveConfig();
void LoadConfig(LPCTSTR iniFile);


BOOL WINAPI WaveCfgProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
#endif
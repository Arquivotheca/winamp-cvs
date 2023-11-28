#include "../nu/Config.h"
#include "DSConfig.h"
#include <strsafe.h>
#include "wa2.h"

Nullsoft::Utility::Config dsConfig;
using namespace out_ds;

int out_ds::cfg_buf_ms;
int out_ds::cfg_prebuf2;
int out_ds::cfg_sil_db;
int out_ds::cfg_trackhack;
int out_ds::cfg_oldpause;
int out_ds::cfg_killsil;
int out_ds::cfg_wait;
int out_ds::cfg_createprimary;
int out_ds::cfg_volume;
int out_ds::cfg_fadevol;
int out_ds::cfg_autocpu;
int out_ds::cfg_volmode;
int out_ds::cfg_logvol_min;
int out_ds::cfg_logfades;
int out_ds::cfg_hw_mix;
int out_ds::cfg_override;
int out_ds::cfg_override_freq;
int out_ds::cfg_override_bps;
int out_ds::cfg_override_nch;
int out_ds::cfg_refresh;
GUID out_ds::cfg_dev2;
int out_ds::cfg_def_fade;
int out_ds::cfg_status_update_freq;
__int64 out_ds::cfg_total_time;
int out_ds::cfg_cur_tab;

#define load_var(V) V=dsConfig.cfg_int(TEXT(#V), V)
#define save_var(V) dsConfig.cfg_int(TEXT(#V), V)=V
#define load_var_guid(V) V=dsConfig.cfg_guid(TEXT(#V)) 
#define save_var_guid(V) dsConfig.cfg_guid(TEXT(#V))=V
#define load_var_64(V) V = dsConfig.cfg_int64(TEXT(#V)) 
#define save_var_64(V) dsConfig.cfg_int64(TEXT(#V))=V


void DSLoadConfig(TCHAR *iniFile)
{
	dsConfig.SetFile(iniFile, TEXT("out_ds"));
	DSResetConfig();

	load_var(cfg_buf_ms);
	load_var(cfg_prebuf2);
	load_var(cfg_sil_db);
	load_var(cfg_trackhack);
	load_var(cfg_oldpause);
	load_var(cfg_killsil);
	load_var(cfg_wait);
	load_var(cfg_createprimary);
	load_var(cfg_volume);
	load_var(cfg_fadevol);
	load_var(cfg_autocpu);
	load_var(cfg_volmode);
	load_var(cfg_logvol_min);
	load_var(cfg_logfades);
	load_var(cfg_hw_mix);
	load_var(cfg_override);
	load_var(cfg_override_freq);
	load_var(cfg_override_bps);
	load_var(cfg_override_nch);
	load_var(cfg_refresh);
	load_var_guid(cfg_dev2);
	load_var(cfg_def_fade);
	load_var(cfg_status_update_freq);
	load_var_64(cfg_total_time);
	load_var(cfg_cur_tab);

	cfg_fade_start.Load();
	cfg_fade_firststart.Load();
	cfg_fade_stop.Load();
	cfg_fade_pause.Load();
	cfg_fade_seek.Load();
}

void DSSaveConfig()
{

	save_var(cfg_buf_ms);
	save_var(cfg_prebuf2);
	save_var(cfg_sil_db);
	save_var(cfg_trackhack);
	save_var(cfg_oldpause);
	save_var(cfg_killsil);
	save_var(cfg_wait);
	save_var(cfg_createprimary);
	save_var(cfg_volume);
	save_var(cfg_fadevol);
	save_var(cfg_autocpu);
	save_var(cfg_volmode);
	save_var(cfg_logvol_min);
	save_var(cfg_logfades);
	save_var(cfg_hw_mix);
	save_var(cfg_override);
	save_var(cfg_override_freq);
	save_var(cfg_override_bps);
	save_var(cfg_override_nch);
	save_var(cfg_refresh);
	save_var_guid(cfg_dev2);
	save_var(cfg_def_fade);
	save_var(cfg_status_update_freq);
	save_var_64(cfg_total_time);
	save_var(cfg_cur_tab);

	cfg_fade_start.Save();
	cfg_fade_firststart.Save();
	cfg_fade_stop.Save();
	cfg_fade_pause.Save();
	cfg_fade_seek.Save();
}

void DSResetConfig()
{
	cfg_buf_ms=2000;
	cfg_prebuf2=500;
	cfg_sil_db=400;
	cfg_trackhack=500;
	cfg_oldpause=0;
	cfg_killsil=0;
	cfg_wait=1;
	cfg_createprimary=(GetVersion() & 0x80000000) ? 1 : 0;
	cfg_volume=1;
	cfg_fadevol=1;
	cfg_autocpu=0;
	cfg_volmode=0;
	cfg_logvol_min=100;
	cfg_logfades=0;
	cfg_hw_mix=1;
	cfg_override=0;
	cfg_override_freq=44100;
	cfg_override_bps=16;
	cfg_override_nch=2;
	cfg_refresh=10;
	cfg_def_fade=333;
	cfg_status_update_freq=50;
	cfg_total_time=0;
	cfg_cur_tab = 0;

	cfg_fade_start.Set(333,0,1);
	cfg_fade_firststart.Set(333,0,1);
	cfg_fade_stop.Set(333,0,1);
	cfg_fade_pause.Set(333,1,1);
	cfg_fade_seek.Set(333,1,1);

}


//#define cfg_hw_mix dsConfig.cfg_int("cfg_hw_mix");
/*
cfg_int cfg_status_update_freq("cfg_status_update_freq", 50);
cfg_struct_t<GUID> cfg_dev2("cfg_dev2", 0);
cfg_struct_t<__int64> cfg_total_time("cfg_total_time", 0);

*/
FadeCfg
cfg_fade_start(TEXT("start"), TEXT("cfg_fade_start"), 333, 0, 1),
cfg_fade_firststart(TEXT("first start"), TEXT("cfg_fade_firststart"), 333, 0, 1),
cfg_fade_stop(TEXT("end of song"), TEXT("cfg_fade_stop"), 333, 0, 1),
cfg_fade_pause(TEXT("pause/stop"), TEXT("cfg_fade_pause"), 333, 1, 1),
cfg_fade_seek(TEXT("seek"), TEXT("cfg_fade_seek"), 333, 1, 1);

FadeCfg *fades[N_FADES] = {&cfg_fade_start, &cfg_fade_firststart, &cfg_fade_stop, &cfg_fade_pause, &cfg_fade_seek};
HWND fades_config_wnd = NULL;

class FooString //ghey. no StringPrintf().
{
private:
	TCHAR foo[32];
public:
	operator LPCTSTR() { return foo;}
	FooString(LPCTSTR s1, LPCTSTR s2)
	{
		StringCchCopy(foo, 32, s1);
		StringCchCat(foo, 32, s2);
	}
};

void FadeCfg::Load()
{
	TCHAR temp[FADE_NAME_SIZE+7];
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.time"), vname);
	time = dsConfig.cfg_int(temp, time);
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.on"), vname);
	on = dsConfig.cfg_int(temp, on);
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.usedef"), vname);
	usedef = dsConfig.cfg_int(temp, usedef);
}

FadeCfg::FadeCfg(LPCTSTR _name, LPCTSTR _vname, int vtime, bool von, bool vusedef)
{
	StringCchCopy(vname, FADE_NAME_SIZE, _vname);

	time = vtime;
	on = von;
	usedef = vusedef;

	FadeCfg::Load();
	StringCchCopy(name, FADE_NAME_SIZE, _name);
}

void FadeCfg::Save()
{
	TCHAR temp[FADE_NAME_SIZE+7];
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.time"), vname);
	dsConfig.cfg_int(temp, time) = time;
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.on"), vname);
	dsConfig.cfg_int(temp, on) = on;
	StringCchPrintf(temp, FADE_NAME_SIZE+7, TEXT("%s.usedef"), vname);
	dsConfig.cfg_int(temp, usedef) = usedef;
}
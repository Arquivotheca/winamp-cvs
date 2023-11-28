#ifndef NULLSOFT_DSCONFIGH
#define NULLSOFT_DSCONFIGH


using namespace Nullsoft::Utility;

namespace out_ds
{
extern int cfg_buf_ms;
extern int cfg_prebuf2;
extern int cfg_sil_db;
extern int cfg_trackhack;
extern int cfg_oldpause;
extern int cfg_killsil;
extern int cfg_wait;
extern int cfg_createprimary;
extern int cfg_volume;
extern int cfg_fadevol;
extern int cfg_autocpu;
extern int cfg_volmode;
extern int cfg_logvol_min;
extern int cfg_logfades;
extern int cfg_hw_mix;
extern int cfg_override;
extern int cfg_override_freq;
extern int cfg_override_bps;
extern int cfg_override_nch;
extern int cfg_refresh;
extern GUID cfg_dev2;
extern int cfg_def_fade;
extern int cfg_status_update_freq;
extern __int64 cfg_total_time;
extern int cfg_cur_tab;
}

#define FADE_NAME_SIZE 128
class FadeCfg
{
public:
	TCHAR name[FADE_NAME_SIZE], vname[FADE_NAME_SIZE];
	
	void Set(int _time, int _on, int _usedef)
	{
		time=_time;
		on=_on;
		usedef=_usedef;
	}
	void Save();
	inline UINT get_time()
	{
		if (on)
		{
			if (usedef)
			{
				return out_ds::cfg_def_fade;// dsConfig.cfg_int("cfg_def_fade");
			}
			else
			{
				return time;
			}

		}
		else
			return 0;

	}
	inline operator int()
	{
		return get_time();
	}
	void Load();
	FadeCfg(LPCTSTR _name, LPCTSTR _vname, int vtime, bool von, bool vusedef);
	int on, usedef, time;

};
typedef struct
{
	TCHAR name[FADE_NAME_SIZE];
	int on, usedef, time;
}
FadeCfgCopy;
extern FadeCfg cfg_fade_start, cfg_fade_firststart, cfg_fade_stop, cfg_fade_pause, cfg_fade_seek;

#define N_FADES 5
extern FadeCfg *fades[N_FADES];


BOOL CALLBACK CfgProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);


void format_fade(LPTSTR txt, FadeCfgCopy * c, int idx);
extern HWND fades_config_wnd;
//void dsDefaultConfig();
void DSLoadConfig(LPTSTR iniFile);
void DSSaveConfig();
void DSResetConfig();
#endif

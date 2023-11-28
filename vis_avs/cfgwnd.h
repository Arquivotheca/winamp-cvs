void CfgWnd_Create(struct winampVisModule *this_mod);
void CfgWnd_Destroy(void);

void CfgWnd_Populate(int force=0);
void CfgWnd_Unpopulate(int force=0);
void CfgWnd_RePopIfNeeded(void);

extern int cfg_fs_w,cfg_fs_h,cfg_fs_d,cfg_fs_bpp,cfg_fs_fps,cfg_fs_rnd,cfg_fs_flip,cfg_fs_height,cfg_speed,cfg_fs_rnd_time;
extern int cfg_cfgwnd_x,cfg_cfgwnd_y,cfg_cfgwnd_open;
extern int cfg_trans,cfg_trans_amount;
extern int cfg_dont_min_avs;
extern int cfg_smartbeat, cfg_smartbeatsticky, cfg_smartbeatresetnewsong, cfg_smartbeatonlysticky;
extern int cfg_transitions, cfg_transitions2, cfg_transitions_speed, cfg_transition_mode;
extern int cfg_bkgnd_render,cfg_bkgnd_render_color;
extern int cfg_render_prio;

extern char config_pres_subdir[MAX_PATH];
extern HWND g_hwndDlg;

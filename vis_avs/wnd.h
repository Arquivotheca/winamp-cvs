int Wnd_Init(struct winampVisModule *this_mod);
void Wnd_Quit(void);
void SetTransparency(HWND hWnd, int enable, int amount);
HWND GetWinampHwnd(void);
void about(HWND hwndParent);

extern HWND g_hwnd;
extern HINSTANCE g_hInstance;
extern int g_in_destroy;
extern int g_rnd_cnt;
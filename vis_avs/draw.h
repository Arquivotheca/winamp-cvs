int DDraw_Init();
void DDraw_Quit(void);
void DDraw_Resize(int w, int h, int dsize);
void DDraw_BeginResize(void);
void DDraw_Enter(int *w, int *h, int **fb1, int **fb2);
void DDraw_Exit(int which);
void DDraw_SetFullScreen(int fs, int w, int h, int dbl, int bps);
int DDraw_IsFullScreen(void);
void DDraw_EnumDispModes(HWND);
double DDraw_translatePoint(POINT p, int isY);

void DDraw_SetStatusText(char *text, int life=0);
int DDraw_IsMode(int w, int h, int bpp);
#ifndef _R_TRANSITION_H_
#define _R_TRANSITION_H_

#include "undo.h"

class C_RenderTransitionClass  {
	protected:
	    int *fbs[4];
		int ep[2];
		int l_w, l_h;
		int enabled;
		int start_time;
		int curtrans;
		int mask;
	    HANDLE initThread;
		char last_file[MAX_PATH];
	    int last_which;
	    int _dotransitionflag;

	public:
	    static  unsigned int WINAPI m_initThread(LPVOID p);
	    int LoadPreset(char *file, int which, C_UndoItem *item=0); // 0 on success
		C_RenderTransitionClass();
		virtual ~C_RenderTransitionClass();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
};

#endif // _R_TRANSITION_H_
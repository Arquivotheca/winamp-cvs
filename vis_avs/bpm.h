#ifndef __BPM_H
#define __BPM_H

#define BEAT_REAL						1
#define BEAT_GUESSED				2

#define MAX_BPM						170
#define MIN_BPM						 60

#define BETTER_CONF_ADOPT     2
#define TOP_CONF_ADOPT        8
#define MIN_STICKY	          8
#define STICKY_THRESHOLD     70
#define STICKY_THRESHOLD_LOW 85

typedef struct {
	DWORD TC; // Tick count
	int Type; // Real/guessed
}BeatType;

BOOL CALLBACK DlgProc_Bpm(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
void initBpm(void);
int refineBeat(int isBeat);

#endif

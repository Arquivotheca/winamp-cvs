#ifndef NULLSOFT_AUDIOTHREADH
#define NULLSOFT_AUDIOTHREADH
#include <windows.h>
VOID CALLBACK APCSeek(ULONG_PTR data);
VOID CALLBACK APCPause(ULONG_PTR data);
VOID CALLBACK APCStart(ULONG_PTR data);
VOID CALLBACK APCStop(ULONG_PTR data);
void Kill();
void AudioThreadInit();
void AudioThreadQuit();
extern HANDLE audioThread;
extern HANDLE stopped;
extern HANDLE events[2];
#endif
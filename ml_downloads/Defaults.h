#ifndef NULLSOFT_DEFAULTSH
#define NULLSOFT_DEFAULTSH
#include <windows.h>
extern wchar_t defaultDownloadPath[MAX_PATH];

#define DOWNLOADSSOURCEWIDTHDEFAULT 200
#define DOWNLOADSTITLEWIDTHDEFAULT 200
#define DOWNLOADSPROGRESSWIDTHDEFAULT 100
#define DOWNLOADSDATEWIDTHDEFAULTS 100
#define DOWNLOADSPATHWIDTHDEFAULTS 200

extern int downloadsSourceWidth;
extern int downloadsTitleWidth;
extern int downloadsProgressWidth;
extern int downloadsPathWidth;
extern int downloadsDateWidth;

extern bool needToMakePodcastsView;

void BuildDefaults(HWND);
#endif
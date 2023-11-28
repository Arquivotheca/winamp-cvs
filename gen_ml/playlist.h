#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_

#include "gen.h"

#ifdef __cplusplus
extern "C" {
#endif

  // this is actually in add.cpp, heh
extern int getFileInfo(const char *filename, const char *metadata, char *dest, int len);
int getFileInfoW(const char *filename, const char *metadata, wchar_t *dest, int len);

extern winampGeneralPurposePlugin plugin;
int loadm3ufn(char *filename, int whattodo);
int loadplsfn(char *filename, int whattodo);
int savem3ufn(char *filename, int rel, int ext);
void saveplsfn(char *fn, int rel);

char *scanstr_back(char *str, char *toscan, char *defval);
char *extension(char *fn) ;
int PlayList_getitem2(int position, char *filename, char *filetitle); // in this implementation, returns length!
int PlayList_getitem_pl(int position, char *);
int PlayList_getlength(void);
int PlayList_deleteitem(int item);
void PlayList_delete(void);
void PlayList_append(char *filename);
void PlayList_appendthing(char *url);
void PlayList_append_withinfo(char *filename, char *title, int length);
void PlayList_appendthing_withinfo(char *url, char *title, int length);
void PlayList_swap(int e1, int e2);
void PlayList_addfromdlg(char *fns);
void PlayList_randomize(void);
void PlayList_reverse(void);
void PlayList_sort(int, int start_p);
void PlayList_adddir(char *path, int recurse);
void PlayList_makerelative(char *listfile,char *filename);
int PlayList_gettotallength(void);
int PlayList_getselect(int);
void PlayList_setselect(int,int);
void PlayList_setitem(int x, char *filename, char *filetitle, int len);
void PlayList_saveend(int start);
void PlayList_restoreend(void);
void PlayList_setcached(int x, int cached);
int PlayList_getcached(int x);

typedef struct
{
  void *list;
  int list_size, malloced_size;
  void *tlist;
  int t_len;
} plstate;

// DO NOT USE THESE UNLESS YOU REALLY, REALLY, MEAN IT AND UNDERSTAND THE UNDOCUMENTEDNESS
void PlayList_savestate(plstate *state);
void PlayList_loadstate(plstate *state);


BOOL CDOpen(MCIDEVICEID* lpDeviceID, int device);
void CDClose(MCIDEVICEID* lpDeviceID);
unsigned int CDGetTracks(MCIDEVICEID wDeviceID);

#ifdef __cplusplus
};
#endif

#endif
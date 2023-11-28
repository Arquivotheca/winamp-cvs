#ifndef NULLSOFT_IN_VORBIS_GENRES_H
#define NULLSOFT_IN_VORBIS_GENRES_H

#ifdef  __cplusplus
extern "C" {
#endif

void genres_read(HWND wnd,char * fn);
void genres_write(HWND wnd,char * fn);
#define MAX_GENRE 256

#ifdef __cplusplus
}
#endif

#endif
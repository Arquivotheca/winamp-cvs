#ifndef NULLSOFT_MPEGUTILH
#define NULLSOFT_MPEGUTILH

#include "../mp3/mp3dec/mpeg.h"

extern float g_vis_table[2][2][32][18];
void do_layer3_vis(short *samples, float *xr, int nch, int ts);
void mp3Equalize(float *xr, int nch, int srate);
void mp2Equalize(float *xr, int nch, int srate, int nparts);
void mp3GiveVisData(float vistable[2][32][18],int gr, int nch);
#endif
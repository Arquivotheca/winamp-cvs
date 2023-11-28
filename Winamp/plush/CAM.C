/******************************************************************************
Plush Version 1.1
cam.c
Camera Control
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/
#include <windows.h>
#include "plush.h"

void plCamDelete(pl_Cam *c) {
  if (c) GlobalFree(c);
}

void plCamSetTarget(pl_Cam *c, pl_Float x, pl_Float y, pl_Float z) {
  pl_Float dx, dy, dz;
  dx = x - c->X;
  dy = y - c->Y;
  dz = z - c->Z;
  c->Roll = 0;
  if (dz > 0.0001f) {
    c->Pan = -atan((double) (dx/dz))*180.0/PL_PI;
    dz /= cos((double) (c->Pan*PL_PI/180.0));
    c->Pitch = atan((double) (dy/dz))*180.0/PL_PI;
  } else if (dz < -0.0001f) { 
    c->Pan = 180.0-atan((double) (dx/dz))*180.0/PL_PI;
    dz /= cos((double) ((c->Pan-180.0)*PL_PI/180.0));
    c->Pitch = -atan((double) (dy/dz))*180.0/PL_PI;
  } else {
    c->Pan = 0.0;
    c->Pitch = -90.0;
  }
}

pl_Cam *plCamCreate(pl_uInt sw, pl_uInt sh, pl_Float ar, pl_Float fov,
                    pl_uChar *fb, pl_ZBuffer *zb) {
  pl_Cam *c;
  c = GlobalAlloc(GPTR,sizeof(pl_Cam));
  if (!c) return 0;
  c->Fov = fov;
  c->AspectRatio = ar;
  c->ClipRight = c->ScreenWidth = sw;
  c->ClipBottom = c->ScreenHeight = sh;
  c->CenterX = sw>>1;
  c->CenterY = sh>>1;
  c->ClipBack = 8.0e30;
  c->frameBuffer = fb;
  c->zBuffer = zb;
  c->Sort = 1;
  if (zb) c->Sort = 0;
  return (c);
}

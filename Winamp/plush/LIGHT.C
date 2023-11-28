/******************************************************************************
Plush Version 1.1
light.c
Light Control
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/

#include <windows.h>
#include "plush.h"

pl_Light *plLightSet(pl_Light *light, pl_uChar mode, pl_Float x, pl_Float y,
                     pl_Float z, pl_Float intensity, pl_Float halfDist) {
  pl_Float m[16], m2[16];
  if (light)
  {
    light->Type = mode;
    light->Intensity = intensity;
    light->HalfDistSquared = halfDist*halfDist;
    switch (mode) {
      case PL_LIGHT_VECTOR:
        plMatrixRotate(m,1,x);
        plMatrixRotate(m2,2,y);
        plMatrixMultiply(m,m2);
        plMatrixRotate(m2,3,z);
        plMatrixMultiply(m,m2);
        plMatrixApply(m,0.0,0.0,-1.0,&light->Xp, &light->Yp, &light->Zp);
      break;
      case PL_LIGHT_POINT_ANGLE:
      case PL_LIGHT_POINT_DISTANCE:
      case PL_LIGHT_POINT:
        light->Xp = x;
        light->Yp = y; 
        light->Zp = z;
      break;
    }
  }
  return light;
}

pl_Light *plLightCreate() {
  pl_Light *l;
  l = (pl_Light *)GlobalAlloc(GPTR,sizeof(pl_Light));
  if (!l) return 0;
  return (l);
}

void plLightDelete(pl_Light *l) {
  if (l) GlobalFree(l);
}

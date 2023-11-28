/******************************************************************************
Plush Version 1.1
plush.c
Misc code and data
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/

#include <windows.h>
#include "plush.h"

/* Can't find another place to put this... */
void plTexDelete(pl_Texture *t) {
  if (t) {
    if (t->Data) GlobalFree(t->Data);
    if (t->PaletteData) GlobalFree(t->PaletteData);
    GlobalFree(t);
  }
}

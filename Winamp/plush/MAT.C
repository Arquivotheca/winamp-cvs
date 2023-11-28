/******************************************************************************
Plush Version 1.1
mat.c
Material Control
All code copyright (c) 1996-1997, Justin Frankel
GlobalFree for non-commercial use. See license.txt for more information.
******************************************************************************/

#include <windows.h>
#include "plush.h"

static void _plGeneratePhongPalette(pl_Mat *);
static void _plGeneratePhongTexturePalette(pl_Mat *, pl_Texture *);
static void _plSetMaterialPutFace(pl_Mat *m);

pl_Mat *plMatCreate() {
  pl_Mat *m; 
  m = (pl_Mat *) GlobalAlloc(GPTR,sizeof(pl_Mat));
  if (!m) return 0;
  m->EnvScaling = 1.0;
  m->TexScaling = 1.0;
  m->Ambient[0] = m->Ambient[1] = m->Ambient[2] = 0;
  m->Diffuse[0] = m->Diffuse[1] = m->Diffuse[2] = 128;
  m->Specular[0] = m->Specular[1] = m->Specular[2] = 128;
  m->Shininess = 4;
  m->NumGradients = 32;
  m->FadeDist = 1000.0;
  m->zBufferable = 1;
  return m;
}

void plMatDelete(pl_Mat *m) {
  if (m) {
    if (m->_ReMapTable) GlobalFree(m->_ReMapTable);
    if (m->_RequestedColors) GlobalFree(m->_RequestedColors);
    if (m->_AddTable) GlobalFree(m->_AddTable);
    GlobalFree(m);
  }
}

void plMatInit(pl_Mat *m) {
  if (m->Shininess < 1) m->Shininess = 1;
  m->_ft = m->Texture ? PL_FILL_TEXTURE : 0;
  m->_st = m->ShadeType;

  if (m->_ft == PL_FILL_SOLID) { 
    _plGeneratePhongPalette(m);
  } else if (m->_ft == PL_FILL_TEXTURE) {
    _plGeneratePhongTexturePalette(m,m->Texture);
  } 
  _plSetMaterialPutFace(m);
}


void plMatMapToPal(pl_Mat *m, pl_uChar *pal, pl_uChar pstart, pl_uChar pend) {
  pl_sInt32 i, k, j, r, g, b, bestdiff, bestpos, r2, g2, b2;
  pl_uChar *p;
  if (m->_ReMapTable) GlobalFree(m->_ReMapTable);
  m->_ReMapTable = (pl_uChar *) GlobalAlloc(GPTR,m->_ColorsUsed);
  if (!m->_RequestedColors) plMatInit(m);
  for (i = 0; i < m->_ColorsUsed; i ++) {
    bestdiff = 1000000000;
    bestpos = pstart;
    r = m->_RequestedColors[i*3];
    g = m->_RequestedColors[i*3+1];
    b = m->_RequestedColors[i*3+2];
    p = pal + pstart*3;
    for (k = pstart; k < pend; k ++) {
      r2 = p[0] - r;
      g2 = p[1] - g;
      b2 = p[2] - b;
      p += 3;
      j = r2*r2+g2*g2+b2*b2;
      if (j < bestdiff) {
        bestdiff = j;
        bestpos = k;
      }
    }
    m->_ReMapTable[i] = bestpos;
  }
//  memset(m->_ReMapTable,m->_ReMapTable[256],256);
 // memset(m->_ReMapTable+m->_ColorsUsed+256,m->_ReMapTable[m->_ColorsUsed+255],256);
}



static void _plGeneratePhongPalette(pl_Mat *m) {
  pl_uInt i = m->NumGradients, x;
  pl_sInt c;
  pl_uChar *pal;
  pl_Float a, da, ca, cb;
  m->_ColorsUsed = m->NumGradients;
  if (m->_RequestedColors) GlobalFree(m->_RequestedColors);
  pal =  m->_RequestedColors = (pl_uChar *) GlobalAlloc(GPTR,m->_ColorsUsed*3);
  a = PL_PI/2.0;
  if (m->NumGradients>0)
    da = -PL_PI/((m->NumGradients-1)<<1);
  else da=0.0;
  do {
    if (m->NumGradients == 1) ca = 1;
    else {
      ca = cos((double) a);
      a += da;
    }
    cb = pow((double) ca, (double) m->Shininess);
    for (x = 0; x < 3; x ++) {
      c = (cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x];
      *(pal++) = plMax(0,plMin(c,255));
    }
  } while (--i); 
}

static void _plGeneratePhongTexturePalette(pl_Mat *m, pl_Texture *t) {
  pl_Float a, ca, da, cb;
  pl_uInt16 *addtable;
  pl_uChar *ppal, *pal;
  pl_sInt c, i, i2, x;
  pl_uInt num_shades = (m->NumGradients / t->NumColors);

  if (num_shades<1) num_shades = 1;
  m->_ColorsUsed = num_shades*t->NumColors;
  if (m->_RequestedColors) GlobalFree(m->_RequestedColors);
  pal = m->_RequestedColors = (pl_uChar *) GlobalAlloc(GPTR,m->_ColorsUsed*3);
  a = PL_PI/2.0;
  if (num_shades>1)
    da = (-PL_PI/2.0)/(num_shades-1);
  else da=0.0;
  i2 = num_shades;
  
  do {
    ppal = t->PaletteData;
    ca = cos((double) a);
    a += da;
    cb = pow(ca, (double) m->Shininess);
    i = t->NumColors;
    do {
      for (x = 0; x < 3; x ++) {
        c = (cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x] + *ppal++;
        *(pal++) = plMax(0,plMin(c,255));
      }
    } while (--i);
  } while (--i2);
  ca = 0;
  if (m->_AddTable) GlobalFree(m->_AddTable);
  m->_AddTable = (pl_uInt16 *) GlobalAlloc(GPTR,256*sizeof(pl_uInt16));
  addtable = m->_AddTable;
  i = 256;
  do {
    a = sin(ca) * num_shades;
    ca += PL_PI/512.0;
    *addtable++ = ((pl_sInt) a)*t->NumColors;
  } while (--i);
}

static void _plSetMaterialPutFace(pl_Mat *m) {
  m->_PutFace = 0;
  switch (m->_ft) {

    /*
  case PL_FILL_TRANSPARENT: switch(m->_st) {
      case PL_SHADE_NONE: case PL_SHADE_FLAT: 
      case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT: 
//        m->_PutFace = plPF_TransF; 
      break;
      case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE: 
      case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE: 
  //      m->_PutFace = plPF_TransG;
      break;
    }
    break;
    */
    case PL_FILL_SOLID: switch(m->_st) {
      case PL_SHADE_NONE: case PL_SHADE_FLAT: 
      case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT: 
        //m->_PutFace = plPF_SolidF;
      break;
      case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE: 
      case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE: 
        m->_PutFace = plPF_SolidG;
      break;
    } 
    break;
    case PL_FILL_ENVIRONMENT:
    case PL_FILL_TEXTURE: 
      if (m->PerspectiveCorrect) switch (m->_st) {
        case PL_SHADE_NONE: case PL_SHADE_FLAT: 
        case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT: 
//          m->_PutFace = plPF_PTexF;
        break;
        case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE: 
        case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE: 
          m->_PutFace = plPF_PTexG;
        break;
      } 
      /*
      else switch (m->_st) {
        case PL_SHADE_NONE: case PL_SHADE_FLAT: 
        case PL_SHADE_FLAT_DISTANCE: case PL_SHADE_FLAT_DISTANCE|PL_SHADE_FLAT: 
          m->_PutFace = plPF_TexF;
        break;
        case PL_SHADE_GOURAUD: case PL_SHADE_GOURAUD_DISTANCE: 
        case PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE: 
          m->_PutFace = plPF_TexG;
        break;
      }
    break;
    case PL_FILL_TEXTURE|PL_FILL_ENVIRONMENT: 
      m->_PutFace = plPF_TexEnv;
    break;
    */
  }
}

typedef struct __ct {
  pl_uChar r,g,b;
  pl_Bool visited;
  struct __ct *next;
} _ct;

int mdist(_ct *a, _ct *b) {
  return ((a->r-b->r)*(a->r-b->r)+(a->g-b->g)*(a->g-b->g)+(a->b-b->b)*(a->b-b->b));
}

void plMatMakeOptPal2(pl_uChar *p, pl_uChar pstart, 
                     pl_uChar pend, pl_Mat **materials) {
  pl_uChar *allColors = 0;
  pl_uInt numColors = 0, nc;
  pl_uChar len = pend - pstart;
  pl_sInt32 x, current, newnext, bestdist, thisdist;
  _ct *colorBlock, *best, *cp;

  for (x = 0; materials[x]; x ++) {
    numColors += materials[x]->_ColorsUsed;
  }
  allColors = (pl_uChar *)GlobalAlloc(GPTR,numColors*3);
  if (!allColors)
  {
    return;
  }

  numColors=0;
  for (x = 0; materials[x]; x ++) {
    memcpy(allColors + (numColors*3), materials[x]->_RequestedColors, materials[x]->_ColorsUsed*3);
    numColors += materials[x]->_ColorsUsed;
  }

  if (numColors <= len) {
    memcpy(p+pstart*3,allColors,numColors*3);
    GlobalFree(allColors);
    return;
  } 

  colorBlock = (_ct *) GlobalAlloc(GPTR,sizeof(_ct)*numColors);
  for (x = 0; x < numColors; x++) {
    colorBlock[x].r = allColors[x*3];
    colorBlock[x].g = allColors[x*3+1];
    colorBlock[x].b = allColors[x*3+2];
    colorBlock[x].visited = 0;
    colorBlock[x].next = 0;
  }
  GlobalFree(allColors);

  /* Build a list, starting at color 0 */
  current = 0;
  nc = numColors;
  do {
    newnext = -1;
    bestdist = 300000000;
    colorBlock[current].visited = 1;
    for (x = 0; x < nc; x ++) {
      if (!colorBlock[x].visited) {
        thisdist = mdist(colorBlock + x, colorBlock + current);
        if (thisdist < 5) { colorBlock[x].visited = 1; numColors--; }
        else if (thisdist < bestdist) { bestdist = thisdist; newnext = x; }
      }
    }
    if (newnext != -1) {
      colorBlock[current].next = colorBlock + newnext;
      current = newnext;
    } 
  } while (newnext != -1);
  colorBlock[current].next = 0; /* terminate the list */

  /* we now have a linked list starting at colorBlock, which is each one and
     it's closest neighbor */

  while (numColors > len) {
    bestdist = mdist(colorBlock,colorBlock->next);
    for (best = cp = colorBlock; cp->next; cp = cp->next) {
      if (bestdist > (thisdist = mdist(cp,cp->next))) {
        best = cp;
        bestdist = thisdist;
      }
    }
    best->r = ((int) best->r + (int) best->next->r)>>1;
    best->g = ((int) best->g + (int) best->next->g)>>1;
    best->b = ((int) best->b + (int) best->next->b)>>1;
    best->next = best->next->next;
    numColors--;
  }
  x = pstart*3;
  for (cp = colorBlock; cp; cp = cp->next) {
    p[x++] = cp->r;
    p[x++] = cp->g;
    p[x++] = cp->b;
  }
  GlobalFree(colorBlock);
}

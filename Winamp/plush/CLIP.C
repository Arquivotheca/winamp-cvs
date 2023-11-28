/******************************************************************************
Plush Version 1.1
clip.c
3D Frustum Clipping
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/

#include "plush.h"

typedef struct {
  pl_Float n[3], d;
} _plane;

#define NUM_CLIP_PLANES 5
static _plane _clipPlanes[NUM_CLIP_PLANES];
static pl_Cam *_cam;
static pl_sInt32 _cx, _cy; 
static pl_Float _fov;
static pl_Float _adj_asp;

static void _FindNormal(pl_Float x1, pl_Float x2, pl_Float x3,
                        pl_Float y1, pl_Float y2, pl_Float y3,
                        pl_Float z1, pl_Float z2, pl_Float z3,
                        pl_Float *x, pl_Float *y, pl_Float *z);
static pl_uInt _ClipToPlane(pl_uInt numVerts,  
                       pl_Vertex *inVertices, pl_Float *inShades,
                       pl_Float *ineMappingU, pl_Float *ineMappingV,
                       pl_Float *inMappingU, pl_Float *inMappingV,
                       _plane *plane, 
                       pl_Vertex *Vertices, pl_Float *Shades, 
                       pl_Float *eMappingU, pl_Float *eMappingV, 
                       pl_Float *MappingU, pl_Float *MappingV);

static void _FindNormal(pl_Float x1, pl_Float x2, pl_Float x3,
                          pl_Float y1, pl_Float y2, pl_Float y3,
                          pl_Float z1, pl_Float z2, pl_Float z3,
                          pl_Float *x, pl_Float *y, pl_Float *z) {
  pl_Float tx1, tx2, ty1, ty2, tz1, tz2;
  tx1 = x1-x2;
  tx2 = x1-x3;
  ty1 = y1-y2;
  ty2 = y1-y3;
  tz1 = z1-z2;
  tz2 = z1-z3;
  *x = ty1*tz2 - tz1*ty2;
  *y = tz1*tx2 - tx1*tz2;
  *z = tx1*ty2 - ty1*tx2;
}

void plClipSetFrustum(pl_Cam *cam) {
  _plane *p;
  _adj_asp = 1.0 / cam->AspectRatio;
  _fov = plMin(plMax(cam->Fov,1),179);
  _fov = (1.0/tan(_fov*(PL_PI/360.0)))*(cam->ClipRight-cam->ClipLeft);
  _cx = cam->CenterX<<20;
  _cy = cam->CenterY<<20;
  _cam = cam;
  p = _clipPlanes;
  /* Back */
  p->n[0] = p->n[1] = 0; p->n[2] = -1.0; p->d = -_cam->ClipBack;
  p++;
  /* Left */
  p->d = 0.00000001;
  if (_cam->ClipLeft == _cam->CenterX) {
    p->n[0] = 1.0;
    p->n[1] = 0;
    p->n[2] = 0;
  }
  else _FindNormal(0,-100,-100, 
                0, 100, -100,
                0,_fov*-100.0/(_cam->ClipLeft-_cam->CenterX),
                _fov*-100.0/(_cam->ClipLeft-_cam->CenterX),
                p->n,p->n+1,p->n+2);
  if (_cam->ClipLeft > _cam->CenterX) {
    p->n[0] = -p->n[0];
    p->n[1] = -p->n[1];
    p->n[2] = -p->n[2];
  }
  p++;
  /* Right */
  p->d = 0.00000001;
  if (_cam->ClipRight == _cam->CenterX) {
    p->n[0] = -1.0;
    p->n[1] = 0;
    p->n[2] = 0;
  }
  else _FindNormal(0,100,100, 
                0, -100, 100,
                0,_fov*100.0/(_cam->ClipRight-_cam->CenterX),
                _fov*100.0/(_cam->ClipRight-_cam->CenterX),
                p->n,p->n+1,p->n+2);
  if (_cam->ClipRight < _cam->CenterX) {
    p->n[0] = -p->n[0];
    p->n[1] = -p->n[1];
    p->n[2] = -p->n[2];
  }
  p++;
  /* Top */
  p->d = 0.00000001;
  if (_cam->ClipTop == _cam->CenterY) {
    p->n[0] = 0;
    p->n[1] = -1.0;
    p->n[2] = 0;
  } else _FindNormal(0, 100, -100, 
                0, 100, 100,
                0,_fov*_adj_asp*100.0/(_cam->CenterY-_cam->ClipTop),
                _fov*_adj_asp*100.0/(_cam->CenterY-_cam->ClipTop),
                p->n,p->n+1,p->n+2);
  if (_cam->ClipTop > _cam->CenterY) {
    p->n[0] = -p->n[0];
    p->n[1] = -p->n[1];
    p->n[2] = -p->n[2];
  }
 
  /* Bottom */
  p++;
  p->d = 0.00000001;
  if (_cam->ClipBottom == _cam->CenterY) {
    p->n[0] = 0;
    p->n[1] = 1.0;
    p->n[2] = 0;
  } else _FindNormal(0, -100, 100, 
                0, -100, -100,
                0,_fov*_adj_asp*-100.0/(_cam->CenterY-_cam->ClipBottom),
                _fov*_adj_asp*-100.0/(_cam->CenterY-_cam->ClipBottom),
                p->n,p->n+1,p->n+2);
  if (_cam->ClipBottom < _cam->CenterY) {
    p->n[0] = -p->n[0];
    p->n[1] = -p->n[1];
    p->n[2] = -p->n[2];
  }
}

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static pl_uInt _ClipToPlane(pl_uInt numVerts,  
                       pl_Vertex *inVertices, pl_Float *inShades,
                       pl_Float *ineMappingU, pl_Float *ineMappingV,
                       pl_Float *inMappingU, pl_Float *inMappingV,
                      _plane *plane, 
                       pl_Vertex *Vertices, pl_Float *Shades, 
                       pl_Float *eMappingU, pl_Float *eMappingV, 
                       pl_Float *MappingU, pl_Float *MappingV) {
  pl_uInt i, nextvert, curin, nextin;
  pl_Float curdot, nextdot, scale;
  pl_uInt invert, outvert;
  invert = 0;
  outvert = 0;
  curdot = inVertices[0].xformedx*plane->n[0] +
           inVertices[0].xformedy*plane->n[1] +
           inVertices[0].xformedz*plane->n[2];
  curin = (curdot >= plane->d);

  for (i=0 ; i < numVerts; i++) {
    nextvert = (i + 1) % numVerts;
    if (curin) {
      Shades[outvert] = inShades[invert];
      MappingU[outvert] = inMappingU[invert];
      MappingV[outvert] = inMappingV[invert];
      eMappingU[outvert] = ineMappingU[invert];
      eMappingV[outvert] = ineMappingV[invert];
      Vertices[outvert++] = inVertices[invert];
    }
    nextdot = inVertices[nextvert].xformedx*plane->n[0] +
              inVertices[nextvert].xformedy*plane->n[1] +
              inVertices[nextvert].xformedz*plane->n[2];
    nextin = (nextdot >= plane->d);
    if (curin != nextin) {
      scale = (plane->d - curdot) / (nextdot - curdot);
      Vertices[outvert].xformedx = inVertices[invert].xformedx +
           (inVertices[nextvert].xformedx - inVertices[invert].xformedx)
             * scale;
      Vertices[outvert].xformedy = inVertices[invert].xformedy +
           (inVertices[nextvert].xformedy - inVertices[invert].xformedy)
             * scale;
      Vertices[outvert].xformedz = inVertices[invert].xformedz +
           (inVertices[nextvert].xformedz - inVertices[invert].xformedz)
             * scale;
      Shades[outvert] = inShades[invert] + 
                        (inShades[nextvert] - inShades[invert]) * scale;
      MappingU[outvert] = inMappingU[invert] + 
           (inMappingU[nextvert] - inMappingU[invert]) * scale;
      MappingV[outvert] = inMappingV[invert] + 
           (inMappingV[nextvert] - inMappingV[invert]) * scale;
      eMappingU[outvert] = ineMappingU[invert] + 
           (ineMappingU[nextvert] - ineMappingU[invert]) * scale;
      eMappingV[outvert] = ineMappingV[invert] + 
           (ineMappingV[nextvert] - ineMappingV[invert]) * scale;
      outvert++;
    }
    curdot = nextdot;
    curin = nextin;
    invert++;
  }
  return outvert;
}

void plClipRenderFace(pl_Face *face) {
  pl_uInt k, a, w, numVerts, q;
  pl_Float tmp, tmp2;
  pl_Face newface;
  static pl_Vertex newVertices[2][8];
  static pl_Float Shades[2][8];
  static pl_Float MappingU[2][8];
  static pl_Float MappingV[2][8];
  static pl_Float eMappingU[2][8];
  static pl_Float eMappingV[2][8];

  if (!face->Material->_PutFace) return;

  for (a = 0; a < 3; a ++) {
    newVertices[0][a] = *(face->Vertices[a]);
    Shades[0][a] = face->Shades[a];
    MappingU[0][a] = face->MappingU[a];
    MappingV[0][a] = face->MappingV[a];
    eMappingU[0][a] = face->eMappingU[a];
    eMappingV[0][a] = face->eMappingV[a];
  }

  numVerts = 3;
  q = 0;
  for (a = (_clipPlanes->d < 0.0 ? 0 : 1);
       a < NUM_CLIP_PLANES && numVerts > 2; a ++) {
    numVerts = _ClipToPlane(numVerts, newVertices[q], Shades[q],
                           eMappingU[q], eMappingV[q], MappingU[q],
                           MappingV[q], _clipPlanes+a, newVertices[q^1],
                           Shades[q^1], eMappingU[q^1],
                           eMappingV[q^1], MappingU[q^1],
                           MappingV[q^1]);
    q ^= 1;
  }
  if (numVerts > 2) {
    memcpy(&newface,face,sizeof(pl_Face));
    for (k = 2; k < numVerts; k ++) {
      newface.fShade = plMax(0,plMin(face->fShade,1));
      for (a = 0; a < 3; a ++) {
        if (a == 0) w = 0;
        else w = a+(k-2);
        newface.Vertices[a] = newVertices[q] + w;
        newface.Shades[a] = Shades[q][w];
        newface.MappingU[a] = MappingU[q][w];
        newface.MappingV[a] = MappingV[q][w];
        newface.eMappingU[a] = eMappingU[q][w];
        newface.eMappingV[a] = eMappingV[q][w];
        newface.Scrz[a] = 1.0/newface.Vertices[a]->xformedz;
        tmp2 = _fov * newface.Scrz[a];
        tmp = tmp2*newface.Vertices[a]->xformedx;
        tmp2 *= newface.Vertices[a]->xformedy;
        newface.Scrx[a] = _cx + ((pl_sInt32)((tmp*(float) (1<<20))));
        newface.Scry[a] = _cy - ((pl_sInt32)((tmp2*_adj_asp*(float) (1<<20))));
      }
      newface.Material->_PutFace(_cam,&newface);
      plRender_TriStats[3] ++; 
    }
    plRender_TriStats[2] ++; 
  }
}

void plClipRenderFaceNC(pl_Face *face) {
  pl_uChar a;
  pl_Float tmp, tmp2;
  if (face->Material->_PutFace)  {
    for (a = 0; a < 3; a ++) {
      face->Scrz[a] = 1.0 / face->Vertices[a]->xformedz;
      tmp2 = _fov * face->Scrz[a];
      tmp = tmp2*face->Vertices[a]->xformedx;
      tmp2 *= face->Vertices[a]->xformedy;
      face->Scrx[a] = _cx + ((pl_sInt32)((tmp*(float) (1<<20))));
      face->Scry[a] = _cy - ((pl_sInt32)((tmp2*_adj_asp*(float) (1<<20))));
    }
    face->Material->_PutFace(_cam,face);
    plRender_TriStats[2]++;
    plRender_TriStats[3]++;
  }
}

pl_sInt plClipNeeded(pl_Face *face) {
  pl_sInt dr,dl,db,dt; 
  pl_Float f;
  dr = (_cam->ClipRight-_cam->CenterX);
  dl = (_cam->ClipLeft-_cam->CenterX);
  db = (_cam->ClipBottom-_cam->CenterY);
  dt = (_cam->ClipTop-_cam->CenterY);
  f = _fov*_adj_asp;
  return ((_cam->ClipBack <= 0.0 ||
           face->Vertices[0]->xformedz <= _cam->ClipBack ||
           face->Vertices[1]->xformedz <= _cam->ClipBack ||
           face->Vertices[2]->xformedz <= _cam->ClipBack) &&
          (face->Vertices[0]->xformedz >= 0 ||
           face->Vertices[1]->xformedz >= 0 || 
           face->Vertices[2]->xformedz >= 0) &&
          (face->Vertices[0]->xformedx*_fov<=dr*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*_fov<=dr*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*_fov<=dr*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedx*_fov>=dl*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*_fov>=dl*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*_fov>=dl*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f<=db*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f<=db*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f<=db*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f>=dt*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f>=dt*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f>=dt*face->Vertices[2]->xformedz));
}

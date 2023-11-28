/******************************************************************************
Plush Version 1.1
render.c
Rendering code: this includes transformation, lighting, etc
All code copyright (c) 1996-1997, Justin Frankel
Free for non-commercial use. See license.txt for more information.
******************************************************************************/

#include "plush.h"
#include <windows.h>

typedef struct {
  pl_Float zd;
  pl_Face *face;
  pl_Bool clipneeded;
} _faceInfo;

#define plMatrixApply(m,x,y,z,outx,outy,outz) \
      *( outx ) = ( x )*( m )[0] + ( y )*( m )[1] + ( z )*( m )[2] + ( m )[3];\
      *( outy ) = ( x )*( m )[4] + ( y )*( m )[5] + ( z )*( m )[6] + ( m )[7];\
      *( outz ) = ( x )*( m )[8] + ( y )*( m )[9] + ( z )*( m )[10] + ( m )[11]

#define plDotProduct(x1,y1,z1,x2,y2,z2) \
      ((( x1 )*( x2 ))+(( y1 )*( y2 ))+(( z1 )*( z2 )))

#define plNormalizeVector(x,y,z) { \
  register pl_Float length; \
  length = (*( x ))*(*( x ))+(*( y ))*(*( y ))+(*( z ))*(*( z )); \
  if (length > 0.0000000001) { \
    length = sqrt(length); \
    *( x ) /= length; \
    *( y ) /= length; \
    *( z ) /= length; \
  } \
}

pl_uInt32 plRender_TriStats[4];

static pl_uInt32 _numfaces;
static _faceInfo *_faces;

static pl_Float _cMatrix[16];
static pl_Float *_l;
static pl_uInt32 _numlights;
static pl_Light **_lights;
static pl_Cam *_cam;
static void _RenderObj(pl_Obj *, pl_Float *, pl_Float *);
static void _sift_down(int L, int U, int dir);
static void _hsort(_faceInfo *base, int nel, int dir);

void plRenderBegin(pl_Cam *Camera) {
  pl_Float tempMatrix[16];
  memset(plRender_TriStats,0,sizeof(plRender_TriStats));
  _cam = Camera;
  _numlights = 0;
  _numfaces = 0;
  _faces=(_faceInfo*)GlobalAlloc(GMEM_FIXED,sizeof(_faceInfo)*8192);
  plMatrixRotate(_cMatrix,2,-Camera->Pan);
  plMatrixRotate(tempMatrix,1,-Camera->Pitch);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plMatrixRotate(tempMatrix,3,-Camera->Roll);
  plMatrixMultiply(_cMatrix,tempMatrix);
  plClipSetFrustum(_cam);
}

void plRenderLight(pl_Light *light) {
  static pl_Float the_light_l[3];
  static pl_Light *the_light;
  pl_Float *pl, xp, yp, zp;
  if (light->Type == PL_LIGHT_NONE) return;
  _numlights=1;
  pl = the_light_l;
  if (light->Type == PL_LIGHT_VECTOR) {
    xp = light->Xp;
    yp = light->Yp;
    zp = light->Zp;
    plMatrixApply(_cMatrix,xp,yp,zp,pl,pl+1,pl+2);
  } else if (light->Type & PL_LIGHT_POINT) {
    xp = light->Xp-_cam->X;
    yp = light->Yp-_cam->Y;
    zp = light->Zp-_cam->Z;
    plMatrixApply(_cMatrix,xp,yp,zp,pl,pl+1,pl+2);
  }
  _l=the_light_l;
  _lights=&the_light;
  _lights[0] = light;
}

static void _RenderObj(pl_Obj *obj, pl_Float *bmatrix, pl_Float *bnmatrix) {
  pl_uInt32 i, x, facepos;
  pl_Float nx = 0.0, ny = 0.0, nz = 0.0;
  pl_Float nx2, ny2, nz2;
  pl_Float tmp, tmp2;
  pl_Float oMatrix[16], nMatrix[16], tempMatrix[16];

  pl_Vertex *vertex;
  pl_Face *face;
  pl_Light *light;

  if (obj->GenMatrix) {
    plMatrixRotate(nMatrix,1,obj->Xa);
    plMatrixRotate(tempMatrix,2,obj->Ya);
    plMatrixMultiply(nMatrix,tempMatrix);
    plMatrixRotate(tempMatrix,3,obj->Za);
    plMatrixMultiply(nMatrix,tempMatrix);
    memcpy(oMatrix,nMatrix,sizeof(pl_Float)*16);
  } else memcpy(nMatrix,obj->RotMatrix,sizeof(pl_Float)*16);

  if (bnmatrix) plMatrixMultiply(nMatrix,bnmatrix);

  if (obj->GenMatrix) {
    plMatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
    plMatrixMultiply(oMatrix,tempMatrix);
  } else memcpy(oMatrix,obj->Matrix,sizeof(pl_Float)*16);
  if (bmatrix) plMatrixMultiply(oMatrix,bmatrix);

  for (i = 0; i < PL_MAX_CHILDREN; i ++)
    if (obj->Children[i]) _RenderObj(obj->Children[i],oMatrix,nMatrix);
  if (!obj->NumFaces || !obj->NumVertices) return;

  plMatrixTranslate(tempMatrix, -_cam->X, -_cam->Y, -_cam->Z);
  plMatrixMultiply(oMatrix,tempMatrix);
  plMatrixMultiply(oMatrix,_cMatrix);
  plMatrixMultiply(nMatrix,_cMatrix);
  
  x = obj->NumVertices;
  vertex = obj->Vertices;

  do {
    plMatrixApply(oMatrix,vertex->x,vertex->y,vertex->z, 
                  &vertex->xformedx, &vertex->xformedy, &vertex->xformedz); 
    plMatrixApply(nMatrix,vertex->nx,vertex->ny,vertex->nz,
                  &vertex->xformednx,&vertex->xformedny,&vertex->xformednz);
    vertex++;
  } while (--x);

  face = obj->Faces;
  facepos = _numfaces;

  plRender_TriStats[0] += obj->NumFaces; 
  _numfaces += obj->NumFaces;
  x = obj->NumFaces;

  do {
    if (obj->BackfaceCull || face->Material->_st & PL_SHADE_FLAT)
      plMatrixApply(nMatrix,face->nx,face->ny,face->nz,&nx,&ny,&nz);
    if (!obj->BackfaceCull || (plDotProduct(nx,ny,nz, 
        face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,
        face->Vertices[0]->xformedz) < 0.0000001f)) {
      if ((i = plClipNeeded(face))) {
        _faces[facepos].clipneeded = (i!=2);
        if (face->Material->_st & (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE)) {
          tmp = face->sLighting;
          if (face->Material->_st & PL_SHADE_FLAT) {
            for (i = 0; i < _numlights; i ++) {
              tmp2 = 0.0f;
              light = _lights[i];
              if (light->Type & PL_LIGHT_POINT_ANGLE) {
                nx2 = _l[i*3+0] - face->Vertices[0]->xformedx; 
                ny2 = _l[i*3+1] - face->Vertices[0]->xformedy; 
                nz2 = _l[i*3+2] - face->Vertices[0]->xformedz;
                plNormalizeVector(&nx2,&ny2,&nz2);
                tmp2 = plDotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
              } 
              if (light->Type & PL_LIGHT_POINT_DISTANCE) {
                nx2 = _l[i*3+0] - face->Vertices[0]->xformedx; 
                ny2 = _l[i*3+1] - face->Vertices[0]->xformedy; 
                nz2 = _l[i*3+2] - face->Vertices[0]->xformedz;
                if (light->Type & PL_LIGHT_POINT_ANGLE) {
                   nx2 = (1.0f - 0.5f*((nx2*nx2+ny2*ny2+nz2*nz2)/
                           light->HalfDistSquared));
                  tmp2 *= plMax(0,plMin(1.0f,nx2))*light->Intensity;
                } else { 
                  tmp2 = (1.0f - 0.5f*((nx2*nx2+ny2*ny2+nz2*nz2)/
                    light->HalfDistSquared));
                  tmp2 = plMax(0,plMin(1.0f,tmp2))*light->Intensity;
                }
              } 
              if (light->Type == PL_LIGHT_VECTOR) 
                tmp2 = plDotProduct(nx,ny,nz,_l[i*3],_l[i*3+1],_l[i*3+2])
                  * light->Intensity;
              if (tmp2 > 0.0f) tmp += tmp2;
              else if (obj->BackfaceIllumination) tmp -= tmp2;
            } /* End of light loop */ 
          } /* End of flat shading if */
          if (face->Material->_st & PL_SHADE_FLAT_DISTANCE)
            tmp += 1.0f-(face->Vertices[0]->xformedz+face->Vertices[1]->xformedz+
                        face->Vertices[2]->xformedz) /
                       (face->Material->FadeDist*3.0f);
          face->fShade = tmp;
        } else face->fShade = 0.0; /* End of flatmask lighting if */
        if (face->Material->_ft & PL_FILL_ENVIRONMENT) {
          face->eMappingU[0] = 32768 + pl_f2i(face->Vertices[0]->xformednx*32768.0f);
          face->eMappingV[0] = 32768 - pl_f2i(face->Vertices[0]->xformedny*32768.0f);
          face->eMappingU[1] = 32768 + pl_f2i(face->Vertices[1]->xformednx*32768.0f);
          face->eMappingV[1] = 32768 - pl_f2i(face->Vertices[1]->xformedny*32768.0f);
          face->eMappingU[2] = 32768 + pl_f2i(face->Vertices[2]->xformednx*32768.0f);
          face->eMappingV[2] = 32768 - pl_f2i(face->Vertices[2]->xformedny*32768.0f);
        } 
        if (face->Material->_st &(PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE)) {
          register pl_uChar a;
          for (a = 0; a < 3; a ++) {
            tmp = face->vsLighting[a];
            if (face->Material->_st & PL_SHADE_GOURAUD) {
              for (i = 0; i < _numlights ; i++) {
                tmp2 = 0.0;
                light = _lights[i];
                if (light->Type & PL_LIGHT_POINT_ANGLE) {
                  nx = _l[i*3] - face->Vertices[a]->xformedx; 
                  ny = _l[i*3+1] - face->Vertices[a]->xformedy; 
                  nz = _l[i*3+2] - face->Vertices[a]->xformedz;
                  plNormalizeVector(&nx,&ny,&nz);
                  tmp2 = plDotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      nx,ny,nz) * light->Intensity;
                } 
                if (light->Type & PL_LIGHT_POINT_DISTANCE) {
                  nx = _l[i*3+0] - face->Vertices[a]->xformedx; 
                  ny = _l[i*3+1] - face->Vertices[a]->xformedy; 
                  nz = _l[i*3+2] - face->Vertices[a]->xformedz;
                  if (light->Type & PL_LIGHT_POINT_ANGLE) {
                     nx = (1.0f - 0.5f*((nx*nx+ny*ny+nz*nz)/
                           light->HalfDistSquared));
                     tmp2 *= plMax(0,plMin(1.0f,nx))*light->Intensity;
                  } else {
                    tmp2 = (1.0f - 0.5f*((nx2*nx2+ny2*ny2+nz2*nz2)/
                      light->HalfDistSquared));
                    tmp2 = plMax(0,plMin(1.0f,tmp2))*light->Intensity;
                  }
                }
                if (light->Type == PL_LIGHT_VECTOR)
                  tmp2 = plDotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      _l[i*3],_l[i*3+1],_l[i*3+2])
                                        * light->Intensity;
                if (tmp2 > 0.0) tmp += tmp2;
                else if (obj->BackfaceIllumination) tmp -= tmp2;
              } /* End of light loop */
            } /* End of gouraud shading if */
            if (face->Material->_st & PL_SHADE_GOURAUD_DISTANCE)
              tmp += 1.0f-face->Vertices[a]->xformedz/face->Material->FadeDist;
            if (tmp<0)tmp=0;
            if (tmp > 1.0) tmp=1.0f;
            face->Shades[a] = tmp;
          } /* End of vertex loop for */ 
        } /* End of gouraud shading mask if */
        _faces[facepos].zd = face->Vertices[0]->xformedz+
        face->Vertices[1]->xformedz+face->Vertices[2]->xformedz;
        _faces[facepos++].face = face;
        plRender_TriStats[1] ++; 
      } /* Is it in our area Check */
    } /* Backface Check */
    _numfaces = facepos;
    face++;
  } while (--x); /* Face loop */
}

void plRenderObj(pl_Obj *obj) {
  _RenderObj(obj,0,0);
}

void plRenderEnd() {
  _faceInfo *f;
  if (_cam->Sort > 0) _hsort(_faces,_numfaces,0);
  else if (_cam->Sort < 0) _hsort(_faces,_numfaces,1);
  f = _faces;
  while (_numfaces--) {
    if (f->clipneeded) plClipRenderFace(f->face);
    else plClipRenderFaceNC(f->face);
    f++;
  }
  if (_faces) GlobalFree(_faces); 
  _l = 0;
  _faces = 0;
  _lights = 0;
  _numlights = 0;
}

static _faceInfo *Base, tmp;

static void _hsort(_faceInfo *base, int nel, int dir) {
  static int i;
  Base=base-1;
  for (i=nel/2; i>0; i--) _sift_down(i,nel,dir);
  for (i=nel; i>1; ) {
    tmp = base[0]; base[0] = Base[i]; Base[i] = tmp;
    _sift_down(1,i-=1,dir);
  }
}

#define Comp(x,y) (( x ).zd < ( y ).zd ? 1 : 0)

static void _sift_down(int L, int U, int dir) {	
  static int c;
  while (1) { 
    c=L+L;
    if (c>U) break;
    if ( (c < U) && dir^Comp(Base[c+1],Base[c])) c++;
    if (dir^Comp(Base[L],Base[c])) return;
    tmp = Base[L]; Base[L] = Base[c]; Base[c] = tmp;
    L=c;
  }
}
#undef Comp

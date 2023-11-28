/*
  Hybrid Filter Bank header file
*/
#ifndef _HYBRID_H
#define _HYBRID_H
#include "sbr_const.h"
#ifndef NO_QMF_CHANNELS_IN_HYBRID
#define NO_QMF_CHANNELS_IN_HYBRID   3
#endif
#define HYBRID_FILTER_LENGTH  13
#define HYBRID_FILTER_DELAY    6
typedef enum {
  HYBRID_2_REAL = 2,
  HYBRID_4_CPLX = 4,
  HYBRID_8_CPLX = 8
} HYBRID_RES;
typedef struct
{
  int   nQmfBands;
  int   pResolution[NO_QMF_CHANNELS_IN_HYBRID];
  int   qmfBufferMove;
  float pWorkReal[HYBRID_FILTER_LENGTH];
  float pWorkImag[HYBRID_FILTER_LENGTH];
  float mQmfBufferReal[NO_QMF_CHANNELS_IN_HYBRID][HYBRID_FILTER_LENGTH - 1];
  float mQmfBufferImag[NO_QMF_CHANNELS_IN_HYBRID][HYBRID_FILTER_LENGTH - 1];
  float mTempReal[HYBRID_8_CPLX];
  float mTempImag[HYBRID_8_CPLX];
} HYBRID;
typedef HYBRID *HANDLE_HYBRID;
void HybridAnalysis ( const float **mQmfReal,
                 const float **mQmfImag,
                 float *mHybridReal,
                 float *mHybridImag,
                 HANDLE_HYBRID hHybrid );
void HybridSynthesis(const float *mHybridReal, const float *mHybridImag, float *mQmfReal, float *mQmfImag, HANDLE_HYBRID hHybrid);
int CreateHybridFilterBank(HANDLE_HYBRID hHybrid, int noBands, const int *pResolution);
void DeleteHybridFilterBank(HANDLE_HYBRID *phHybrid);
#endif /* _HYBRID_H */

/*
  Sbr decoder
*/
#ifndef __SBR_DEC_H
#define __SBR_DEC_H
#include "sbr_const.h"
#include "lpp_tran.h"
#include "qmf_dec.h"
#include "env_calc.h"
typedef struct
{
  float sbr_OverlapBuffer[2 * MAX_OV_COLS * NO_SYNTHESIS_CHANNELS];
  SBR_QMF_FILTER_BANK     CodecQmfBank;
  SBR_QMF_FILTER_BANK     SynthesisQmfBank;
  SBR_CALCULATE_ENVELOPE  SbrCalculateEnvelope;
  SBR_LPP_TRANS           LppTrans;
  unsigned char qmfLpChannel;
  unsigned char bApplyQmfLp;
}
SBR_DEC;
typedef SBR_DEC *HANDLE_SBR_DEC;
typedef struct
{
  SBR_PREV_FRAME_DATA PrevFrameData;
  SBR_DEC SbrDec;
}
SBR_CHANNEL;
typedef struct
{
  SBRBITSTREAM Bitstream;
  int          FrameOk;
} SBR_CONCEAL_DATA;

typedef SBR_CONCEAL_DATA *HANDLE_SBR_CONCEAL_DATA;
void sbr_dec (HANDLE_SBR_DEC hSbrDec,
              float *timeIn,
              float *timeOut,
              float *interimResult,
              HANDLE_SBR_HEADER_DATA hHeaderData,
              HANDLE_SBR_FRAME_DATA hFrameData,
              HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData,
              int applyProcessing,
              struct PS_DEC *h_ps_d,
              SBR_QMF_FILTER_BANK *hSynthesisQmfBankRight,
              int nChannels);
int
createSbrDec (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA hHeaderData,
              int chan,
              int bApplyQmfLp,
              int sampleFreq);
int
createSbrQMF (SBR_CHANNEL * hSbrChannel,
              HANDLE_SBR_HEADER_DATA hHeaderData,
              int chan,
              int bDownSample);
int
resetSbrQMF (HANDLE_SBR_DEC hSbrDec,
             HANDLE_SBR_HEADER_DATA hHeaderData,
             int sbrChannel,
             int nChannels,
             HANDLE_SBR_PREV_FRAME_DATA hPrevFrameData);
#endif

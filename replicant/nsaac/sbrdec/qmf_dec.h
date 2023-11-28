/*
  Complex qmf analysis/synthesis
*/
#ifndef __QMF_DEC_H
#define __QMF_DEC_H
#include "sbrdecsettings.h"
#include "ps_dec.h"
struct dct4Twiddle
{
  const float *cos_twiddle;
  const float *sin_twiddle;
  const float *alt_sin_twiddle;
};
typedef struct
{
  int no_channels;
  const float *p_filter;
  struct dct4Twiddle *pDct4Twiddle;
#ifndef LP_SBR_ONLY
  const float *cos_twiddle;
  const float *sin_twiddle;
  const float *alt_sin_twiddle;
  const float *t_cos;
  const float *t_sin;
#endif
  float FilterStatesAna[QMF_FILTER_STATE_ANA_SIZE];
  float FilterStatesSyn[QMF_FILTER_STATE_SYN_SIZE];
  int no_col;
  int lsb;
  int usb;
  int qmf_filter_state_size;
}
SBR_QMF_FILTER_BANK;
typedef SBR_QMF_FILTER_BANK *HANDLE_SBR_QMF_FILTER_BANK;
void
cplxAnalysisQmfFiltering (
                          const float *timeIn,
                          float **qmfReal,
                          float **qmfImag,
                          HANDLE_SBR_QMF_FILTER_BANK qmfBank,
                          int   bUseLP);
void
cplxSynthesisQmfFiltering (float **qmfReal,
                           float **qmfImag,
                           float *timeOut,
                           HANDLE_SBR_QMF_FILTER_BANK qmfBank,
                           int   bUseLP,
                           HANDLE_PS_DEC ps_d,
                           int   active
                           );
int createCplxAnalysisQmfBank(HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf, int noCols, int lsb, int usb);
int createCplxSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf, int noCols, int lsb, int usb, int bDownSample);
#endif

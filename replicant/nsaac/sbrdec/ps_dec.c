/*
Parametric stereo decoding
*/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sbr_rom.h"
#include "sbr_ram.h"
#include "ps_dec.h"
#include "math/counters.h"
#include <assert.h>
#include "math/FloatFR.h"
#include "aacdec/aac_ram.h"
static void deCorrelate( HANDLE_PS_DEC h_ps_dec,
	float *rIntBufferLeft, float *iIntBufferLeft,
	float *rIntBufferRight, float *iIntBufferRight);
static void applyRotation( HANDLE_PS_DEC pms,
	float *qmfLeftReal , float *qmfLeftImag,
	float *qmfRightReal, float *qmfRightImag );
/***************************************************************************/
/*
\brief  Creates one instance of the PS_DEC struct
\return Error info
****************************************************************************/
int CreatePsDec(HANDLE_PS_DEC h_ps_dec, unsigned int noSubSamples)
{
	int i;
	int *pErr;
	const int pHybridResolution[] = { HYBRID_8_CPLX,
		HYBRID_2_REAL,
		HYBRID_2_REAL };

	STORE(sizeof(struct PS_DEC));
	memset(h_ps_dec,0,sizeof(struct PS_DEC));
	/* initialisation */

	h_ps_dec->noSubSamples = noSubSamples;
	DIV(1); 
	h_ps_dec->invNoSubSamples = 1.0f / noSubSamples;

	h_ps_dec->bPsDataAvail = 0;
	h_ps_dec->bEnableIid = 0;
	h_ps_dec->bEnableIcc = 0;

	h_ps_dec->bEnableExt = 0;
	h_ps_dec->bFineIidQ  = 0;
	h_ps_dec->freqResIid = 0;
	h_ps_dec->freqResIcc = 0;
	h_ps_dec->lastUsb = 0;

	memset(h_ps_dec->aPeakDecayFast, 0, sizeof(float)*NO_BINS);
	memset(h_ps_dec->aPrevNrg,0,sizeof(float)*NO_BINS);
	memset(h_ps_dec->aPrevPeakDiff,0,sizeof(float)*NO_BINS);

	pErr = (int*)CreateHybridFilterBank(&h_ps_dec->Hybrid, NO_QMF_CHANNELS_IN_HYBRID, pHybridResolution);

	memset(h_ps_dec->mHybridRealLeft,0,sizeof(float)*SUBQMF_GROUPS);
	memset(h_ps_dec->mHybridImagLeft,0,sizeof(float)*SUBQMF_GROUPS);
	memset(h_ps_dec->mHybridRealRight,0,sizeof(float)*SUBQMF_GROUPS);
	memset(h_ps_dec->mHybridImagRight,0,sizeof(float)*SUBQMF_GROUPS);

	h_ps_dec->delayBufIndex   = 0;

	for (i=0 ; i < NO_DELAY_CHANNELS ; i++) 
	{    
		h_ps_dec->aDelayBufIndex[i] = 0;

		if (i<SHORT_DELAY_START)
		{
			h_ps_dec->aNoSampleDelay[i] = LONG_DELAY;
		}
		else
		{
			h_ps_dec->aNoSampleDelay[i] = SHORT_DELAY;

		}
	}

				memset(h_ps_dec->aaRealDelayBufferShort,0,sizeof(h_ps_dec->aaRealDelayBufferShort));
				memset(h_ps_dec->aaImagDelayBufferShort,0,sizeof(h_ps_dec->aaImagDelayBufferShort));

				memset(h_ps_dec->aaRealDelayBufferLong,0,sizeof(h_ps_dec->aaRealDelayBufferLong));
				memset(h_ps_dec->aaImagDelayBufferLong,0,sizeof(h_ps_dec->aaImagDelayBufferLong));

	memset(h_ps_dec->aaRealDelayBufferQmf,0,sizeof(h_ps_dec->aaRealDelayBufferQmf));
	memset(h_ps_dec->aaImagDelayBufferQmf,0,sizeof(h_ps_dec->aaRealDelayBufferQmf));

	memset(h_ps_dec->aaRealDelayBufferSubQmf,0,sizeof(h_ps_dec->aaRealDelayBufferSubQmf));
	memset(h_ps_dec->aaImagDelayBufferSubQmf,0,sizeof(h_ps_dec->aaImagDelayBufferSubQmf));

	memset(h_ps_dec->aaaRealDelayRBufferSerQmf,0,sizeof(h_ps_dec->aaaRealDelayRBufferSerQmf));
	memset(h_ps_dec->aaaImagDelayRBufferSerQmf,0,sizeof(h_ps_dec->aaaImagDelayRBufferSerQmf));
	memset(h_ps_dec->aaaRealDelayRBufferSerSubQmf,0,sizeof(h_ps_dec->aaaRealDelayRBufferSerSubQmf));
	memset(h_ps_dec->aaaImagDelayRBufferSerSubQmf,0,sizeof(h_ps_dec->aaaImagDelayRBufferSerSubQmf));
	for (i=0 ; i < NO_SERIAL_ALLPASS_LINKS ; i++) 
		h_ps_dec->aDelayRBufIndexSer[i] = 0;

	for ( i = 0; i < NO_IID_GROUPS; i++ )
	{

		h_ps_dec->h11Prev[i] = 1.0f;
		h_ps_dec->h12Prev[i] = 1.0f;
	}
	memset( h_ps_dec->h21Prev, 0, sizeof( h_ps_dec->h21Prev ) );
	memset( h_ps_dec->h22Prev, 0, sizeof( h_ps_dec->h22Prev ) );

	return 0;
} /*END CreatePsDec*/
/***************************************************************************/
/*
\brief  Applies IID, ICC,
****************************************************************************/
void ApplyPsSlot(HANDLE_PS_DEC h_ps_dec, float **rIntBufferLeft, float **iIntBufferLeft, float *rIntBufferRight, float *iIntBufferRight)
{
	HybridAnalysis(rIntBufferLeft, iIntBufferLeft, h_ps_dec->mHybridRealLeft, h_ps_dec->mHybridImagLeft, &h_ps_dec->Hybrid);

	deCorrelate(h_ps_dec, *rIntBufferLeft, *iIntBufferLeft, rIntBufferRight, iIntBufferRight);  

	applyRotation(h_ps_dec,	*rIntBufferLeft, *iIntBufferLeft, rIntBufferRight, iIntBufferRight);     

	HybridSynthesis(h_ps_dec->mHybridRealLeft, h_ps_dec->mHybridImagLeft,
		*rIntBufferLeft, *iIntBufferLeft,
		&h_ps_dec->Hybrid);    

	HybridSynthesis(h_ps_dec->mHybridRealRight, h_ps_dec->mHybridImagRight,
		rIntBufferRight, iIntBufferRight,
		&h_ps_dec->Hybrid);    

}/* END ApplyPsSlot */
/***************************************************************************/
/*
\brief  deCorrelate
****************************************************************************/
static void deCorrelate(HANDLE_PS_DEC h_ps_dec, float *rIntBufferLeft, float *iIntBufferLeft, float *rIntBufferRight, float *iIntBufferRight)
{
	int sb, maxsb, gr, sb_delay,  bin;
	int m;
	float iInputLeft;
	float rInputLeft;
	float peakDiff, nrg;
	float aPower[NO_BINS];
	float aTransRatio[NO_BINS];
	int   usb;

	usb = h_ps_dec->usb;   

	for (bin=0; bin < NO_BINS; bin++) 
		aPower[bin] = 0;	

	for (gr=0; gr < SUBQMF_GROUPS; gr++) 
	{
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     

		maxsb = groupBorders[gr]+1;     

		for (sb = groupBorders[gr]; sb < maxsb; sb++) 
			aPower[bin] += h_ps_dec->mHybridRealLeft[sb]*h_ps_dec->mHybridRealLeft[sb] + h_ps_dec->mHybridImagLeft[sb]*h_ps_dec->mHybridImagLeft[sb];  
	} /* gr */

	for (; gr < NO_IID_GROUPS; gr++) 
	{
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     
		maxsb = min(usb, groupBorders[gr+1]);   

		for (sb = groupBorders[gr]; sb < maxsb; sb++) 
			aPower[bin] += rIntBufferLeft[sb]*rIntBufferLeft[sb] + iIntBufferLeft[sb]*iIntBufferLeft[sb];  
	} /* gr */


	for (bin=0; bin < NO_BINS; bin++) 
	{
		h_ps_dec->aPeakDecayFast[bin] *= PEAK_DECAY_FACTOR; 

		if (h_ps_dec->aPeakDecayFast[bin] < aPower[bin]) {
			h_ps_dec->aPeakDecayFast[bin] = aPower[bin];     
		}
		peakDiff = h_ps_dec->aPrevPeakDiff[bin] + (h_ps_dec->aPeakDecayFast[bin] - aPower[bin] - h_ps_dec->aPrevPeakDiff[bin]) * NRG_INT_COEFF;    
		h_ps_dec->aPrevPeakDiff[bin] = peakDiff;     
		nrg = h_ps_dec->aPrevNrg[bin] + (aPower[bin] - h_ps_dec->aPrevNrg[bin])*NRG_INT_COEFF;
		h_ps_dec->aPrevNrg[bin] = nrg;   
		peakDiff *= 1.5f;     

		if (peakDiff <= nrg) 
			aTransRatio[bin] = 1.0f;  
		else 
			aTransRatio[bin] = nrg / peakDiff;     
	} /* bin */


	for (gr=0; gr < SUBQMF_GROUPS; gr++) 
	{
		float decayScaleFactor;
		float rTmp, iTmp, rTmp0, iTmp0, rIn, iIn;

		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     

		sb = groupBorders[gr]; 

		sb_delay = sb; 
		rInputLeft = h_ps_dec->mHybridRealLeft[sb];     
		iInputLeft = h_ps_dec->mHybridImagLeft[sb];     

		decayScaleFactor = 1.0f;

		/* QMF */
		rIn = h_ps_dec->aaRealDelayBufferSubQmf[sb_delay][h_ps_dec->delayBufIndex];  
		iIn = h_ps_dec->aaImagDelayBufferSubQmf[sb_delay][h_ps_dec->delayBufIndex]; 

		rTmp = rIn*aFractDelayPhaseFactorReSubQmf[sb_delay] - iIn*aFractDelayPhaseFactorImSubQmf[sb_delay];

		iTmp = rIn*aFractDelayPhaseFactorImSubQmf[sb_delay] + iIn*aFractDelayPhaseFactorReSubQmf[sb_delay];
		rIn = rTmp; 
		iIn = iTmp; 
		h_ps_dec->aaRealDelayBufferSubQmf[sb_delay][h_ps_dec->delayBufIndex] = rInputLeft;
		h_ps_dec->aaImagDelayBufferSubQmf[sb_delay][h_ps_dec->delayBufIndex] = iInputLeft;

		for (m=0; m<NO_SERIAL_ALLPASS_LINKS ; m++) 
		{
			rTmp0 = h_ps_dec->aaaRealDelayRBufferSerSubQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay];
			iTmp0 = h_ps_dec->aaaImagDelayRBufferSerSubQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay];
			rTmp = rTmp0*aaFractDelayPhaseFactorSerReSubQmf[m][sb_delay] - iTmp0*aaFractDelayPhaseFactorSerImSubQmf[m][sb_delay];
			iTmp = rTmp0*aaFractDelayPhaseFactorSerImSubQmf[m][sb_delay] + iTmp0*aaFractDelayPhaseFactorSerReSubQmf[m][sb_delay];
			rTmp += -decayScaleFactor * aRevLinkDecaySer[m] * rIn;   
			iTmp += -decayScaleFactor * aRevLinkDecaySer[m] * iIn;   
			h_ps_dec->aaaRealDelayRBufferSerSubQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay] = rIn + decayScaleFactor * aRevLinkDecaySer[m] * rTmp;
			h_ps_dec->aaaImagDelayRBufferSerSubQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay] = iIn + decayScaleFactor * aRevLinkDecaySer[m] * iTmp;
			rIn = rTmp;     
			iIn = iTmp;     
		}


		h_ps_dec->mHybridRealRight[sb] = aTransRatio[bin] * rIn;     
		h_ps_dec->mHybridImagRight[sb] = aTransRatio[bin] * iIn;     

	} /* gr */

	for (; gr < NO_IID_GROUPS-2; gr++) 
	{
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     
		maxsb = min(usb, groupBorders[gr+1]);   
		for (sb = groupBorders[gr]; sb < maxsb; sb++) 
		{
			float decayScaleFactor;
			float rTmp, iTmp, rTmp0, iTmp0, rIn, iIn;
			sb_delay = sb - NO_QMF_CHANNELS_IN_HYBRID; 
			rInputLeft = rIntBufferLeft[sb];     
			iInputLeft = iIntBufferLeft[sb];     

				/* QMF */
				if (sb <= DECAY_CUTOFF) 
					decayScaleFactor = 1.0f;
				else 
					decayScaleFactor = ( 1.0f + DECAY_CUTOFF * DECAY_SLOPE ) - DECAY_SLOPE * sb;   

				rIn = h_ps_dec->aaRealDelayBufferQmf[sb_delay][h_ps_dec->delayBufIndex];  
				iIn = h_ps_dec->aaImagDelayBufferQmf[sb_delay][h_ps_dec->delayBufIndex]; 

				rTmp = rIn*aFractDelayPhaseFactorReQmf[sb_delay] - iIn*aFractDelayPhaseFactorImQmf[sb_delay];

				iTmp = rIn*aFractDelayPhaseFactorImQmf[sb_delay] + iIn*aFractDelayPhaseFactorReQmf[sb_delay];
				rIn = rTmp; 
				iIn = iTmp; 
				h_ps_dec->aaRealDelayBufferQmf[sb_delay][h_ps_dec->delayBufIndex] = rInputLeft;
				h_ps_dec->aaImagDelayBufferQmf[sb_delay][h_ps_dec->delayBufIndex] = iInputLeft;

				for (m=0; m<NO_SERIAL_ALLPASS_LINKS ; m++) 
				{
					rTmp0 = h_ps_dec->aaaRealDelayRBufferSerQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay];
					iTmp0 = h_ps_dec->aaaImagDelayRBufferSerQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay];
					rTmp = rTmp0*aaFractDelayPhaseFactorSerReQmf[m][sb_delay] - iTmp0*aaFractDelayPhaseFactorSerImQmf[m][sb_delay];
					iTmp = rTmp0*aaFractDelayPhaseFactorSerImQmf[m][sb_delay] + iTmp0*aaFractDelayPhaseFactorSerReQmf[m][sb_delay];
					rTmp += -decayScaleFactor * aRevLinkDecaySer[m] * rIn;   
					iTmp += -decayScaleFactor * aRevLinkDecaySer[m] * iIn;   
					h_ps_dec->aaaRealDelayRBufferSerQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay] = rIn + decayScaleFactor * aRevLinkDecaySer[m] * rTmp;
					h_ps_dec->aaaImagDelayRBufferSerQmf[m][h_ps_dec->aDelayRBufIndexSer[m]][sb_delay] = iIn + decayScaleFactor * aRevLinkDecaySer[m] * iTmp;
					rIn = rTmp;     
					iIn = iTmp;     
				}
			
			rIntBufferRight[sb] = aTransRatio[bin] * rIn;     
			iIntBufferRight[sb] = aTransRatio[bin] * iIn;     
		} /* sb */
	} /* gr */

	/* long delays */
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     
		maxsb = min(usb, groupBorders[gr+1]);   
		for (sb = groupBorders[gr]; sb < maxsb; sb++) 
		{
			float rIn, iIn;
			sb_delay = sb - NO_ALLPASS_CHANNELS; 
			rInputLeft = rIntBufferLeft[sb];     
			iInputLeft = iIntBufferLeft[sb];     

				rIn = h_ps_dec->aaRealDelayBufferLong[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]];
				iIn = h_ps_dec->aaImagDelayBufferLong[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]];
				h_ps_dec->aaRealDelayBufferLong[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]] = rInputLeft;
				h_ps_dec->aaImagDelayBufferLong[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]] = iInputLeft;

				if (++h_ps_dec->aDelayBufIndex[sb_delay] >= h_ps_dec->aNoSampleDelay[sb_delay]) 
					h_ps_dec->aDelayBufIndex[sb_delay] = 0;    

			rIntBufferRight[sb] = aTransRatio[bin] * rIn;     
			iIntBufferRight[sb] = aTransRatio[bin] * iIn;     
		} /* sb */
	

	gr++;

		/* short delays */
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[gr];     
		maxsb = min(usb, groupBorders[gr+1]);   
		for (sb = groupBorders[gr]; sb < maxsb; sb++) 
		{
			float rIn, iIn;
			sb_delay = sb - NO_ALLPASS_CHANNELS; 
			rInputLeft = rIntBufferLeft[sb];     
			iInputLeft = iIntBufferLeft[sb];     

				rIn = h_ps_dec->aaRealDelayBufferShort[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]];
				iIn = h_ps_dec->aaImagDelayBufferShort[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]];
				h_ps_dec->aaRealDelayBufferShort[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]] = rInputLeft;
				h_ps_dec->aaImagDelayBufferShort[sb_delay][h_ps_dec->aDelayBufIndex[sb_delay]] = iInputLeft;

				if (++h_ps_dec->aDelayBufIndex[sb_delay] >= h_ps_dec->aNoSampleDelay[sb_delay]) 
					h_ps_dec->aDelayBufIndex[sb_delay] = 0;    

			rIntBufferRight[sb] = aTransRatio[bin] * rIn;     
			iIntBufferRight[sb] = aTransRatio[bin] * iIn;     
		} /* sb */

	if (++h_ps_dec->delayBufIndex >= DELAY_ALLPASS) {
		h_ps_dec->delayBufIndex = 0;     
	}

	for (m=0; m<NO_SERIAL_ALLPASS_LINKS ; m++) {

		if (++h_ps_dec->aDelayRBufIndexSer[m] >= aRevLinkDelaySer[m]) {
			h_ps_dec->aDelayRBufIndexSer[m] = 0; 
		}
	}

} /* END deCorrelate */
/***************************************************************************/
/*
\brief  Initialise rotation
****************************************************************************/
void InitRotationEnvelope(HANDLE_PS_DEC pms, int env,	int usb)
{
	int     group;
	int     bin;
	float   scaleL, scaleR;
	float   alpha, beta;
	float   h11, h12, h21, h22;
	float   invEnvLength;
	const float  *pScaleFactors;
	int     noIidSteps;

	if (pms->bFineIidQ)
	{
		noIidSteps = NO_IID_STEPS_FINE;   
		pScaleFactors = scaleFactorsFine; 
	}
	else{
		noIidSteps = NO_IID_STEPS;  
		pScaleFactors = scaleFactors;     
	}

	if (env == 0)
	{
		pms->lastUsb = pms->usb;    
		pms->usb = usb; 

		if (usb > pms->lastUsb && pms->lastUsb!=0)
		{
			int sb, i, k, kmax;
			kmax = 2;     

			for (sb = pms->lastUsb-NO_QMF_CHANNELS_IN_HYBRID; sb < usb-NO_QMF_CHANNELS_IN_HYBRID; sb++)
			{
				if (sb<NO_QMF_ALLPASS_CHANNELS) 
				{
					for (i=0 ; i<NO_SERIAL_ALLPASS_LINKS ; i++) 
					{
						for (k=0 ; k < aRevLinkDelaySer[i]; k++) 
						{
							pms->aaaRealDelayRBufferSerQmf[i][k][sb] = 0;   
							pms->aaaImagDelayRBufferSerQmf[i][k][sb] = 0;   
						}
					}
					/* QMF */
					for (k=0 ; k < kmax; k++) 
					{
						pms->aaRealDelayBufferQmf[sb][k] = 0;   
						pms->aaImagDelayBufferQmf[sb][k] = 0;   
					}
				}
				else 
				{					
					/* Long or Short Delay */
					kmax = pms->aNoSampleDelay[sb-NO_QMF_ALLPASS_CHANNELS];
					if (kmax==SHORT_DELAY)
					{
						pms->aaRealDelayBufferShort[sb-NO_QMF_ALLPASS_CHANNELS][0] = 0;   
						pms->aaImagDelayBufferShort[sb-NO_QMF_ALLPASS_CHANNELS][0] = 0;   
					}
					else
					{
						for (k=0 ; k < kmax; k++) 
						{
							pms->aaRealDelayBufferLong[sb-NO_QMF_ALLPASS_CHANNELS][k] = 0;   
							pms->aaImagDelayBufferLong[sb-NO_QMF_ALLPASS_CHANNELS][k] = 0;   
						}
					}
				}
			}
		}
	}

	invEnvLength = ( float )( pms->aEnvStartStop[env + 1] - pms->aEnvStartStop[env] );

	if (invEnvLength == pms->noSubSamples){
		invEnvLength = pms->invNoSubSamples;    
	}
	else{
		invEnvLength = 1.0f/invEnvLength; 
	}

	for ( group = 0; group < NO_IID_GROUPS; group++ )
	{
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[group];  
		scaleR = pScaleFactors[noIidSteps + pms->aaIidIndex[env][bin]];   
		scaleL = pScaleFactors[noIidSteps - pms->aaIidIndex[env][bin]];   
		alpha  = alphas[pms->aaIccIndex[env][bin]];    
		beta   = alpha * ( scaleR - scaleL ) / PSC_SQRT2F;   
		h11 = ( float )( scaleL * cos( beta + alpha ) );
		h12 = ( float )( scaleR * cos( beta - alpha ) );
		h21 = ( float )( scaleL * sin( beta + alpha ) );     
		h22 = ( float )( scaleR * sin( beta - alpha ) );     
		pms->deltaH11[group]  = ( h11 - pms->h11Prev[group] ) * invEnvLength;    
		pms->deltaH12[group]  = ( h12 - pms->h12Prev[group] ) * invEnvLength;    
		pms->deltaH21[group]  = ( h21 - pms->h21Prev[group] ) * invEnvLength;    
		pms->deltaH22[group]  = ( h22 - pms->h22Prev[group] ) * invEnvLength;    
		pms->H11[group]  = pms->h11Prev[group];  
		pms->H12[group]  = pms->h12Prev[group];  
		pms->H21[group]  = pms->h21Prev[group];  
		pms->H22[group]  = pms->h22Prev[group];  
		pms->h11Prev[group] = h11;  
		pms->h12Prev[group] = h12;  
		pms->h21Prev[group] = h21;  
		pms->h22Prev[group] = h22;  
	} /* groups loop */

} /* END InitRotationEnvelope */
/***************************************************************************/
/*
\brief  Rotation
****************************************************************************/
static void	applyRotation(HANDLE_PS_DEC  pms,	float*qmfLeftReal,float*qmfLeftImag, float*qmfRightReal, float*qmfRightImag)
{
	int     group;
	int     bin;
	int     subband, maxSubband;
	float  *hybrLeftReal;
	float  *hybrLeftImag;
	float  *hybrRightReal;
	float  *hybrRightImag;
	float   tempLeftReal, tempLeftImag;
	float   tempRightReal, tempRightImag;
	int     usb;

	usb = pms->usb;   
	hybrLeftReal  = pms->mHybridRealLeft;     
	hybrLeftImag  = pms->mHybridImagLeft;     
	hybrRightReal = pms->mHybridRealRight;    
	hybrRightImag = pms->mHybridImagRight;    

	for ( group = 0; group < NO_IID_GROUPS; group++ )
	{
		bin = ( ~NEGATE_IPD_MASK ) & bins2groupMap[group];  

		if ( group == SUBQMF_GROUPS )
		{
			hybrLeftReal  = qmfLeftReal;    
			hybrLeftImag  = qmfLeftImag;    
			hybrRightReal = qmfRightReal;   
			hybrRightImag = qmfRightImag;   
		}

		if ( group < SUBQMF_GROUPS )
		{
			maxSubband = groupBorders[group] + 1;     
		}
		else
		{
			maxSubband = min(usb, groupBorders[group + 1]);   
		}
		pms->H11[group]    += pms->deltaH11[group];    
		pms->H12[group]    += pms->deltaH12[group];    
		pms->H21[group]    += pms->deltaH21[group];    
		pms->H22[group]    += pms->deltaH22[group];    

		for ( subband = groupBorders[group]; subband < maxSubband; subband++ )
		{

			tempLeftReal  = pms->H11[group] * hybrLeftReal[subband] + pms->H21[group] * hybrRightReal[subband];

			tempLeftImag  = pms->H11[group] * hybrLeftImag[subband] + pms->H21[group] * hybrRightImag[subband];

			tempRightReal = pms->H12[group] * hybrLeftReal[subband] + pms->H22[group] * hybrRightReal[subband];

			tempRightImag = pms->H12[group] * hybrLeftImag[subband] + pms->H22[group] * hybrRightImag[subband];
			hybrLeftReal [subband] = tempLeftReal;
			hybrLeftImag [subband] = tempLeftImag;
			hybrRightReal[subband] = tempRightReal;     
			hybrRightImag[subband] = tempRightImag;     
		} /* subband loop */
	} /* groups loop */

} /* END applyRotation */

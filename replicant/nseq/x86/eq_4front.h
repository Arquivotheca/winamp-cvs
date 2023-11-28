#pragma once

#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define EQ10_NOFBANDS 10     /* want more bands? not a problem */


typedef struct eq10band_s
{
    double gain;    /* gain of current band. Do not use this value, 
                      use eq10_setgain instead */

    double ua0,ub1,ub2;  /* internal - do not use */
    double da0,db1,db2;  /* internal - do not use */
    double x1,x2,y1,y2;  /* internal - do not use */

} eq10band_t;

typedef struct eq10_s
{
    double rate;         /* sample rate; do not modify */
                        /* use eq10_setup to change */

    double detect;       /* global detector value. do not use */
    double detectdecay;  /* internal - do not use */

		eq10band_t band[EQ10_NOFBANDS]; /* bands of equalizer */
} eq10_t;

double eq10_db2gain(double gain_dB); /* converts decibels to internal gain value*/
double eq10_gain2db(double gain);    /* converts internal gain value to decibels*/

/* prepare eq array for processing, 

   eq   - pointer to array,
   eqs  - number of elements in array (number of audio channels)
   rate - sample rate
	 frequences - array[10] of frequencies corresponding to the 10 bands

   WARNING! this function resets all data in eq and sets all gains to 0dB
*/
void eq10_setup(eq10_t *eq, unsigned int channels, double sample_rate, const double *frequencies);

/* set band gain */
/*
   eq     - pointer to array,
   eqs    - number of elements in array (number of audio channels)
   bandnr - # of band (0...EQ_NOFBANDS-1)
*/
void eq10_setgain(eq10_t *eq, unsigned int channels, unsigned int band_number, double gain_dB);


/* get current band gain */
/* eq - pointer to element, possible to read gain on each channel
        separately */
double eq10_getgain(eq10_t *eq, unsigned int band_number);


/* process function

   eq     - pointer to eq structure, corresponding to wanted channel
   buf    - input buffer (interleaved multichannel)
   outbuf - output buffer
   sz     - number of samples in input buffer 
   index  - index of processed channel (0...N-1)
   step   - total number of channels in interleaved stream (N)
	 limit  - pass 1 to enable the limiter and 0 otherwise

*/

void eq10_processf(eq10_t *eq, const float *buf, float *outbuf, size_t sz, size_t index, size_t step, int limit);

#ifdef __cplusplus
}
#endif

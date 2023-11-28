#include "eq_4front.h"
#include <math.h>
#include <string.h>

#define DENORMAL_FIX // comment this for no denormal fixes

/* Dynamic limiter, which prevents EQ from distortion. In no case you 
   can overflow EQ and cause it to clip */

#define EQ10_TRIM_CODE    0.930f  /* trim at -0.6dB */
#define EQ10_TRIM_RELEASE 0.700f  /* trim release, in seconds */
#define EQ10_Q        1.41  /* global `Q' factor */

static double eq10_freq[EQ10_NOFBANDS]={ 70, 180, 320, 600, 1000, 3000, 6000, 12000, 14000, 16000 }; // winamp style frequency table;
static double eq10_freq_iso[EQ10_NOFBANDS]={31,62,125,250,500,1000,2000,4000,8000,16000}; // ISO frequency table

#ifdef EQ10_DQ
static double eq10_q[EQ10_NOFBANDS]=EQ10_DQ;
#endif


static void eq10_bsetup2(int u,double rate,eq10band_t *band,double freq,double Q)
{
	double angle;
	double a0,/*a1,a2,*/b0,b1,b2,alpha;

	if (rate<4000.0)     rate=4000.0;
	if (rate>384000.0)   rate=384000.0;
	if (freq<20.0)       freq=20.0;
	if (freq>=(rate*0.499)) {band->ua0=band->da0=0;return;}

	angle = 2.0*3.1415926535897932384626433832795*freq/rate;
	alpha = sin(angle)/(2.0*Q);

	b0 = 1.0/(1.0+alpha); 
	a0 = b0*alpha;  
	b1 = b0*2*cos(angle); 
	b2 = b0*(alpha-1);

	if (u>0) 
	{
		band->ua0=a0;
		band->ub1=b1;
		band->ub2=b2;
	}
	else
	{
		band->da0=a0;
		band->db1=b1;
		band->db2=b2;
	}
}

static void eq10_bsetup(double rate, eq10band_t *band, double freq, double Q)
{
	memset(band,0,sizeof(*band));
	eq10_bsetup2(-1,rate,band,freq,Q*0.5);
	eq10_bsetup2(1,rate,band,freq,Q*2.0);

}

void eq10_setup(eq10_t *eq,  double rate, const double *frequencies)
{
	unsigned int t,k;

			for(t=0;t<EQ10_NOFBANDS;t++)
		{
#ifndef EQ10_DQ
			eq10_bsetup(rate, &eq->band[t], frequencies[t], EQ10_Q);
#else
			eq10_bsetup(rate, &eq->band[t], frequencies[t], eq10_q[t]);
#endif
		}

		eq->detect=0;
		/* release of trimmer */
		eq->detectdecay=pow(0.001,1.0/(rate*EQ10_TRIM_RELEASE)); 
	
}

void eq10_processf(eq10_t *eq, const float *buf, float *outbuf, size_t sz, size_t step, int limit)
{
	unsigned int t,k;
	const float *in;
	float *out;

	if (!eq) 
		return;

	in=buf;

	for(k=0;k<EQ10_NOFBANDS;k++)
	{
		float a0,b1,b2;
		float x1 = eq->band[k].x1;
		float x2 = eq->band[k].x2;
		float y0;
		float y1 = eq->band[k].y1;
		float y2 = eq->band[k].y2;
		float gain = eq->band[k].gain;

		out = outbuf;

		if (gain>0.0f)
		{
			a0 = eq->band[k].ua0*gain;
			b1 = eq->band[k].ub1;
			b2 = eq->band[k].ub2;
		}
		else
		{
			a0 = eq->band[k].da0*gain;
			b1 = eq->band[k].db1;
			b2 = eq->band[k].db2;
		}

		if (a0==0.0f) continue;

		for(t=0;t<sz;t++,in+=step,out+=step)
		{
			/* 2-pole IIR */
			y0 = (in[0]-x2)*a0 + y1*b1 + y2*b2 + 1e-30f; /* last is for dernomal */

			/* store history */
			x2=x1; 
			x1=in[0]; 
			y2=y1; 
			y1=y0;

			out[0] = (float)(y0 + in[0]);
		}

		in=outbuf;

		eq->band[k].x1=x1;
		eq->band[k].x2=x2;
		eq->band[k].y1=y1;
		eq->band[k].y2=y2;

	}

	if (limit)
	{
		float detect=eq->detect;
		float  detectdecay=eq->detectdecay;
		out=outbuf;
		for(t=0;t<sz;t++,in+=step,out+=step)
		{
			/* *0.99 - reserve */
			if (fabs(in[0])>detect) detect=fabs(in[0]);


			if (detect>EQ10_TRIM_CODE) 
				out[0]=in[0]*(float)(EQ10_TRIM_CODE/detect);
			else
				out[0]=in[0];

			detect*=detectdecay;
			detect+=1e-30f;
		}
		eq->detect=detect;
	}
	else if ((in==buf)&&(buf!=outbuf))
	{ 
		out=outbuf;
		for(t=0;t<sz;t++,in+=step,out+=step) 
			out[0]=in[0];
	}
}

double eq10_db2gain(double gain_dB)
{
	return pow(10.0,gain_dB/20.0)-1.0;
}

double eq10_gain2db(double gain)
{
	return 20.0*log10(gain+1.0);
}

void eq10_setgain(eq10_t *eq, unsigned int band_number, double gain_dB)
{
	double realgain;
	unsigned int k;

	if (!eq)
		return;

	realgain=eq10_db2gain(gain_dB);
	eq->band[band_number].gain=realgain;
}

double eq10_getgain(eq10_t *eq, unsigned int band_number)
{
	return eq10_gain2db(eq->band[band_number].gain);
}


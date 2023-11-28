/***************************************************************************\
*                     (c) 1996 - 2006 Fraunhofer IIS
*              (c) 2004 Fraunhofer IIS and Agere Systems Inc.
*                          All Rights Reserved.
*
*    This software and/or program is protected by copyright law and
*    international treaties. Any reproduction or distribution of this
*    software and/or program, or any portion of it, may result in severe
*    civil and criminal penalties, and will be prosecuted to the maximum
*    extent possible under law.
*
***************************************************************************/


/* operation mode */

#define COMMAND_LINE_EXE

/* default values for synthesis */

#define ITDCUTOFF       1500. /* frequency below which ICTD processing is carried out */

/* audio analysis modes
   (defines the data types in the bitstream) */

#define ILD_ANAL	1  /* ild analysis enabled */
#define ITD_ANAL	2  /* itd analysis enabled */
#define COR_ANAL	4  /* correlation analysis enabled */
#define SEP_ANAL	8  /* separate source analysis enabled */

#include <time.h>

typedef struct BCC_dvars
{
    BCC_dparams*    bcc_dparams;
    BCC_dstate*     bcc_dstate;
    Streamin_state* strm;
    int             bitstreamformat;    /* bitstream format */
    char	    decodingfinished;
    FILE	    *audioin;
    FILE	    *bitsin;
    FILE	    *audioout;
    short	    *buf;
    int             bufptr;
#ifdef WIN32
	unsigned int time0;
#else
 	time_t	    time0;
#endif
    char            diagnostics;
    void            (*getsamples)(void*);
    char            active[MAXSOURCES];

    /* parameters and data for generating reference audio */

    char         ref;
    char         nreffiles;
    Mixer_data*  mixer_data;
    FILE*        srcinput[MAXSOURCES];
    FILE*        refaudio;
    short*       mixerin;
    short*       mixerout;
} BCC_dvars;

void bcc_dinitialize(int nargv, char *argv[]);
void bcc_dcycle(void*);
void bcc_dfinish(void);



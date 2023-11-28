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


#define NBUFFRAMES    80
#define BUFOFFSET     NBUFFRAMES/2

typedef struct Mixer_data
{
    /* parameters */

    int     npbchan;                     /* number of channels */
    int     refchan;                     /* reference channel for the spatial cues */
    int     ncues;                       /* number of inter-channel cues */
    int     nsepsrc;                     /* > 0: number of sources, 0: complex audio signal */
    int     sfreq;                       /* sample-frequency of input */
    int     framesize;                   /* number of input samples available at once */
    float   monoscale;                   /* factor of scaling before rendering */
    float   levs[MAXSOURCES][MAXPBCHAN]; /* scalers for each channel and source */
    float   dels[MAXSOURCES][MAXPBCHAN]; /* delays for each channel and source */

    /* data */

    float   *buf;
} Mixer_data;


/****************************************************************/

/* function headers */

void mixer_parameterupdate(char, BCC_dparams*, Mixer_data*);
void mixer_init(int, int, int, int, int, BCC_dparams*, Mixer_data*);
void mixer_done(Mixer_data*);
void mixer_processframe(Mixer_data*, short*, void*, char, char);


/****************************************************************/

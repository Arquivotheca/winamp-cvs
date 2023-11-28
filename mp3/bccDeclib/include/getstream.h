/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1996 - 2008 Fraunhofer IIS
 *          (c) 2004 Fraunhofer IIS and Agere Systems Inc.
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#ifndef __GETSTREAM_H__
#define __GETSTREAM_H__



/* must be the same as in bcc_enc_api.h ! */


#define MAXPBCHAN    8	          /* maximum number of playback channels */
#define MAXCUES      MAXPBCHAN-1  /* maximum number of playback channels */

/* internal buffer size */

#define              INBUFSIZE 1024

/* include headers */

#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* structure for storing params and state of bitstream */

typedef struct Streamin_state
{
  FILE*         inputfile;        /* input file (NULL=no input file) */
  unsigned char inbuf[INBUFSIZE]; /* internally used bit buffer */
  int	        nbits;            /* bitstream state variables */
  long	        cword;
  long          totalbits;
  int           framebits;
  int           dataptr;
  int           dataend;
  int           streamend;
  char          fileend;
  char          usedtoomanybits;
  /* Huffman coder training */
  int            hufftrain;
  /* Huffman coder structures */
  int            ncues;
  int            corlevels;
  int            cuelevels;
  int            nunodcues;
  int*           node1cues;
  int*           node2cues;
  int            nunodcor;
  int*           node1cor;
  int*           node2cor;
  int*           prevqcues[MAXCUES];
  int*           prevqcor;
  /* (for saving memory, most of the above fields can be set to char*) */
} Streamin_state;

/* function headers */

void initinstream(Streamin_state*, FILE*);
int  initdecoder(Streamin_state*, int, int, int, int, int);
void instreamdone(Streamin_state* strm);
int getheader(Streamin_state*, int*, float*, float*, int*, int*, int*, int*, int*,
       int*, int*, int*, int*, int*, int*, int*, int*, int*, int*, char*);
int get_src(Streamin_state*, int, int, int, /*int, */char*);
void copy_cues( int*, int, int, char* );
int get_cues(Streamin_state*, int, int, int, int, char*, int);
unsigned long getbits(Streamin_state*, int);
int  framedone(Streamin_state*);
int  endofstream(Streamin_state*);
long totalbitsread(Streamin_state*);
void copyinbuf(Streamin_state*, unsigned char*, int);

/* internally used only */

int  bytealign(Streamin_state*);
int  getbyte(Streamin_state*);

#endif

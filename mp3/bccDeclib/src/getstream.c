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

#include "getstream.h"
#include "bcc_utils.h"


/********************* Huffman codebooks filename *************************/

/* Huffman codebooks */

char* codebooksdec = "huffdec.dat";


/********************** main bitstream functions ***************************/

void initinstream(
       Streamin_state* strm,
       FILE* stream
)
{
  strm->cword           = 0;
  strm->nbits           = 0;
  strm->totalbits       = 0;
  strm->framebits       = 0;
  strm->dataptr         = 0;
  strm->dataend         = 0;
  strm->streamend       = -1;
  strm->fileend         = 0;
  strm->usedtoomanybits = 0;
  strm->inputfile       = stream;
}

int initdecoder(
       Streamin_state* strm,
       int npart,
       int ncues,
       int cuelevels,
       int corlevels,
       int hufftrain
)
{
  char  failed;
  int   n, i, nread, nquant;
  FILE* file;

  /* Huffman coder  init */

  strm->hufftrain = hufftrain;
  strm->ncues     = ncues;
  strm->corlevels = corlevels;
  strm->cuelevels = cuelevels;
  strm->prevqcor  = NULL;
  strm->node1cues = NULL;
  strm->node2cues = NULL;
  strm->node1cor  = NULL;
  strm->node2cor  = NULL;
  for (n = 0; n < ncues; n++)
    strm->prevqcues[n] = NULL;

  if (strm->hufftrain == 0) {

    /* allocate memeory */

    failed = 0;

    strm->node1cues = (int*) malloc((2*cuelevels-1)*sizeof(int));
    if (strm->node1cues == NULL) failed = 1;

    strm->node2cues = (int*) malloc((2*cuelevels-1)*sizeof(int));
    if (strm->node2cues == NULL) failed = 1;

    strm->node1cor = (int*) malloc((2*corlevels-1)*sizeof(int));
    if (strm->node1cor == NULL) failed = 1;

    strm->node2cor = (int*) malloc((2*corlevels-1)*sizeof(int));
    if (strm->node2cor == NULL) failed = 1;

    strm->prevqcor = (int*) malloc(npart*sizeof(int));
    if (strm->prevqcor == NULL) failed = 1;

    for (n = 0; n < ncues; n++) {
      strm->prevqcues[n] = (int*) malloc(npart*sizeof(int));
      if (strm->prevqcues[n] == NULL) failed = 1;
    }

    if (failed == 1) {
#ifndef NDEBUG
      printf("func initstream: could not allocate memory.\n");
#endif
      return(0);
    }

    /* init */

    for (i = 0; i < npart; i++)
      strm->prevqcor[i] = corlevels-1;

    for (n = 0; n < ncues; n++) {
      for (i = 0; i < npart; i++)
        *(strm->prevqcues[n]+i) = (cuelevels-1)/2;
    }

    /* read Huffman codebooks */

    file = fopen(codebooksdec,"rb");
    if (file == NULL) {
#ifndef NDEBUG
      printf("\nCould not open Huffman tree file!! (1)\n\n");
#endif
      return(0);
    }

    nquant = 2*cuelevels-1;
    nread = fread((char*)&strm->nunodcues, sizeof(int), 1, file);
    if (nread != 1) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (1)\n\n");
#endif
      return(0);
    }
    swapint_(&strm->nunodcues, 1);
    nread = fread((char*)strm->node1cues, sizeof(int), nquant, file);
    if (nread != nquant) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (2)\n\n");
#endif
      return(0);
    }
    swapint_(strm->node1cues, nquant);
    nread = fread((char*)strm->node2cues, sizeof(int), nquant, file);
    if (nread != nquant) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (3)\n\n");
#endif
      return(0);
    }
    swapint_(strm->node2cues, nquant);

    nquant = 2*corlevels-1;
    nread = fread((char*)&strm->nunodcor, sizeof(int), 1, file);
    if (nread != 1) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (3)\n\n");
#endif
      return(0);
    }
    swapint_(&strm->nunodcor, 1);
    nread = fread((char*)strm->node1cor, sizeof(int), nquant, file);
    if (nread != nquant) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (4)\n\n");
#endif
      return(0);
    }
    swapint_(strm->node1cor, nquant);
    nread = fread((char*)strm->node2cor, sizeof(int), nquant, file);
    if (nread != nquant) {
#ifndef NDEBUG
      printf("\nError reading Huffman tree file!! (5)\n\n");
#endif
      return(0);
    }
    swapint_(strm->node2cor, nquant);

    fclose(file);

    nquant = 2*strm->cuelevels-1;
#ifndef NDEBUG
    printf("Tree nodes 1 (%d nodes):\n", strm->nunodcues);
    for (n = 0; n < nquant; n++)
      printf("  %d", strm->node1cues[n]);
    printf("\n");
    printf("Tree nodes 2 (%d nodes):\n", strm->nunodcues);
    for (n = 0; n < nquant; n++)
      printf("  %d", strm->node2cues[n]);
    printf("\n\n");
#endif

    nquant = 2*strm->corlevels-1;
#ifndef NDEBUG
    printf("Tree nodes 1 (%d nodes):\n", strm->nunodcor);
    for (n = 0; n < nquant; n++)
      printf("  %d", strm->node1cor[n]);
    printf("\n");
    printf("Tree nodes 2 (%d nodes):\n", strm->nunodcor);
    for (n = 0; n < nquant; n++)
      printf("  %d", strm->node2cor[n]);
    printf("\n\n");
#endif
  }
  return(1);
}

void instreamdone(Streamin_state* strm)
{
  int n;

  /* free memory */

  freeifnotNULL_(strm->prevqcor);
  freeifnotNULL_(strm->node1cues);
  freeifnotNULL_(strm->node2cues);
  freeifnotNULL_(strm->node1cor);
  freeifnotNULL_(strm->node2cor);
  for (n = 0; n < strm->ncues; n++)
    freeifnotNULL_(strm->prevqcues[n]);
}

int getheader(
       Streamin_state *strm,
       int            *mode,
       float          *levrange,
       float          *delrange,
       int            *cuelevels,
       int            *cuebits,
       int            *corlevels,
       int            *corbits,
       int            *npbchan,
       int            *nsepsrc,
       int            *refchan,
       int            *lfchan,
       int            *sfreq,
       int            *framesize,
       int            *fftsize,
       int            *winspan,
       int            *winshift,
       int            *wintype,
       int            *pwidth,
       char           *hybrid
)
{
    int  n;
    char version[6];

    /* bitstream version */

    for (n = 0; n < 6; n++)
      version[n] = (char)getbits(strm, 8);

    if (version[0] != 'B' ||
        version[1] != 'C' ||
        version[2] != 'C' ||
        version[3] != 'v' ||
        version[4] != '0' ||
        version[5] != '1') {
#ifndef NDEBUG
        printf("\nBitstream has wrong format!!\n\n");
#endif
        return(0);
    }

    /* parameters */

    *mode      = getbits(strm, 8);
    *levrange  = (float)getbits(strm, 6);
    *delrange  = (float)getbits(strm, 7);
    *cuelevels = getbits(strm, 5);
    *cuebits   = getbits(strm, 3);
    *corlevels = getbits(strm, 5);
    *corbits   = getbits(strm, 3);
    *npbchan   = getbits(strm, 4);
    *nsepsrc   = getbits(strm, 7);
    *refchan   = getbits(strm, 4);
    *lfchan    = getbits(strm, 4);
    if (*lfchan == 15) *lfchan = -1;
    *sfreq     = getbits(strm, 16);
    *framesize = getbits(strm, 14);
    *fftsize   = (int) pow(2, getbits(strm, 4));
    *winspan   = getbits(strm, 11);
    *winshift  = getbits(strm, 10);
    *wintype   = getbits(strm, 10);
    *pwidth    = getbits(strm, 6);
    *hybrid    = (char)getbits(strm, 2);
  return(1);
}

int get_src(
       Streamin_state *strm,
       int            nsepsrc,
       int            npart,
       int            runlengthcodebits,
       /*int            runlengthnumvalues, */
       char           *src
)
{
  int prev, runlength, k, this, i;

  i = 0;
  while (i < npart) {
    this = getbits(strm, runlengthcodebits);
    if (this < nsepsrc) {
      src[i] = this;
      prev   = this;
      i++;
    } else {
      runlength = this - nsepsrc + 2;
      for (k = 0; k < runlength; k++) {
        src[i] = prev;
        i++;
      }
    }
  }
  if (i != npart) {
#ifndef NDEBUG
    printf("Corrupt bitstream (function get_src)!!! i = %d npart = %d\n", i, npart);
#endif
    return(0);
  }
  return(1);
}



void copy_cues(
       int*   qvalues,
       int    npart,
       int    nhpart,
       char*  cues
)
{
  int k;

  for (k = nhpart; k < npart; k++) {
    cues[k] = (char)qvalues[k];
  }

}


int get_cues(
       Streamin_state *strm,
       int            npart,
       int            codebits,
       int            numvalues,
       int            nhpart,
       char           *cues,
       int            index
)
{
  int   k, symb, offset, diffkind, value, firstvalue, nunod;
  int   *node1, *node2, *prevqvalues;

  if (strm->hufftrain == 0) {

    /* select first value for frequency differential coding
       and Huffman codebook */

    if (index != -1) {
      firstvalue  = (numvalues-1)/2;
      nunod       = strm->nunodcues;
      node1       = strm->node1cues;
      node2       = strm->node2cues;
      prevqvalues = strm->prevqcues[index];

      /* hybrid mode: set cues of lower partitions
         to zero */

      for (k = 0; k < nhpart; k++)
        cues[k] = (numvalues-1)/2;
    } else {
      firstvalue  = numvalues-1;
      nunod       = strm->nunodcor;
      node1       = strm->node1cor;
      node2       = strm->node2cor;
      prevqvalues = strm->prevqcor;
      /* hybrid mode: set cues of lower partitions
         to zero */

      for (k = 0; k < nhpart; k++)
        cues[k] = numvalues-1;
    }

    /* get flag indicating if differential coding is applied
       over time or frequency */

    diffkind = getbits(strm, 1);

    /* Huffman decode */

    offset = -numvalues + 1;
    while (k < npart) {
      symb = 0;
      while (symb >= 0 && symb < nunod) {
        if (getbits(strm, 1) == 0) symb = node1[symb];
        else                       symb = node2[symb];
      }

      if(symb >= nunod || symb > 0) {
#ifndef NDEBUG
        printf("Huffman decoding error! (symb = %d, nunod = %d)\n", symb, nunod);
#endif
        return(0);
      }

      value = -symb - 1;

      /* differential decoding */

      if (diffkind == 0) {
        cues[k] = prevqvalues[k] + value + offset;
        prevqvalues[k] = cues[k];
      } else {
        if (k > nhpart)
          cues[k] = cues[k-1] + value + offset;
        else
          cues[k] = firstvalue + value + offset;
        prevqvalues[k] = cues[k];
      }

      k++;
    }
  } else {

    int   k;

    for (k = 0; k < nhpart; k++)
      cues[k] = 0;

    for (k = nhpart; k < npart; k++) {

      /* get quantizer index from bitstream */

      cues[k] = (char)getbits(strm, codebits);
    }
  }
  return(1);
}

unsigned long getbits(Streamin_state *strm, int n)
{
  int data;

  strm->totalbits += n;
  strm->framebits += n;

  while (strm->nbits < n && strm->fileend == 0) {
    data = getbyte(strm);

    if(data < 0)
      break;    /* error reading byte */

    if (!strm->fileend) {
        strm->cword = (strm->cword<<8) | data;
                    strm->nbits += 8;
     }

  }

  if (strm->nbits - n < 0) {
    return 0xffffffff;        /* signaling error */
  }

  strm->nbits -= n;
  return (strm->cword >> strm->nbits) & ((1L<<n)-1);
}

int framedone(Streamin_state *strm)
{
    int n;

    if(!bytealign(strm))
      return(-1);
    n = strm->framebits;
    strm->framebits = 0;
    return n;
}

int endofstream(Streamin_state *strm)
{
    /* error: used more bits than were in the file */

    if (strm->usedtoomanybits) return -1;

    /* 1: end of stream, 0: not end of stream */

    return strm->fileend;
}

long totalbitsread(Streamin_state *strm)
{
    return strm->totalbits;
}

void copyinbuf(Streamin_state *strm, unsigned char* buf, int N)
{
    int i, offset;

    /* move remaining bytes to beginning */

    offset = strm->dataend-strm->dataptr;
    for (i = 0; i < offset; i++)
      strm->inbuf[i] = strm->inbuf[i+strm->dataptr];

    /* copy new bytes into buffer */

    for (i = 0; i < N; i++)
      strm->inbuf[i+offset] = buf[i];

    /* init variables */

    strm->dataend = N+offset;
    strm->dataptr = 0;
}


/******************** bitstream utility functions **************************/

int bytealign(Streamin_state *strm)
{
    int n;

    n = strm->framebits % 8;
    n = 8 - n;

    if (n == 8) n = 0;

    if (n > 0)
      n = getbits(strm, n);

    if (n != 0) {
#ifndef NDEBUG
      printf("Byte alignment bits were not zero!!\n");
#endif
      return(0);
    }
  return(1);
}

int getbyte(Streamin_state *strm)
{
  int nread;

  if (strm->dataptr == strm->streamend) {
    strm->fileend = 1;
    return 0;
  }

  if (strm->dataptr == strm->dataend && strm->inputfile != NULL) {
      nread = fread(strm->inbuf, sizeof(char), INBUFSIZE, strm->inputfile);
      if (nread < 0) {
#ifndef NDEBUG
        printf("\nCould not read from bitstream file!!\n\n");
#endif
        return(-1);
      }
      if (nread < INBUFSIZE) {
        strm->streamend = nread;
      }
        strm->dataend = nread;
        strm->dataptr = 0;
    }

    if (strm->dataptr == strm->dataend && strm->inputfile == NULL) {
#ifndef NDEBUG
      printf("\nNot enough bytes provided for stream!!\n\n");
#endif
      return(-1);
    }

    strm->dataptr++;
    return strm->inbuf[strm->dataptr-1];
}

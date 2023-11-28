/* store.c, picture output routines                                         */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "global.h"
#include "decoder.h"

/* private prototypes */
static void store_one (MPEGDecoder *decoder, char *outname, unsigned char *src[],  int offset, int incr, int height);
static void store_yuv (MPEGDecoder *decoder, char *outname, unsigned char *src[],  int offset, int incr, int height);
static void store_yuv1 (char *name, unsigned char *src,  int offset, int incr, int width, int height);
static void putbyte (int c);
static void putword (int w);

#define OBFRSIZE 4096
static unsigned char obfr[OBFRSIZE];
static unsigned char *optr;
static int outfile;
static unsigned char *last_src[3];
unsigned char **GetLastSrc()
{
	return last_src;
}
/*
 * store a picture as either one frame or two fields
 */
void Write_Frame(MPEGDecoder *decoder, unsigned char *src[], int frame)
{
	last_src[0] = src[0];
	last_src[1] = src[1];
	last_src[2] = src[2];
#if 0
  char outname[FILENAME_LENGTH];

  if (decoder->progressive_sequence || decoder->progressive_frame/* || Frame_Store_Flag*/)
  {
    /* progressive */
    sprintf(outname,Output_Picture_Filename,frame,'f');
    store_one(decoder, outname,src,0,decoder->Coded_Picture_Width,decoder->vertical_size);
  }
  else
  {
    /* interlaced */
    sprintf(outname,Output_Picture_Filename,frame,'a');
    store_one(decoder, outname,src,0,decoder->Coded_Picture_Width<<1,decoder->vertical_size>>1);

    sprintf(outname,Output_Picture_Filename,frame,'b');
    store_one(decoder, outname,src, decoder->Coded_Picture_Width,decoder->Coded_Picture_Width<<1,decoder->vertical_size>>1);
  }
#endif
}

/*
 * store one frame or one field
 */
static void store_one(MPEGDecoder *decoder, char *outname, unsigned char *src[], int offset, int incr, int height)
{
  //  store_yuv(decoder, outname,src,offset,incr,height);
}

/* separate headerless files for y, u and v */
static void store_yuv(decoder, outname,src,offset,incr,height)
MPEGDecoder *decoder;
char *outname;
unsigned char *src[];
int offset,incr,height;
{
  int hsize;
  char tmpname[FILENAME_LENGTH];

  hsize = decoder->horizontal_size;

  sprintf(tmpname,"%s.Y",outname);
  store_yuv1(tmpname,src[0],offset,incr,hsize,height);

  if (decoder->chroma_format!=CHROMA444)
  {
    offset>>=1; incr>>=1; hsize>>=1;
  }

  if (decoder->chroma_format==CHROMA420)
  {
    height>>=1;
  }

  sprintf(tmpname,"%s.U",outname);
  store_yuv1(tmpname,src[1],offset,incr,hsize,height);

  sprintf(tmpname,"%s.V",outname);
  store_yuv1(tmpname,src[2],offset,incr,hsize,height);
}

/* auxiliary routine */
static void store_yuv1(name,src,offset,incr,width,height)
char *name;
unsigned char *src;
int offset,incr,width,height;
{
  int i, j;
  unsigned char *p;

  if ((outfile = open(name,O_CREAT|O_TRUNC|O_WRONLY|O_BINARY,0666))==-1)
  {
    sprintf(Error_Text,"Couldn't create %s\n",name);
    Error(Error_Text);
  }

  optr=obfr;

  for (i=0; i<height; i++)
  {
    p = src + offset + incr*i;
    for (j=0; j<width; j++)
      putbyte(*p++);
  }

  if (optr!=obfr)
    write(outfile,obfr,optr-obfr);

  close(outfile);
}

static void putbyte(c)
int c;
{
  *optr++ = c;

  if (optr == obfr+OBFRSIZE)
  {
    write(outfile,obfr,OBFRSIZE);
    optr = obfr;
  }
}

static void putword(w)
int w;
{
  putbyte(w); putbyte(w>>8);
}


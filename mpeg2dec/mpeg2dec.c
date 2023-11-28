
/* mpeg2dec.c, main(), initialization, option processing                    */

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
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include "mpegvid_api.h"

#define GLOBAL
#include "global.h"
#undef GLOBAL
#include "decoder.h"

/* private prototypes */
static int  video_sequence(MPEGDecoder *decoder);
static int Decode_Bitstream(MPEGDecoder *decoder);
static void Initialize_Decoder();


/* #define DEBUG */

static void Clear_Options();
#ifdef DEBUG
static void Print_Options();
#endif

unsigned char *Clip = 0;

int MPEGVideo_DecodeFrame(mpegvideo_decoder_t d, void *buffer, int len)
{
  int ret, code;
	MPEGDecoder *decoder=d;

  Clear_Options();

	decoder->base.bitstream.data = buffer;
	decoder->base.bitstream.numBits = len*8;

  // TODO  Initialize_Buffer(); 
  
	if (decoder->start_code == 0)
	{
    if(Show_Bits(&decoder->base.bitstream, 8)==0x47)
    {
      Error("Decoder currently does not parse transport streams\n");
    }

    if (next_start_code(decoder) == 0)
			return 1;
    code = Show_Bits(&decoder->base.bitstream, 32);

    switch(code)
    {
    case SEQUENCE_HEADER_CODE:
      break;
    case PACK_START_CODE:
    case VIDEO_ELEMENTARY_STREAM:
      break;
    default:
      Error("Unable to recognize stream type\n");
      break;
    }
		decoder->start_code = code;
	}
  


  ret = Decode_Bitstream(decoder);

  return 0;
}

mpegvideo_decoder_t MPEGVideo_CreateDecoder()
{
	MPEGDecoder *decoder = malloc(sizeof(MPEGDecoder));
	memset(decoder, 0, sizeof(MPEGDecoder));
	Initialize_Decoder();
	return decoder;
}

/* IMPLEMENTAION specific rouintes */
static void Initialize_Decoder()
{
  int i;

  /* Clip table */
	if (!Clip)
	{
  if (!(Clip=(unsigned char *)malloc(1024)))
    Error("Clip[] malloc failed\n");


  Clip += 384;

  for (i=-384; i<640; i++)
    Clip[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  /* IDCT */
  if (Reference_IDCT_Flag)
    Initialize_Reference_IDCT();
  else
    Initialize_Fast_IDCT();
	}
}

/* mostly IMPLEMENTAION specific rouintes */
void Initialize_Sequence(MPEGDecoder *decoder)
{
  int cc, size;
  static int Table_6_20[3] = {6,8,12};

	decoder->sequence_framenum = 0;

  /* force MPEG-1 parameters for proper decoder behavior */
  /* see ISO/IEC 13818-2 section D.9.14 */
  if (!decoder->base.MPEG2_Flag)
  {
    decoder->progressive_sequence = 1;
    decoder->progressive_frame = 1;
    decoder->picture_structure = FRAME_PICTURE;
    decoder->frame_pred_frame_dct = 1;
    decoder->chroma_format = CHROMA420;
    decoder->matrix_coefficients = 5;
  }

  /* round to nearest multiple of coded macroblocks */
  /* ISO/IEC 13818-2 section 6.3.3 sequence_header() */
  decoder->mb_width = (decoder->horizontal_size+15)/16;
  decoder->mb_height = (decoder->base.MPEG2_Flag && !decoder->progressive_sequence) ? 2*((decoder->vertical_size+31)/32)
                                        : (decoder->vertical_size+15)/16;

  decoder->Coded_Picture_Width = 16*decoder->mb_width;
  decoder->Coded_Picture_Height = 16*decoder->mb_height;

  /* ISO/IEC 13818-2 sections 6.1.1.8, 6.1.1.9, and 6.1.1.10 */
  decoder->Chroma_Width = (decoder->chroma_format==CHROMA444) ? decoder->Coded_Picture_Width
                                           : decoder->Coded_Picture_Width>>1;
  decoder->Chroma_Height = (decoder->chroma_format!=CHROMA420) ? decoder->Coded_Picture_Height
                                            : decoder->Coded_Picture_Height>>1;
  
  /* derived based on Table 6-20 in ISO/IEC 13818-2 section 6.3.17 */
  decoder->block_count = Table_6_20[decoder->chroma_format-1];

  for (cc=0; cc<3; cc++)
  {
    if (cc==0)
      size = decoder->Coded_Picture_Width*decoder->Coded_Picture_Height;
    else
      size = decoder->Chroma_Width*decoder->Chroma_Height;

    if (!(decoder->backward_reference_frame[cc] = (unsigned char *)malloc(size)))
      Error("backward_reference_frame[] malloc failed\n");

    if (!(decoder->forward_reference_frame[cc] = (unsigned char *)malloc(size)))
      Error("forward_reference_frame[] malloc failed\n");

    if (!(decoder->auxframe[cc] = (unsigned char *)malloc(size)))
      Error("auxframe[] malloc failed\n");

  }
}

void Error(char *text)
{
  fprintf(stderr,text);
  exit(1);
}

static int Decode_Bitstream(MPEGDecoder *decoder)
{
  int ret;

    ret = Get_Hdr(decoder);
    
    if(ret==1)
    {
      ret = video_sequence(decoder);
    }
    else
      return(ret);

		return ret;
}


void Deinitialize_Sequence(MPEGDecoder *decoder)
{
  int i;

  /* clear flags */
  decoder->base.MPEG2_Flag=0;

  for(i=0;i<3;i++)
  {
    free(decoder->backward_reference_frame[i]);
    free(decoder->forward_reference_frame[i]);
    free(decoder->auxframe[i]);
  }
}


static int video_sequence(MPEGDecoder *decoder)
{

  /* decode picture whose header has already been parsed in 
     Decode_Bitstream() */
  int ret = Decode_Picture(decoder, decoder->bitstream_framenum, decoder->sequence_framenum);

	if (ret == SLICE_ALL_MACROBLOCKS_DECODED)
	{
				/* write or display current or previously decoded reference frame */
		/* ISO/IEC 13818-2 section 6.1.1.11: Frame reordering */
		frame_reorder(decoder, decoder->bitstream_framenum, decoder->sequence_framenum);

		if (decoder->picture_structure!=FRAME_PICTURE)
			decoder->Second_Field = !decoder->Second_Field;

		/* update picture numbers */
		if (!decoder->Second_Field)
		{
	    decoder->bitstream_framenum++;
			decoder->sequence_framenum++;
		}

	}

  return 1; //(Return_Value);
}



static void Clear_Options()
{
  Reference_IDCT_Flag = 0;
}


#ifdef DEBUG
static void Print_Options()
{
    printf("Reference_IDCT_Flag                  = %d\n", Reference_IDCT_Flag);
}
#endif

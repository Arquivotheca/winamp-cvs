/* getvlc.c, variable length decoding                                       */

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

#include "global.h"
#include "getvlc.h"
#include "decoder.h"
/* private prototypes */
/* generic picture macroblock type processing functions */
static int Get_I_macroblock_type (MPEGDecoder *decoder);
static int Get_P_macroblock_type (MPEGDecoder *decoder);
static int Get_B_macroblock_type (MPEGDecoder *decoder);
static int Get_D_macroblock_type (MPEGDecoder *decoder);

int Get_macroblock_type(MPEGDecoder *decoder)
{
  int macroblock_type = 0;

    switch (decoder->picture_coding_type)
    {
    case I_TYPE:
      macroblock_type = Get_I_macroblock_type(decoder);
      break;
    case P_TYPE:
      macroblock_type = Get_P_macroblock_type(decoder);
      break;
    case B_TYPE:
      macroblock_type = Get_B_macroblock_type(decoder);
      break;
    case D_TYPE:
      macroblock_type = Get_D_macroblock_type(decoder);
      break;
    default:
      printf("Get_macroblock_type(): unrecognized picture coding type\n");
      break;
    }

  return macroblock_type;
}

static int Get_I_macroblock_type(MPEGDecoder *decoder)
{
  if (Get_Bits1(&decoder->base.bitstream))
  {
    return MACROBLOCK_INTRA;
  }

  if (!Get_Bits1(&decoder->base.bitstream))
  {
    decoder->Fault_Flag = 1;
  }

  return MACROBLOCK_INTRA|MACROBLOCK_QUANT;
}

static char *MBdescr[]={
  "",                  "Intra",        "No MC, Coded",         "",
  "Bwd, Not Coded",    "",             "Bwd, Coded",           "",
  "Fwd, Not Coded",    "",             "Fwd, Coded",           "",
  "Interp, Not Coded", "",             "Interp, Coded",        "",
  "",                  "Intra, Quant", "No MC, Coded, Quant",  "",
  "",                  "",             "Bwd, Coded, Quant",    "",
  "",                  "",             "Fwd, Coded, Quant",    "",
  "",                  "",             "Interp, Coded, Quant", ""
};

static int Get_P_macroblock_type(MPEGDecoder *decoder)
{
  int code;

  if ((code = Show_Bits(&decoder->base.bitstream, 6))>=8)
  {
    code >>= 3;
    Flush_Buffer(&decoder->base.bitstream, PMBtab0[code].len);
    return PMBtab0[code].val;
  }

  if (code==0)
  {
    decoder->Fault_Flag = 1;
    return 0;
  }

  Flush_Buffer(&decoder->base.bitstream, PMBtab1[code].len);

  return PMBtab1[code].val;

}

static int Get_B_macroblock_type(MPEGDecoder *decoder)
{
  int code;

  if ((code = Show_Bits(&decoder->base.bitstream, 6))>=8)
  {
    code >>= 2;
    Flush_Buffer(&decoder->base.bitstream, BMBtab0[code].len);

    return BMBtab0[code].val;
  }

  if (code==0)
  {
    decoder->Fault_Flag = 1;
    return 0;
  }

  Flush_Buffer(&decoder->base.bitstream, BMBtab1[code].len);

  return BMBtab1[code].val;
}

static int Get_D_macroblock_type(MPEGDecoder *decoder)
{
  if (!Get_Bits1(&decoder->base.bitstream))
  {
    decoder->Fault_Flag=1;
  }

  return 1;
}

int Get_motion_code(MPEGDecoder *decoder)
{
  int code;

  if (Get_Bits1(&decoder->base.bitstream))
  {
    return 0;
  }

  if ((code = Show_Bits(&decoder->base.bitstream, 9))>=64)
  {
    code >>= 6;
    Flush_Buffer(&decoder->base.bitstream, MVtab0[code].len);
    return Get_Bits1(&decoder->base.bitstream)?-MVtab0[code].val:MVtab0[code].val;
  }

  if (code>=24)
  {
    code >>= 3;
    Flush_Buffer(&decoder->base.bitstream, MVtab1[code].len);

    return Get_Bits1(&decoder->base.bitstream)?-MVtab1[code].val:MVtab1[code].val;
  }

  if ((code-=12)<0)
  {
    decoder->Fault_Flag=1;
    return 0;
  }

  Flush_Buffer(&decoder->base.bitstream, MVtab2[code].len);
  return Get_Bits1(&decoder->base.bitstream) ? -MVtab2[code].val : MVtab2[code].val;
}

/* get differential motion vector (for dual prime prediction) */
int Get_dmvector(MPEGDecoder *decoder)
{
  if (Get_Bits(&decoder->base.bitstream, 1))
  {
    return Get_Bits(&decoder->base.bitstream, 1) ? -1 : 1;
  }
  else
  {
    return 0;
  }
}

int Get_coded_block_pattern(MPEGDecoder *decoder)
{
  int code;
  if ((code = Show_Bits(&decoder->base.bitstream, 9))>=128)
  {
    code >>= 4;
    Flush_Buffer(&decoder->base.bitstream, CBPtab0[code].len);

    return CBPtab0[code].val;
  }

  if (code>=8)
  {
    code >>= 1;
    Flush_Buffer(&decoder->base.bitstream, CBPtab1[code].len);
    return CBPtab1[code].val;
  }

  if (code<1)
  {
    decoder->Fault_Flag = 1;
    return 0;
  }

  Flush_Buffer(&decoder->base.bitstream, CBPtab2[code].len);


  return CBPtab2[code].val;
}

int Get_macroblock_address_increment(MPEGDecoder *decoder)
{
  int code, val;

  val = 0;

	if (decoder->base.MPEG2_Flag)
	{
		// from H.262 6.3.17
		// “macroblock_stuffing” which is supported in ISO/IEC 11172-2 shall not be used in a bitstream defined by this specification.
		while ((code = Show_Bits(&decoder->base.bitstream, 11))==8)
		{
			val+=33;
			Flush_Buffer(&decoder->base.bitstream, 11);
		}
	}
	else
	{
		while ((code = Show_Bits(&decoder->base.bitstream, 11))<24)
		{
			if (code!=15) /* if not macroblock_stuffing */
			{
				if (code==8) /* if macroblock_escape */
				{
					val+= 33;
				}
				else
				{
					decoder->Fault_Flag = 1;
					return 1;
				}
			}
			else /* macroblock suffing */
			{
			}

			Flush_Buffer(&decoder->base.bitstream, 11);
		}
	}

  /* macroblock_address_increment == 1 */
  /* ('1' is in the MSB position of the lookahead) */
  if (code>=1024)
  {
    Flush_Buffer(&decoder->base.bitstream, 1);

    return val + 1;
  }

  /* codes 00010 ... 011xx */
  if (code>=128)
  {
    /* remove leading zeros */
    code >>= 6;
    Flush_Buffer(&decoder->base.bitstream, MBAtab1[code].len);

    
    return val + MBAtab1[code].val;
  }
  
  /* codes 00000011000 ... 0000111xxxx */
  code-= 24; /* remove common base */
  Flush_Buffer(&decoder->base.bitstream, MBAtab2[code].len);

  return val + MBAtab2[code].val;
}

/* combined MPEG-1 and MPEG-2 stage. parse VLC and 
   perform dct_diff arithmetic.

   MPEG-1:  ISO/IEC 11172-2 section
   MPEG-2:  ISO/IEC 13818-2 section 7.2.1 
   
   Note: the arithmetic here is presented more elegantly than
   the spec, yet the results, dct_diff, are the same.
*/

int Get_Luma_DC_dct_diff(MPEGDecoder *decoder)
{
  int code, size, dct_diff;

  /* decode length */
  code = Show_Bits(&decoder->base.bitstream, 5);

  if (code<31)
  {
    size = DClumtab0[code].val;
    Flush_Buffer(&decoder->base.bitstream, DClumtab0[code].len);
  }
  else
  {
    code = Show_Bits(&decoder->base.bitstream, 9) - 0x1f0;
    size = DClumtab1[code].val;
    Flush_Buffer(&decoder->base.bitstream, DClumtab1[code].len);
  }
  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = Get_Bits(&decoder->base.bitstream, size);
    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }

  return dct_diff;
}


int Get_Chroma_DC_dct_diff(MPEGDecoder *decoder)
{
  int code, size, dct_diff;

  /* decode length */
  code = Show_Bits(&decoder->base.bitstream, 5);

  if (code<31)
  {
    size = DCchromtab0[code].val;
    Flush_Buffer(&decoder->base.bitstream, DCchromtab0[code].len);

  }
  else
  {
    code = Show_Bits(&decoder->base.bitstream, 10) - 0x3e0;
    size = DCchromtab1[code].val;
    Flush_Buffer(&decoder->base.bitstream, DCchromtab1[code].len);
  }
  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = Get_Bits(&decoder->base.bitstream, size);
    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }
  return dct_diff;
}

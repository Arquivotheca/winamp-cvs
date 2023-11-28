/* getblk.c, DCT block decoding                                             */

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
#include "decoder.h"

/* defined in getvlc.h */
typedef struct {
  char run, level, len;
} DCTtab;

extern DCTtab DCTtabfirst[],DCTtabnext[],DCTtab0[],DCTtab1[];
extern DCTtab DCTtab2[],DCTtab3[],DCTtab4[],DCTtab5[],DCTtab6[];
extern DCTtab DCTtab0a[],DCTtab1a[];


/* decode one intra coded MPEG-1 block */
// TODO: ippiReconstructDCTBlockIntra_MPEG1_32s
void Decode_MPEG1_Intra_Block(MPEGDecoder *decoder, int comp, int dc_dct_pred[])
{
  int val, i, j, sign;
  unsigned int code;
  DCTtab *tab;
  short *bp;

  bp = decoder->base.block[comp];

  /* ISO/IEC 11172-2 section 2.4.3.7: Block layer. */
  /* decode DC coefficients */
  if (comp<4)
    bp[0] = (dc_dct_pred[0]+=Get_Luma_DC_dct_diff(decoder)) << 3;
  else if (comp==4)
    bp[0] = (dc_dct_pred[1]+=Get_Chroma_DC_dct_diff(decoder)) << 3;
  else
    bp[0] = (dc_dct_pred[2]+=Get_Chroma_DC_dct_diff(decoder)) << 3;

  if (decoder->Fault_Flag) return;

  /* D-pictures do not contain AC coefficients */
  if(decoder->picture_coding_type == D_TYPE)
    return;

  /* decode AC coefficients */
  for (i=1; ; i++)
  {
    code = Show_Bits(&decoder->base.bitstream, 16);
    if (code>=16384)
      tab = &DCTtabnext[(code>>12)-4];
    else if (code>=1024)
      tab = &DCTtab0[(code>>8)-4];
    else if (code>=512)
      tab = &DCTtab1[(code>>6)-8];
    else if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      decoder->Fault_Flag = 1;
      return;
    }

    Flush_Buffer(&decoder->base.bitstream, tab->len);

    if (tab->run==64) /* end_of_block */
      return;

    if (tab->run==65) /* escape */
    {
      i+= Get_Bits(&decoder->base.bitstream, 6);

      val = Get_Bits(&decoder->base.bitstream, 8);
      if (val==0)
        val = Get_Bits(&decoder->base.bitstream, 8);
      else if (val==128)
        val = Get_Bits(&decoder->base.bitstream, 8) - 256;
      else if (val>128)
        val -= 256;

      if((sign = (val<0)))
        val = -val;
    }
    else
    {
      i+= tab->run;
      val = tab->level;
      sign = Get_Bits(&decoder->base.bitstream, 1);
    }

    if (i>=64)
    {
      decoder->Fault_Flag = 1;
      return;
    }

    j = scan[ZIG_ZAG][i];
    val = (val*decoder->base.quantizer_scale*decoder->base.intra_quantizer_matrix[j]) >> 3;

    /* mismatch control ('oddification') */
    if (val!=0) /* should always be true, but it's not guaranteed */
      val = (val-1) | 1; /* equivalent to: if ((val&1)==0) val = val - 1; */

    /* saturation */
    if (!sign)
      bp[j] = (val>2047) ?  2047 :  val; /* positive */
    else
      bp[j] = (val>2048) ? -2048 : -val; /* negative */
  }
}


/* decode one non-intra coded MPEG-1 block */
// TODO: ippiReconstructDCTBlock_MPEG1_32s
void Decode_MPEG1_Non_Intra_Block(MPEGDecoder *decoder, int comp)
{
  int val, i, j, sign;
  unsigned int code;
  DCTtab *tab;
  short *bp;

  bp = decoder->base.block[comp];

  /* decode AC coefficients */
  for (i=0; ; i++)
  {
    code = Show_Bits(&decoder->base.bitstream, 16);
    if (code>=16384)
    {
      if (i==0)
        tab = &DCTtabfirst[(code>>12)-4];
      else
        tab = &DCTtabnext[(code>>12)-4];
    }
    else if (code>=1024)
      tab = &DCTtab0[(code>>8)-4];
    else if (code>=512)
      tab = &DCTtab1[(code>>6)-8];
    else if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      decoder->Fault_Flag = 1;
      return;
    }

    Flush_Buffer(&decoder->base.bitstream, tab->len);

    if (tab->run==64) /* end_of_block */
      return;

    if (tab->run==65) /* escape */
    {
      i+= Get_Bits(&decoder->base.bitstream, 6);

      val = Get_Bits(&decoder->base.bitstream, 8);
      if (val==0)
        val = Get_Bits(&decoder->base.bitstream, 8);
      else if (val==128)
        val = Get_Bits(&decoder->base.bitstream, 8) - 256;
      else if (val>128)
        val -= 256;

      if((sign = (val<0)))
        val = -val;
    }
    else
    {
      i+= tab->run;
      val = tab->level;
      sign = Get_Bits(&decoder->base.bitstream, 1);
    }

    if (i>=64)
    {
      decoder->Fault_Flag = 1;
      return;
    }

    j = scan[ZIG_ZAG][i];
    val = (((val<<1)+1)*decoder->base.quantizer_scale*decoder->base.non_intra_quantizer_matrix[j]) >> 4;

    /* mismatch control ('oddification') */
    if (val!=0) /* should always be true, but it's not guaranteed */
      val = (val-1) | 1; /* equivalent to: if ((val&1)==0) val = val - 1; */

    /* saturation */
    if (!sign)
      bp[j] = (val>2047) ?  2047 :  val; /* positive */
    else
      bp[j] = (val>2048) ? -2048 : -val; /* negative */
  }
}


/* decode one intra coded MPEG-2 block */

void Decode_MPEG2_Intra_Block(MPEGDecoder *decoder, int comp, int dc_dct_pred[])
{
  int val, i, j, sign, nc, cc, run;
  unsigned int code;
  DCTtab *tab;
  short *bp;
  int *qmat;

  bp = decoder->base.block[comp];

  cc = (comp<4) ? 0 : (comp&1)+1;

  qmat = (comp<4 || decoder->chroma_format==CHROMA420)
         ? decoder->base.intra_quantizer_matrix
         : decoder->base.chroma_intra_quantizer_matrix;

  /* ISO/IEC 13818-2 section 7.2.1: decode DC coefficients */
  if (cc==0)
    val = (dc_dct_pred[0]+= Get_Luma_DC_dct_diff(decoder));
  else if (cc==1)
    val = (dc_dct_pred[1]+= Get_Chroma_DC_dct_diff(decoder));
  else
    val = (dc_dct_pred[2]+= Get_Chroma_DC_dct_diff(decoder));

  if (decoder->Fault_Flag) return;

  bp[0] = val << (3-decoder->intra_dc_precision);

  nc=0;

  /* decode AC coefficients */
  for (i=1; ; i++)
  {
    code = Show_Bits(&decoder->base.bitstream, 16);
    if (code>=16384 && !decoder->intra_vlc_format)
      tab = &DCTtabnext[(code>>12)-4];
    else if (code>=1024)
    {
      if (decoder->intra_vlc_format)
        tab = &DCTtab0a[(code>>8)-4];
      else
        tab = &DCTtab0[(code>>8)-4];
    }
    else if (code>=512)
    {
      if (decoder->intra_vlc_format)
        tab = &DCTtab1a[(code>>6)-8];
      else
        tab = &DCTtab1[(code>>6)-8];
    }
    else if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      decoder->Fault_Flag = 1;
      return;
    }

    Flush_Buffer(&decoder->base.bitstream, tab->len);


    if (tab->run==64) /* end_of_block */
    {
      return;
    }

    if (tab->run==65) /* escape */
    {

      i+= run = Get_Bits(&decoder->base.bitstream, 6);

      val = Get_Bits(&decoder->base.bitstream, 12);
      if ((val&2047)==0)
      {
        decoder->Fault_Flag = 1;
        return;
      }
      if((sign = (val>=2048)))
        val = 4096 - val;
    }
    else
    {
      i+= run = tab->run;
      val = tab->level;
      sign = Get_Bits(&decoder->base.bitstream, 1);

    }

    if (i>=64)
    {
      decoder->Fault_Flag = 1;
      return;
    }

    j = scan[decoder->base.alternate_scan][i];
    val = (val * decoder->base.quantizer_scale * qmat[j]) >> 4;
    bp[j] = sign ? -val : val;
    nc++;

  }
}


/* decode one non-intra coded MPEG-2 block */
// TOOD: ippiReconstructDCTBlock_MPEG2_32s
void Decode_MPEG2_Non_Intra_Block(MPEGDecoder *decoder, int comp)
{
  int val, i, j, sign, nc, run;
  unsigned int code;
  DCTtab *tab;
  short *bp;
  int *qmat;
  struct layer_data *ld1;

  /* with data partitioning, data always goes to base layer */
  ld1 = &decoder->base;
  bp = ld1->block[comp];

  qmat = (comp<4 || decoder->chroma_format==CHROMA420)
         ? ld1->non_intra_quantizer_matrix
         : ld1->chroma_non_intra_quantizer_matrix;

  nc = 0;

  /* decode AC coefficients */
  for (i=0; ; i++)
  {
    code = Show_Bits(&decoder->base.bitstream, 16);
    if (code>=16384)
    {
      if (i==0)
        tab = &DCTtabfirst[(code>>12)-4];
      else
        tab = &DCTtabnext[(code>>12)-4];
    }
    else if (code>=1024)
      tab = &DCTtab0[(code>>8)-4];
    else if (code>=512)
      tab = &DCTtab1[(code>>6)-8];
    else if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      decoder->Fault_Flag = 1;
      return;
    }

    Flush_Buffer(&decoder->base.bitstream, tab->len);


    if (tab->run==64) /* end_of_block */
    {
      return;
    }

    if (tab->run==65) /* escape */
    {

      i+= run = Get_Bits(&decoder->base.bitstream, 6);


      val = Get_Bits(&decoder->base.bitstream, 12);
      if ((val&2047)==0)
      {
        decoder->Fault_Flag = 1;
        return;
      }
      if((sign = (val>=2048)))
        val = 4096 - val;
    }
    else
    {
      i+= run = tab->run;
      val = tab->level;
      sign = Get_Bits(&decoder->base.bitstream, 1);
    }

    if (i>=64)
    {
      decoder->Fault_Flag = 1;
      return;
    }

    j = scan[ld1->alternate_scan][i];
    val = (((val<<1)+1) * ld1->quantizer_scale * qmat[j]) >> 5;
    bp[j] = sign ? -val : val;
    nc++;
  }
}

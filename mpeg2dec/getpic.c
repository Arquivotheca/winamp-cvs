/* getpic.c, picture decoding                                               */

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

/* private prototypes*/
static int picture_data(MPEGDecoder *decoder, int framenum);
static void macroblock_modes(MPEGDecoder *decoder, int *pmacroblock_type, int *pstwtype,  int *pstwclass, int *pmotion_type, int *pmotion_vector_count, int *pmv_format, int *pdmv,  int *pmvscale, int *pdct_type);
static void Clear_Block(MPEGDecoder *decoder, int comp);
static void Saturate(short *bp);
static void Add_Block(MPEGDecoder *decoder, int comp, int bx, int by,  int dct_type, int addflag);
static void Update_Picture_Buffers(MPEGDecoder *decoder);

static void motion_compensation(MPEGDecoder *decoder, int MBA, int macroblock_type,  int motion_type, int PMV[2][2][2], int motion_vertical_field_select[2][2],  int dmvector[2], int stwtype, int dct_type);
static void skipped_macroblock(MPEGDecoder *decoder, int dc_dct_pred[3],   int PMV[2][2][2], int *motion_type, int motion_vertical_field_select[2][2],  int *stwtype, int *macroblock_type);
static int slice (MPEGDecoder *decoder, int framenum, int MBAmax);
static int start_of_slice(MPEGDecoder *decoder, int MBAmax, int *MBA,  int *MBAinc, int dc_dct_pred[3], int PMV[2][2][2]);
static int decode_macroblock (MPEGDecoder *decoder, int *macroblock_type,   int *stwtype, int *stwclass, int *motion_type, int *dct_type,  int PMV[2][2][2], int dc_dct_pred[3],   int motion_vertical_field_select[2][2], int dmvector[2]);


/* decode one frame or field picture */
int Decode_Picture(MPEGDecoder *decoder, int bitstream_framenum, int sequence_framenum)
{
	int ret;
  if (decoder->picture_structure==FRAME_PICTURE && decoder->Second_Field)
  {
    /* recover from illegal number of field pictures */
    printf("odd number of field pictures\n");
    decoder->Second_Field = 0;
  }

  /* IMPLEMENTATION: update picture buffer pointers */
  Update_Picture_Buffers(decoder);

  /* decode picture data ISO/IEC 13818-2 section 6.2.3.7 */
  return picture_data(decoder, bitstream_framenum);
}


/* decode all macroblocks of the current picture */
/* stages described in ISO/IEC 13818-2 section 7 */
static int picture_data(MPEGDecoder *decoder, int framenum)
{
  int MBAmax;

  /* number of macroblocks per picture */
  MBAmax = decoder->mb_width*decoder->mb_height;

  if (decoder->picture_structure!=FRAME_PICTURE)
    MBAmax>>=1; /* field picture has half as mnay macroblocks as frame */

  for(;;)
  {
		int ret = slice(decoder, framenum, MBAmax);
		if (ret == SLICE_MORE_DATA_PLEASE)
		{
			return ret;
		}
		else if (ret == SLICE_ALL_MACROBLOCKS_DECODED)
		{
			return SLICE_ALL_MACROBLOCKS_DECODED;
		}
  }

}

/* decode all macroblocks of the current picture */
/* ISO/IEC 13818-2 section 6.3.16 */
static int slice(MPEGDecoder *decoder, int framenum, int MBAmax)
{
	int MBA;
  int MBAinc, macroblock_type, motion_type, dct_type;
  int dc_dct_pred[3];
  int PMV[2][2][2], motion_vertical_field_select[2][2];
  int dmvector[2];
  int stwtype, stwclass;
  int ret;

  MBA = 0; /* macroblock address */
  MBAinc = 0;

  if((ret=start_of_slice(decoder, MBAmax, &MBA, &MBAinc, dc_dct_pred, PMV))!=SLICE_MACROBLOCK_DECODE)
    return(ret);

  decoder->Fault_Flag=0;

  for (;;)
  {
    /* this is how we properly exit out of picture */
    if (MBA>=MBAmax)
      return SLICE_ALL_MACROBLOCKS_DECODED; /* all macroblocks decoded */

    if (MBAinc==0)
    {
			if (Num_Bits(&decoder->base.bitstream) < 23)
				;//return SLICE_MORE_DATA_PLEASE;

      if (!Show_Bits(&decoder->base.bitstream, 23) || decoder->Fault_Flag) /* next_start_code or fault */
      {
resync: /* if decoder->Fault_Flag: resynchronize to next next_start_code */
        decoder->Fault_Flag = 0;
        return SLICE_RESYNC;     /* trigger: go to next slice */
      }
      else /* neither next_start_code nor decoder->Fault_Flag */
      {
        /* decode macroblock address increment */
        MBAinc = Get_macroblock_address_increment(decoder);

        if (decoder->Fault_Flag) goto resync;
      }
    }

    if (MBA>=MBAmax)
    {
      /* MBAinc points beyond picture dimensions */
      return SLICE_ALL_MACROBLOCKS_DECODED;
    }

    if (MBAinc==1) /* not skipped */
    {
      ret = decode_macroblock(decoder, &macroblock_type, &stwtype, &stwclass, &motion_type, &dct_type, PMV, dc_dct_pred, motion_vertical_field_select, dmvector);

      if(ret!=SLICE_MACROBLOCK_DECODE)
        return ret;
   
      if(ret==0)
        goto resync;

    }
    else /* MBAinc!=1: skipped macroblock */
    {      
      /* ISO/IEC 13818-2 section 7.6.6 */
      skipped_macroblock(decoder, dc_dct_pred, PMV, &motion_type, motion_vertical_field_select, &stwtype, &macroblock_type);
    }

     /* ISO/IEC 13818-2 section 7.6 */
    motion_compensation(decoder, MBA, macroblock_type, motion_type, PMV, motion_vertical_field_select, dmvector, stwtype, dct_type);


    /* advance to next macroblock */
    MBA++;
    MBAinc--;

    if (MBA>=MBAmax)
      return SLICE_ALL_MACROBLOCKS_DECODED; /* all macroblocks decoded */
  }
}

 
/* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */
static void macroblock_modes(MPEGDecoder *decoder, int *pmacroblock_type, int *pstwtype,  int *pstwclass, int *pmotion_type, int *pmotion_vector_count, int *pmv_format, int *pdmv,  int *pmvscale, int *pdct_type)
{
	static int xxx;
  int macroblock_type;
  int stwtype, stwclass;
  int motion_type = 0;
  int motion_vector_count, mv_format, dmv, mvscale;
  int dct_type;
  static unsigned char stwc_table[3][4]
    = { {6,3,7,4}, {2,1,5,4}, {2,5,7,4} };
  static unsigned char stwclass_table[9]
    = {0, 1, 2, 1, 1, 2, 3, 3, 4};

		xxx++;
if (xxx == 46171 )
{
	int x;
	x=0;
}
  /* get macroblock_type */
  macroblock_type = Get_macroblock_type(decoder);

  if (decoder->Fault_Flag) return;

  /* get spatial_temporal_weight_code */
  if (macroblock_type & MB_WEIGHT)
  {
		Error("Unsupported spatial_temporal_weight_code");
  }
  else
    stwtype = (macroblock_type & MB_CLASS4) ? 8 : 0;

  /* SCALABILITY: derive spatial_temporal_weight_class (Table 7-18) */
  stwclass = stwclass_table[stwtype];

  /* get frame/field motion type */
  if (macroblock_type & (MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD))
  {
    if (decoder->picture_structure==FRAME_PICTURE) /* frame_motion_type */
    {
			if (decoder->frame_pred_frame_dct)
			{
				motion_type = MC_FRAME;
			}
			else
			{
				motion_type = Get_Bits(&decoder->base.bitstream, 2);
				if (motion_type == 0)
					motion_type=MC_FRAME;
				//else
					//Flush_Buffer(&decoder->base.bitstream, 2);
			}
    }
    else /* field_motion_type */
    {
      motion_type = Get_Bits(&decoder->base.bitstream, 2);
    }
  }
  else if ((macroblock_type & MACROBLOCK_INTRA) && decoder->concealment_motion_vectors)
  {
    /* concealment motion vectors */
    motion_type = (decoder->picture_structure==FRAME_PICTURE) ? MC_FRAME : MC_FIELD;
  }

  /* derive motion_vector_count, mv_format and dmv, (table 6-17, 6-18) */
  if (decoder->picture_structure==FRAME_PICTURE)
  {
    motion_vector_count = (motion_type==MC_FIELD && stwclass<2) ? 2 : 1;
    mv_format = (motion_type==MC_FRAME) ? MV_FRAME : MV_FIELD;
  }
  else
  {
    motion_vector_count = (motion_type==MC_16X8) ? 2 : 1;
    mv_format = MV_FIELD;
  }

  dmv = (motion_type==MC_DMV); /* dual prime */

  /* field mv predictions in frame pictures have to be scaled
   * ISO/IEC 13818-2 section 7.6.3.1 Decoding the motion vectors
   * IMPLEMENTATION: mvscale is derived for later use in motion_vectors()
   * it displaces the stage:
   *
   *    if((mv_format=="field")&&(t==1)&&(decoder->picture_structure=="Frame picture"))
   *      prediction = PMV[r][s][t] DIV 2;
   */

  mvscale = ((mv_format==MV_FIELD) && (decoder->picture_structure==FRAME_PICTURE));

  /* get dct_type (frame DCT / field DCT) */
  dct_type = (decoder->picture_structure==FRAME_PICTURE)
             && (!decoder->frame_pred_frame_dct)
             && (macroblock_type & (MACROBLOCK_PATTERN|MACROBLOCK_INTRA))
             ? Get_Bits(&decoder->base.bitstream, 1)
             : 0;


	if (xxx > 46169 && motion_type == 0)
	{
		int x;
		x=0;
	}
  /* return values */
  *pmacroblock_type = macroblock_type;
  *pstwtype = stwtype;
  *pstwclass = stwclass;
  *pmotion_type = motion_type;
  *pmotion_vector_count = motion_vector_count;
  *pmv_format = mv_format;
  *pdmv = dmv;
  *pmvscale = mvscale;
  *pdct_type = dct_type;
}


/* move/add 8x8-Block from block[comp] to backward_reference_frame */
/* copy reconstructed 8x8 block from block[comp] to current_frame[]
 * ISO/IEC 13818-2 section 7.6.8: Adding prediction and coefficient data
 * This stage also embodies some of the operations implied by:
 *   - ISO/IEC 13818-2 section 7.6.7: Combining predictions
 *   - ISO/IEC 13818-2 section 6.1.3: Macroblock
*/
static void Add_Block(MPEGDecoder *decoder, int comp, int bx, int by,  int dct_type, int addflag)
{
  int cc,i, j, iincr;
  unsigned char *rfp;
  short *bp;

  
  /* derive color component index */
  /* equivalent to ISO/IEC 13818-2 Table 7-1 */
  cc = (comp<4) ? 0 : (comp&1)+1; /* color component index */

  if (cc==0)
  {
    /* luminance */

    if (decoder->picture_structure==FRAME_PICTURE)
      if (dct_type)
      {
        /* field DCT coding */
        rfp = decoder->current_frame[0]
              +decoder->Coded_Picture_Width*(by+((comp&2)>>1)) + bx + ((comp&1)<<3);
        iincr = (decoder->Coded_Picture_Width<<1) - 8;
      }
      else
      {
        /* frame DCT coding */
        rfp = decoder->current_frame[0]
              + decoder->Coded_Picture_Width*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
        iincr = decoder->Coded_Picture_Width - 8;
      }
    else
    {
      /* field picture */
      rfp = decoder->current_frame[0]
            + (decoder->Coded_Picture_Width<<1)*(by+((comp&2)<<2)) + bx + ((comp&1)<<3);
      iincr = (decoder->Coded_Picture_Width<<1) - 8;
    }
  }
  else
  {
    /* chrominance */

    /* scale coordinates */
    if (decoder->chroma_format!=CHROMA444)
      bx >>= 1;
    if (decoder->chroma_format==CHROMA420)
      by >>= 1;
    if (decoder->picture_structure==FRAME_PICTURE)
    {
      if (dct_type && (decoder->chroma_format!=CHROMA420))
      {
        /* field DCT coding */
        rfp = decoder->current_frame[cc]
              + decoder->Chroma_Width*(by+((comp&2)>>1)) + bx + (comp&8);
        iincr = (decoder->Chroma_Width<<1) - 8;
      }
      else
      {
        /* frame DCT coding */
        rfp = decoder->current_frame[cc]
              + decoder->Chroma_Width*(by+((comp&2)<<2)) + bx + (comp&8);
        iincr = decoder->Chroma_Width - 8;
      }
    }
    else
    {
      /* field picture */
      rfp = decoder->current_frame[cc]
            + (decoder->Chroma_Width<<1)*(by+((comp&2)<<2)) + bx + (comp&8);
      iincr = (decoder->Chroma_Width<<1) - 8;
    }
  }

  bp = decoder->base.block[comp];

  if (addflag)
  {
    for (i=0; i<8; i++)
    {
      for (j=0; j<8; j++)
      {
								short x = *bp++ + *rfp;
				if (x < 0)
					x=0;
				if (x > 255)
					x = 255;

        *rfp = x;//Clip[*bp++ + *rfp];
        rfp++;
      }

      rfp+= iincr;
    }
  }
  else
  {
    for (i=0; i<8; i++)
    {
      for (j=0; j<8; j++)
			{
				short x = *bp++ + 128;
				if (x < 0)
					x=0;
				if (x > 255)
					x = 255;

        *rfp++ = x;//Clip[*bp++ + 128];
			}

      rfp+= iincr;
    }
  }
}

/* IMPLEMENTATION: set scratch pad macroblock to zero */
static void Clear_Block(MPEGDecoder *decoder, int comp)
{
  short *Block_Ptr;
  int i;

  Block_Ptr = decoder->base.block[comp];

  for (i=0; i<64; i++)
    *Block_Ptr++ = 0;
}

/* limit coefficients to -2048..2047 */
/* ISO/IEC 13818-2 section 7.4.3 and 7.4.4: Saturation and Mismatch control */
static void Saturate(short *Block_Ptr)
{
  int i, sum, val;

  sum = 0;

  /* ISO/IEC 13818-2 section 7.4.3: Saturation */
  for (i=0; i<64; i++)
  {
    val = Block_Ptr[i];

    if (val>2047)
      val = 2047;
    else if (val<-2048)
      val = -2048;

    Block_Ptr[i] = val;
    sum+= val;
  }

  /* ISO/IEC 13818-2 section 7.4.4: Mismatch control */
  if ((sum&1)==0)
    Block_Ptr[63]^= 1;

}


/* reuse old picture buffers as soon as they are no longer needed 
   based on life-time axioms of MPEG */
static void Update_Picture_Buffers(MPEGDecoder *decoder)
{                           
  int cc;              /* color component index */
  unsigned char *tmp;  /* temporary swap pointer */

  for (cc=0; cc<3; cc++)
  {
    /* B pictures do not need to be save for future reference */
    if (decoder->picture_coding_type==B_TYPE)
    {
      decoder->current_frame[cc] = decoder->auxframe[cc];
    }
    else
    {
      /* only update at the beginning of the coded frame */
      if (!decoder->Second_Field)
      {
        tmp = decoder->forward_reference_frame[cc];

        /* the previously decoded reference frame is stored
           coincident with the location where the backward 
           reference frame is stored (backwards prediction is not
           needed in P pictures) */
        decoder->forward_reference_frame[cc] = decoder->backward_reference_frame[cc];
        
        /* update pointer for potential future B pictures */
        decoder->backward_reference_frame[cc] = tmp;
      }

      /* can erase over old backward reference frame since it is not used
         in a P picture, and since any subsequent B pictures will use the 
         previously decoded I or P frame as the backward_reference_frame */
      decoder->current_frame[cc] = decoder->backward_reference_frame[cc];
    }

    /* IMPLEMENTATION:
       one-time folding of a line offset into the pointer which stores the
       memory address of the current frame saves offsets and conditional 
       branches throughout the remainder of the picture processing loop */
    if (decoder->picture_structure==BOTTOM_FIELD)
      decoder->current_frame[cc]+= (cc==0) ? decoder->Coded_Picture_Width : decoder->Chroma_Width;
  }
}


/* store last frame */

void Output_Last_Frame_of_Sequence(MPEGDecoder *decoder, int Framenum)
{
  if (decoder->Second_Field)
    printf("last frame incomplete, not stored\n");
  else
    Write_Frame(decoder, decoder->backward_reference_frame,Framenum-1);
}



void frame_reorder(MPEGDecoder *decoder, int Bitstream_Framenum, int Sequence_Framenum)
{
  /* tracking variables to insure proper output in spatial scalability */
  static int Oldref_progressive_frame, Newref_progressive_frame;

  if (Sequence_Framenum!=0)
  {
    if (decoder->picture_structure==FRAME_PICTURE || decoder->Second_Field)
    {
      if (decoder->picture_coding_type==B_TYPE)
        Write_Frame(decoder, decoder->auxframe,Bitstream_Framenum-1);
      else
      {
        Newref_progressive_frame = decoder->progressive_frame;
        decoder->progressive_frame = Oldref_progressive_frame;

        Write_Frame(decoder, decoder->forward_reference_frame,Bitstream_Framenum-1);

        Oldref_progressive_frame = decoder->progressive_frame = Newref_progressive_frame;
      }
    }
  }
  else
    Oldref_progressive_frame = decoder->progressive_frame;

}


/* ISO/IEC 13818-2 section 7.6 */
static void motion_compensation(MPEGDecoder *decoder, int MBA, int macroblock_type,  int motion_type, int PMV[2][2][2], int motion_vertical_field_select[2][2],  int dmvector[2], int stwtype, int dct_type)
{
  int bx, by;
  int comp;

  /* derive current macroblock position within picture */
  /* ISO/IEC 13818-2 section 6.3.1.6 and 6.3.1.7 */
  bx = 16*(MBA%decoder->mb_width);
  by = 16*(MBA/decoder->mb_width);

  /* motion compensation */
  if (!(macroblock_type & MACROBLOCK_INTRA))
    form_predictions(decoder, bx,by,macroblock_type,motion_type,PMV, motion_vertical_field_select,dmvector,stwtype);
  
  /* copy or add block data into picture */
  for (comp=0; comp<decoder->block_count; comp++)
  {
    /* ISO/IEC 13818-2 section Annex A: inverse DCT */
    if (Reference_IDCT_Flag)
      Reference_IDCT(decoder->base.block[comp]);
    else
      Fast_IDCT(decoder->base.block[comp]);
    
    /* ISO/IEC 13818-2 section 7.6.8: Adding prediction and coefficient data */
    Add_Block(decoder, comp,bx,by,dct_type,(macroblock_type & MACROBLOCK_INTRA)==0);
  }

}



/* ISO/IEC 13818-2 section 7.6.6 */
static void skipped_macroblock(MPEGDecoder *decoder, int dc_dct_pred[3],   int PMV[2][2][2], int *motion_type, int motion_vertical_field_select[2][2],  int *stwtype, int *macroblock_type)
{
  int comp;
  
  for (comp=0; comp<decoder->block_count; comp++)
    Clear_Block(decoder, comp);

  /* reset intra_dc predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
  dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;

  /* reset motion vector predictors */
  /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
  if (decoder->picture_coding_type==P_TYPE)
    PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;

  /* derive motion_type */
  if (decoder->picture_structure==FRAME_PICTURE)
    *motion_type = MC_FRAME;
  else
  {
    *motion_type = MC_FIELD;

    /* predict from field of same parity */
    /* ISO/IEC 13818-2 section 7.6.6.1 and 7.6.6.3: P field picture and B field
       picture */
    motion_vertical_field_select[0][0]=motion_vertical_field_select[0][1] = 
      (decoder->picture_structure==BOTTOM_FIELD);
  }

  /* skipped I are spatial-only predicted, */
  /* skipped P and B are temporal-only predicted */
  /* ISO/IEC 13818-2 section 7.7.6: Skipped macroblocks */
  *stwtype = (decoder->picture_coding_type==I_TYPE) ? 8 : 0;

 /* IMPLEMENTATION: clear MACROBLOCK_INTRA */
  *macroblock_type&= ~MACROBLOCK_INTRA;

}


/* return==-1 means go to next picture */
/* the expression "start of slice" is used throughout the normative
   body of the MPEG specification */
static int start_of_slice(MPEGDecoder *decoder, int MBAmax, int *MBA,  int *MBAinc, int dc_dct_pred[3], int PMV[2][2][2])
{
  unsigned int code;
  int slice_vert_pos_ext;

  decoder->Fault_Flag = 0;

  if (next_start_code(decoder) == 0)
		return SLICE_MORE_DATA_PLEASE;
  code = Show_Bits(&decoder->base.bitstream, 32);

  if (code<SLICE_START_CODE_MIN || code>SLICE_START_CODE_MAX)
  {
    /* only slice headers are allowed in picture_data */
		Flush_Buffer(&decoder->base.bitstream, 8);
    return SLICE_RESYNC;  /* trigger: go to next picture */
  }

  Flush_Buffer32(&decoder->base.bitstream); 

  /* decode slice header (may change quantizer_scale) */
  slice_vert_pos_ext = slice_header(decoder);

  /* decode macroblock address increment */
  *MBAinc = Get_macroblock_address_increment(decoder);

  if (decoder->Fault_Flag) 
  {
    printf("start_of_slice(): MBAinc unsuccessful\n");
    return SLICE_RESYNC;   /* trigger: go to next slice */
  }

  /* set current location */
  /* NOTE: the arithmetic used to derive macroblock_address below is
   *       equivalent to ISO/IEC 13818-2 section 6.3.17: Macroblock
   */
  *MBA = ((slice_vert_pos_ext<<7) + (code&255) - 1)*decoder->mb_width + *MBAinc - 1;
  *MBAinc = 1; /* first macroblock in slice: not skipped */

  /* reset all DC coefficient and motion vector predictors */
  /* reset all DC coefficient and motion vector predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
  dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;
  
  /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
  PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
  PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;

  /* successfull: trigger decode macroblocks in slice */
  return SLICE_MACROBLOCK_DECODE;
}


/* ISO/IEC 13818-2 sections 7.2 through 7.5 */
static int decode_macroblock(MPEGDecoder *decoder, int *macroblock_type, int *stwtype, int *stwclass,
  int *motion_type, int *dct_type, int PMV[2][2][2], int dc_dct_pred[3], 
  int motion_vertical_field_select[2][2], int dmvector[2])
{
  /* locals */
  int quantizer_scale_code; 
  int comp;

  int motion_vector_count; 
  int mv_format; 
  int dmv; 
  int mvscale;
  int coded_block_pattern;

  /* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */
  macroblock_modes(decoder, macroblock_type, stwtype, stwclass, motion_type, &motion_vector_count, &mv_format, &dmv, &mvscale, dct_type);

  if (decoder->Fault_Flag)
		return SLICE_RESYNC;  /* trigger: go to next slice */

  if (*macroblock_type & MACROBLOCK_QUANT)
  {
    quantizer_scale_code = Get_Bits(&decoder->base.bitstream, 5);


    /* ISO/IEC 13818-2 section 7.4.2.2: Quantizer scale factor */
    if (decoder->base.MPEG2_Flag)
      decoder->base.quantizer_scale =
      decoder->base.q_scale_type ? Non_Linear_quantizer_scale[quantizer_scale_code] 
       : (quantizer_scale_code << 1);
    else
      decoder->base.quantizer_scale = quantizer_scale_code;
  }

  /* motion vectors */


  /* ISO/IEC 13818-2 section 6.3.17.2: Motion vectors */

  /* decode forward motion vectors */
  if ((*macroblock_type & MACROBLOCK_MOTION_FORWARD) 
    || ((*macroblock_type & MACROBLOCK_INTRA) 
    && decoder->concealment_motion_vectors))
  {
    if (decoder->base.MPEG2_Flag)
      motion_vectors(decoder, PMV,dmvector,motion_vertical_field_select, 0,motion_vector_count,mv_format,decoder->f_code[0][0]-1,decoder->f_code[0][1]-1, dmv,mvscale);
    else
      motion_vector(decoder, PMV[0][0],dmvector,      decoder->forward_f_code-1,decoder->forward_f_code-1,0,0,decoder->full_pel_forward_vector);
  }

  if (decoder->Fault_Flag) 
		return SLICE_RESYNC;  /* trigger: go to next slice */

  /* decode backward motion vectors */
  if (*macroblock_type & MACROBLOCK_MOTION_BACKWARD)
  {
    if (decoder->base.MPEG2_Flag)
      motion_vectors(decoder, PMV,dmvector,motion_vertical_field_select, 1,motion_vector_count,mv_format,decoder->f_code[1][0]-1,decoder->f_code[1][1]-1,0, mvscale);
    else
      motion_vector(decoder, PMV[0][1],dmvector, decoder->backward_f_code-1,decoder->backward_f_code-1,0,0,decoder->full_pel_backward_vector);
  }

  if (decoder->Fault_Flag) return SLICE_RESYNC;  /* trigger: go to next slice */

  if ((*macroblock_type & MACROBLOCK_INTRA) && decoder->concealment_motion_vectors)
    Flush_Buffer(&decoder->base.bitstream, 1); /* remove marker_bit */

  /* macroblock_pattern */
  /* ISO/IEC 13818-2 section 6.3.17.4: Coded block pattern */
  if (*macroblock_type & MACROBLOCK_PATTERN)
  {
    coded_block_pattern = Get_coded_block_pattern(decoder);

    if (decoder->chroma_format==CHROMA422)
    {
      /* coded_block_pattern_1 */
      coded_block_pattern = (coded_block_pattern<<2) | Get_Bits(&decoder->base.bitstream, 2); 
     }
     else if (decoder->chroma_format==CHROMA444)
     {
      /* coded_block_pattern_2 */
      coded_block_pattern = (coded_block_pattern<<6) | Get_Bits(&decoder->base.bitstream, 6); 
    }
  }
  else
    coded_block_pattern = (*macroblock_type & MACROBLOCK_INTRA) ? 
      (1<<decoder->block_count)-1 : 0;

  if (decoder->Fault_Flag)
		return SLICE_RESYNC;  /* trigger: go to next slice */

  /* decode blocks */
  for (comp=0; comp<decoder->block_count; comp++)
  {
    Clear_Block(decoder, comp);

    if (coded_block_pattern & (1<<(decoder->block_count-1-comp)))
    {
      if (*macroblock_type & MACROBLOCK_INTRA)
      {
        if (decoder->base.MPEG2_Flag)
          Decode_MPEG2_Intra_Block(decoder, comp,dc_dct_pred);
        else
          Decode_MPEG1_Intra_Block(decoder, comp,dc_dct_pred);
      }
      else
      {
        if (decoder->base.MPEG2_Flag)
          Decode_MPEG2_Non_Intra_Block(decoder, comp);
        else
          Decode_MPEG1_Non_Intra_Block(decoder, comp);
      }

      if (decoder->Fault_Flag)
				return SLICE_RESYNC;  /* trigger: go to next slice */
    }
  }

  if(decoder->picture_coding_type==D_TYPE)
  {
    /* remove end_of_macroblock (always 1, prevents startcode emulation) */
    /* ISO/IEC 11172-2 section 2.4.2.7 and 2.4.3.6 */
    marker_bit(decoder, "D picture end_of_macroblock bit");
  }

  /* reset intra_dc predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */
  if (!(*macroblock_type & MACROBLOCK_INTRA))
    dc_dct_pred[0]=dc_dct_pred[1]=dc_dct_pred[2]=0;

  /* reset motion vector predictors */
  if ((*macroblock_type & MACROBLOCK_INTRA) && !decoder->concealment_motion_vectors)
  {
    /* intra mb without concealment motion vectors */
    /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
    PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
    PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
  }

  /* special "No_MC" macroblock_type case */
  /* ISO/IEC 13818-2 section 7.6.3.5: Prediction in P pictures */
  if ((decoder->picture_coding_type==P_TYPE) 
    && !(*macroblock_type & (MACROBLOCK_MOTION_FORWARD|MACROBLOCK_INTRA)))
  {
    /* non-intra mb without forward mv in a P picture */
    /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
    PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;

    /* derive motion_type */
    /* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes, frame_motion_type */
    if (decoder->picture_structure==FRAME_PICTURE)
      *motion_type = MC_FRAME;
    else
    {
      *motion_type = MC_FIELD;
      /* predict from field of same parity */
      motion_vertical_field_select[0][0] = (decoder->picture_structure==BOTTOM_FIELD);
    }
  }

  if (*stwclass==4)
  {
    /* purely spatially predicted macroblock */
    /* ISO/IEC 13818-2 section 7.7.5.1: Resetting motion vector predictions */
    PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
    PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
  }

  /* successfully decoded macroblock */
  return SLICE_MACROBLOCK_DECODE;

} /* decode_macroblock */



/* gethdr.c, header decoding                                                */

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

/* private prototypes */
static void sequence_header(MPEGDecoder *decoder);
static void group_of_pictures_header(MPEGDecoder *decoder);
static void picture_header(struct mpeg_decoder *decoder);
static void extension_and_user_data(MPEGDecoder *decoder);
static void sequence_extension(MPEGDecoder *decoder);
static void sequence_display_extension(MPEGDecoder *decoder);
static void quant_matrix_extension(MPEGDecoder *decoder);
static void sequence_scalable_extension(MPEGDecoder *decoder);
static void picture_display_extension(MPEGDecoder *decoder);
static void picture_coding_extension(MPEGDecoder *decoder);
static void picture_spatial_scalable_extension(MPEGDecoder *decoder);
static void picture_temporal_scalable_extension(MPEGDecoder *decoder);
static int  extra_bit_information(MPEGDecoder *decoder);
static void copyright_extension(MPEGDecoder *decoder);
static void user_data(MPEGDecoder *decoder);


#define RESERVED    -1 
static double frame_rate_Table[16] =
{
  0.0,
  ((23.0*1000.0)/1001.0),
  24.0,
  25.0,
  ((30.0*1000.0)/1001.0),
  30.0,
  50.0,
  ((60.0*1000.0)/1001.0),
  60.0,
 
  RESERVED,
  RESERVED,
  RESERVED,
  RESERVED,
  RESERVED,
  RESERVED,
  RESERVED
};

/*
 * decode headers from one input stream
 * until an End of Sequence or picture start code
 * is found
 */
int Get_Hdr(MPEGDecoder *decoder)
{
  unsigned int code;

  while(decoder->base.bitstream.numBits)
  {
    /* look for next_start_code */
    if (next_start_code(decoder) == 0)
			return 0;
    code = Get_Bits32(&decoder->base.bitstream);
  
    switch (code)
    {
    case SEQUENCE_HEADER_CODE:
      sequence_header(decoder);
			Initialize_Sequence(decoder);
      break;
    case GROUP_START_CODE:
      group_of_pictures_header(decoder);
      break;
    case PICTURE_START_CODE:
      picture_header(decoder);
      return 1;
      break;
    case SEQUENCE_END_CODE:
			Output_Last_Frame_of_Sequence(decoder, decoder->bitstream_framenum);
			Deinitialize_Sequence(decoder);
      return 0;
      break;
    default:
      break;
    }
  }
	return 0;
}


/* align to start of next next_start_code */

int next_start_code(MPEGDecoder *decoder)
{
  /* byte align */
	int num_bytes;
	if (decoder->base.bitstream.numBits < 7)
		return 0;
  Flush_Buffer(&decoder->base.bitstream, decoder->base.bitstream.numBits&7);
	num_bytes = decoder->base.bitstream.numBits>>3;
  while (num_bytes && decoder->base.bitstream.numBits && Show_Bits(&decoder->base.bitstream, 24)!=0x01L)
	{
    Flush_Buffer(&decoder->base.bitstream, 8);
		num_bytes--;
	}
	return num_bytes?1:0;
}


/* decode sequence header */

static void sequence_header(MPEGDecoder *decoder)
{
  int i;

  decoder->horizontal_size             = Get_Bits(&decoder->base.bitstream, 12);
  decoder->vertical_size               = Get_Bits(&decoder->base.bitstream, 12);
  decoder->aspect_ratio_information    = Get_Bits(&decoder->base.bitstream, 4);
  decoder->frame_rate_code             = Get_Bits(&decoder->base.bitstream, 4);
  decoder->bit_rate_value              = Get_Bits(&decoder->base.bitstream, 18);
  marker_bit(decoder, "sequence_header()");
  decoder->vbv_buffer_size             = Get_Bits(&decoder->base.bitstream, 10);
  decoder->constrained_parameters_flag = Get_Bits(&decoder->base.bitstream, 1);

  if (Get_Bits(&decoder->base.bitstream, 1)) // load_intra_quantizer_matrix;
  {
    for (i=0; i<64; i++)
      decoder->base.intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(&decoder->base.bitstream, 8);
  }
  else
  {
    for (i=0; i<64; i++)
      decoder->base.intra_quantizer_matrix[i] = default_intra_quantizer_matrix[i];
  }

  if (Get_Bits(&decoder->base.bitstream, 1)) // load_non_intra_quantizer_matrix
  {
    for (i=0; i<64; i++)
      decoder->base.non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(&decoder->base.bitstream, 8);
  }
  else
  {
    for (i=0; i<64; i++)
      decoder->base.non_intra_quantizer_matrix[i] = 16;
  }

  /* copy luminance to chrominance matrices */
  for (i=0; i<64; i++)
  {
    decoder->base.chroma_intra_quantizer_matrix[i] =      decoder->base.intra_quantizer_matrix[i];
    decoder->base.chroma_non_intra_quantizer_matrix[i] =      decoder->base.non_intra_quantizer_matrix[i];
  }

  extension_and_user_data(decoder);
}



/* decode group of pictures header */
/* ISO/IEC 13818-2 section 6.2.2.6 */
static void group_of_pictures_header(MPEGDecoder *decoder)
{
  decoder->drop_flag   = Get_Bits(&decoder->base.bitstream, 1);
  decoder->hour        = Get_Bits(&decoder->base.bitstream, 5);
  decoder->minute      = Get_Bits(&decoder->base.bitstream, 6);
  marker_bit(decoder, "group_of_pictures_header()");
  decoder->sec         = Get_Bits(&decoder->base.bitstream, 6);
  decoder->frame       = Get_Bits(&decoder->base.bitstream, 6);
  decoder->closed_gop  = Get_Bits(&decoder->base.bitstream, 1);
  decoder->broken_link = Get_Bits(&decoder->base.bitstream, 1);


  extension_and_user_data(decoder);

}


/* decode picture header */

/* ISO/IEC 13818-2 section 6.2.3 */
static void picture_header(MPEGDecoder *decoder)
{
  int Extra_Information_Byte_Count;

  decoder->temporal_reference  = Get_Bits(&decoder->base.bitstream, 10);
  decoder->picture_coding_type = Get_Bits(&decoder->base.bitstream, 3);
  decoder->vbv_delay           = Get_Bits(&decoder->base.bitstream, 16);

  if (decoder->picture_coding_type==P_TYPE || decoder->picture_coding_type==B_TYPE)
  {
    decoder->full_pel_forward_vector = Get_Bits(&decoder->base.bitstream, 1);
    decoder->forward_f_code = Get_Bits(&decoder->base.bitstream, 3);
  }
  if (decoder->picture_coding_type==B_TYPE)
  {
    decoder->full_pel_backward_vector = Get_Bits(&decoder->base.bitstream, 1);
    decoder->backward_f_code = Get_Bits(&decoder->base.bitstream, 3);
  }
  Extra_Information_Byte_Count = 
    extra_bit_information(decoder);
  
  extension_and_user_data(decoder);
}

/* decode slice header */

/* ISO/IEC 13818-2 section 6.2.4 */
int slice_header(MPEGDecoder *decoder)
{
  int slice_vertical_position_extension;
  int quantizer_scale_code;
  int slice_picture_id_enable = 0;
  int slice_picture_id = 0;
  int extra_information_slice = 0;

    slice_vertical_position_extension =
    (decoder->base.MPEG2_Flag && decoder->vertical_size>2800) ? Get_Bits(&decoder->base.bitstream, 3) : 0;

  quantizer_scale_code = Get_Bits(&decoder->base.bitstream, 5);
  decoder->base.quantizer_scale =
    decoder->base.MPEG2_Flag ? (decoder->base.q_scale_type ? Non_Linear_quantizer_scale[quantizer_scale_code] : quantizer_scale_code<<1) : quantizer_scale_code;

  /* slice_id introduced in March 1995 as part of the video corridendum
     (after the IS was drafted in November 1994) */
  if (Get_Bits(&decoder->base.bitstream, 1))
  {
    decoder->base.intra_slice = Get_Bits(&decoder->base.bitstream, 1);

    slice_picture_id_enable = Get_Bits(&decoder->base.bitstream, 1);
	slice_picture_id = Get_Bits(&decoder->base.bitstream, 6);

    extra_information_slice = extra_bit_information(decoder);
  }
  else
    decoder->base.intra_slice = 0;



  return slice_vertical_position_extension;
}


/* decode extension and user data */
/* ISO/IEC 13818-2 section 6.2.2.2 */
static void extension_and_user_data(MPEGDecoder *decoder)
{
  int code,ext_ID;

  if (next_start_code(decoder) == 0)
		return;

  while ((code = (Show_Bits(&decoder->base.bitstream, 32)))==EXTENSION_START_CODE || code==USER_DATA_START_CODE)
  {
    if (code==EXTENSION_START_CODE)
    {
      Flush_Buffer32(&decoder->base.bitstream);
      ext_ID = Get_Bits(&decoder->base.bitstream, 4);
      switch (ext_ID)
      {
      case SEQUENCE_EXTENSION_ID:
        sequence_extension(decoder);
        break;
      case SEQUENCE_DISPLAY_EXTENSION_ID:
        sequence_display_extension(decoder);
        break;
      case QUANT_MATRIX_EXTENSION_ID:
        quant_matrix_extension(decoder);
        break;
      case SEQUENCE_SCALABLE_EXTENSION_ID:
        sequence_scalable_extension(decoder);
        break;
      case PICTURE_DISPLAY_EXTENSION_ID:
        picture_display_extension(decoder);
        break;
      case PICTURE_CODING_EXTENSION_ID:
        picture_coding_extension(decoder);
        break;
      case PICTURE_SPATIAL_SCALABLE_EXTENSION_ID:
        picture_spatial_scalable_extension(decoder);
        break;
      case PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID:
        picture_temporal_scalable_extension(decoder);
        break;
      case COPYRIGHT_EXTENSION_ID:
        copyright_extension(decoder);
        break;
     default:
        fprintf(stderr,"reserved extension start code ID %d\n",ext_ID);
        break;
      }
      if (next_start_code(decoder) == 0)
				return;
    }
    else
    {
      Flush_Buffer32(&decoder->base.bitstream);
      user_data(decoder);
    }
  }
}


/* decode sequence extension */

/* ISO/IEC 13818-2 section 6.2.2.3 */
static void sequence_extension(MPEGDecoder *decoder)
{
  int horizontal_size_extension;
  int vertical_size_extension;
  int bit_rate_extension;
  int vbv_buffer_size_extension;


  decoder->base.MPEG2_Flag = 1;
 
  decoder->profile_and_level_indication = Get_Bits(&decoder->base.bitstream, 8);
  decoder->progressive_sequence         = Get_Bits(&decoder->base.bitstream, 1);
  decoder->chroma_format                = Get_Bits(&decoder->base.bitstream, 2);
  horizontal_size_extension    = Get_Bits(&decoder->base.bitstream, 2);
  vertical_size_extension      = Get_Bits(&decoder->base.bitstream, 2);
  bit_rate_extension           = Get_Bits(&decoder->base.bitstream, 12);
  marker_bit(decoder, "sequence_extension");
  vbv_buffer_size_extension    = Get_Bits(&decoder->base.bitstream, 8);
  decoder->low_delay                    = Get_Bits(&decoder->base.bitstream, 1);
  decoder->frame_rate_extension_n       = Get_Bits(&decoder->base.bitstream, 2);
  decoder->frame_rate_extension_d       = Get_Bits(&decoder->base.bitstream, 5);

  decoder->frame_rate = frame_rate_Table[decoder->frame_rate_code] *
    ((decoder->frame_rate_extension_n+1)/(decoder->frame_rate_extension_d+1));

  /* special case for 422 profile & level must be made */
  if((decoder->profile_and_level_indication>>7) & 1)
  {  /* escape bit of profile_and_level_indication set */
  
    /* 4:2:2 Profile @ Main Level */
    if((decoder->profile_and_level_indication&15)==5)
    {
      decoder->profile = PROFILE_422;
      decoder->level   = MAIN_LEVEL;  
    }
  }
  else
  {
    decoder->profile = decoder->profile_and_level_indication >> 4;  /* Profile is upper nibble */
    decoder->level   = decoder->profile_and_level_indication & 0xF;  /* Level is lower nibble */
  }
  
 
  decoder->horizontal_size = (horizontal_size_extension<<12) | (decoder->horizontal_size&0x0fff);
  decoder->vertical_size = (vertical_size_extension<<12) | (decoder->vertical_size&0x0fff);


  /* ISO/IEC 13818-2 does not define bit_rate_value to be composed of
   * both the original bit_rate_value parsed in sequence_header() and
   * the optional bit_rate_extension in sequence_extension_header(). 
   * However, we use it for bitstream verification purposes. 
   */

  decoder->bit_rate_value += (bit_rate_extension << 18);
  decoder->bit_rate = ((double) decoder->bit_rate_value) * 400.0;
  decoder->vbv_buffer_size += (vbv_buffer_size_extension << 10);

}


/* decode sequence display extension */

static void sequence_display_extension(MPEGDecoder *decoder)
{

    decoder->video_format      = Get_Bits(&decoder->base.bitstream, 3);
  decoder->color_description = Get_Bits(&decoder->base.bitstream, 1);

  if (decoder->color_description)
  {
    decoder->color_primaries          = Get_Bits(&decoder->base.bitstream, 8);
    decoder->transfer_characteristics = Get_Bits(&decoder->base.bitstream, 8);
    decoder->matrix_coefficients      = Get_Bits(&decoder->base.bitstream, 8);
  }

  decoder->display_horizontal_size = Get_Bits(&decoder->base.bitstream, 14);
  marker_bit(decoder, "sequence_display_extension");
  decoder->display_vertical_size   = Get_Bits(&decoder->base.bitstream, 14);

}


/* decode quant matrix entension */
/* ISO/IEC 13818-2 section 6.2.3.2 */
static void quant_matrix_extension(MPEGDecoder *decoder)
{
  int i;

  if (Get_Bits(&decoder->base.bitstream, 1)) // load_intra_quantizer_matrix
  {
    for (i=0; i<64; i++)
    {
      decoder->base.chroma_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
      = decoder->base.intra_quantizer_matrix[scan[ZIG_ZAG][i]]
      = Get_Bits(&decoder->base.bitstream, 8);
    }
  }

  if (Get_Bits(&decoder->base.bitstream, 1)) // load_non_intra_quantizer_matrix
  {
    for (i=0; i<64; i++)
    {
      decoder->base.chroma_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
      = decoder->base.non_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
      = Get_Bits(&decoder->base.bitstream, 8);
    }
  }

  if((decoder->base.load_chroma_intra_quantizer_matrix = Get_Bits(&decoder->base.bitstream, 1)))
  {
    for (i=0; i<64; i++)
      decoder->base.chroma_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(&decoder->base.bitstream, 8);
  }

  if((decoder->base.load_chroma_non_intra_quantizer_matrix = Get_Bits(&decoder->base.bitstream, 1)))
  {
    for (i=0; i<64; i++)
      decoder->base.chroma_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(&decoder->base.bitstream, 8);
  }

}


/* decode sequence scalable extension */
/* ISO/IEC 13818-2   section 6.2.2.5 */
static void sequence_scalable_extension(MPEGDecoder *decoder)
{
  /* values (without the +1 offset) of scalable_mode are defined in 
     Table 6-10 of ISO/IEC 13818-2 */
	    Error("Scalability not implemented\n");
}


/* decode picture display extension */
/* ISO/IEC 13818-2 section 6.2.3.3. */
static void picture_display_extension(MPEGDecoder *decoder)
{
  int i;
  int number_of_frame_center_offsets;

  /* based on ISO/IEC 13818-2 section 6.3.12 
    (November 1994) Picture display extensions */

  /* derive number_of_frame_center_offsets */
  if(decoder->progressive_sequence)
  {
    if(decoder->repeat_first_field)
    {
      if(decoder->top_field_first)
        number_of_frame_center_offsets = 3;
      else
        number_of_frame_center_offsets = 2;
    }
    else
    {
      number_of_frame_center_offsets = 1;
    }
  }
  else
  {
    if(decoder->picture_structure!=FRAME_PICTURE)
    {
      number_of_frame_center_offsets = 1;
    }
    else
    {
      if(decoder->repeat_first_field)
        number_of_frame_center_offsets = 3;
      else
        number_of_frame_center_offsets = 2;
    }
  }


  /* now parse */
  for (i=0; i<number_of_frame_center_offsets; i++)
  {
    decoder->frame_center_horizontal_offset[i] = Get_Bits(&decoder->base.bitstream, 16);
    marker_bit(decoder, "picture_display_extension, first marker bit");
    
    decoder->frame_center_vertical_offset[i]   = Get_Bits(&decoder->base.bitstream, 16);
    marker_bit(decoder, "picture_display_extension, second marker bit");
  }

}


/* decode picture coding extension */
static void picture_coding_extension(MPEGDecoder *decoder)
{
  decoder->f_code[0][0] = Get_Bits(&decoder->base.bitstream, 4);
  decoder->f_code[0][1] = Get_Bits(&decoder->base.bitstream, 4);
  decoder->f_code[1][0] = Get_Bits(&decoder->base.bitstream, 4);
  decoder->f_code[1][1] = Get_Bits(&decoder->base.bitstream, 4);

  decoder->intra_dc_precision         = Get_Bits(&decoder->base.bitstream, 2);
  decoder->picture_structure          = Get_Bits(&decoder->base.bitstream, 2);
  decoder->top_field_first            = Get_Bits(&decoder->base.bitstream, 1);
  decoder->frame_pred_frame_dct       = Get_Bits(&decoder->base.bitstream, 1);
  decoder->concealment_motion_vectors = Get_Bits(&decoder->base.bitstream, 1);
  decoder->base.q_scale_type           = Get_Bits(&decoder->base.bitstream, 1);
  decoder->intra_vlc_format           = Get_Bits(&decoder->base.bitstream, 1);
  decoder->base.alternate_scan         = Get_Bits(&decoder->base.bitstream, 1);
  decoder->repeat_first_field         = Get_Bits(&decoder->base.bitstream, 1);
  decoder->chroma_420_type            = Get_Bits(&decoder->base.bitstream, 1);
  decoder->progressive_frame          = Get_Bits(&decoder->base.bitstream, 1);
  decoder->composite_display_flag     = Get_Bits(&decoder->base.bitstream, 1);

  if (decoder->composite_display_flag)
  {
    decoder->v_axis            = Get_Bits(&decoder->base.bitstream, 1);
    decoder->field_sequence    = Get_Bits(&decoder->base.bitstream, 3);
    decoder->sub_carrier       = Get_Bits(&decoder->base.bitstream, 1);
    decoder->burst_amplitude   = Get_Bits(&decoder->base.bitstream, 7);
    decoder->sub_carrier_phase = Get_Bits(&decoder->base.bitstream, 8);
  }

}


/* decode picture spatial scalable extension */
/* ISO/IEC 13818-2 section 6.2.3.5. */
static void picture_spatial_scalable_extension(MPEGDecoder *decoder)
{
	Error("spatial scalability not supported\n");
}


/* decode picture temporal scalable extension
 *
 * not implemented
 */
/* ISO/IEC 13818-2 section 6.2.3.4. */
static void picture_temporal_scalable_extension(MPEGDecoder *decoder)
{
  Error("temporal scalability not supported\n");
}


/* decode extra bit information */
/* ISO/IEC 13818-2 section 6.2.3.4. */
static int extra_bit_information(MPEGDecoder *decoder)
{
  int Byte_Count = 0;

  while (Get_Bits1(&decoder->base.bitstream))
  {
    Flush_Buffer(&decoder->base.bitstream, 8);
    Byte_Count++;
  }

  return(Byte_Count);
}



/* ISO/IEC 13818-2 section 5.3 */
/* Purpose: this function is mainly designed to aid in bitstream conformance
   testing.  A simple Flush_Buffer(&decoder->base.bitstream, 1) would do */
void marker_bit(MPEGDecoder *decoder, char *text)
{
  int marker;

  marker = Get_Bits(&decoder->base.bitstream, 1);

#ifdef VERIFY  
  if(!marker)
    printf("ERROR: %s--marker_bit set to 0",text);
#endif
}


/* ISO/IEC 13818-2  sections 6.3.4.1 and 6.2.2.2.2 */
static void user_data(MPEGDecoder *decoder)
{
  /* skip ahead to the next start code */
  next_start_code(decoder);
}



/* Copyright extension */
/* ISO/IEC 13818-2 section 6.2.3.6. */
/* (header added in November, 1994 to the IS document) */


static void copyright_extension(MPEGDecoder *decoder)
{
  int reserved_data;

  decoder->copyright_flag =       Get_Bits(&decoder->base.bitstream, 1); 
  decoder->copyright_identifier = Get_Bits(&decoder->base.bitstream, 8);
  decoder->original_or_copy =     Get_Bits(&decoder->base.bitstream, 1);
  
  /* reserved */
  reserved_data = Get_Bits(&decoder->base.bitstream, 7);

  marker_bit(decoder, "copyright_extension(), first marker bit");
  decoder->copyright_number_1 =   Get_Bits(&decoder->base.bitstream, 20);
  marker_bit(decoder, "copyright_extension(), second marker bit");
  decoder->copyright_number_2 =   Get_Bits(&decoder->base.bitstream, 22);
  marker_bit(decoder, "copyright_extension(), third marker bit");
  decoder->copyright_number_3 =   Get_Bits(&decoder->base.bitstream, 22);



#ifdef VERIFY
  verify_copyright_extension++;
#endif /* VERIFY */
}




#pragma once
/* global.h, global variables                                               */

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

#include "mpeg2dec.h"

/* choose between declaration (GLOBAL undefined)
 * and definition (GLOBAL defined)
 * GLOBAL is defined in exactly one file mpeg2dec.c)
 */

#ifndef GLOBAL
#define EXTERN extern
#else
#define EXTERN
#endif

/* prototypes of global functions */


enum
{
	SLICE_ALL_MACROBLOCKS_DECODED = -1,
	SLICE_MACROBLOCK_DECODE = -1,
	SLICE_RESYNC = 0,
	SLICE_MORE_DATA_PLEASE = 1,
};

/* Get_Bits.c */
unsigned int Num_Bits(struct bit_stream *bitstream);
unsigned int Show_Bits(struct bit_stream *bitstream, int n);
unsigned int Get_Bits1(struct bit_stream *bitstream);
void Flush_Buffer(struct bit_stream *bitstream, int n);
unsigned int Get_Bits(struct bit_stream *bitstream, int n);
int Get_Byte(struct bit_stream *bitstream);
int Get_Word(struct bit_stream *bitstream);
int Get_Long (struct bit_stream *bitstream);
void Flush_Buffer32 (struct bit_stream *bitstream);
unsigned int Get_Bits32 (struct bit_stream *bitstream);

/* systems.c */
void Next_Packet (struct bit_stream *bitstream);

/* getblk.c */
void Decode_MPEG1_Intra_Block (struct mpeg_decoder *decoder, int comp, int dc_dct_pred[]);
void Decode_MPEG1_Non_Intra_Block (struct mpeg_decoder *decoder, int comp);
void Decode_MPEG2_Intra_Block (struct mpeg_decoder *decoder, int comp, int dc_dct_pred[]);
void Decode_MPEG2_Non_Intra_Block (struct mpeg_decoder *decoder, int comp);

/* gethdr.c */
int Get_Hdr(struct mpeg_decoder *decoder);
int next_start_code (struct mpeg_decoder *decoder);
int slice_header(struct mpeg_decoder *decoder);
void marker_bit(struct mpeg_decoder *decoder, char *text);

/* getpic.c */
int Decode_Picture(struct mpeg_decoder *decoder, int bitstream_framenum, int sequence_framenum);
void Output_Last_Frame_of_Sequence(struct mpeg_decoder *decoder, int framenum);

/* getvlc.c */
int Get_macroblock_type(struct mpeg_decoder *decoder);
int Get_motion_code (struct mpeg_decoder *decoder);
int Get_dmvector (struct mpeg_decoder *decoder);
int Get_coded_block_pattern (struct mpeg_decoder *decoder);
int Get_macroblock_address_increment (struct mpeg_decoder *decoder);
int Get_Luma_DC_dct_diff (struct mpeg_decoder *decoder);
int Get_Chroma_DC_dct_diff (struct mpeg_decoder *decoder);

/* idct.c */
void Fast_IDCT (short *block);
void Initialize_Fast_IDCT (void);

/* Reference_IDCT.c */
void Initialize_Reference_IDCT (void);
void Reference_IDCT (short *block);

/* motion.c */
void motion_vectors(struct mpeg_decoder *decoder, int PMV[2][2][2], int dmvector[2],  int motion_vertical_field_select[2][2], int s, int motion_vector_count,  int mv_format, int h_r_size, int v_r_size, int dmv, int mvscale);
void motion_vector(struct mpeg_decoder *decoder, int *PMV, int *dmvector,  int h_r_size, int v_r_size, int dmv, int mvscale, int full_pel_vector);
void Dual_Prime_Arithmetic(struct mpeg_decoder *decoder, int DMV[][2], int *dmvector, int mvx, int mvy);

/* mpeg2dec.c */
void Error (char *text);
void Warning (char *text);
void Initialize_Sequence(struct mpeg_decoder *decoder);
void Deinitialize_Sequence (struct mpeg_decoder  *decoder);

/* recon.c */
void form_predictions(struct mpeg_decoder *decoder, int bx, int by, int macroblock_type,   int motion_type, int PMV[2][2][2], int motion_vertical_field_select[2][2],  int dmvector[2], int stwtype);

/* store.c */
void Write_Frame(struct mpeg_decoder *decoder, unsigned char *src[], int frame);


/* zig-zag and alternate scan patterns */
static const unsigned char scan[2][64]=
{
  { /* Zig-Zag scan pattern  */
    0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
    12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
    35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
  },
  { /* Alternate scan pattern */
    0,8,16,24,1,9,2,10,17,25,32,40,48,56,57,49,
    41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
    51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
    53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63
  }
}
;

/* default intra quantization matrix */
static const unsigned char default_intra_quantizer_matrix[64]=
{
  8, 16, 19, 22, 26, 27, 29, 34,
  16, 16, 22, 24, 27, 29, 34, 37,
  19, 22, 26, 27, 29, 34, 34, 38,
  22, 22, 26, 27, 29, 34, 37, 40,
  22, 26, 27, 29, 32, 35, 40, 48,
  26, 27, 29, 32, 35, 40, 48, 58,
  26, 27, 29, 34, 38, 46, 56, 69,
  27, 29, 35, 38, 46, 56, 69, 83
};

/* non-linear quantization coefficient table */
static const unsigned char Non_Linear_quantizer_scale[32]=
{
   0, 1, 2, 3, 4, 5, 6, 7,
   8,10,12,14,16,18,20,22,
  24,28,32,36,40,44,48,52,
  56,64,72,80,88,96,104,112
};

/* color space conversion coefficients
 * for YCbCr -> RGB mapping
 *
 * entries are {crv,cbu,cgu,cgv}
 *
 * crv=(255/224)*65536*(1-cr)/0.5
 * cbu=(255/224)*65536*(1-cb)/0.5
 * cgu=(255/224)*65536*(cb/cg)*(1-cb)/0.5
 * cgv=(255/224)*65536*(cr/cg)*(1-cr)/0.5
 *
 * where Y=cr*R+cg*G+cb*B (cr+cg+cb=1)
 */

/* ISO/IEC 13818-2 section 6.3.6 sequence_display_extension() */

static const int Inverse_Table_6_9[8][4]=
{
  {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
  {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
  {104597, 132201, 25675, 53279}, /* unspecified */
  {104597, 132201, 25675, 53279}, /* reserved */
  {104448, 132798, 24759, 53109}, /* FCC */
  {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
  {104597, 132201, 25675, 53279}, /* SMPTE 170M */
  {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};


/* decoder operation control flags */
EXTERN int Reference_IDCT_Flag;

/* buffers for multiuse purposes */
EXTERN char Error_Text[256];
extern unsigned char *Clip;

typedef struct bit_stream
{
	const unsigned char *data;
	unsigned int numBits;
} BitStream;

/* layer specific variables (needed for SNR and DP scalability) */
typedef struct layer_data 
{
  /* bit input */
	BitStream bitstream;

  /* sequence header and quant_matrix_extension() */
  int intra_quantizer_matrix[64];
  int non_intra_quantizer_matrix[64];
  int chroma_intra_quantizer_matrix[64];
  int chroma_non_intra_quantizer_matrix[64];
  
  int load_chroma_intra_quantizer_matrix;
  int load_chroma_non_intra_quantizer_matrix;

  int MPEG2_Flag;

  /* picture coding extension */
  int q_scale_type;
  int alternate_scan;

  /* slice/macroblock */
  int quantizer_scale;
  int intra_slice;
  short block[12][64];
} LayerData;


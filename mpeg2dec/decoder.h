#pragma once
#include "global.h"

typedef struct mpeg_decoder
{
	int start_code;
	LayerData base;

	/* pointers to generic picture buffers */
	unsigned char *backward_reference_frame[3];
	unsigned char *forward_reference_frame[3];

	unsigned char *auxframe[3];
	unsigned char *current_frame[3];

	/* non-normative variables derived from normative elements */
	int Coded_Picture_Width;
	int Coded_Picture_Height;
	int Chroma_Width;
	int Chroma_Height;
	int block_count;
	int Second_Field;
	int profile, level;

	/* ISO/IEC 13818-2 section 6.2.2.6: group_of_pictures_header()  */
	int drop_flag;
	int hour;
	int minute;
	int sec;
	int frame;
	int closed_gop;
	int broken_link;

	/* normative derived variables (as per ISO/IEC 13818-2) */
	int horizontal_size;
	int vertical_size;
	int mb_width;
	int mb_height;
	double bit_rate;
	double frame_rate; 

	/* ISO/IEC 13818-2 section 6.2.2.1:  sequence_header() */
	int aspect_ratio_information;
	int frame_rate_code; 
	int bit_rate_value; 
	int vbv_buffer_size;
	int constrained_parameters_flag;

	/* ISO/IEC 13818-2 section 6.2.2.3:  sequence_extension() */
	int profile_and_level_indication;
	int progressive_sequence;
	int chroma_format;
	int low_delay;
	int frame_rate_extension_n;
	int frame_rate_extension_d;

	/* ISO/IEC 13818-2 section 6.2.2.4:  sequence_display_extension() */
	int video_format;  
	int color_description;
	int color_primaries;
	int transfer_characteristics;
	int matrix_coefficients;
	int display_horizontal_size;
	int display_vertical_size;

	/* ISO/IEC 13818-2 section 6.2.3: picture_header() */
	int temporal_reference;
	int picture_coding_type;
	int vbv_delay;
	int full_pel_forward_vector;
	int forward_f_code;
	int full_pel_backward_vector;
	int backward_f_code;


	/* ISO/IEC 13818-2 section 6.2.3.1: picture_coding_extension() header */
	int f_code[2][2];
	int intra_dc_precision;
	int picture_structure;
	int top_field_first;
	int frame_pred_frame_dct;
	int concealment_motion_vectors;

	int intra_vlc_format;

	int repeat_first_field;

	int chroma_420_type;
	int progressive_frame;
	int composite_display_flag;
	int v_axis;
	int field_sequence;
	int sub_carrier;
	int burst_amplitude;
	int sub_carrier_phase;

	/* ISO/IEC 13818-2 section 6.2.3.3: picture_display_extension() header */
	int frame_center_horizontal_offset[3];
	int frame_center_vertical_offset[3];

	/* ISO/IEC 13818-2 section 6.2.3.6: copyright_extension() header */
	int copyright_flag;
	int copyright_identifier;
	int original_or_copy;
	int copyright_number_1;
	int copyright_number_2;
	int copyright_number_3;

	int Decode_Layer;

	int sequence_framenum;
	int bitstream_framenum;
	int Fault_Flag;

} MPEGDecoder;

//extern MPEGDecoder *decoder;

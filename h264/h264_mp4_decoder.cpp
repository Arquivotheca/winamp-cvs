#include "h264_mp4_decoder.h"
#include "../winamp/wa_ipc.h"

uint32_t Read24(const uint8_t *data)
{
				// ugh, 24bit size
			uint32_t this_size=0;
			uint8_t *this_size_p = (uint8_t *)&this_size;
			this_size_p[0] = data[2];
			this_size_p[1] = data[1];
			this_size_p[2] = data[0];
			return this_size;
}

uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len)
{
	if ((data_len) < (nalu_size_bytes))
		return 0;

	switch(nalu_size_bytes)
	{
	case 1:
		return *h264_data;
	case 2:
		{
			uint16_t this_size = *(uint16_t *)h264_data;
			this_size = htons(this_size);
			return this_size;
		}
	case 3:
		{
			return Read24(h264_data);
		}
	case 4:
		{
			uint32_t this_size = *(uint32_t *)h264_data;
			this_size = htonl(this_size);
			return this_size;
		}
	}
return 0;
}

#ifdef FHG_H264
H264MP4Decoder::~H264MP4Decoder()
{
}
static JVTBASEDEC_HANDLE_LIST *dec_handle_list=0;
H264MP4Decoder::H264MP4Decoder()
{
	if (dec_handle_list == 0)
	{
		iis_jvtbasedec_init_library(&dec_handle_list);
	}

	decoder=0;
	nalu_size_bytes=0;
}

int H264MP4Decoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	iis_jvtbasedec_new_instance(dec_handle_list, &decoder);
	iis_jvtbasedec_command(CMD_JVTBASEDEC_SET_CU_OUTPUT_CROP_MODE, decoder, CU_OUT_CROPPED, 0, 0); // apply cropping
	iis_jvtbasedec_command(CMD_JVTBASEDEC_SET_CU_OUTPUT_MEM_MODE, decoder, CU_OUT_REFERENCE_MEMORY, 0, 0); // shared frame memory
	iis_jvtbasedec_command(CMD_JVTBASEDEC_SET_MATH_MODE, decoder, 4, 0, 0); 

	
	if (decoder)
	{
		INT frames_ready;
		JVTBaseDecNALU nalu = {0,};
		// TODO error checking
		uint8_t **seqHeaders, **pictHeaders;
		uint32_t *seqHeadersSize, *pictHeadersSize;
		MP4GetTrackH264SeqPictHeaders(mp4_file, mp4_track,
			&seqHeaders, &seqHeadersSize,
			&pictHeaders, &pictHeadersSize);

		if (seqHeadersSize)
		{
		for (uint32_t i = 0; seqHeadersSize[i] != 0; i++) 
		{
			nalu.nal_unit_data = seqHeaders[i];
			nalu.nal_unit_size = seqHeadersSize[i];
			iis_jvtbasedec_decode_nalu(decoder, &nalu, &frames_ready);
		}
		}

		if (pictHeadersSize)
		{
		for (uint32_t i = 0; pictHeadersSize[i] != 0; i++) 
		{
						nalu.nal_unit_data = pictHeaders[i];
			nalu.nal_unit_size = pictHeadersSize[i];
			iis_jvtbasedec_decode_nalu(decoder, &nalu, &frames_ready);
		}
		}

	MP4GetTrackH264LengthSize(mp4_file, mp4_track, &nalu_size_bytes);

		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;
}

int H264MP4Decoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
		*aspect_ratio =1.0;
		JVTBaseDecInfo info;
		iis_jvtbasedec_command(CMD_JVTBASEDEC_GET_SETUP_INFO, decoder, 0, (JVT_LPARAM *) &info, 0);
		
		//if (format)
		{
			*x = info.frame_width_cropped;
			*y = info.frame_height_cropped;
			switch(info.chroma_format)
			{
			case CHROMA_FORMAT_420:
				*color_format = htonl('YV12');
				break;
			default: // some color format we havn't implemented yet
				return MP4_VIDEO_FAILURE;
			}
		}
		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;
}

int H264MP4Decoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes)
	{
		uint32_t this_size =GetNALUSize(nalu_size_bytes, h264_data, inputBufferBytes);
		if (this_size == 0)
			return MP4_VIDEO_FAILURE;
	

		inputBufferBytes-=nalu_size_bytes;
		h264_data+=nalu_size_bytes;
		if (this_size > inputBufferBytes)
			return MP4_VIDEO_FAILURE;

		JVTBaseDecNALU nalu = {(BYTE *)h264_data, this_size, timestamp,};
		INT frames_ready;
		iis_jvtbasedec_decode_nalu(decoder, &nalu, &frames_ready);
	
		inputBufferBytes-=this_size;
		h264_data+=this_size;
	}
	return MP4_VIDEO_SUCCESS;
}

int H264MP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "avc1") || !strcmp(codecName, "h264");
}

void H264MP4Decoder::Flush()
{
//	H264_Flush(decoder);
}

int H264MP4Decoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	INT frames_ready;
	JVTBaseDecPU pic;
	if (iis_jvtbasedec_get_decoded_frame(decoder, &pic, &frames_ready) == JVT_OK)
	{
		YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
		planes->y.baseAddr = pic.pic_data_y;
		planes->y.rowBytes = pic.stride;
		planes->u.baseAddr = pic.pic_data_u;
		planes->u.rowBytes = pic.stride/2;
		planes->v.baseAddr = pic.pic_data_v;
		planes->v.rowBytes = pic.stride/2;
		*data = planes;
		*decoder_data = 0;
		*timestamp = pic.composition_timestamp;
		return MP4_VIDEO_SUCCESS;
	}

	return MP4_VIDEO_FAILURE;
}

void H264MP4Decoder::FreePicture(void *data, void *decoder_data)
{
	free(data);
}

#else
H264MP4Decoder::H264MP4Decoder()
{
	decoder=0;
	nalu_size_bytes=0;
}

H264MP4Decoder::~H264MP4Decoder()
{
	H264_DestroyDecoder(decoder);
	decoder=0;
}

int H264MP4Decoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	decoder = H264_CreateDecoder();
	if (decoder)
	{
		// TODO error checking
		uint8_t **seqHeaders, **pictHeaders;
		uint32_t *seqHeadersSize, *pictHeadersSize;
		MP4GetTrackH264SeqPictHeaders(mp4_file, mp4_track,
			&seqHeaders, &seqHeadersSize,
			&pictHeaders, &pictHeadersSize);

		if (seqHeadersSize)
		{
		for (uint32_t i = 0; seqHeadersSize[i] != 0; i++) 
		{
			H264_DecodeFrame(decoder, seqHeaders[i], seqHeadersSize[i], 0);
		}
		}

		if (pictHeadersSize)
		{
		for (uint32_t i = 0; pictHeadersSize[i] != 0; i++) 
		{
			H264_DecodeFrame(decoder, pictHeaders[i], pictHeadersSize[i], 0);
		}
		}

	MP4GetTrackH264LengthSize(mp4_file, mp4_track, &nalu_size_bytes);

		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;
}

int H264MP4Decoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
		const FrameFormat *format = H264_GetOutputFormat(decoder, aspect_ratio);
		if (format)
		{			
			*x = format->width_crop;
			*y = format->height_crop;
			switch(format->yuv_format)
			{
			case YUV420:
				*color_format = htonl('YV12');
				break;
			default: // some color format we havn't implemented yet
				return MP4_VIDEO_FAILURE;
			}
		}
		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;
}


int H264MP4Decoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes)
	{
		uint32_t this_size =GetNALUSize(nalu_size_bytes, h264_data, inputBufferBytes);
		if (this_size == 0)
			return MP4_VIDEO_FAILURE;
	

		inputBufferBytes-=nalu_size_bytes;
		h264_data+=nalu_size_bytes;
		if (this_size > inputBufferBytes)
			return MP4_VIDEO_FAILURE;

		H264_DecodeFrame(decoder, h264_data, this_size, timestamp);
	
		inputBufferBytes-=this_size;
		h264_data+=this_size;
	}
	return MP4_VIDEO_SUCCESS;
}

int H264MP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "avc1");
}

void H264MP4Decoder::Flush()
{
	H264_Flush(decoder);
}

int H264MP4Decoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	StorablePicture *pic=0;
	H264_GetPicture(decoder, &pic);
	if (pic)
	{
		int crop_y = pic->frame_cropping_rect_top_offset;
		int crop_x = pic->frame_cropping_rect_left_offset;
		YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
		planes->y.baseAddr = &(pic->imgY->img[crop_y][crop_x]);
		planes->y.rowBytes = pic->imgY->stride;
		planes->u.baseAddr = &(pic->imgUV[0]->img[crop_y/2][crop_x/2]);
		planes->u.rowBytes = pic->imgUV[0]->stride;
		planes->v.baseAddr = &(pic->imgUV[1]->img[crop_y/2][crop_x/2]);
		planes->v.rowBytes = pic->imgUV[1]->stride;
		*data = planes;
		*decoder_data = pic;
		*timestamp = pic->time_code;
		return MP4_VIDEO_SUCCESS;
	}

	return MP4_VIDEO_FAILURE;
}

void H264MP4Decoder::FreePicture(void *data, void *decoder_data)
{
	StorablePicture *pic=(StorablePicture *)decoder_data;
	H264_FreePicture(decoder, pic);
	free(data);
}

void H264MP4Decoder::HurryUp(int state)
{
	if (decoder)
		H264_HurryUp(decoder, state);
}


#endif

#define CBCLASS H264MP4Decoder
START_DISPATCH;
CB(MPEG4_VIDEO_OPEN, Open)
CB(MPEG4_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(MPEG4_VIDEO_DECODE, DecodeSample)
CB(MPEG4_VIDEO_HANDLES_CODEC, CanHandleCodec)
VCB(MPEG4_VIDEO_FLUSH, Flush)
CB(MPEG4_VIDEO_GET_PICTURE, GetPicture)
VCB(MPEG4_VIDEO_FREE_PICTURE, FreePicture)
VCB(MPEG4_VIDEO_HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS


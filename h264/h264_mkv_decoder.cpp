#include "h264_mkv_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <winsock.h>
#include <mmsystem.h>

int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MPEG4/ISO/AVC"))
	{
		const uint8_t *init_data = (const uint8_t *)track_entry_data->codec_private;
		size_t init_data_len = track_entry_data->codec_private_len;
		if (init_data && init_data_len >= 6)
		{
			h264_decoder_t ctx = H264_CreateDecoder();
			if (!ctx)
				return CREATEDECODER_FAILURE;

			init_data+=4; // don't care about level & profile
			init_data_len-=4;

			// read NALU header size length
			uint8_t nalu_minus_one = *init_data++ & 0x3;
			init_data_len--;

			// number of SPS NAL units
			uint8_t num_sps = *init_data++ & 0x1F;
			init_data_len--;
			for (uint8_t i=0;i!=num_sps;i++)
			{
				if (init_data_len < 2)
				{
					H264_DestroyDecoder(ctx);
					return CREATEDECODER_FAILURE;
				}
				uint16_t *s = (uint16_t *)init_data;
				uint16_t sps_size = htons(*s);
				init_data+=2;
				init_data_len-=2;
				if (init_data_len < sps_size)
				{
					H264_DestroyDecoder(ctx);
					return CREATEDECODER_FAILURE;
				}
				H264_DecodeFrame(ctx, init_data, sps_size, 0);
				init_data+=sps_size;
				init_data_len-=sps_size;
			}

			// read PPS NAL units
			if (init_data_len)
			{
				// number of PPS NAL units
				uint8_t num_pps = *init_data++ & 0x1F;
				init_data_len--;
				for (uint8_t i=0;i!=num_pps;i++)
				{
					if (init_data_len < 2)
					{
						H264_DestroyDecoder(ctx);
						return CREATEDECODER_FAILURE;
					}
					uint16_t *s = (uint16_t *)init_data;
					uint16_t pps_size = htons(*s);
					init_data+=2;
					init_data_len-=2;
					if (init_data_len < pps_size)
					{
						H264_DestroyDecoder(ctx);
						return CREATEDECODER_FAILURE;
					}
					H264_DecodeFrame(ctx, init_data, pps_size, 0);
					init_data+=pps_size;
					init_data_len-=pps_size;
				}
			}
			// if we made it here, we should be good
			*decoder = new MKVH264(ctx, nalu_minus_one, video_data);
			return CREATEDECODER_SUCCESS;
		}
		else
		{
			return CREATEDECODER_FAILURE;
		}
	}
	else
	{
		return CREATEDECODER_NOT_MINE;
	}
}


#define CBCLASS MKVDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

MKVH264::MKVH264(h264_decoder_t ctx, uint8_t nalu_minus_one, const nsmkv::VideoData *video_data) :video_data(video_data), decoder(ctx)
{
	nalu_size = nalu_minus_one + 1;
}

int MKVH264::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
		const FrameFormat *format = H264_GetOutputFormat(decoder, aspect_ratio);
		if (format)
		{
			*x = format->width_crop  - (video_data?(video_data->pixel_crop_left+video_data->pixel_crop_right):0);
			*y = format->height_crop - (video_data?(video_data->pixel_crop_top+video_data->pixel_crop_bottom):0);
			switch(format->yuv_format)
			{
			case YUV420:
				*color_format = mmioFOURCC('Y','V','1','2');
				break;
			default: // some color format we havn't implemented yet
				return MKV_FAILURE;
			}
		}
		return MKV_SUCCESS;
	}
	return MKV_FAILURE;
}

uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len);

int MKVH264::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes)
	{
		uint32_t this_size = GetNALUSize(nalu_size, h264_data, inputBufferBytes);
		if (this_size == 0)
			return MKV_FAILURE;

		inputBufferBytes-=nalu_size;
		h264_data+=nalu_size;
		if (this_size > inputBufferBytes)
			return MKV_FAILURE;

		H264_DecodeFrame(decoder, h264_data, this_size, timestamp);

		inputBufferBytes-=this_size;
		h264_data+=this_size;
	}
	return MKV_SUCCESS;
}

void MKVH264::Flush()
{
	if (decoder)
		H264_Flush(decoder);
}

int MKVH264::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	StorablePicture *pic=0;
	H264_GetPicture(decoder, &pic);
	if (pic)
	{
		int crop_y = pic->frame_cropping_rect_top_offset + video_data?video_data->pixel_crop_left:0;
		int crop_x = pic->frame_cropping_rect_left_offset + video_data?video_data->pixel_crop_top:0;
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
		return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

void MKVH264::FreePicture(void *data, void *decoder_data)
{
	StorablePicture *pic=(StorablePicture *)decoder_data;
	H264_FreePicture(decoder, pic);
	free(data);
}

void MKVH264::EndOfStream()
{
	H264_EndOfStream(decoder);
}

void MKVH264::HurryUp(int state)
{
	if (decoder)
		H264_HurryUp(decoder, state);
}

void MKVH264::Close()
{
	if (decoder)
	{
		H264_DestroyDecoder(decoder);
	}
	delete this;
}

#define CBCLASS MKVH264
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(END_OF_STREAM, EndOfStream)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS


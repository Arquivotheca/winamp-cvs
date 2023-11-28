#include "h264_flv_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <winsock.h>

int FLVDecoderCreator::CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder)
{
	if (format_type == FLV::VIDEO_FORMAT_AVC)
	{
		h264_decoder_t ctx = H264_CreateDecoder();
		if (!ctx)
			return CREATEDECODER_FAILURE;
		*decoder = new FLVH264(ctx);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesVideo(int format_type)
{
	if (format_type == FLV::VIDEO_FORMAT_AVC)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
CB(HANDLES_VIDEO, HandlesVideo)
END_DISPATCH;
#undef CBCLASS

/* --- */
uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len);
uint32_t Read24(const uint8_t *data);

FLVH264::FLVH264(void *decoder) : decoder(decoder)
{
	sequence_headers_parsed=0;
	nalu_size_bytes=0;
}

int FLVH264::GetOutputFormat(int *x, int *y, int *color_format)
{
	if (decoder)
	{
		double aspect_ratio;
		const FrameFormat *format = H264_GetOutputFormat(decoder, &aspect_ratio);
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
				return FLV_VIDEO_FAILURE;
			}
		}
		return FLV_VIDEO_SUCCESS;
	}
	return FLV_VIDEO_FAILURE;
}

int FLVH264::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	if (*h264_data == 0 && inputBufferBytes >= 10) // sequence headers 
	{
		h264_data += 4; // skip packet type and timestamp
		inputBufferBytes -=4;
		h264_data+=4; // don't care about level & profile
		inputBufferBytes -=4;
		nalu_size_bytes = (*h264_data++ & 0x3)+1;
		inputBufferBytes--;
		size_t num_sps = *h264_data++ & 0x1F;
		inputBufferBytes--;
		for (size_t i=0;i!=num_sps;i++)
		{
			if (inputBufferBytes > 2)
			{
				uint16_t *s = (uint16_t *)h264_data;
				uint16_t sps_size = htons(*s);
				h264_data+=2;
				inputBufferBytes-=2;
				//H264_ProcessSPS(decoder, h264_data+1, sps_size);
				if (inputBufferBytes >= sps_size)
				{
				H264_DecodeFrame(decoder, h264_data, sps_size, 0);
				h264_data+=sps_size;
				inputBufferBytes-=sps_size;
				}
			}
		}
		if (inputBufferBytes)
		{
			size_t num_pps = *h264_data++;
			inputBufferBytes--;
			for (size_t i=0;i!=num_pps;i++)
			{
				if (inputBufferBytes > 2)
				{
					uint16_t *s = (uint16_t *)h264_data;
					uint16_t sps_size = htons(*s);
					h264_data+=2;
					inputBufferBytes-=2;
					//H264_ProcessPPS(decoder, h264_data+1, sps_size);
					if (inputBufferBytes >= sps_size)
					{
						H264_DecodeFrame(decoder, h264_data, sps_size, 0);
						h264_data+=sps_size;
						inputBufferBytes-=sps_size;
					}
				}
			}
		}
		sequence_headers_parsed=1;
	}
	else if (*h264_data == 1) // frame
	{
		h264_data++;
		inputBufferBytes--;
		if (inputBufferBytes < 3)
			return FLV_VIDEO_FAILURE;
		uint32_t timestamp_offset = Read24(h264_data);
		
		h264_data+=3;
		inputBufferBytes-=3;
			
		while (inputBufferBytes)
		{
			uint32_t this_size =GetNALUSize(nalu_size_bytes, h264_data, inputBufferBytes);
			if (this_size == 0)
				return FLV_VIDEO_FAILURE;

			inputBufferBytes-=nalu_size_bytes;
			h264_data+=nalu_size_bytes;
			if (this_size > inputBufferBytes)
				return FLV_VIDEO_FAILURE;

			H264_DecodeFrame(decoder, h264_data, this_size, timestamp+timestamp_offset);

			inputBufferBytes-=this_size;
			h264_data+=this_size;
		}
	}

	return FLV_VIDEO_SUCCESS;
}

void FLVH264::Flush()
{
	H264_Flush(decoder);
}

void FLVH264::Close()
{
	if (decoder)
		H264_DestroyDecoder(decoder);
	delete this;
}

int FLVH264::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
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
		return FLV_VIDEO_SUCCESS;
	}

	return FLV_VIDEO_FAILURE;
}

void FLVH264::FreePicture(void *data, void *decoder_data)
{
	StorablePicture *pic=(StorablePicture *)decoder_data;
	H264_FreePicture(decoder, pic);
	free(data);
}

int FLVH264::Ready()
{
	return sequence_headers_parsed;
}

#define CBCLASS FLVH264
START_DISPATCH;
CB(FLV_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_VIDEO_DECODE, DecodeSample)
VCB(FLV_VIDEO_FLUSH, Flush)
VCB(FLV_VIDEO_CLOSE, Close)
CB(FLV_VIDEO_GET_PICTURE, GetPicture)
VCB(FLV_VIDEO_FREE_PICTURE, FreePicture)
CB(FLV_VIDEO_READY, Ready)
END_DISPATCH;
#undef CBCLASS


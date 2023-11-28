#include "mkv_mpgv_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES

extern "C" int mpegmain(void *buffer, int len);
extern "C" void *MPEGVideo_CreateDecoder();
static int retrieved;
int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MPEG1"))
	{
		mpegvideo_decoder_t d = MPEGVideo_CreateDecoder();
		if (d)
		{
			*decoder = new MKVMPGV(d, video_data);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
		}

		return CREATEDECODER_FAILURE;
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

MKVMPGV::MKVMPGV(mpegvideo_decoder_t decoder, const nsmkv::VideoData *video_data) : video_data(video_data), decoder(decoder)
{

}

int MKVMPGV::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	*x = 352;
	*y = 288;
	*color_format = '21VY';
	*aspect_ratio = 1.0;
	return MKV_SUCCESS;
	/*
	if (decoder)
	{
		if (MPEG4Video_GetOutputFormat(decoder, x, y, aspect_ratio) == 0)
		{
			*color_format = htonl('YV12');
			return MKV_SUCCESS;
		}
	}*/
	return MKV_FAILURE;
}

int MKVMPGV::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	MPEGVideo_DecodeFrame(decoder, (void *)inputBuffer, inputBufferBytes);
	retrieved=0;
	//if (decoder)
	{
//		MPEG4Video_DecodeFrame(decoder, inputBuffer, inputBufferBytes, timestamp);
		//return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

void MKVMPGV::Flush()
{
//	if (decoder)
//		MPEG4Video_Flush(decoder);
}

extern "C" unsigned char **GetLastSrc();
int MKVMPGV::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	
	if (decoder)
	{
		unsigned char **last_src = GetLastSrc();;
		if (!retrieved)
		{
			retrieved=1;
			YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
			planes->y.baseAddr = last_src[0];
			planes->y.rowBytes = 352;
			planes->u.baseAddr = last_src[1];
			planes->u.rowBytes = 352/2;
			planes->v.baseAddr = last_src[2];
			planes->v.rowBytes = 352/2;
			*data = planes;
			*decoder_data = 0;
			//*timestamp = 0;//pic->timestamp;
			return MKV_SUCCESS;
		}
	}

	return MKV_FAILURE;
}

void MKVMPGV::FreePicture(void *data, void *decoder_data)
{

}

void MKVMPGV::HurryUp(int state)
{

}

#define CBCLASS MKVMPGV
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS


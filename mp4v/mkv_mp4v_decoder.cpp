#include "mkv_mp4v_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <mmsystem.h>

int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MPEG4/ISO/ASP")
		|| !strcmp(codec_id, "V_MPEG4/ISO/SP"))
	{
		mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_MP4, MPEG4_CODEC_DEFAULT);
		if (ctx)
		{
			if (track_entry_data->codec_private && track_entry_data->codec_private_len)
			{
				// mp4 stores headers up to first VOP in esds
				MPEG4Video_DecodeFrame(ctx, track_entry_data->codec_private, track_entry_data->codec_private_len, 0);
			}
			*decoder = new MKVMP4V(ctx, video_data);
			return CREATEDECODER_SUCCESS;
		}
		else
		{
			return CREATEDECODER_FAILURE;
		}
	}
	else if (!strcmp(codec_id, "V_MS/VFW/FOURCC"))
	{
		if (track_entry_data->codec_private && track_entry_data->codec_private_len)
		{
			const BITMAPINFOHEADER *header = (const BITMAPINFOHEADER *)track_entry_data->codec_private;
			if (header->biCompression == 'DIVX')
			{
				mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_MP4, MPEG4_CODEC_DEFAULT);
				*decoder = new MKVMP4V(ctx, video_data);
				return CREATEDECODER_SUCCESS;
			}
			else if (header->biCompression == '05XD')
			{
				mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_MP4, MPEG4_CODEC_DIVX5);
				*decoder = new MKVMP4V(ctx, video_data);
				return CREATEDECODER_SUCCESS;
			}
		}
		return CREATEDECODER_NOT_MINE;
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

MKVMP4V::MKVMP4V(mpeg4vid_decoder_t decoder, const nsmkv::VideoData *video_data) : decoder(decoder), video_data(video_data)
{

}

int MKVMP4V::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
		if (MPEG4Video_GetOutputFormat(decoder, x, y, aspect_ratio) == 0)
		{
			*color_format = mmioFOURCC('Y','V','1','2');
			return MKV_SUCCESS;
		}
	}
	return MKV_FAILURE;
}

int MKVMP4V::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	if (decoder)
	{
		MPEG4Video_DecodeFrame(decoder, inputBuffer, inputBufferBytes, timestamp);
		return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

void MKVMP4V::Flush()
{
	if (decoder)
		MPEG4Video_Flush(decoder);
}

int MKVMP4V::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (decoder)
	{
		mp4_Frame *pic=0;
		MPEG4Video_GetPicture(decoder, &pic);
		if (pic)
		{
			YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
			planes->y.baseAddr = pic->pY;
			planes->y.rowBytes = pic->stepY;
			planes->u.baseAddr = pic->pCb;
			planes->u.rowBytes = pic->stepCb;
			planes->v.baseAddr = pic->pCr;
			planes->v.rowBytes = pic->stepCr;
			*data = planes;
			*decoder_data = pic;
			*timestamp = pic->timestamp;
			return MKV_SUCCESS;
		}
	}

	return MKV_FAILURE;
}

void MKVMP4V::FreePicture(void *data, void *decoder_data)
{
	if (decoder)
		MPEG4Video_ReleaseFrame(decoder, (mp4_Frame *)decoder_data);
	free(data);
}

void MKVMP4V::HurryUp(int state)
{
	if (decoder)
		MPEG4Video_HurryUp(decoder, state);
}

void MKVMP4V::Close()
{
	if (decoder)
		MPEG4Video_DestroyDecoder(decoder);
	delete this;
}

#define CBCLASS MKVMP4V
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(HURRY_UP, HurryUp)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS


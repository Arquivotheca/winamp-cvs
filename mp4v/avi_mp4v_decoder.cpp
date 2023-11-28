#include "avi_mp4v_decoder.h"
#include "../Winamp/wa_ipc.h"
#include "../nsavi/read.h"
#include <mmsystem.h>

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == 'DIVX' || format->compression == 'divx') // xvid  
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DEFAULT);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}
		else if (format->compression == 'xvid' || format->compression == 'XVID') // divx
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DEFAULT);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}
		else if (format->compression == 'v4pm') // mp4v
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DEFAULT);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}
		else if (format->compression == '05XD') // divx 5
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DIVX5);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}
		else if (format->compression == nsaviFOURCC('S','E','D','G')) // dunno what this is exactly
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DEFAULT);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}
		/*else if (format->compression == '3VID') // divx 3, let's hope it plays
		{
			mpeg4vid_decoder_t ctx= MPEG4Video_CreateDecoder(MPEG4_FILETYPE_AVI, MPEG4_CODEC_DEFAULT);
			*decoder = new AVIMP4V(ctx, stream_header, format);
			return CREATEDECODER_SUCCESS;
		}*/
	}

	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

AVIMP4V::AVIMP4V(mpeg4vid_decoder_t decoder, const nsavi::STRH *stream_header, const nsavi::video_format *stream_format) 
: decoder(decoder), stream_header(stream_header), stream_format(stream_format)
{
	if (stream_format->size_bytes > 40)
	{
		MPEG4Video_DecodeFrame(decoder, ((const uint8_t *)stream_format) + 44, stream_format->size_bytes - stream_format->video_format_size_bytes, 0);
	}
}

int AVIMP4V::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (decoder)
	{
		if (MPEG4Video_GetOutputFormat(decoder, x, y, aspect_ratio) == 0)
		{
			*flip = 0;
			*color_format = mmioFOURCC('Y','V','1','2');
			return AVI_SUCCESS;
		}
	}
	return AVI_FAILURE;
}

int AVIMP4V::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	if (decoder)
	{
		MPEG4Video_DecodeFrame(decoder, inputBuffer, inputBufferBytes, 0);
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVIMP4V::Flush()
{
	if (decoder)
		MPEG4Video_Flush(decoder);
}

int AVIMP4V::GetPicture(void **data, void **decoder_data)
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
			return AVI_SUCCESS;
		}
	}

	return AVI_FAILURE;
}

void AVIMP4V::FreePicture(void *data, void *decoder_data)
{
	free(data);
	if (decoder)
		MPEG4Video_ReleaseFrame(decoder, (mp4_Frame *)decoder_data);
}

void AVIMP4V::HurryUp(int state)
{
	if (decoder)
		MPEG4Video_HurryUp(decoder, state);
}

void AVIMP4V::Close()
{
	if (decoder)
		MPEG4Video_DestroyDecoder(decoder);
	delete this;
}

#define CBCLASS AVIMP4V
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(HURRY_UP, HurryUp)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS


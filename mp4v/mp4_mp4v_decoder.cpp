#include "mp4_mp4v_decoder.h"
#include "../winamp/wa_ipc.h"


MP4VMP4Decoder::MP4VMP4Decoder()
{
	decoder=0;
}

MP4VMP4Decoder::~MP4VMP4Decoder()
{
	if (decoder)
		MPEG4Video_DestroyDecoder(decoder);
	decoder=0;
}

int MP4VMP4Decoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	decoder = MPEG4Video_CreateDecoder(MPEG4_FILETYPE_MP4, MPEG4_CODEC_DEFAULT);
	if (decoder)
	{
		uint8_t *buffer;
		uint32_t buffer_size = 0;
    MP4GetTrackESConfiguration(mp4_file, mp4_track, &buffer, &buffer_size);
		if (buffer && buffer_size)
		{
			// mp4 stores headers up to first VOP in esds
			MPEG4Video_DecodeFrame(decoder, buffer, buffer_size, 0);
			MP4Free(buffer);
		}
		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;
}

int MP4VMP4Decoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
		if (MPEG4Video_GetOutputFormat(decoder, x, y, aspect_ratio) == 0)
		{
			*color_format = htonl('YV12');
			return MP4_VIDEO_SUCCESS;
		}
	} 
	return MP4_VIDEO_FAILURE;
}

int MP4VMP4Decoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	if (decoder)
	{
		MPEG4Video_DecodeFrame(decoder, inputBuffer, inputBufferBytes, timestamp);
		return MP4_VIDEO_SUCCESS;
	}
	
	return MP4_VIDEO_FAILURE;
}

int MP4VMP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "mp4v");
}

void MP4VMP4Decoder::Flush()
{
	if (decoder)
		MPEG4Video_Flush(decoder);
}

int MP4VMP4Decoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
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
			return MP4_VIDEO_SUCCESS;
		}
	}

	return MP4_VIDEO_FAILURE;
}

void MP4VMP4Decoder::FreePicture(void *data, void *decoder_data)
{
		if (decoder)
		MPEG4Video_ReleaseFrame(decoder, (mp4_Frame *)decoder_data);
	free(data);
}

void MP4VMP4Decoder::Close()
{
	//delete this;
}

#define CBCLASS MP4VMP4Decoder
START_DISPATCH;
CB(MPEG4_VIDEO_OPEN, Open)
CB(MPEG4_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(MPEG4_VIDEO_DECODE, DecodeSample)
CB(MPEG4_VIDEO_HANDLES_CODEC, CanHandleCodec)
VCB(MPEG4_VIDEO_FLUSH, Flush)
CB(MPEG4_VIDEO_GET_PICTURE, GetPicture)
VCB(MPEG4_VIDEO_FREE_PICTURE, FreePicture)
VCB(MPEG4_VIDEO_CLOSE, Close)
END_DISPATCH;
#undef CBCLASS


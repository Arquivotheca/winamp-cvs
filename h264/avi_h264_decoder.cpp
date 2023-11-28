#include "avi_h264_decoder.h"
#include "../Winamp/wa_ipc.h"
#include "annexb.h"
#include <mmsystem.h>

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == '462H')
		{
			h264_decoder_t ctx = H264_CreateDecoder();
			if (!ctx)
				return CREATEDECODER_FAILURE;

			h264_annexb_demuxer_t demuxer = AnnexB_Create(MAX_CODED_FRAME_SIZE);
			if (!demuxer)
			{
				H264_DestroyDecoder(ctx);
				return CREATEDECODER_FAILURE;
			}
			*decoder = new AVIH264(ctx, demuxer, stream_header);
			return CREATEDECODER_SUCCESS;
		}
	}
	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

AVIH264::AVIH264(h264_decoder_t ctx, h264_annexb_demuxer_t demuxer, const nsavi::STRH *stream_header) : decoder(ctx), demuxer(demuxer), stream_header(stream_header)
{

}

int AVIH264::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (decoder)
	{
		const FrameFormat *format = H264_GetOutputFormat(decoder, aspect_ratio);
		if (format)
		{
			*x = format->width_crop  ;//- (video_data?(video_data->pixel_crop_left+video_data->pixel_crop_right):0);
			*y = format->height_crop ;//- (video_data?(video_data->pixel_crop_top+video_data->pixel_crop_bottom):0);
			switch(format->yuv_format)
			{
			case YUV420:
				*color_format = mmioFOURCC('Y','V','1','2');
				break;
			default: // some color format we havn't implemented yet
				return AVI_FAILURE;
			}
		}
		return AVI_SUCCESS;
	}
	return AVI_FAILURE;
}

int AVIH264::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes)
	{
		if (AnnexB_AddData(demuxer, &inputBuffer, &inputBufferBytes) == AnnexB_UnitAvailable)
		{
			const void *nalu;
			size_t nalu_len;
			if (AnnexB_GetUnit(demuxer, &nalu, &nalu_len) == AnnexB_UnitAvailable)
			{
				H264_DecodeFrame(decoder, nalu, nalu_len, 0);
			}
		}
	}
	
	return AVI_SUCCESS;
}

void AVIH264::Flush()
{
	if (decoder)
		H264_Flush(decoder);
}

int AVIH264::GetPicture(void **data, void **decoder_data)
{
	StorablePicture *pic=0;
	H264_GetPicture(decoder, &pic);
	if (pic)
	{
		int crop_y = 0;//pic->frame_cropping_rect_top_offset + video_data?video_data->pixel_crop_left:0;
		int crop_x = 0;//pic->frame_cropping_rect_left_offset + video_data?video_data->pixel_crop_top:0;
		YV12_PLANES *planes = (YV12_PLANES *)malloc(sizeof(YV12_PLANES));
		planes->y.baseAddr = &(pic->imgY->img[crop_y][crop_x]);
		planes->y.rowBytes = pic->imgY->stride;
		planes->u.baseAddr = &(pic->imgUV[0]->img[crop_y/2][crop_x/2]);
		planes->u.rowBytes = pic->imgUV[0]->stride;
		planes->v.baseAddr = &(pic->imgUV[1]->img[crop_y/2][crop_x/2]);
		planes->v.rowBytes = pic->imgUV[1]->stride;
		*data = planes;
		*decoder_data = pic;
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVIH264::FreePicture(void *data, void *decoder_data)
{
	StorablePicture *pic=(StorablePicture *)decoder_data;
	H264_FreePicture(decoder, pic);
	free(data);
}

void AVIH264::EndOfStream()
{
	H264_EndOfStream(decoder);
}

void AVIH264::HurryUp(int state)
{
	if (decoder)
		H264_HurryUp(decoder, state);
}

void AVIH264::Close()
{
	if (decoder)
	{
		if (demuxer)
			AnnexB_Destroy(demuxer);
		demuxer=0;
		H264_DestroyDecoder(decoder);
	}
	delete this;
}

#define CBCLASS AVIH264
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(END_OF_STREAM, EndOfStream)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS


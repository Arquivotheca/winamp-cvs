#include "nsv_h264_decoder.h"
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
H264_Decoder::H264_Decoder()
{
	decoder = H264_CreateDecoder();
	demuxer = AnnexB_Create(MAX_CODED_FRAME_SIZE);
	last_pic = 0;
}

H264_Decoder::~H264_Decoder()
{
	if (last_pic)
		H264_FreePicture(decoder, last_pic);
	H264_DestroyDecoder(decoder);
	decoder=0;
	AnnexB_Destroy(demuxer);
	demuxer=0;
}

int H264_Decoder::decode(int need_kf, 
												 void *_in, int _in_len, 
												 void **out, // out is set to a pointer to data
												 unsigned int *out_type, // 'Y','V','1','2' is currently defined
												 int *is_kf)
{

	if (last_pic)
	{
		H264_FreePicture(decoder, last_pic);
		last_pic=0;
	}
	*out_type=NSV_MAKETYPE('Y','V','1','2');
	size_t in_len = _in_len;
	const void *in = const_cast<const void *>(_in);
	while (in_len)
	{
		if (AnnexB_AddData(demuxer, &in, &in_len) == AnnexB_UnitAvailable)
		{
			const void *nalu;
			size_t nalu_len;
			if (AnnexB_GetUnit(demuxer, &nalu, &nalu_len) == AnnexB_UnitAvailable)
			{
				H264_DecodeFrame(decoder, nalu, nalu_len, 0);
			}
		}
	}

	H264_GetPicture(decoder, &last_pic);
	if (last_pic)
	{
		int crop_y = last_pic->frame_cropping_rect_top_offset;
		int crop_x = last_pic->frame_cropping_rect_left_offset;
		vidbufdec.y.baseAddr = &(last_pic->imgY->img[crop_y][crop_x]);
		vidbufdec.y.rowBytes = last_pic->imgY->stride;
		vidbufdec.u.baseAddr = &(last_pic->imgUV[0]->img[crop_y/2][crop_x/2]);
		vidbufdec.u.rowBytes = last_pic->imgUV[0]->stride;
		vidbufdec.v.baseAddr = &(last_pic->imgUV[1]->img[crop_y/2][crop_x/2]);
		vidbufdec.v.rowBytes = last_pic->imgUV[1]->stride;
		*out = &vidbufdec;
		*is_kf = 1;
		return 0;
	}
	else
		*out = 0;

	return 0;
}

void H264_Decoder::flush()
{
	if (decoder)
	{
		H264_Flush(decoder);
	}
}
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

	typedef void *mpegvideo_decoder_t;
	int MPEGVideo_DecodeFrame(mpegvideo_decoder_t d, void *buffer, int len);
	mpegvideo_decoder_t MPEGVideo_CreateDecoder();

#ifdef __cplusplus
}
#endif
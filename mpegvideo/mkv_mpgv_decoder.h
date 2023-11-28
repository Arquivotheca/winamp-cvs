#pragma once
#include "../in_mkv/ifc_mkvvideodecoder.h"
#include "../in_mkv/svc_mkvdecoder.h"
#include "../mpeg2dec/mpegvid_api.h"

// {06862547-915A-4912-8C11-5F5EA0CF981B}
static const GUID mkv_mpgv_guid = 
{ 0x6862547, 0x915a, 0x4912, { 0x8c, 0x11, 0x5f, 0x5e, 0xa0, 0xcf, 0x98, 0x1b } };

class MKVDecoderCreator : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "MPEG Video MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_mpgv_guid; } 
	int CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};


class MKVMPGV : public ifc_mkvvideodecoder
{
public:
	MKVMPGV(mpegvideo_decoder_t decoder, const nsmkv::VideoData *video_data);

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp);
	void Flush();
	int GetPicture(void **data, void **decoder_data, uint64_t *timestamp); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
private:
	const nsmkv::VideoData *video_data;
	mpegvideo_decoder_t decoder;

protected: 
	RECVS_DISPATCH;
};
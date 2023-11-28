#pragma once
#include "../in_mp4/mpeg4video.h"
#ifdef FHG_H264
#include "iisjvtbasedec/iis_jvtbasedec.h"
#else
#include "../h264dec/dec_api.h"
#endif

// {F13CB206-E8F2-4353-B8B9-587D02CB701C}
static const GUID mp4_h264_guid = 
{ 0xf13cb206, 0xe8f2, 0x4353, { 0xb8, 0xb9, 0x58, 0x7d, 0x2, 0xcb, 0x70, 0x1c } };

class H264MP4Decoder : public MP4VideoDecoder
{
public:
	static const char *getServiceName() { return "H.264 MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_h264_guid; } 
	H264MP4Decoder();
	~H264MP4Decoder();

private:
	/* mpeg4video interface */
	int Open(MP4FileHandle mp4_file, MP4TrackId mp4_track);
	int GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp);
	void Flush();
	void Close();
	int CanHandleCodec(const char *codecName);
	int GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp);
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
#ifdef FHG_H264
JVTBASEDEC_HANDLE decoder;
#else
	h264_decoder_t decoder;
#endif
	uint32_t nalu_size_bytes;
protected:
	RECVS_DISPATCH;
};
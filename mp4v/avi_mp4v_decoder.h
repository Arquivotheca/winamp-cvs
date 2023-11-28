#pragma once
#include "../in_avi/ifc_avivideodecoder.h"
#include "../in_avi/svc_avidecoder.h"
#include "../mpeg4dec/mpeg4vid_api.h"

// {3E450454-0286-4ad8-811F-60A49C933E9B}
static const GUID avi_mp4v_guid = 
{ 0x3e450454, 0x286, 0x4ad8, { 0x81, 0x1f, 0x60, 0xa4, 0x9c, 0x93, 0x3e, 0x9b } };

class AVIDecoderCreator : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "MPEG-4 Part 2 AVI Decoder"; }
	static GUID getServiceGuid() { return avi_mp4v_guid; } 
	int CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIMP4V : public ifc_avivideodecoder
{
public:
	AVIMP4V(mpeg4vid_decoder_t decoder, const nsavi::STRH *stream_header, const nsavi::video_format *stream_format);

	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	int GetPicture(void **data, void **decoder_data); 
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);
	void Close();
private:
	mpeg4vid_decoder_t decoder;
	const nsavi::STRH *stream_header;
	const nsavi::video_format *stream_format;
protected: 
	RECVS_DISPATCH;
};
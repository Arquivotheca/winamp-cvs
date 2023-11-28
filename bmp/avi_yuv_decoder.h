#pragma once
#include "../in_avi/ifc_avivideodecoder.h"
#include "../nsavi/avi_header.h"

class AVIYUV : public ifc_avivideodecoder
{
public:
	AVIYUV(nsavi::video_format *stream_format);
	~AVIYUV();
	static AVIYUV *CreateDecoder(nsavi::video_format *stream_format);
	int Initialize();
	int GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip);
	int DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes);
	void Flush();
	void Close();
	int GetPicture(void **data, void **decoder_data); 
private:
	nsavi::video_format *stream_format;
	void *video_frame;
	size_t video_frame_size_bytes;
	bool o;
protected: 
	RECVS_DISPATCH;
};
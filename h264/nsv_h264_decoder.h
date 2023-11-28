#pragma once
#include "../h264dec/dec_api.h"
#include "../nsv/dec_if.h"
#include "annexb.h"
class H264_Decoder : public IVideoDecoder 
{
  public:
    H264_Decoder();
    ~H264_Decoder();
    int decode(int need_kf, 
            void *in, int in_len, 
            void **out, // out is set to a pointer to data
            unsigned int *out_type, // 'Y','V','1','2' is currently defined
            int *is_kf);
    void flush();

  private:
		h264_annexb_demuxer_t demuxer;
		h264_decoder_t decoder;
    YV12_PLANES vidbufdec;
		StorablePicture *last_pic;
};

/*
  current bitstream parameters
*/
#include "aac_ram.h"
#include "streaminfo.h"

/*  Stream Configuration and Information.
    This class holds configuration and information data for a stream to be decoded. It
    provides the calling application as well as the decoder with substantial information,
    e.g. profile, sampling rate, number of channels found in the bitstream etc.
*/
void CStreamInfoOpen(CStreamInfo *pStreamInfo)
{
  /* initialize CStreamInfo */
  pStreamInfo->SamplingRateIndex = 0;
  pStreamInfo->SamplingRate = 0;
  pStreamInfo->Profile = 0;
  pStreamInfo->ChannelConfig = 0;
  pStreamInfo->Channels = 0;
  pStreamInfo->BitRate = 0;
  pStreamInfo->SamplesPerFrame = FRAME_SIZE;
}

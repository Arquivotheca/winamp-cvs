#pragma once
#ifdef __cplusplus
extern "C" {
#endif
enum
{
	FRAME_SIZE = 1024,
  OverlapBufferSize = FRAME_SIZE/2,
};
enum
{
  ID_SCE = 0,
  ID_CPE,
  ID_CCE,
  ID_LFE,
  ID_DSE,
  ID_PCE,
  ID_FIL,
  ID_END,
};
enum
{
  L = 0,
  R = 1,
  Channels = 2
};
#ifdef __cplusplus
}
#endif
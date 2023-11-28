#pragma once
#include <bfc/platform/types.h>

#ifdef __cplusplus
extern "C" {
#endif
	enum
	{
		MPEG_UnitAvailable = 0, // data was added succesfully and a new unit is available via GetUnit().  
		MPEG_BufferFull = 1, // no start code found within the maximum unit length
		MPEG_NeedMoreData = 2, // no unit ready yet, pass in the next data chunk
		MPEG_Error = 3, // general error (out of memory, null pointer, etc)
	};

typedef void *mpeg_demuxer_t;
mpeg_demuxer_t MPEG_Create(int size);
void MPEG_Destroy(mpeg_demuxer_t demuxer);
int MPEG_AddData(mpeg_demuxer_t demuxer, const void **data, size_t *data_len);
void MPEG_EndOfStream(mpeg_demuxer_t demuxer);
int MPEG_GetUnit(mpeg_demuxer_t demuxer, const void **data, size_t *data_len);

#ifdef __cplusplus
}
#endif
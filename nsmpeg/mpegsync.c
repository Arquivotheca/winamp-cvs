#include "mpegsync.h"
#include <bfc/platform/types.h>

enum 
{
	NewUnit = 1, // start finding start code during AddData
	MidUnit = 2, // need to find the next start code from next AddData call to form a complete unit
	UnitReady = 3, // a new unit is ready and we are waiting for a GetUnit call
};

typedef struct mpeg_demuxer
{
	size_t buffer_position;
	size_t current_zero_words; // current zero word count, saved in case NALU crosses two AddData calls
	int start_code_found;
	int end_of_stream; // set to 1 when there's no more data (so we know not to look for the next start code)
	int state;
	size_t buffer_size;
	uint8_t buffer[1]; // make sure this is last
	
} MPEGDemuxer;

int AddData(const uint8_t **data, size_t *data_len); // data and length remaining are updated on exit.  if data_len>0 on exit, call again after calling GetUnit
void EndOfStream();

typedef void *mpeg_demuxer_t;
mpeg_demuxer_t MPEG_Create(int size)
{
	MPEGDemuxer *demuxer = (MPEGDemuxer *)malloc(sizeof(MPEGDemuxer) + size);
	demuxer->buffer_size = size; // MAX_CODED_FRAME_SIZE;
	demuxer->state = NewUnit;
	demuxer->buffer_position = 0;
	demuxer->current_zero_words = 0;
	demuxer->end_of_stream = 0;
	demuxer->start_code_found = 0;
	return (mpeg_demuxer_t)demuxer;
}

static int MPEG_GetByte(const uint8_t **data, size_t *data_len, uint8_t *data_byte)
{
	if (*data_len)
	{
		*data_byte = **data;
		*data = *data + 1;
		*data_len = *data_len - 1;
		return 1;
	}
	else
		return 0;
}

int MPEG_AddData(mpeg_demuxer_t d, const void **_data, size_t *data_len)
{
	const uint8_t **data;
	uint8_t data_byte;
	MPEGDemuxer *demuxer = (MPEGDemuxer *)d;
	if (demuxer)
	{
		if (demuxer->state == UnitReady)
			return MPEG_UnitAvailable;

		if (demuxer->state == NewUnit)
		{
			while (demuxer->current_zero_words)
			{
				// write any zero bytes that we read to the stream
				if (demuxer->buffer_position >= demuxer->buffer_size)
					return MPEG_BufferFull;
				demuxer->buffer[demuxer->buffer_position++] = 0;
				demuxer->current_zero_words--;
			}
			if (demuxer->start_code_found)
			{
				if (demuxer->buffer_position >= demuxer->buffer_size)
					return MPEG_BufferFull;
				demuxer->buffer[demuxer->buffer_position++] = 1;
				demuxer->start_code_found = 0;

				if (demuxer->buffer_position >= demuxer->buffer_size)
					return MPEG_BufferFull;
				demuxer->buffer[demuxer->buffer_position++] = 0;
			}
		}

		demuxer->state = MidUnit;
		data = (const uint8_t **)_data; // cast to something easier to do pointer math with

		while (MPEG_GetByte(data, data_len, &data_byte))
		{
			if (data_byte == 0)
			{
				if (demuxer->start_code_found)
				{
					demuxer->state = UnitReady;
					return MPEG_UnitAvailable;
				}
				else
				{
					demuxer->current_zero_words++; // might be the next start word
				}
			}
			else if (!demuxer->start_code_found && data_byte == 1 && demuxer->current_zero_words >= 2)
			{
				while (demuxer->current_zero_words > 2)
				{
					// write trailing zero bytes to stream
					if (demuxer->buffer_position >= demuxer->buffer_size)
						return MPEG_BufferFull;
					demuxer->buffer[demuxer->buffer_position++] = 0;
					demuxer->current_zero_words--;
				}
				demuxer->start_code_found = 1;
			}
			else
			{
				while (demuxer->current_zero_words)
				{
					// write any zero bytes that we read to the stream
					if (demuxer->buffer_position >= demuxer->buffer_size)
						return MPEG_BufferFull;
					demuxer->buffer[demuxer->buffer_position++] = 0;
					demuxer->current_zero_words--;
				}
				if (demuxer->start_code_found)
				{
					if (demuxer->buffer_position >= demuxer->buffer_size)
						return MPEG_BufferFull;
					demuxer->buffer[demuxer->buffer_position++] = 1;
					demuxer->start_code_found = 0;
				}

				if (demuxer->buffer_position >= demuxer->buffer_size)
					return MPEG_BufferFull;
				demuxer->buffer[demuxer->buffer_position++] = data_byte;
			}
		}

		if (demuxer->end_of_stream)
		{
			demuxer->state = UnitReady;
			return MPEG_UnitAvailable;
		}
		else
		{
			return MPEG_NeedMoreData;
		}
}
	else
		return MPEG_Error;
}

void MPEG_EndOfStream(mpeg_demuxer_t d)
{
	MPEGDemuxer *demuxer = (MPEGDemuxer *)d;
	if (demuxer)
		demuxer->end_of_stream = 1;
}

int MPEG_GetUnit(mpeg_demuxer_t d, const void **data, size_t *data_len)
{
	MPEGDemuxer *demuxer = (MPEGDemuxer *)d;
	if (demuxer)
	{
		if (demuxer->state == UnitReady)
		{
			*data = demuxer->buffer;
			*data_len = demuxer->buffer_position;
			demuxer->buffer_position = 0;

			demuxer->state=  NewUnit;
			return MPEG_UnitAvailable;
		}
		else
		{
			return MPEG_NeedMoreData;
		}
	}

	return MPEG_Error;
}

void MPEG_Destroy(mpeg_demuxer_t d)
{
	MPEGDemuxer *demuxer = (MPEGDemuxer *)d;
	if (demuxer)
		free(demuxer);
}
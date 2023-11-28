#include "WAVParser.h"

WAVParser::WAVParser()
{
	data_position=0;
	fmt_chunk=0;
	fact_chunk=0;
	fmt_chunk_size=0;
	fact_chunk_size=0;
	data_size=0;
}

WAVParser::~WAVParser()
{
	free(fact_chunk);
	free(fmt_chunk);
}

/* IFF Callbacks */
nsiff_callbacks_s WAVParser::iff_callbacks =
{
	0,
	0,
	WAVParser::_on_list_start,
	WAVParser::_on_chunk,
	WAVParser::_on_list_end,
	0,
	0,
};


nsiff_return_t WAVParser::_on_list_start(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	WAVParser *playback = (WAVParser *)context;
	return playback->on_list_start(iff_object, path, chunk);
}

nsiff_return_t WAVParser::_on_chunk(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	WAVParser *playback = (WAVParser *)context;
	return playback->on_chunk(iff_object, path, chunk);
}

nsiff_return_t WAVParser::_on_list_end(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	WAVParser *playback = (WAVParser *)context;
	return playback->on_list_end(iff_object, path, chunk);
}

nsiff_return_t WAVParser::on_list_start(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	return nsiff_continue;
}

nsiff_return_t WAVParser::on_chunk(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	if (!memcmp(chunk->chunk, "fmt ", 4) && path[0]==0)
	{
		if (!fmt_chunk)
		{
			fmt_chunk = malloc(chunk->size);
			if (!fmt_chunk)
			{
				parse_error = NErr_OutOfMemory;
				return nsiff_abort;
			}
			fmt_chunk_size = chunk->size;
			size_t bytes_read;
			ns_error_t ret = nsiff_read_current_chunk(iff_object, fmt_chunk, fmt_chunk_size, &bytes_read);
			if (ret != NErr_Success)
			{
				parse_error = ret;
				return nsiff_abort;
			}
		}
	}
	else if (!memcmp(chunk->chunk, "fact", 4) && path[0]==0)
	{
		if (!fact_chunk)
		{
			fact_chunk = malloc(chunk->size);
			if (!fact_chunk)
			{
				parse_error = NErr_OutOfMemory;
				return nsiff_abort;
			}
			fact_chunk_size = chunk->size;
			size_t bytes_read;
			ns_error_t ret = nsiff_read_current_chunk(iff_object, fact_chunk, fact_chunk_size, &bytes_read);
			if (ret != NErr_Success)
			{
				parse_error = ret;
				return nsiff_abort;
			}
		}
	}
	else if (!memcmp(chunk->chunk, "data", 4) && path[0]==0)
	{
		data_position = chunk->data_position;
		data_size = chunk->size;
	}
	return nsiff_continue;
}

nsiff_return_t WAVParser::on_list_end(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk)
{
	return nsiff_continue;
}
#pragma once
#include "nsiff/nsiff.h"
#include "foundation/types.h"

class WAVParser
{
public:
	WAVParser();
	~WAVParser();
protected:
	uint64_t data_position;
	uint64_t data_size;
	void *fmt_chunk, *fact_chunk;
	size_t fmt_chunk_size, fact_chunk_size;
	ns_error_t parse_error;

	static nsiff_callbacks_s iff_callbacks;

private:
	/* IFF callbacks */	
	static nsiff_return_t _on_list_start(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
	static nsiff_return_t _on_chunk(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
	static nsiff_return_t _on_list_end(void *context, nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);

	nsiff_return_t on_list_start(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
	nsiff_return_t on_chunk(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
	nsiff_return_t on_list_end(nsiff_t iff_object, const uint8_t *path, const nsiff_chunk_t chunk);
};
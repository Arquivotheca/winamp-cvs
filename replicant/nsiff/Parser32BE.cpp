#include "Parser.h"
#include "Chunk.h"
#include "nu/ByteReader.h"

ns_error_t NSIFF::Parser32BE::ReadChunk(NSIFF::Chunk *chunk)
{
	bytereader_value_t byte_reader;
	uint8_t buffer[8];
	size_t bytes_read;
	int ret = NXFileRead(file, buffer, 8, &bytes_read);
	if (ret != NErr_Success)
		return ret;
	if (bytes_read != 8)
		return NErr_ReadTruncated;
	position+=8;
	memcpy(chunk->chunk, buffer, 4);

	bytereader_init(&byte_reader, &buffer[4], 4);
	chunk->size = bytereader_read_u32_be(&byte_reader);
	if (IsList(chunk->chunk))
	{
		if (chunk->size < 4)
			return NErr_Malformed;

		ret = NXFileRead(file, chunk->type, 4, &bytes_read);
		if (ret != NErr_Success)
			return ret;

		position+=4;
	}

	
	return NErr_Success;
}
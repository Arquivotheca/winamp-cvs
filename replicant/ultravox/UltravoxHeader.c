#include "UltravoxHeader.h"
#include "foundation/error.h"
#include "foundation/types.h"
#include "nu/ByteReader.h"

int uvox_header_parse(ultravox_header_t header, const void *header_data, size_t header_length)
{
	bytereader_value_t byte_reader;
	uint8_t uvox_sync;

	if (header_length < 6)
		return NErr_NeedMoreData;

	bytereader_init(&byte_reader, header_data, header_length);

	uvox_sync = bytereader_read_u8(&byte_reader);
	
	if (uvox_sync != ULTRAVOX_SYNC)
		return NErr_LostSynchronization;

	header->uvox_sync = uvox_sync;
	header->uvox_qos = bytereader_read_u8(&byte_reader);
	header->uvox_classtype = bytereader_read_u16_be(&byte_reader);
	header->uvox_length = bytereader_read_u16_be(&byte_reader);

	/* pre-calculate these fields to make life easier */
	header->uvox_class = (header->uvox_classtype >> 12) & 0xF;
	header->uvox_type = header->uvox_classtype & 0xFFF;
	return NErr_Success;
}

int uvox_is_data(const ultravox_header_t header)
{
	if (header->uvox_class >= ULTRAVOX_CLASS_DATA_MIN && header->uvox_class <= ULTRAVOX_CLASS_DATA_MAX)
		return NErr_True;
	else
		return NErr_False;
}

int uvox_metadata_parse(ultravox_metadata_header_t metadata, const void *metadata_header, size_t metadata_header_length)
{
	bytereader_value_t byte_reader;
	if (metadata_header_length < 6)
		return NErr_NeedMoreData;

	bytereader_init(&byte_reader, metadata_header, metadata_header_length);

	metadata->metadata_id = bytereader_read_u16_be(&byte_reader);
	metadata->metadata_span = bytereader_read_u16_be(&byte_reader);
	metadata->metadata_index = bytereader_read_u16_be(&byte_reader);
	return NErr_Success;
}
#pragma once

#include "foundation/types.h"

#ifdef __cplusplus
extern "C" {
#endif

	enum
	{
		ULTRAVOX_SYNC = 0x5A,
		ULTRAVOX_CLASS_DATA_MIN = 0x7,
		ULTRAVOX_CLASS_DATA_MAX = 0x8,
	};

	typedef struct UltravoxHeader
	{
		unsigned int uvox_sync;
		unsigned int uvox_qos;
		unsigned int uvox_classtype; // pre-parsed, for easy access if needed
		unsigned int uvox_class;
		unsigned int uvox_type;
		unsigned int uvox_length;

	} UltravoxHeader, *ultravox_header_t;

	int uvox_header_parse(ultravox_header_t header, const void *header_data, size_t header_length);

	/* returns NErr_True / NErr_False !!!! */
	int uvox_is_data(const ultravox_header_t header);


	typedef struct UltravoxMetadataHeader
	{
		uint16_t metadata_id;
		uint16_t metadata_span;
		uint16_t metadata_index;	
	} UltravoxMetadataHeader, *ultravox_metadata_header_t;
	int uvox_metadata_parse(ultravox_metadata_header_t metadata, const void *metadata_header, size_t metadata_header_length);
#ifdef __cplusplus
}
#endif
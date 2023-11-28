#include "api.h"
#include "main.h"
#include "WAVRawReader.h"
#include <limits.h>
#include "nswasabi/ReferenceCounted.h"
#include <new>

int WAVRawReaderService::FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **out_reader, nx_uri_t filename, nx_file_t f, ifc_metadata *parent_metadata)
{
	if (IsMyExtension(filename, EXTENSION_FOR_PLAYBACK))
	{
		WAVRawReader *reader = new (std::nothrow) ReferenceCounted<WAVRawReader>;
		if (!reader)
		{
			return NErr_OutOfMemory;
		}

		int ret = reader->Initialize(f);
		if (ret != NErr_Success)
		{
			reader->Release();
			return ret;
		}

		*out_reader = reader;
		return NErr_Success;
	}
	else
	{
		return NErr_False;
	}
}

WAVRawReader::WAVRawReader()
{
	iff_object = 0;
}

WAVRawReader::~WAVRawReader()
{
	if (iff_object)
		nsiff_destroy(iff_object);	
}

int WAVRawReader::Initialize(nx_file_t file)
{
	int ret = nsiff_create_parser_from_file(&iff_object, file, &iff_callbacks, (WAVParser *)this);
	if (ret != NErr_Success)
		return ret;

	ret = nsiff_parse(iff_object);
	if (ret != NErr_Success)
	{
		if (parse_error != NErr_Success)
			return parse_error;
		else
			return ret;
	}

	nsiff_seek(iff_object, data_position);
	// TODO: bind file to region
	return NErr_Success;	
}

int WAVRawReader::RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	return nsiff_read(iff_object, buffer, buffer_size, bytes_read);
}


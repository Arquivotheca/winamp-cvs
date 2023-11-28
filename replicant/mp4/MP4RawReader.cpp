#include "MP4RawReader.h"
#include "main.h"
#include <limits.h>
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"

bool IsMyExtension(nx_uri_t filename, int search_style);

int MP4RawReaderService::FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **out_reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	if (IsMyExtension(filename, EXTENSION_FOR_METADATA))
	{
		MP4FileHandle mp4 = MP4ReadFile(filename, file);
		if (!mp4) 
			return NErr_FileNotFound;

		MP4RawReader *reader = new (std::nothrow) ReferenceCounted<MP4RawReader>;
		if (!reader)
		{
			MP4Close(mp4); 
			return NErr_OutOfMemory;
		}

		int ret = reader->Initialize(mp4);
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



MP4RawReader::MP4RawReader()
{
	file=0;
	track_num=0;
	number_of_tracks=0;
	current_track = MP4_INVALID_TRACK_ID;
	chunk_position=0;
	chunk_size=0;
	chunk_buffer=0;
}

MP4RawReader::~MP4RawReader()
{
	if (chunk_buffer)
		MP4Free(chunk_buffer);
	MP4Close(file); 
}

int MP4RawReader::Initialize(MP4FileHandle file)
{
	this->file = file;
	number_of_tracks=MP4GetNumberOfTracks(file);
	return NErr_Success;
}

int MP4RawReader::ReadNextChunk()
{
again:
	/* see if it's time to cycle to the next track */
	if (current_track == MP4_INVALID_TRACK_ID)
	{
		if (track_num == number_of_tracks)
			return NErr_EndOfFile;

		current_track = MP4FindTrackId(file, track_num);
		if (current_track == MP4_INVALID_TRACK_ID)
			return NErr_EndOfFile;

		track_num++;

		const char* trackType = MP4GetTrackType(file, current_track);
		if (!MP4_IS_AUDIO_TRACK_TYPE(trackType) && !MP4_IS_VIDEO_TRACK_TYPE(trackType))
		{
			current_track = MP4_INVALID_TRACK_ID;
			goto again;
		}

		chunk_id = 1;
		number_of_chunks= MP4GetTrackNumberOfChunks(file, current_track);		
	}

	/* see if we've read all of our samples */
	if (chunk_id > number_of_chunks)
	{
		current_track = MP4_INVALID_TRACK_ID;
		goto again;
	}

	bool readSuccess = MP4ReadChunk(file, current_track, chunk_id, &chunk_buffer, &chunk_size);
	if (!readSuccess)
		return NErr_Error;

	chunk_position=0;
	chunk_id++;
	return NErr_Success;
}

int MP4RawReader::RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	if (buffer_size > INT_MAX)
		return NErr_BadParameter;

	if (chunk_position==chunk_size)
	{
		MP4Free(chunk_buffer);
		chunk_buffer=0;
	}

	if (!chunk_buffer)
	{
		int ret = ReadNextChunk();
		if (ret != NErr_Success)
			return ret;
	}

	size_t to_read = chunk_size-chunk_position;
	if (to_read > buffer_size)
		to_read = buffer_size;

	memcpy(buffer, &chunk_buffer[chunk_position], to_read);
	chunk_position += to_read;
	*bytes_read = to_read;
	return NErr_Success;
}

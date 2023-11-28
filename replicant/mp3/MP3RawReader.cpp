#include "main.h"
#include "MP3RawReader.h"
#include <limits.h>
#include "nswasabi/ReferenceCounted.h"
#include <new>

int MP3RawReaderService::FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **out_reader, nx_uri_t filename, nx_file_t f, ifc_metadata *parent_metadata)
{
	if (IsMyExtension(filename))
	{
		GioFile *file = new (std::nothrow) GioFile();
		if (!file)
			return NErr_OutOfMemory;

		if (file->Open(filename, f) != SSC_OK)
		{
			delete file;
			return NErr_FileNotFound;
		}

		MP3RawReader *reader = new (std::nothrow) ReferenceCounted<MP3RawReader>;
		if (!reader)
		{
			file->Close();
			delete file;
			return NErr_OutOfMemory;
		}

		int ret = reader->Initialize(file);
		if (ret != NErr_Success)
		{
			reader->Release();
			file->Close();
			delete file;
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

MP3RawReader::MP3RawReader()
{
	file = 0;
}

MP3RawReader::~MP3RawReader()
{
	if (file)
		file->Close();
	delete file;
}

int MP3RawReader::Initialize(GioFile *file)
{
	this->file = file;
	return NErr_Success;
}

int MP3RawReader::RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	if (buffer_size > INT_MAX)
		return NErr_BadParameter;

	int file_bytes_read=0;
	SSC ret = file->Read(buffer, (int)buffer_size, &file_bytes_read);

	if (SSC_SUCCESS(ret))
	{
		*bytes_read = (size_t)file_bytes_read;
		if (!file_bytes_read && file->IsEof())
			return NErr_EndOfFile;

		return NErr_Success;
	}
	else
		return NErr_Error;
}


#include "FLACRawReader.h"
#include "main.h"
#include "FLACFileCallbacks.h"
#include "nx/nxpath.h"
#include "nswasabi/ReferenceCounted.h"

int FLACRawReaderService::FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **out_reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	if (NXPathMatchExtension(filename, flac_extension) == NErr_Success)
	{
		FLACRawReader *raw_reader = new (std::nothrow) ReferenceCounted<FLACRawReader>();
		if (!raw_reader)
		{
			return NErr_OutOfMemory;
		}

		int ret = raw_reader->Initialize(file, parent_metadata);
		if (ret != NErr_Success)
		{
			delete raw_reader;
			return ret;
		}
		
		*out_reader = raw_reader;		
		return NErr_Success;
	}
	return NErr_False;
}

static void FLACOnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	client_data=client_data; // dummy line so i can set a breakpoint
}

static void FLACOnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	// TODO: we might want to get metadata (eventually)
}

static FLAC__StreamDecoderWriteStatus FLACOnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLACRawReader::FLACRawReader()
{
	file=0;
	decoder=0;
}

FLACRawReader::~FLACRawReader()
{
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
	NXFileRelease(file);
}

int FLACRawReader::Initialize(nx_file_t file, ifc_metadata *parent_metadata)
{
	this->file = NXFileRetain(file);
	decoder = FLAC__stream_decoder_new();
	if (!decoder)
		return NErr_OutOfMemory;

	FLAC__stream_decoder_set_metadata_ignore(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	client_data.SetFile(file);
	client_data.SetObject(this);
	FLAC__StreamDecoderInitStatus status =  FLAC__stream_decoder_init_stream(
		decoder,
		FLAC_NXFile_Read,
		FLAC_NXFile_Seek,
		FLAC_NXFile_Tell,
		FLAC_NXFile_Length,
		FLAC_NXFile_EOF,  // or NULL
		FLACOnAudio,
		FLACOnMetadata,
		FLACOnError,
		&client_data);

	if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		return NErr_Error;

	FLAC__stream_decoder_process_until_end_of_metadata(decoder);
	uint64_t position;
	FLAC__stream_decoder_get_decode_position(decoder, &position);
	FLAC__stream_decoder_delete(decoder);
	decoder=0;
	NXFileSeek(file, position);

	return NErr_Success;
}

int FLACRawReader::RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read)
{
	return NXFileRead(file, buffer, buffer_size, bytes_read);
}
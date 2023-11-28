#include "api.h"
#include "FLACPlayback.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "nx/nxsleep.h"
#include "nswasabi/ReferenceCounted.h"
#include "FLACFileCallbacks.h"



static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

FLACPlayback::FLACPlayback()
{
	file=0;
	parent=0;
	stream_info=0;

	decoder=0;
	output_pointers=0;
	sample_rate=0;

	metadata=0;
	flac_position=0;
	decode_ret=NErr_Success;
	output_opened=false;
	start_frame=0;
}

FLACPlayback::~FLACPlayback()
{
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
	// stream_info is "owned" by metadata, so we don't delete it here
	if (metadata)
		metadata->Release();

	if (file)
		NXFileRelease(file);

	free(output_pointers);
}

int FLACPlayback::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	this->file = file;
	if (file)
		NXFileRetain(file);

	this->parent = parent;

	return Init(parent_metadata);
}

ns_error_t FLACPlayback::FilePlayback_Interrupt(Agave_Seek *resume_information)
{
	resume_information->position.sample_frames = flac_position;
	if (decoder)
		FLAC__stream_decoder_finish(decoder);
	decoder=0;
	if (metadata)
		metadata->Release();
	metadata=0;
	if (file)
		NXFileRelease(file);
	file=0;
	return NErr_Success;
}

ns_error_t FLACPlayback::FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)
{
	this->file = file;

	int ret = Init(parent_metadata);
	if (ret != NErr_Success)
		return ret;

	FLAC__stream_decoder_seek_absolute(decoder, (uint64_t)flac_position);
	if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
	{
		FLAC__stream_decoder_flush(decoder);
	}	
	FLAC__stream_decoder_get_decode_position(decoder, &flac_position);
	return NErr_Success;
}

void FLACPlayback::FilePlayback_Close()
{
	if (decoder)
		FLAC__stream_decoder_finish(decoder);
	decoder=0;
	if (metadata)
		metadata->Release();
	metadata=0;
	if (file)
		NXFileRelease(file);
	file=0;
}

ns_error_t FLACPlayback::FilePlayback_Seekable()
{
	return NErr_True;
}

ns_error_t FLACPlayback::FilePlayback_GetMetadata(ifc_metadata **out_metadata)
{
	if (metadata)
	{
		*out_metadata = metadata;
		metadata->Retain();
		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t FLACPlayback::FilePlayback_GetLength(double *length, ns_error_t *exact)
{
	*length = ((double)(FLAC__int64)stream_info->data.stream_info.total_samples / (double)stream_info->data.stream_info.sample_rate);
	*exact = NErr_True;
	return NErr_Success;
}

ns_error_t FLACPlayback::FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)
{
	if (metadata->GetReal(MetadataKeys::BITRATE, 0, bitrate) == NErr_Success)
	{
		*exact = NErr_True;
		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t FLACPlayback::FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)
{
	switch(seek->position_type)
	{
	case AGAVE_PLAYPOSITION_SECONDS:
		{

			double frames = seek->position.seconds * (double)stream_info->data.stream_info.sample_rate;
			if (output_opened)
			{
				FLAC__stream_decoder_seek_absolute(decoder, (uint64_t)frames);
				if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
				{
					FLAC__stream_decoder_flush(decoder);
				}	
			}
			else
			{
				start_frame = (uint64_t)frames;
			}
			*seek_error = NErr_Success;
			*new_position = seek->position.seconds;
		}
		return NErr_Success;
	case AGAVE_PLAYPOSITION_SAMPLE_FRAMES:
		{

			double seconds = (double)seek->position.sample_frames / (double)stream_info->data.stream_info.sample_rate;
			if (output_opened)
			{
				FLAC__stream_decoder_seek_absolute(decoder, seek->position.sample_frames);
				if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
				{
					FLAC__stream_decoder_flush(decoder);
				}
			}
			else
			{
				start_frame = seek->position.sample_frames;
			}
			*seek_error = NErr_Success;
			*new_position = seconds;
		}
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

ns_error_t FLACPlayback::FilePlayback_DecodeStep()
{
	if (!output_opened)
	{
		// open audio output
		ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
		parameters.audio.sample_rate = stream_info->data.stream_info.sample_rate;
		parameters.audio.format_flags = nsaudio::FORMAT_FLAG_NONINTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
		parameters.audio.format_type = nsaudio::format_type_pcm;
		parameters.audio.bytes_per_sample = sizeof(FLAC__int32);
		parameters.audio.bits_per_sample = stream_info->data.stream_info.bits_per_sample;
		parameters.audio.number_of_channels = stream_info->data.stream_info.channels;

		sample_rate = stream_info->data.stream_info.sample_rate;

		output_pointers = (const FLAC__int32 **)calloc(parameters.audio.number_of_channels, sizeof(const FLAC__int32 *));
		if (!output_pointers)
			return NErr_OutOfMemory;

		ns_error_t ret = parent->OpenOutput(&parameters);
		if (ret != NErr_Success)
			return ret;

		output_opened=true;
	}

	if (start_frame)
	{
		FLAC__stream_decoder_seek_absolute(decoder, start_frame);
		if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
		{
			FLAC__stream_decoder_flush(decoder);
		}	
		start_frame=0;
	}

	FLAC__bool decode_successful = FLAC__stream_decoder_process_single(decoder);
	FLAC__StreamDecoderState flac_state = FLAC__stream_decoder_get_state(decoder);
	if (decode_ret != NErr_Success)
		return decode_ret;
	else if (flac_state == FLAC__STREAM_DECODER_END_OF_STREAM)
	{
		return NErr_EndOfFile;
	}

	return NErr_Success;
}


int FLACPlayback::Init(ifc_metadata *parent_metadata)
{
	metadata = new (std::nothrow) ReferenceCounted<MetadataChain<FLACMetadata> >;
	if (!metadata)
		return NErr_OutOfMemory;

	metadata->SetParentMetadata(parent_metadata);

	// create decoder object
	if (!decoder)
	{
		decoder = FLAC__stream_decoder_new();
		if (!decoder)
			return NErr_OutOfMemory; // uhh ohh
	}

	// set some config stuff on the decoder
	FLAC__stream_decoder_set_md5_checking(decoder, false);
	FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_PICTURE);	

	// TODO: this should get merged into FilePlayback
	nx_file_stat_s file_stats;
	int ret = NXFileStat(file, &file_stats);
	if (ret != NErr_Success)
	{
		return ret;
	}

	metadata->SetFileStats(&file_stats);
	client_data.SetFile(file);
	client_data.SetObject(this);
	if (FLAC__stream_decoder_init_stream(
		decoder,
		FLAC_NXFile_Read,
		FLAC_NXFile_Seek,
		FLAC_NXFile_Tell,
		FLAC_NXFile_Length,
		FLAC_NXFile_EOF,  // or NULL
		OnAudio,
		OnMetadata,  // or NULL
		OnError,
		&client_data
		) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		return NErr_Error; // decoder initialization failed
	}

	stream_info = 0;
	if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder))
	{
		return NErr_Error; 
	}

	// make sure we got stream info
	if (!stream_info)
		return NErr_OutOfMemory;
	// yay! if we got here, we're all good
	return NErr_Success;
}

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	client_data = client_data; // dummy line so i can set a breakpoint
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FLAC] OnError called, status=%u", status);
#endif
}

void FLACPlayback::OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	FLACPlayback *playback = FLAC_GetObject<FLACPlayback>(client_data);		
	switch (metadata->type)
	{
	case FLAC__METADATA_TYPE_STREAMINFO:
		playback->stream_info = FLAC__metadata_object_clone(metadata);
		playback->metadata->OwnStreamInfo(playback->stream_info);
		break;

	case FLAC__METADATA_TYPE_VORBIS_COMMENT:
		playback->metadata->OwnMetadataBlock(FLAC__metadata_object_clone(metadata));
		break;

	case FLAC__METADATA_TYPE_PICTURE:
		playback->metadata->OwnPicture(FLAC__metadata_object_clone(metadata));
		break;
	}
}

FLAC__StreamDecoderWriteStatus FLACPlayback::Internal_OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[])
{
	size_t byte_length;
	/* channels is 3 bits, blocksize is 16 bits and sizeof(FLAC__int32) is 3 bits, so this multiply is safe on 32-bit machines */
	byte_length = sizeof(FLAC__int32)*stream_info->data.stream_info.channels*frame->header.blocksize;

	for (size_t c=0;c<stream_info->data.stream_info.channels;c++)
	{
		output_pointers[c] = buffer[c];
	}

	/* write to output */
	flac_position = frame->header.number.sample_number;

	double position = ((double)flac_position / (double)sample_rate);
	size_t frames_written=0;
	decode_ret = parent->OutputNonInterleaved(output_pointers, byte_length, &frames_written, position);
	flac_position = frame->header.number.sample_number + frames_written;	
	if (decode_ret != NErr_Success && decode_ret != NErr_NoAction)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;	

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

}

FLAC__StreamDecoderWriteStatus FLACPlayback::OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	FLACPlayback *player = FLAC_GetObject<FLACPlayback>(client_data);		
	return player->Internal_OnAudio(decoder, frame, buffer);
}

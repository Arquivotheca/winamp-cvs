#include "FLACDecoderCallback.h"
#include "decode/svc_decode.h"
#include "FLACFileCallbacks.h"

FLACDecoderCallback::FLACDecoderCallback()
{
	callback=0;
	decoder=0;
	flags=0;
	aborted=false;
	file=0;
}

FLACDecoderCallback::~FLACDecoderCallback()
{
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
	decoder=0;

	NXFileRelease(file);

}

static void FLACOnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	client_data=client_data; // dummy line so i can set a breakpoint
}

void FLACDecoderCallback::FLACOnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	FLACDecoderCallback *state = FLAC_GetObject<FLACDecoderCallback>(client_data);

	switch(metadata->type)
	{
	case FLAC__METADATA_TYPE_STREAMINFO:
		state->stream_info = FLAC__metadata_object_clone(metadata);
		break;
	case FLAC__METADATA_TYPE_VORBIS_COMMENT:
		if (!(state->flags & svc_decode::FLAG_NO_METADATA))
		{
			if (state->metadata_block)
				FLAC__metadata_object_delete(state->metadata_block);
			state->metadata_block = FLAC__metadata_object_clone(metadata);
		}
		break;
	default:
		break;
	}
}

FLAC__StreamDecoderWriteStatus FLACDecoderCallback::FLACOnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	FLACDecoderCallback *state = FLAC_GetObject<FLACDecoderCallback>(client_data);

	int ret = state->callback->OnAudio((const void *)buffer, frame->header.blocksize);
	if (ret == NErr_Success)
		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	else
	{
		state->aborted=true; /* annoyingly, returning FLAC__STREAM_DECODER_WRITE_STATUS_ABORT won't propagate to FLAC__stream_decoder_get_state() when using FLAC__stream_decoder_process_single */
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
}

int FLACDecoderCallback::Initialize(nx_uri_t filename, nx_file_t file, FLAC__StreamDecoder *decoder, int flags, nsaudio::Parameters *parameters, ifc_metadata *parent_metadata)
{
	this->decoder = decoder;
	this->flags = flags;
	this->file = NXFileRetain(file);

	SetParentMetadata(parent_metadata);

	// set some config stuff on the decoder
	if (flags & svc_decode::FLAG_VALIDATION)
		FLAC__stream_decoder_set_md5_checking(decoder, true);
	else
		FLAC__stream_decoder_set_md5_checking(decoder, false);

	if (flags & svc_decode::FLAG_NO_METADATA)
		FLAC__stream_decoder_set_metadata_ignore(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	else
		FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

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
	{
		/* TODO: switch/case on the various error messages */
		return NErr_Error;
	}


	FLAC__bool success = FLAC__stream_decoder_process_until_end_of_metadata(decoder);
	if (!success)
	{
		FLAC__stream_decoder_finish(decoder);
		/* TODO: switch/case on FLAC__stream_decoder_get_state() */
		return NErr_Error;
	}

	parameters->sample_rate = stream_info->data.stream_info.sample_rate;
	parameters->format_type = nsaudio::format_type_pcm;
	parameters->format_flags = nsaudio::FORMAT_FLAG_NONINTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->bytes_per_sample = sizeof(FLAC__int32);
	parameters->bits_per_sample = stream_info->data.stream_info.bits_per_sample;
	parameters->number_of_channels = stream_info->data.stream_info.channels;
	parameters->channel_layout = 0; /* TODO! */

	return NErr_Success;
}

int FLACDecoderCallback::AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)
{
	ifc_metadata *metadata_retain = this;
	metadata_retain->Retain();
	*metadata = metadata_retain;
	return NErr_Success;
}

int FLACDecoderCallback::AudioDecoderCallback_GetFrameSize(size_t *frame_size)
{
	*frame_size = stream_info->data.stream_info.max_blocksize;
	return NErr_Success;

}

int FLACDecoderCallback::AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)
{
	this->callback = callback;

	if (FLAC__stream_decoder_process_single(decoder) == 0)
	{
		FLAC__StreamDecoderState flac_state = FLAC__stream_decoder_get_state(decoder);
		FLAC__stream_decoder_finish(decoder);

		if (flac_state == FLAC__STREAM_DECODER_ABORTED || aborted)
			return NErr_Interrupted;

		/* TODO: find out a better error code */
		return NErr_Error;
	}

	FLAC__StreamDecoderState FLACstate = FLAC__stream_decoder_get_state(decoder);
	if (FLACstate == FLAC__STREAM_DECODER_END_OF_STREAM) /* break out if we hit EOF */
	{
		FLAC__stream_decoder_finish(decoder);
		return NErr_EndOfFile; 	/* TODO: find out a better error code */
	}

	return NErr_Success;
}

int FLACDecoderCallback::AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)
{
	int ret;

	for (;;)
	{
		ret = AudioDecoderCallback_DecodeStep(callback);
		if (ret == NErr_EndOfFile)
			return NErr_Success;
		else if (ret != NErr_Success)
			return ret;
	}
}

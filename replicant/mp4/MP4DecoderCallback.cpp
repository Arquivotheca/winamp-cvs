#include "MP4DecoderCallback.h"
#include "decode/svc_decode.h"

MP4DecoderCallback::MP4DecoderCallback()
{
	mp4_file=0;
	audio_decoder=0;
	flags=0;
	pregap=0;
	frame_size=0;
	channels=0;
	mp4_file_object=0;
}

MP4DecoderCallback::~MP4DecoderCallback()
{
	if (audio_decoder)
		audio_decoder->Release();
	if (mp4_file)
		MP4CloseFile(mp4_file);
	if (mp4_file_object)
		mp4_file_object->ifc_mp4file::Release();
}


int MP4DecoderCallback::Initialize(MP4FileHandle mp4_file, ifc_mp4audiodecoder *decoder, int flags, nsaudio::Parameters *parameters, MetadataChain<MP4FileObject> *mp4_file_object)
{
	int ret;
	this->audio_decoder = decoder;
	this->flags = flags;
	this->mp4_file = mp4_file;
	this->mp4_file_object = mp4_file_object;

	// set some config stuff on the decoder
#if 0
	if (flags & svc_decode::FLAG_VALIDATION)
		FLAC__stream_decoder_set_md5_checking(decoder, true);
	else
		FLAC__stream_decoder_set_md5_checking(decoder, false);

	if (flags & svc_decode::FLAG_NO_METADATA)
		FLAC__stream_decoder_set_metadata_ignore(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	else
		FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
#endif

	/* we need to get the audio parameters.  
	but some decoders need to decode a few frames before it's valid, they'll signal by returning NErr_NeedMoreData
	we _could_ buffer the results, but for ease of implementation, we're going to decode a few frames and reset when we're done */
	ifc_audioout::Parameters audio_parameters={0,};
		audio_parameters.sizeof_parameters = sizeof(ifc_audioout::Parameters);
	bool need_reset=false;
	for (;;)
	{	
		ret = audio_decoder->FillAudioParameters(&audio_parameters);
		if (ret == NErr_Success)
			break; /* good to go */
		else if (ret == NErr_NeedMoreData) /* ugh, have to decode a frame or two */
		{
			const void *dummy_buffer;
			size_t dummy_size;
			double dummy_start, dummy_end;
			ret = decoder->Decode(&dummy_buffer, &dummy_size, &dummy_start, &dummy_end);
			if (ret != NErr_Success)
				return ret;
			need_reset=true;
		}
		else if (ret != NErr_Success)
			return ret;
	}

	if (need_reset)
		decoder->Seek(1);

	*parameters = audio_parameters.audio;
	pregap=audio_parameters.frames_trim_start;

	channels=audio_parameters.audio.number_of_channels;

	frame_size=audio_parameters.audio.bytes_per_sample * channels;
	audio_decoder->Retain();
	mp4_file_object->ifc_mp4file::Retain();
	return NErr_Success;
}
	
int MP4DecoderCallback::AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)
{
	ifc_metadata *metadata_retain = mp4_file_object;
	metadata_retain->Retain();
	*metadata = metadata_retain;
	return NErr_Success;
}

int MP4DecoderCallback::AudioDecoderCallback_GetFrameSize(size_t *frame_size)
{
	//return audio_decoder->GetFrameSize(frame_size);
	return NErr_NotImplemented;
}

int MP4DecoderCallback::AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)
{
	const void *output_buffer;
	size_t bytes_available;
	double start_position, end_position;
	int ret = audio_decoder->Decode(&output_buffer, &bytes_available, &start_position, &end_position);
	if (ret != NErr_Success)
		return ret;

	/* do pre-gap stuff. TODO: deal with post-gap. we might end up totally redoing this, to some extent */
	size_t skip=0;
	size_t frames_available = bytes_available / frame_size;
	if (pregap)
	{
		if (frames_available > pregap)
		{
			skip = pregap * channels;
			frames_available -= pregap;
			pregap=0;
		}
		else
		{
			/* need to completely skip this frame */
			pregap -= frames_available;
			return AudioDecoderCallback_DecodeStep(callback);
		}
	}
	// TODO: this assumes interleaved.  deal with non-interleaved separately
	if (callback->OnAudio((const uint8_t *)output_buffer + skip, frames_available) != NErr_Success)
		return NErr_Interrupted;
	else
		return NErr_Success;
}

int MP4DecoderCallback::AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)
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
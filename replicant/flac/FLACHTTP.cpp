#include "api.h"
#include "FLACHTTP.h"
#include "jnetlib/jnetlib.h"
#include "service/ifc_servicefactory.h"
#include "nswasabi/ReferenceCounted.h"
#include "replaygain/ifc_replaygain_settings.h"
#include <new>
#ifdef __ANDROID__
#include <android/log.h>
#endif

static const char *flac_mime_types[] = {
	"audio/flac",
};


const char *FLACHTTPService::HTTPDemuxerService_EnumerateAcceptedTypes(size_t i)
{
	if (i < sizeof(flac_mime_types) / sizeof(*flac_mime_types))
		return flac_mime_types[i];
	else
		return 0;

	return 0;
}

const char *FLACHTTPService::HTTPDemuxerService_GetUserAgent()
{
	return 0;
}

void FLACHTTPService::HTTPDemuxerService_CustomizeHTTP(jnl_http_t http)
{
}

static bool AcceptableMIMEType(const char *mime_type)
{
	for(size_t i=0;i< sizeof(flac_mime_types) / sizeof(*flac_mime_types);i++)
	{
		if (!strcmp(mime_type, flac_mime_types[i]))
			return true;
	}
	return false;
}

NError FLACHTTPService::HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass)
{
	if (pass == 0)
		return NErr_TryAgain;

	if (pass == 1) /* wait for second pass to let shoutcast demuxer have a chance */
	{
		const char *content_type = jnl_http_getheader(http, "content-type");
		if (content_type && AcceptableMIMEType(content_type))
		{
			FLACHTTP *flac_demuxer = new (std::nothrow) ReferenceCounted<FLACHTTP>;
			if (!flac_demuxer)
				return NErr_OutOfMemory;
			flac_demuxer->Initialize(uri, http);
			*demuxer = flac_demuxer;
			return NErr_Success;
		}
	}

	if (pass == 2)
	{
		// TODO: check based on file extension
	}

	return NErr_False;
}

/* ----------------------------------------- */
FLACHTTP::FLACHTTP() 
{
	http=0;
	uri=0;
	file=0;
	stream_info=0;
	decoder=0;
	metadata=0;
	output_pointers=0;
	sample_rate=0;
	decode_ret=NErr_Success;
	flac_position=0;
	out=0;
}

FLACHTTP::~FLACHTTP()
{
	jnl_http_release(http);
	NXURIRelease(uri);
	NXFileRelease(file);
	// stream_info is "owned" by metadata, so we don't delete it here
	if (metadata)
		metadata->Release();
	if (decoder)
		FLAC__stream_decoder_delete(decoder);
	free(output_pointers);
}

int FLACHTTP::Initialize(nx_uri_t uri, jnl_http_t _http)
{
	this->uri = NXURIRetain(uri);
	http=jnl_http_retain(_http);
	return NErr_Success;
}

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	client_data = client_data; // dummy line so i can set a breakpoint
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FLAC] OnError called, status=%u", status);
#endif
}

void FLACHTTP::OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	FLACHTTP *playback = FLAC_GetObject<FLACHTTP>(client_data);		
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

FLAC__StreamDecoderWriteStatus FLACHTTP::Internal_OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[])
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

	player->SetPosition((double)flac_position / (double)sample_rate - out->Latency());
	size_t frame_size = sizeof(FLAC__int32)*stream_info->data.stream_info.channels;
	while (byte_length)
	{
		size_t to_write = out->CanWrite();
		if (to_write)
		{
			if (byte_length < to_write)
				to_write = byte_length;

			decode_ret = out->Output(output_pointers, to_write);
			if (decode_ret != NErr_Success)
			{
				out->Done();
				out->Release();
				out=0;
				return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
			}
			byte_length -= to_write;
			size_t frames = to_write/frame_size;
			flac_position += frames;
			for (size_t c=0;c<stream_info->data.stream_info.channels;c++)
			{
				output_pointers[c] += frames;
			}
			player->SetPosition((double)flac_position/ (double)sample_rate - out->Latency());
		}
		else
		{
			if (paused)
			{
				/* if we're paused, we need to sit and wait until we're eiter unpaused or stopped */
				for (;;)
				{
					int ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
					if (ret == ifc_http::WAKE_STOP)
					{
						out->Stop();
						out->Release();
						out=0;
						player->OnStopped();
						return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
					}
					else if (ret == ifc_http::WAKE_UNPAUSE)
					{
						out->Pause(0);
						paused=false;
						break;
					}
					else if (ret == ifc_http::WAKE_PAUSE)
					{
						out->Pause(1);
						paused=true;
					}
				}
			}
			else
			{
				int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
				if (ret == ifc_http::WAKE_STOP)
				{
					out->Stop();
					out->Release();
					out=0;
					player->OnStopped();
					return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
				}
			}
		}
	}

	flac_position = frame->header.number.sample_number + frames_written;	
	if (decode_ret != NErr_Success && decode_ret != NErr_NoAction)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;	

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

}

FLAC__StreamDecoderWriteStatus FLACHTTP::OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	FLACHTTP *player = FLAC_GetObject<FLACHTTP>(client_data);		
	return player->Internal_OnAudio(decoder, frame, buffer);
}


int FLACHTTP::HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)
{
	this->player=player;
	this->http_parent = http_parent;
	paused=false;

	/* --- start progressive downloader --- */

	ns_error_t ret = NXFileOpenProgressiveDownloader(&file, uri, nx_file_FILE_read_binary, http, WASABI2_API_APP->GetUserAgent());
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	metadata = new (std::nothrow) ReferenceCounted<FLACMetadata>;
	if (!metadata)
	{
		player->OnError(NErr_OutOfMemory);
		return NErr_OutOfMemory;
	}

	decoder = FLAC__stream_decoder_new();
	if (!decoder)
	{
		player->OnError(NErr_OutOfMemory);
		return NErr_OutOfMemory;
	}

	// set some config stuff on the decoder
	FLAC__stream_decoder_set_md5_checking(decoder, false);
	FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_PICTURE);	

	/* --- try to parse the file --- */
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

	output_pointers = (const FLAC__int32 **)calloc(stream_info->data.stream_info.channels, sizeof(const FLAC__int32 *));
	if (!output_pointers)
		return NErr_OutOfMemory;

	sample_rate = stream_info->data.stream_info.sample_rate;

	/* --- tell the player about various things --- */
	if (http_parent->Seekable() == NErr_True)
		player->SetSeekable(1);
	else 
		player->SetSeekable(0);

	double length = ((double)(FLAC__int64)stream_info->data.stream_info.total_samples / (double)stream_info->data.stream_info.sample_rate);
	player->SetLength(length);

	bool opened=false;

	player->SetMetadata(metadata);
	player->OnReady();

	/* --- wait for playback to start --- */
	for (;;)
	{
		ret = http_parent->Wake(ifc_http::WAKE_PLAY|ifc_http::WAKE_STOP); 
		if (ret == ifc_http::WAKE_PLAY)
		{
			break;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
	parameters.audio.sample_rate = stream_info->data.stream_info.sample_rate;
	parameters.audio.format_flags = nsaudio::FORMAT_FLAG_NONINTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters.audio.format_type = nsaudio::format_type_pcm;
	parameters.audio.bytes_per_sample = sizeof(FLAC__int32);
	parameters.audio.bits_per_sample = stream_info->data.stream_info.bits_per_sample;
	parameters.audio.number_of_channels = stream_info->data.stream_info.channels;

	/* read replaygain info */
	ifc_replaygain_settings *replaygain_settings;
	if (secondary_parameters && secondary_parameters->QueryInterface(&replaygain_settings) == NErr_Success)
	{
		if (replaygain_settings->GetGain(metadata, &parameters.gain, 0) == NErr_Success)
		{
#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FilePlayback] using replaygain adjustment of %f", parameters.gain);
#endif
			parameters.extended_fields_flags |= ifc_audioout::EXTENDED_FLAG_REPLAYGAIN;
		}
		replaygain_settings->Release();
	}

	ret = http_parent->AudioOpen(&parameters, &out);
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	opened=true;
	if (paused)
		out->Pause(1);
	else
		out->Pause(0);

	/* --- TODO: prebuffer based on bitrate --- */
prebuffer:

	/* --- playback loop --- */
	for (;;)
	{
		/* check for kill switch */
		int ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
		if (ret == ifc_http::WAKE_PAUSE)
		{
			if (out)
				out->Pause(1);
			paused=true;
			continue;
		}
		else if (ret== ifc_http::WAKE_UNPAUSE)
		{
			if (out)
				out->Pause(0);
			paused=false;
			continue;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			if (out)
			{
				out->Stop();
				out->Release();
				out=0;
			}
			player->OnStopped();
			return NErr_Success;
		}


		Agave_Seek *seek = http_parent->GetSeek();
		if (seek)
		{
			switch(seek->position_type)
			{
			case AGAVE_PLAYPOSITION_SECONDS:
				{
					double position = seek->position.seconds;
					double frames = position * (double)stream_info->data.stream_info.sample_rate;
					FLAC__stream_decoder_seek_absolute(decoder, (uint64_t)frames);
					if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
					{
						FLAC__stream_decoder_flush(decoder);
					}	

					if (out)
						out->Flush(position);

					player->OnSeekComplete(NErr_Success, position);
					http_parent->FreeSeek(seek);
					goto prebuffer;

				}
				break;
			}

			http_parent->FreeSeek(seek);
		}

		if (paused) /* a seek will break us out of Wake, so we need to account for that */
			continue;

		const uint8_t *decoded_data=0;
		size_t decoded_bytes=0;
		double decoded_position=0, end_position=0;

		FLAC__bool decode_successful = FLAC__stream_decoder_process_single(decoder);
		FLAC__StreamDecoderState flac_state = FLAC__stream_decoder_get_state(decoder);
		if (decode_ret != NErr_Success)
		{
						if (out)
			{
				out->Done();
				out->Release();
				out=0;
			}

			player->OnError(NErr_Error);
			return decode_ret;
		}
		else if (flac_state == FLAC__STREAM_DECODER_ABORTED)
		{
			return NErr_Success;
		}
		else if (flac_state == FLAC__STREAM_DECODER_END_OF_STREAM)
		{
			if (out)
			{
				out->Done();
				out->Release();
				out=0;
			}

			player->OnEndOfFile();
			return NErr_EndOfFile;
		}
					else if (!decode_successful)
		{
			// some other error - abort playback
			// if we can find FLAC files with errors, we might be able to gracefully handle some errors
			if (out) // this will be NULL if the error was already handled inside Internal_OnAudio
			{
				out->Done();
				out->Release();
				player->OnError(NErr_Error); // TODO: find better error code
			}
			return NErr_Success;
		}


	}

	return NErr_Success;
}

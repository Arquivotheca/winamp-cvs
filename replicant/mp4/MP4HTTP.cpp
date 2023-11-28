#include "api.h"
#include "MP4HTTP.h"
#include "jnetlib/jnetlib.h"
#include "service/ifc_servicefactory.h"
#include "nswasabi/ReferenceCounted.h"
#include "MP4FileObject.h"
#include "ifc_mp4audiodecoder.h"
#include "svc_mp4decoder.h"
#include "replaygain/ifc_replaygain_settings.h"
#include <new>
#ifdef __ANDROID__
#include <android/log.h>
#endif

static const char *mpeg4_audio_mime_types[] = {
	"audio/mp4",
	"audio/m4a",
};


const char *MP4HTTPService::HTTPDemuxerService_EnumerateAcceptedTypes(size_t i)
{
	if (i < sizeof(mpeg4_audio_mime_types) / sizeof(*mpeg4_audio_mime_types))
		return mpeg4_audio_mime_types[i];
	else
		return 0;

	return 0;
}

const char *MP4HTTPService::HTTPDemuxerService_GetUserAgent()
{
	return 0;
}

void MP4HTTPService::HTTPDemuxerService_CustomizeHTTP(jnl_http_t http)
{
}

static bool AcceptableMIMEType(const char *mime_type)
{
	for(size_t i=0;i< sizeof(mpeg4_audio_mime_types) / sizeof(*mpeg4_audio_mime_types);i++)
	{
		if (!strcmp(mime_type, mpeg4_audio_mime_types[i]))
			return true;
	}
	return false;
}

NError MP4HTTPService::HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass)
{
	if (pass == 0)
		return NErr_TryAgain;

	if (pass == 1) /* wait for second pass to let shoutcast demuxer have a chance */
	{
		const char *content_type = jnl_http_getheader(http, "content-type");
		if (content_type && AcceptableMIMEType(content_type))
		{
			MP4HTTP *mp4_demuxer = new (std::nothrow) ReferenceCounted<MP4HTTP>;
			if (!mp4_demuxer)
				return NErr_OutOfMemory;
			mp4_demuxer->Initialize(uri, http);
			*demuxer = mp4_demuxer;
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
MP4HTTP::MP4HTTP() 
{
	http=0;
	uri=0;
	file=0;
	mp4_file_object=0;
	audio_decoder=0;
}

MP4HTTP::~MP4HTTP()
{
	jnl_http_release(http);
	NXURIRelease(uri);
	NXFileRelease(file);
	if (mp4_file_object)
		mp4_file_object->ifc_mp4file::Release();
	if (audio_decoder)
		audio_decoder->Release();
}

int MP4HTTP::Initialize(nx_uri_t uri, jnl_http_t _http)
{
	this->uri = NXURIRetain(uri);
	http=jnl_http_retain(_http);
	return NErr_Success;
}

static ifc_mp4audiodecoder *GetAudioDecoder(MP4FileHandle mp4_file, MP4TrackId mp4_track, ifc_mp4file *mp4_file_object)
{

	GUID mp4_decoder_guid = svc_mp4decoder::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(mp4_decoder_guid, n++))
	{
		svc_mp4decoder *l = (svc_mp4decoder*)sf->GetInterface();
		if (l)
		{
			ifc_mp4audiodecoder *decoder=0;
			int ret = l->CreateAudioDecoder(mp4_file_object, mp4_track, &decoder);
			l->Release();

			if (ret == NErr_Success && decoder)
				return decoder;
		}
	}
	return 0;
}

int MP4HTTP::HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters)
{
	ifc_audioout *out=0;
	bool paused=false;

	/* --- start progressive downloader --- */

	ns_error_t ret = NXFileOpenProgressiveDownloader(&file, uri, nx_file_FILE_read_binary, http, WASABI2_API_APP->GetUserAgent());
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return ret;
	}

	/* --- try to parse the file --- */
	MP4FileHandle mp4_file = MP4ReadFile(uri, file, 0);
	if (!mp4_file)
	{
		player->OnError(NErr_FileNotFound);
		return NErr_FileNotFound;
	}

	/* --- build mp4 wrapper object --- */
	mp4_file_object = new (std::nothrow) ReferenceCounted<MP4FileObject>;
	if (!mp4_file_object)
	{
		/* manually close here.  mp4_file_object "owns" mp4_file but we weren't able to make it */
		MP4Close(mp4_file);
		mp4_file=0;
		player->OnError(NErr_OutOfMemory);
		return NErr_OutOfMemory;
	}

	mp4_file_object->Initialize(uri, mp4_file);

	/* --- find appropriate audio decoder plugin --- */
	
	uint32_t num_audio_tracks = MP4GetNumberOfTracks(mp4_file, MP4_AUDIO_TRACK_TYPE);
	for (uint32_t track_num=0;track_num < num_audio_tracks && !audio_decoder;track_num++)
	{
		MP4TrackId audio_track = MP4FindTrackId(mp4_file, track_num, MP4_AUDIO_TRACK_TYPE);
		/* TODO: verify that this isn't a 'dependent' track, e.g. SLS portion of HD-AAC, stored in tref.dpnd */
		if (audio_track != MP4_INVALID_TRACK_ID)
			audio_decoder = GetAudioDecoder(mp4_file, audio_track, mp4_file_object);
	}

	if (!audio_decoder)
	{
		player->OnError(NErr_NoMatchingImplementation);
		return NErr_NoMatchingImplementation;
	}

	/* --- tell the player about various things --- */
	if (http_parent->Seekable() == NErr_True)
		player->SetSeekable(1);
	else 
		player->SetSeekable(0);

	MP4Duration file_duration = MP4GetDuration(mp4_file);
	double length =  MP4ConvertFromMovieDuration(mp4_file, file_duration, MP4_USECS_TIME_SCALE) / (double)MP4_USECS_TIME_SCALE;
	player->SetLength(length);

	bool opened=false;

	player->SetMetadata(mp4_file_object);
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
					audio_decoder->SeekSeconds(&position);

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

		ret = audio_decoder->Decode((const void **)&decoded_data, &decoded_bytes, &decoded_position, &end_position);
		if (ret == NErr_Success)
		{
			if (!out)
			{
				memset(&audio_parameters, 0, sizeof(ifc_audioout::Parameters));
				audio_parameters.sizeof_parameters = sizeof(ifc_audioout::Parameters);
				ret = audio_decoder->FillAudioParameters(&audio_parameters);
				if (ret != NErr_Success)
				{
					player->OnError(ret);
					return ret;
				}

				/* read replaygain info */
				ifc_replaygain_settings *replaygain_settings;
				if (secondary_parameters && secondary_parameters->QueryInterface(&replaygain_settings) == NErr_Success)
				{
					if (replaygain_settings->GetGain(mp4_file_object, &audio_parameters.gain, 0) == NErr_Success)
					{
#ifdef __ANDROID__
						__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FilePlayback] using replaygain adjustment of %f", audio_parameters.gain);
#endif
						audio_parameters.extended_fields_flags |= ifc_audioout::EXTENDED_FLAG_REPLAYGAIN;
					}
					replaygain_settings->Release();
				}

				ret = http_parent->AudioOpen(&audio_parameters, &out);
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
			}

			if (decoded_bytes)
			{
				player->SetPosition(decoded_position - out->Latency());
				if (audio_parameters.audio.format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED)
				{
					// TODO
				}
				else
				{
					const uint8_t *decode8 = (const uint8_t *)decoded_data;
					size_t buffer_position=0;
					while (decoded_bytes)
					{
						size_t to_write = out->CanWrite();
						if (to_write)
						{
							if (decoded_bytes < to_write)
								to_write = decoded_bytes;

							ret = out->Output(&decode8[buffer_position], to_write);
							if (ret != NErr_Success)
							{
								out->Release();
								player->OnError(ret);
								return ret;							
							}

							decoded_bytes -= to_write;
							buffer_position += to_write;
						}
						else
						{
							int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
							if (ret == ifc_http::WAKE_STOP)
							{
								if (out)
								{
									out->Stop();
									out->Release();
								}
								player->OnStopped();
								return NErr_Success;
							}

						}
					}
					player->SetPosition(end_position - out->Latency());
				}
				if (ret != NErr_Success)
					return ret;
			}
		}
		else if (ret == NErr_EndOfFile)
		{
			if (out)
			{
				out->Done();
				out->Release();
			}

			player->OnEndOfFile();
			return NErr_EndOfFile;
		}
		else
		{
			if (out)
			{
				out->Done();
				out->Release();
			}
			player->OnError(ret);
			return ret; 			
		}

	}

	return NErr_Success;


}

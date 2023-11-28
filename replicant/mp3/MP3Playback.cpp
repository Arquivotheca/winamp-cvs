#include "api.h"
#include "main.h"
#include "MP3Playback.h"
#include "nx/nxfile.h"
#include "nx/nxsleep.h"
#include "replaygain/ifc_replaygain_settings.h"
#include "nswasabi/ReferenceCounted.h"
#include "nsmp3/mp3ssc.h"
#ifdef __ANDROID__
#include <android/log.h> // TODO: replace with generic logging API

#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
static void __android_log_print(int, const char *, const char *, ...)
{
}
#endif
#include <new>

MP3Playback::MP3Playback()
{
	filename=0;
	giofile=0;
	mpeg=0;
	samples_written=0;
	samples_per_second=0;
	total_bitrate=0;
	last_length=0;
	start_position = 0;
	total_frames=0;
	output_opened=false;
	current_bitrate=128000;
}

int MP3Playback::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	this->parent = parent;
	this->filename = NXURIRetain(filename);
	return Init(file, parent_metadata);
}

MP3Playback::~MP3Playback()
{
	if (giofile)
		giofile->Release();
	giofile=0;

	if (mpeg)
		delete mpeg;
	mpeg=0;
	NXURIRelease(filename);
	filename=0;

}

int MP3Playback::Init(nx_file_t file, ifc_metadata *parent_metadata)
{
	giofile = new (std::nothrow) ReferenceCounted<MetadataChain<GioFile> >;
	if (!giofile)
		return NErr_OutOfMemory;	

	giofile->SetParentMetadata(parent_metadata);

	int ret = giofile->Open(filename, file);
	if (ret != NErr_Success)
		return ret;

	if (!mpeg)
	{
		mpeg = new CMpgaDecoder;
		if (!mpeg)
			return NErr_OutOfMemory;
	}

	mpeg->Connect(giofile);
	return NErr_Success;
}

void MP3Playback::FilePlayback_Close()
{
	if (giofile)
		giofile->Release();
	giofile=0;

	if (mpeg)
		delete mpeg;
	mpeg=0;
	NXURIRelease(filename);
	filename=0;
}


ns_error_t MP3Playback::FilePlayback_Seekable()
{
	return NErr_True;
}

ns_error_t MP3Playback::FilePlayback_GetMetadata(ifc_metadata **metadata)
{
	*metadata = giofile;
	giofile->Retain();
	return NErr_Success;
}

ns_error_t MP3Playback::FilePlayback_GetLength(double *length, ns_error_t *exact)
{
	if (total_frames)
		*length = giofile->GetLengthSeconds(total_bitrate/(double)total_frames, exact);
	else
		*length = giofile->GetLengthSeconds(0, exact);
	return NErr_Success;
}

ns_error_t MP3Playback::FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)
{
	ns_error_t ret = giofile->GetBitrate(bitrate, exact);
	if (ret != NErr_Success)
	{
		if (total_frames)
		{
			*exact=NErr_False;
			*bitrate = total_bitrate/(double)total_frames;
		}
		else
			return NErr_Unknown;
		
	}
	return NErr_Success;
}

ns_error_t MP3Playback::FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)
{
	switch(seek->position_type)
	{
	case AGAVE_PLAYPOSITION_SECONDS:
		{
			if (total_frames)
				giofile->SeekSeconds(seek->position.seconds, total_bitrate/(double)total_frames);
			else
				giofile->SeekSeconds(seek->position.seconds, 0);
			samples_written = 0;
			start_position = seek->position.seconds;
			mpeg->Reset();
			*seek_error = NErr_Success;
			*new_position = seek->position.seconds;
		}
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

ns_error_t MP3Playback::FilePlayback_DecodeStep()
{
	float decode_buffer[1152*2];
	size_t decoded=0;
	SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &decoded);
	if (SSC_SUCCESS(mpeg_ret))
		{
				const CMp3StreamInfo *info = mpeg->GetStreamInfo();

				total_bitrate += info->GetBitrate();
				total_frames++;
				current_bitrate = total_bitrate / (double)total_frames;
				// if we need to, open the audio output
				if (!output_opened)
				{
					const CMp3StreamInfo *info = mpeg->GetStreamInfo();

					ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
					parameters.audio.sample_rate = info->GetSFreq();
					parameters.audio.format_type = nsaudio::format_type_float;
					parameters.audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
					parameters.audio.bytes_per_sample = 4;
					parameters.audio.bits_per_sample = 32;
					parameters.audio.number_of_channels = info->GetChannels();

					/* read gapless info */
					size_t pregap, postgap;
					if (GetGaps(giofile, &pregap, &postgap) == NErr_Success)
					{
#ifdef __ANDROID__
						__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MP3] pre-gap = %u post-gap = %u", pregap, postgap);
#endif
						parameters.frames_trim_start = pregap;
						parameters.frames_trim_end = postgap;
					}

					samples_per_second = info->GetSFreq();

					ns_error_t ret = parent->OpenOutput(&parameters);
					if (ret != NErr_Success)
						return ret;

					output_opened=true;
				}

				if (decoded)
				{
					double position = start_position + (double)samples_written/samples_per_second;
					size_t frames_consumed=0;
					ns_error_t ret = parent->Output(decode_buffer, decoded, &frames_consumed, position);
					
					samples_written += frames_consumed;
					if (ret != NErr_Success)
						return ret;
				}
		}
		else switch(mpeg_ret)
		{
		case SSC_W_MPGA_SYNCNEEDDATA:
			if (!mpeg->IsEof())
				break;
		case SSC_W_MPGA_SYNCEOF:
			return NErr_EndOfFile;
			
		case SSC_E_MPGA_WRONGLAYER:
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MP3] SSC_E_MPGA_WRONGLAYER");
			if (output_opened)
				return NErr_Malformed;
			else
				mpeg->m_Mbs.Seek(1);

			break;

		default:
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[MP3] SSC = %s", (const char *)CMp3Ssc(mpeg_ret));
			break;
		}

	return NErr_Success;
}

ns_error_t MP3Playback::FilePlayback_Interrupt(Agave_Seek *resume_information)
{

	Agave_Seek_SetBytes(resume_information, giofile->GetPosition());
	giofile->Close();
	giofile->Release();
	giofile=0;
	return NErr_Success;
}

ns_error_t MP3Playback::FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)
{
	ns_error_t ret = Init(file, parent_metadata);
	if (ret != NErr_Success)
		return ret;

	giofile->SetPosition(resume_information->position.bytes);
	return NErr_Success;
}


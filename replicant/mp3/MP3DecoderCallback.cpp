#include "MP3DecoderCallback.h"
#include "main.h"
#include "decode/svc_decode.h"
#include <new>

MP3DecoderCallback::MP3DecoderCallback()
{
	frame_size=0;
	gio_file=0;
	decoder=0;
	flags=0;
	pregap=0;
	postgap=0;
	channels=0;
	done=false;
	samples_per_frame=0;	
}

MP3DecoderCallback::~MP3DecoderCallback()
{
	delete decoder;
	if (gio_file)
		gio_file->Release();

	buffers.deleteAll();
	filled_buffers.deleteAll();	
}


int MP3DecoderCallback::Initialize(MetadataChain<GioFile> *gio_file, CMpgaDecoder *decoder, int flags, nsaudio::Parameters *parameters)
{	
	this->decoder = decoder;
	this->flags = flags;
	this->gio_file = gio_file;
	gio_file->Retain();

	decoder->Connect(gio_file);

	MP3Buffer *buffer = new (std::nothrow) MP3Buffer;
	if (!buffer)
		return NErr_OutOfMemory;
	buffers.push_back(buffer);

	/* now we need to decode until we get one succesful frame */
	int ret = DecodeNextFrame(true);
	if (ret != NErr_Success)
		return ret;

	const CMp3StreamInfo *info = decoder->GetStreamInfo();

	parameters->sample_rate = info->GetSFreq();
	parameters->format_type = nsaudio::format_type_float;
	parameters->format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->bytes_per_sample = sizeof(float);
	parameters->bits_per_sample = sizeof(float) * 8;
	parameters->number_of_channels = channels = info->GetChannels();
	parameters->channel_layout = 0; /* TODO! */
	frame_size = sizeof(float) * info->GetChannels();
	samples_per_frame = info->GetSamplesPerFrame();

	GetGaps(gio_file, &pregap, &postgap);
	size_t buffer_count = 1 + postgap/samples_per_frame; /* we need one extra buffer to properly detect EOF */

	for (size_t i=0;i<buffer_count;i++)
	{
		MP3Buffer *buffer = new (std::nothrow) MP3Buffer;
		if (!buffer)
			return NErr_OutOfMemory;
		buffers.push_back(buffer);
	}

	return NErr_Success;
}

int MP3DecoderCallback::AudioDecoderCallback_GetMetadata(ifc_metadata **metadata)
{
	ifc_metadata *metadata_retain = gio_file;
	metadata_retain->Retain();
	*metadata = metadata_retain;
	return NErr_Success;
}

int MP3DecoderCallback::AudioDecoderCallback_GetFrameSize(size_t *frame_size)
{
	*frame_size = samples_per_frame;
	return NErr_Success;	
}

int MP3DecoderCallback::AudioDecoderCallback_DecodeStep(ifc_audio_decoder_callback::callback *callback)
{
	while (!done && !buffers.empty())
	{
		int ret = DecodeNextFrame(false);
		if (ret == NErr_EndOfFile)
			done=true;
		else if (ret != NErr_Success)
			return ret;
	}

	if (done)
	{
		/* try to remove whole buffers */
		while(postgap > samples_per_frame) 
		{
			MP3Buffer *buffer = filled_buffers.back();
			filled_buffers.pop_back();
			if (buffer)
			{
				postgap -= buffer->valid_bytes / frame_size;
			}
			else
			{
				break;
			}
		}
	}

	MP3Buffer *buffer = filled_buffers.front();
	if (!buffer)
		return NErr_EndOfFile;

	filled_buffers.pop_front();

	/* do pre-gap stuff */	
	size_t skip=0;
	size_t frames_available = buffer->valid_bytes / frame_size;
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
			buffers.push_back(buffer);
			return AudioDecoderCallback_DecodeStep(callback);
		}
	}

	/* do post-gap stuff */
	if (done)
	{
		if (frames_available >= postgap)
		{
			frames_available -= postgap;
			postgap = 0;
		}
		else
		{
			/* need to completely skip this frame (this will only happen for VERY SHORT mp3 files */
			postgap -= frames_available;
			buffers.push_back(buffer);
			return AudioDecoderCallback_DecodeStep(callback);
		}
	}

	int ret = callback->OnAudio(buffer->decode_buffer + skip, frames_available);
	buffers.push_back(buffer);
	if (ret != NErr_Success)
		return NErr_Interrupted;
	else
		return NErr_Success;
}

int MP3DecoderCallback::AudioDecoderCallback_Decode(ifc_audio_decoder_callback::callback *callback)
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

int MP3DecoderCallback::DecodeNextFrame(bool first)
{
	for (;;)
	{
		MP3Buffer *buffer = buffers.back();
		size_t bytes_available;
		SSC mpeg_ret = decoder->DecodeFrame(buffer->decode_buffer, sizeof(buffer->decode_buffer), &bytes_available);
		if (SSC_SUCCESS(mpeg_ret))
		{
			buffers.pop_back();
			buffer->valid_bytes = bytes_available;
			filled_buffers.push_back(buffer);
			return NErr_Success;
		}
		else switch(mpeg_ret)
		{
		case SSC_W_MPGA_SYNCNEEDDATA:
			if (!decoder->IsEof())
				break;

		case SSC_W_MPGA_SYNCEOF:
			return NErr_EndOfFile;

		case SSC_E_MPGA_WRONGLAYER:
			if (first)
			{
				decoder->m_Mbs.Seek(1);
				break;
			}
			else
			{
				return NErr_Error;
			}
		case SSC_W_MPGA_SYNCSEARCHED:
		case SSC_W_MPGA_SYNCLOST:
			break;
		default:
			return NErr_Error;

		}
	}
}
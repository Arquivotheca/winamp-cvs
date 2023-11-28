#include "../api.h"
#include "ICYAAC.h"
#include "nsaac/ADTSHeader.h"
#include "nx/nxsleep.h"
#include <stdio.h> // TODO: cut


static const size_t synchronize_bytes = 16384; // how many bytes to read before giving up on stream synchronization

ICYAAC::ICYAAC() 
{
	http=0;
	decoder=0;
	composition_unit=0; /* output */
	access_unit=0; /* input */

}

ICYAAC::~ICYAAC()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
}

int ICYAAC::Initialize(jnl_http_t http)
{
	this->http = http;
	access_unit = CAccessUnit_Create(0, 0);
	if (!access_unit)
		return NErr_FailedCreate;

	/* cannot create decoder until we get a valid ADTS header */

	return NErr_Success;
}


/* helper function for making sure we read exactly as much as we want to */
int ICYAAC::ICYRead(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested)
{
	size_t bytes_in_buffer=0;
	while (bytes_in_buffer < bytes_requested)
	{
		size_t this_read = bytes_requested-bytes_in_buffer;
		int ret = reader->Read(buffer+bytes_in_buffer, this_read, &this_read); 
		if (ret != NErr_Success)
			return ret;

		bytes_in_buffer += this_read;
	}
	return NErr_Success;
}

int ICYAAC::ICYPeek(ifc_icy_reader *reader, uint8_t *buffer, size_t bytes_requested)
{
	// if we get less than requested, it's probably because there's not enough data in the socket
	// so we'll keep trying until we get it right.  probably not ideal but it should suffice

	for (;;)
	{
		size_t bytes_in_buffer;
		int ret = reader->Peek(buffer, bytes_requested, &bytes_in_buffer);
		if (ret)
			return ret;
		if (bytes_in_buffer == bytes_requested)
			return NErr_Success;
		if (reader->IsClosed())
			return NErr_Closed;
	} 
}

int ICYAAC::ICYSync(ifc_icy_reader *reader)
{
	int ret;
	size_t bytes_in_buffer;
	size_t position=0;
	uint8_t buffer[synchronize_bytes];
	ret = reader->Peek(buffer, synchronize_bytes, &bytes_in_buffer); 
	if (ret)
		return ret;

	for (;position<bytes_in_buffer;position++)
	{
		// find POTENTIAL sync
		if (buffer[position] == 0xFF && bytes_in_buffer - position >= 7)
		{
			ADTSHeader header;
			if (nsaac_adts_parse(&header, &buffer[position]) == NErr_Success)
			{
				int frame_length = header.frame_length;
				if (frame_length && bytes_in_buffer - position - frame_length >= 7)
				{
					ADTSHeader header2;
					if (nsaac_adts_parse(&header2, &buffer[position+frame_length]) == NErr_Success)
					{
						// verify that parameters match
						if (nsaac_adts_match(&header, &header2) != NErr_True)
							return NErr_Changed;

						// do a dummy read to advance the stream
						return reader->Read(buffer, position, &position);
					}
				}
			}
		}
	}

	return NErr_False;	
}

static void ConfigureADTS(CSAudioSpecificConfig* asc, nsaac_adts_header_t header)
{
	asc->m_aot = (AUDIO_OBJECT_TYPE)(header->profile + 1);
	asc->m_channelConfiguration = header->channel_configuration;
	asc->m_channels = nsaac_adts_get_channel_count(header);
	asc->m_nrOfStreams = 1;
	asc->m_samplesPerFrame = 1024;
	asc->m_samplingFrequencyIndex = header->sample_rate_index;
	asc->m_samplingFrequency = nsaac_adts_get_samplerate(header);
	asc->m_avgBitRate = 0;  /* only needed for tvq */
	asc->m_mpsPresentFlag   = -1;
	asc->m_saocPresentFlag  = -1;
	asc->m_ldmpsPresentFlag = -1;
}


int ICYAAC::ICYPlayback_Run(ifc_http *http_parent, ifc_player *player, ifc_icy_reader *reader)
{	
	int ret;
	bool opened=false, paused=false;
	ifc_audioout *out=0;	
	uint64_t samples_written=0;
	double samples_per_second=0;
	player->SetBufferStatus(0);


	/* buffer enough to be able to synchronize */
	while(reader->BytesBuffered() < 16384)
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_KILL_MASK);
		if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	/* Synchronize to the stream */
	ret = ICYSync(reader);
	if (ret)
		return ret;

	/* buffer more data */
	while(reader->BytesBuffered() < 1024*64)
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_STOP);
		if (ret == ifc_http::WAKE_STOP)
		{
			player->OnStopped();
			return NErr_Success;
		}
	}

	player->SetBufferStatus(100);

	for (;;)
	{
		/* check for kill switch */
		ret = http_parent->Wake(ifc_http::WAKE_STOP|ifc_http::WAKE_PAUSE);
		if (ret == ifc_http::WAKE_PAUSE)
		{
			if (opened)
				out->Pause(1);
			paused=true;
			continue;
		}
		else if (ret== ifc_http::WAKE_UNPAUSE)
		{
			if (opened)
				out->Pause(0);
			paused=false;
			continue;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			player->OnStopped();
			return NErr_Success;			
		}

		if (paused)
		{
			continue;
		}

		/* Read the ADTS header and make sure we didn't lose sync */
		ret = ICYRead(reader, buffer, 7);
		if (ret)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return ret;
		}

		ADTSHeader header;
		if (nsaac_adts_parse(&header, buffer) != NErr_Success)
		{
			// TODO: resynchronize
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return NErr_LostSynchronization;
		}

		if (!decoder)
		{
			CSAudioSpecificConfig asc;
			memset(&asc, 0, sizeof(asc));
			ConfigureADTS(&asc, &header);
			/* create Decoder */
			CSAudioSpecificConfig *asc_array = &asc;
			decoder = mp4AudioDecoder_Create(&asc_array, 1);
			if (!decoder)
			{
				if (opened)
				{
					out->Stop();
					out->Release();
				}
				return NErr_FailedCreate;
			}

			mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF);
			mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
			composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);	
			if (!composition_unit)
				return NErr_FailedCreate;
		}

		// TODO: verify that header matches the initial one

		/* Read the ADTS frame */
		size_t header_size=nsaac_adts_get_header_size(&header);
		if (header.protection == 0) /* if we have CRC, need to skip two more bytes */
			ICYRead(reader, buffer, 2);
		
		ret = ICYRead(reader, buffer, header.frame_length-header_size);
		if (ret)
		{
			if (opened)
			{
				out->Stop();
				out->Release();
			}
			return ret;
		}

		CAccessUnit_Reset(access_unit);
		CAccessUnit_Assign(access_unit, (const unsigned char *)buffer, header.frame_length-header_size);
		CCompositionUnit_Reset(composition_unit);

		MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);
		if (result == MP4AUDIODEC_OK)
		{
			unsigned int channels;
			unsigned int samples_per_channel;
			unsigned int sample_rate;
			if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
				||		CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK
				|| CCompositionUnit_GetSamplingRate(composition_unit, &sample_rate) != MP4AUDIODEC_OK)
			{
				if (opened)
				{
					out->Stop();
					out->Release();
				}
				return NErr_Error;
			}

			if (!opened)
			{
				ifc_audioout::Parameters parameters={sizeof(parameters), };
				parameters.audio.sample_rate = sample_rate;
				parameters.audio.format_type = nsaudio::format_type_float;
				parameters.audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
				parameters.audio.bytes_per_sample = 4;
				parameters.audio.bits_per_sample = 32;
				parameters.audio.number_of_channels = channels;
				parameters.gain = 1/32768.0f;
				parameters.extended_fields_flags = ifc_audioout::EXTENDED_FLAG_APPLY_GAIN;

				ret = http_parent->AudioOpen(&parameters, &out);
				if (ret != NErr_Success)
					return ret;

				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				opened=true;
			}

			samples_written += samples_per_channel;
			size_t num_samples = samples_per_channel * channels;
			size_t bytes_decoded = num_samples * sizeof(float);

			const float *audio_output = 0;
			CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);

			/* write to output */
			const uint8_t *decoded_data = (const uint8_t *)audio_output;
			size_t buffer_position=0;
			while (bytes_decoded)
			{
				size_t to_write = out->CanWrite();
				if (to_write)
				{
					if (bytes_decoded < to_write)
						to_write = bytes_decoded;

					ret = out->Output(&decoded_data[buffer_position], to_write);
					if (ret != NErr_Success)
					{
						out->Release();
						return ret;
					}
					bytes_decoded -= to_write;
					buffer_position += to_write;
				}
				else
				{
					reader->Run(); /* might as well run while we wait */
					int ret = http_parent->Wait(10, ifc_http::WAKE_STOP);
					if (ret == ifc_http::WAKE_STOP)
					{
						if (opened)
						{
							out->Stop();
							out->Release();
						}
						player->OnStopped();
						return NErr_Success;
					}
				}
			}
			player->SetPosition((double)samples_written/(double)sample_rate - out->Latency());
		}
		else
		{
			DebugBreak();
		}
	}

	return NErr_Success;
}
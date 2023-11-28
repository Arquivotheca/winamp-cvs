#include "UltravoxAAC.h"
#include "nsaac/ADTSHeader.h"
#include "nx/nxsleep.h"
#include <stdio.h> // TODO: cut
static const size_t synchronize_bytes = 16384; // how many bytes to read before giving up on stream synchronization

UltravoxAAC::UltravoxAAC(jnl_http_t http) : http(http)
{
	memset(&decoder, 0, sizeof(decoder));
}

UltravoxAAC::~UltravoxAAC()
{
	AACDecoderClose(&decoder);
}

int UltravoxAAC::UltravoxPeek(ifc_ultravox_reader *reader, uint8_t *buffer, size_t bytes_requested)
{
	/* unlike ICYReader, UltravoxReader guarantees that Peek will succeed unless the stream was terminated */
	size_t bytes_in_buffer;
	return reader->Peek(buffer, bytes_requested, &bytes_in_buffer);
}

int UltravoxAAC::UltravoxSync(ifc_ultravox_reader *reader)
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

int UltravoxAAC::UltravoxPlayback_Run(ifc_http *http_parent, svc_output *output_service, ifc_player *player, ifc_ultravox_reader *reader, ifc_playback_parameters *secondary_parameters)
{
	int ret;
	size_t samples_decoded;
	bool opened=false, paused=false;
	ifc_audioout *out=0;	

	ret = AACDecoderInit(&decoder);
	if (ret != NErr_Success)
		return ret;

	/* buffer enough to be able to synchronize */
	while(reader->BytesBuffered() < 16384)
	{
		ret = http_parent->Wait(55, ifc_http::WAKE_PLAY|ifc_http::WAKE_KILL);
		if (ret == ifc_http::WAKE_KILL)
		{
			return NErr_Success;
		}
		else if (ret == ifc_http::WAKE_STOP)
		{
			return NErr_Success;
		}
	
	}

	uint8_t buffer[8192];

	/* Synchronize to the stream */
	ret = UltravoxSync(reader);
	if (ret)
		return ret;

/* buffer more data */
while(reader->BytesBuffered() < 1024*64)
{
	ret = http_parent->Wait(55, ifc_http::WAKE_PLAY|ifc_http::WAKE_KILL);
	if (ret == ifc_http::WAKE_KILL)
	{
		return NErr_Success;
	}
	else if (ret == ifc_http::WAKE_STOP)
	{
		return NErr_Success;
	}
}

	for (;;)
	{
		/* check for kill switch */
		ret = http_parent->Wake(ifc_http::WAKE_ALL_MASK);
		if (ret == ifc_http::WAKE_KILL)
		{
			return NErr_Success;
		}
		else if (ret == ifc_http::WAKE_PAUSE)
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
				out->Stop();
			return NErr_Success;			
		}

		/* Read the ADTS header and make sure we didn't lose sync */
		size_t bytes_read;
		ret = reader->Read(buffer, 7, &bytes_read);
		if (ret)
		{
			if (opened)
				out->Stop();
			return ret;
		}

		ADTSHeader header;
		if (nsaac_adts_parse(&header, buffer) != NErr_Success)
		{
			// TODO: resynchronize
			if (opened)
				out->Stop();
			return NErr_LostSynchronization;
		}

		// TODO: verify that header matches the initial one

		/* Read the ADTS frame */	
		ret = reader->Read(buffer+7, header.frame_length-7, &bytes_read);
		if (ret)
		{
			if (opened)
				out->Stop();
			return ret;
		}
		
		if (AACDecodeFrame(&decoder, buffer, header.frame_length, output_buffer, &samples_decoded) == NErr_Success)
		{
			if (!opened)
			{
				ifc_audioout::Parameters parameters={sizeof(parameters), };
				parameters.sample_rate = decoder.decoder.samplingRate;
				parameters.format_type = nsaudio_type_pcm;
				parameters.format_flags = FORMAT_FLAG_INTERLEAVED|FORMAT_FLAG_NATIVE_ENDIAN|FORMAT_FLAG_SIGNED;
				parameters.bytes_per_sample = 2;
				parameters.bits_per_sample = 16;
				parameters.number_of_channels = 2;//frame_info.channels;

				ret = output_service->AudioOpen(&parameters, player, secondary_parameters, &out);
				if (ret != NErr_Success)
					return ret;

				if (paused)
					out->Pause(1);
				else
					out->Pause(0);

				opened=true;
			}

					size_t bytes_decoded = samples_decoded * sizeof(int16_t);
							/* write to output */
				const uint8_t *decoded_data = (const uint8_t *)output_buffer;
				size_t buffer_position=0;
				while (bytes_decoded)
				{
					size_t to_write = out->CanWrite();
					if (to_write)
					{
						if (bytes_decoded < to_write)
							to_write = bytes_decoded;

						out->Output(&decoded_data[buffer_position], to_write);
						bytes_decoded -= to_write;
						buffer_position += to_write;
					}
					else
					{
						NXSleep(55);
					}
				}
		}

	}

	return NErr_Success;
}
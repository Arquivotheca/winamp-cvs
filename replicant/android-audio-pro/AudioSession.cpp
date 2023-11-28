#include "AudioSession.h"
#include "pcmutils.h"
#include "AudioThread.h"
#include "replaygain/ifc_replaygain_settings.h"
#ifdef REPLICANT_OPENSL
#include <android/log.h>
#endif

/* TODO: verify that inner loop on FadeIn and FadeOut is being properly optimized by the compiler */
AudioSession::AudioSession()
{
	volume_adjust=0.0;
	fade_samples=0;
	crossfade_state=IDLE;
	done_flag=0;
	error=NErr_Success;
	latency=0;
	buffer_size_in_samples=0;
	volume_delta=0.0;
	pre_gap_bytes=0;
	post_gap_bytes=0;
	current_pre_gap_bytes=0;
	skip_crossfade=false;

	playback_parameters=0;
	settings=0;
	samples_per_second=0;
	convert=0;
	deinterleave=0;
}

AudioSession::~AudioSession()
{
	if (playback_parameters)
		playback_parameters->Release();

	if (settings)
		settings->Release();

	size_t bytes_left_in_buffer = ring_buffer.size();
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[audiotrack-pro] %u bytes left in buffer\n", bytes_left_in_buffer);
}

/* do some basic sanity checks for things we don't support */
static int SupportedAudioParameters(const ifc_audioout::Parameters *audio_parameters)
{
	/* validate sizeof */
	if (audio_parameters->sizeof_parameters > sizeof(ifc_audioout::Parameters))
		return NErr_IncompatibleVersion;

	/* empty flags is an error */
	if (audio_parameters->audio.format_flags == 0)
		return NErr_BadParameter;

	/* make sure there's no flags we don't understand */
	if (audio_parameters->audio.format_flags & ~nsaudio::FORMAT_FLAG_VALID_MASK)
		return NErr_IncompatibleVersion;

	if (audio_parameters->extended_fields_flags & ~ifc_audioout::EXTENDED_FLAG_VALID_MASK)
		return NErr_IncompatibleVersion;

	/* we don't currently support big endian */
	if (audio_parameters->audio.format_flags & nsaudio::FORMAT_FLAG_BIG_ENDIAN)
		return NErr_NotImplemented;

	/* we don't currently support unsigned PCM */
	if (audio_parameters->audio.format_flags & nsaudio::FORMAT_FLAG_UNSIGNED)
		return NErr_NotImplemented;

	/* we only support PCM and float */
	if (audio_parameters->audio.format_type != nsaudio::format_type_pcm && audio_parameters->audio.format_type != nsaudio::format_type_float)
		return NErr_NotImplemented;

	/* for floating point, we can support 32bit (single) and 64bit (double), but bits_per_sample must match */
	if (audio_parameters->audio.format_type == nsaudio::format_type_float && audio_parameters->audio.bits_per_sample != audio_parameters->audio.bytes_per_sample*8)
		return NErr_NotImplemented;

	/* sanity check some values.  note that some of these might well be valid for other format_type values (e.g. pass-thru AC3) */
	if (audio_parameters->audio.number_of_channels == 0)
		return NErr_BadParameter;

	if (audio_parameters->audio.bits_per_sample == 0)
		return NErr_BadParameter;

	/* make sure the bytes_per_sample is big enough for the bits_per_sample */
	if (audio_parameters->audio.bits_per_sample > audio_parameters->audio.bytes_per_sample*8)
		return NErr_BadParameter;

	/* channels > 2 is not yet supported */
	if (audio_parameters->audio.number_of_channels > 2)
		return NErr_NotImplemented;

	return NErr_Success;
}

int AudioSession::Initialize(AudioThread *parent, const ifc_audioout::Parameters *parameters, ifc_playback_parameters *secondary_parameters)
{
	int ret = SupportedAudioParameters(parameters);
	if (ret != NErr_Success)
		return ret;

	/* copy only the fields we know about.  assume the rest to be zero. */
	memcpy(&audio_parameters, parameters, parameters->sizeof_parameters);
	memset((uint8_t *)&audio_parameters + parameters->sizeof_parameters, 0, sizeof(ifc_audioout::Parameters)-parameters->sizeof_parameters);

	playback_parameters = secondary_parameters;
	if (playback_parameters)
	{
		playback_parameters->Retain();
		if (playback_parameters->QueryInterface(&settings) != NErr_Success)
			settings=0; /* just in case */
	}

	/* read replaygain "default" setting if no gain was specified */
	if (!(audio_parameters.extended_fields_flags & EXTENDED_FLAG_REPLAYGAIN))
	{
		ifc_replaygain_settings *replaygain_settings;
		if (playback_parameters && playback_parameters->QueryInterface(&replaygain_settings) == NErr_Success)
		{
			double gain;
			if (replaygain_settings->GetGain(0, &gain, 0) == NErr_Success)
			{
				audio_parameters.extended_fields_flags |= EXTENDED_FLAG_REPLAYGAIN;
				if (audio_parameters.extended_fields_flags & EXTENDED_FLAG_APPLY_GAIN)
					audio_parameters.gain *= gain;
				else
					audio_parameters.gain = gain;
			}
			replaygain_settings->Release();
		}
	}

	if (audio_parameters.audio.format_type == nsaudio::format_type_pcm)
	{
		if (audio_parameters.audio.format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED)
		{
			switch(audio_parameters.audio.bytes_per_sample)
			{
			case sizeof(int16_t):
				deinterleave = (DeinterleaverFunc)Int16_To_Float32_Deinterleave;
				break;				
			case sizeof(int32_t):
				deinterleave = (DeinterleaverFunc)Int32_To_Float32_Deinterleave;
				break;
			default:
				/* TODO: support more */
				return NErr_NotImplemented;
			}
		}
		else
		{
			switch(audio_parameters.audio.bytes_per_sample)
			{
			case sizeof(int16_t):
				convert=(ConverterFunc)Int16_To_Float32;
				break;
			case sizeof(int32_t):
				convert=(ConverterFunc)Int32_To_Float32;
				break;
			default:
				/* TODO: support more */
				return NErr_NotImplemented;
			}
		}

		/* set up the conversion gain based on the valid bits per sample 
		as well as the passed in gain value, if applicable */
		if (audio_parameters.extended_fields_flags & EXTENDED_FLAG_GAIN_MASK)
			audio_parameters.gain /= (double)(1 << (audio_parameters.audio.bits_per_sample-1));
		else
			audio_parameters.gain = 1.0 / (double)(1 << (audio_parameters.audio.bits_per_sample-1));

	}
	else if (audio_parameters.audio.format_type == nsaudio::format_type_float)
	{
		if (audio_parameters.audio.format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED)
		{
			switch(audio_parameters.audio.bytes_per_sample)
			{
			case sizeof(float):
				deinterleave = (DeinterleaverFunc)Float32_To_Float32_Deinterleave;
				break;
			case sizeof(double):
				deinterleave = (DeinterleaverFunc)Float64_To_Float32_Deinterleave;
				break;
			default:
				return NErr_NotImplemented;
			}

			if (!(audio_parameters.extended_fields_flags & EXTENDED_FLAG_GAIN_MASK))
				audio_parameters.gain = 1.0; /* need this because we pass it to deinterleave */
		}
		else
		{
			switch(audio_parameters.audio.bytes_per_sample)
			{
			case sizeof(float):
				if (audio_parameters.extended_fields_flags & EXTENDED_FLAG_GAIN_MASK)
					convert = (ConverterFunc)Float32_To_Float32;
				break;
			case sizeof(double):
				convert = (ConverterFunc)Float64_To_Float32;
				if (!(audio_parameters.extended_fields_flags & EXTENDED_FLAG_GAIN_MASK))
					audio_parameters.gain = 1.0; /* need this because we pass it to convert */
				break;
			default:
				return NErr_NotImplemented;
			}
		}
	}

	this->parent = parent;
	samples_per_second = audio_parameters.audio.sample_rate * audio_parameters.audio.number_of_channels;
	bytes_per_second = samples_per_second * sizeof(float);

	double buffer_size_in_seconds = 2.0; /* 2 seconds of buffering time */
	double crossfade_time;
	if (settings && settings->GetCrossfadeTime(&crossfade_time) == NErr_Success)
	{
		buffer_size_in_seconds += crossfade_time;
	}

	buffer_size_in_samples = (size_t)((double)samples_per_second * buffer_size_in_seconds);	

	size_t buffer_size = buffer_size_in_samples * sizeof(float);
	current_pre_gap_bytes=pre_gap_bytes = audio_parameters.frames_trim_start * audio_parameters.audio.number_of_channels * sizeof(float);
	post_gap_bytes = audio_parameters.frames_trim_end * audio_parameters.audio.number_of_channels * sizeof(float);
	ring_buffer.reserve(buffer_size);
	return NErr_Success;
}

void AudioSession::Flush(size_t position)
{
	current_pre_gap_bytes=pre_gap_bytes;
	ring_buffer.advance_to(position);
	crossfade_state = PLAYING; /* break out of crossfade */
}

double AudioSession::SecondsBuffered()
{
	return ((double)ring_buffer.size()) / bytes_per_second;
}

void AudioSession::SetError(int error)
{
	this->error = error;
}

void AudioSession::SetLatency(uint32_t latency)
{
	this->latency = latency;
}

const ifc_audioout::Parameters *AudioSession::GetParameters() const
{
	return &audio_parameters;
}

bool AudioSession::Done() const
{
	return !!done_flag;
}

/* adjusts the bytes read from the buffer to eliminate the possibility of reading into the post-gap area */
int AudioSession::Internal_AdjustRead(size_t bytes_requested, size_t *bytes_to_read) const
{
	size_t bytes_available = ring_buffer.size();
	if (bytes_available > post_gap_bytes)
	{
		bytes_available -= post_gap_bytes;

		if (bytes_requested > bytes_available)
		{
			/* requested more bytes than are available, snap to the number of bytes available */
			*bytes_to_read = bytes_available;
		}
		else
		{
			*bytes_to_read = bytes_requested;
		}
		return NErr_True;
	}
	else
	{
		*bytes_to_read = 0;
		return NErr_False;
	}

}

int AudioSession::Internal_Fill(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	int ret = NErr_Success;
	if (!skip_crossfade && done_flag)
	{
		/* make sure we don't read into the fade-out area */
		size_t samples_left_in_buffer;
		Internal_AdjustRead(ring_buffer.size(), &samples_left_in_buffer);
		samples_left_in_buffer /= sizeof(float);

		if (samples_left_in_buffer < fade_samples) /* ergh. crossfade time! */
		{
			*bytes_written=0;
			return NErr_TryAgain;			
		}
		else
		{
			samples_left_in_buffer -= fade_samples;
			if (samples_left_in_buffer < bytes_requested/sizeof(float))
			{
				bytes_requested = samples_left_in_buffer*sizeof(float);
				ret = NErr_TryAgain;
			}
		}
	}

	*bytes_written = ring_buffer.read(buffer, bytes_requested);
	return ret;
}

int AudioSession::Internal_Accumulate(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	int ret = NErr_Success;
	if (!skip_crossfade && done_flag)
	{
		/* TODO: make sure we don't read into the fade-out area */
		bytes_requested = 0;
		ret = NErr_TryAgain;
	}

	size_t written=0;
	while (bytes_requested)
	{
		const float *read_buffer;
		size_t read_buffer_bytes;
		ring_buffer.get_read_buffer(bytes_requested, (const void **)&read_buffer, &read_buffer_bytes);
		size_t samples_read = read_buffer_bytes/sizeof(float);

		for (size_t i=0;i<samples_read;i++)
			buffer[i] += read_buffer[i];

		ring_buffer.advance(read_buffer_bytes);
		bytes_requested -= read_buffer_bytes;
		written += read_buffer_bytes;
		buffer += samples_read;
	}
	*bytes_written = written;
	return ret;
}

int AudioSession::Internal_FadeIn(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	size_t written=0;
	size_t samples_to_read = bytes_requested/sizeof(float);

	/* don't fade-in too far */
	if (samples_to_read > fade_samples)
	{
		samples_to_read = fade_samples;
		crossfade_state = PLAYING;
	}

	size_t bytes_to_read = samples_to_read * sizeof(float);

	while (bytes_to_read)
	{
		const float *read_buffer;
		size_t read_buffer_bytes;
		ring_buffer.get_read_buffer(bytes_to_read, (const void **)&read_buffer, &read_buffer_bytes);
		size_t samples_read = read_buffer_bytes/sizeof(float);

		for (size_t i=0;i<samples_read;i++)
		{
			buffer[i] = read_buffer[i] * volume_adjust;
			volume_adjust += volume_delta;
		}

		ring_buffer.advance(read_buffer_bytes);
		bytes_to_read -= read_buffer_bytes;
		written += read_buffer_bytes;
		buffer += samples_read;
		fade_samples -= samples_read;		
	}

	*bytes_written = written;

	/* if we finished the fade-in, tell the audio thread to call again */
	if (crossfade_state == PLAYING)
		return NErr_TryAgain;
	else 
		return NErr_Success;
}

int AudioSession::Internal_FadeInAccumulate(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	size_t samples_to_read = bytes_requested/sizeof(float);

	/* don't fade-in too far */
	if (samples_to_read > fade_samples)
	{
		samples_to_read = fade_samples;
		crossfade_state = PLAYING;
	}

	size_t bytes_to_read = samples_to_read * sizeof(float);

	size_t written=0;
	while (bytes_to_read)
	{
		const float *read_buffer;
		size_t read_buffer_bytes;
		ring_buffer.get_read_buffer(bytes_to_read, (const void **)&read_buffer, &read_buffer_bytes);
		size_t samples_read = read_buffer_bytes/sizeof(float);

		for (size_t i=0;i<samples_read;i++)
		{
			buffer[i] += read_buffer[i] * volume_adjust;
			volume_adjust += volume_delta;
		}

		ring_buffer.advance(read_buffer_bytes);
		bytes_to_read -= read_buffer_bytes;
		written += read_buffer_bytes;
		buffer += samples_read;
		fade_samples -= samples_read;
	}

	*bytes_written = written;
	return NErr_Success;
}

int AudioSession::Internal_FadeOut(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	size_t written=0;
	while (bytes_requested)
	{
		const float *read_buffer;
		size_t read_buffer_bytes;
		ring_buffer.get_read_buffer(bytes_requested, (const void **)&read_buffer, &read_buffer_bytes);
		size_t samples_read = read_buffer_bytes/sizeof(float);

		for (size_t i=0;i<samples_read;i++)
		{
			buffer[i] = read_buffer[i] * volume_adjust;
			volume_adjust -= volume_delta;
		}

		ring_buffer.advance(read_buffer_bytes);
		bytes_requested -= read_buffer_bytes;
		written += read_buffer_bytes;
		buffer += samples_read;
		fade_samples -= samples_read;		
	}
	*bytes_written = written;
	return NErr_Success;
}

int AudioSession::Internal_FadeOutAccumulate(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	size_t written=0;
	while (bytes_requested)
	{
		const float *read_buffer;
		size_t read_buffer_bytes;
		ring_buffer.get_read_buffer(bytes_requested, (const void **)&read_buffer, &read_buffer_bytes);
		size_t samples_read = read_buffer_bytes/sizeof(float);

		for (size_t i=0;i<samples_read;i++)
		{
			buffer[i] += read_buffer[i] * volume_adjust;
			volume_adjust -= volume_delta;
		}

		ring_buffer.advance(read_buffer_bytes);
		bytes_requested -= read_buffer_bytes;
		written += read_buffer_bytes;
		buffer += samples_read;
		fade_samples -= samples_read;		
	}
	*bytes_written = written;
	return NErr_Success;
}

int AudioSession::ReadFloat(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	int ret = NErr_Success;
	size_t written=0;
	size_t bytes_to_read;

	if (Internal_AdjustRead(bytes_requested, &bytes_to_read) == NErr_True)
	{
		if (crossfade_state == PLAYING)
		{
			ret = Internal_Fill(buffer, bytes_to_read, &written);
		}
		else if (crossfade_state == FADE_IN)
		{
			ret = Internal_FadeIn(buffer, bytes_to_read, &written);
		}
		else if (crossfade_state == FADE_OUT)
		{
			ret = Internal_FadeOut(buffer, bytes_to_read, &written);
		}
	}

	*bytes_written = written;

	if (ret != NErr_Success)
		return ret;
	else if (written != bytes_requested)
	{
		if (done_flag)
			return NErr_EndOfFile;
		else
			return NErr_Underrun;
	}
	else 
		return NErr_Success;
}

int AudioSession::AccumulateFloat(float *buffer, size_t bytes_requested, size_t *bytes_written)
{
	int ret = NErr_Success;
	size_t written=0;
	size_t bytes_to_read;

	if (Internal_AdjustRead(bytes_requested, &bytes_to_read) == NErr_True)
	{
		if (crossfade_state == PLAYING)
		{
			ret = Internal_Accumulate(buffer, bytes_to_read, &written);
		}
		else if (crossfade_state == FADE_IN)
		{
			ret = Internal_FadeInAccumulate(buffer, bytes_to_read, &written);
		}
		else if (crossfade_state == FADE_OUT)
		{
			ret = Internal_FadeOutAccumulate(buffer, bytes_to_read, &written);
		}
	}

	*bytes_written = written;

	if (ret != NErr_Success)
		return ret;
	else if (written != bytes_requested)
	{
		if (done_flag)
			return NErr_EndOfFile;
		else
			return NErr_Underrun;
	}
	else 
		return NErr_Success;
}

int AudioSession::AudioOutput_Output(const void *data, size_t data_size)
{
	/* signal a delayed error */
	int ret = error;
	if (ret != NErr_Success)
		return ret;

	/* do we still need to trim the beginning of input samples? */
	if (current_pre_gap_bytes)
	{
		if (current_pre_gap_bytes > data_size)
		{ 
			/* not enough samples to overcome pre-gap */
			current_pre_gap_bytes -= data_size;
			return NErr_Success;
		}
		else
		{
			/* trim beginning samples */
			data_size -= current_pre_gap_bytes;
			data = (const uint8_t *)data + current_pre_gap_bytes;
			current_pre_gap_bytes = 0;
		}
	}

	if (convert)
	{
		void *buffer=0;
		size_t buffer_bytes=0;
		size_t samples = data_size / audio_parameters.audio.bytes_per_sample;
		while (samples)
		{
			ring_buffer.get_write_buffer(samples*sizeof(float), &buffer, &buffer_bytes);
			size_t samples_available = buffer_bytes/sizeof(float);
			convert((float *)buffer, data, samples_available, audio_parameters.gain);
			ring_buffer.update(buffer_bytes);
			samples-= samples_available;
			data = (const uint8_t *)data + samples_available*audio_parameters.audio.bytes_per_sample;
		}
		return NErr_Success;
	}
	else if (deinterleave)
	{
		void *buffer=0;
		size_t buffer_bytes=0;
		const uint8_t * const *data8 = (const uint8_t * const *)data;

		size_t channels = audio_parameters.audio.number_of_channels;
		size_t samples = data_size / audio_parameters.audio.bytes_per_sample;

		const void *data_copy[2]; /* so we don't destroy anyone's pointers */
		for (size_t c=0;c<channels;c++)
		{
			data_copy[c] = data8[c];
		}

		while (samples)
		{
			ring_buffer.get_write_buffer(samples*sizeof(float), &buffer, &buffer_bytes);
			size_t samples_available = buffer_bytes/sizeof(float);
			size_t frames_available = samples_available/channels;
			deinterleave((float *)buffer, data_copy, channels, frames_available, audio_parameters.gain);
			ring_buffer.update(buffer_bytes);
			samples -= samples_available;

			for (size_t c=0;c<channels;c++)
			{
				data_copy[c] = (const uint8_t *)data_copy[c] + frames_available*audio_parameters.audio.bytes_per_sample;
			}
		}
		return NErr_Success;
	}
	else
	{
		ring_buffer.write(data, data_size);
		return NErr_Success;
	}
}

size_t AudioSession::AudioOutput_CanWrite()
{
	/* if we have a pending error, return a bogus CanWrite so the playback object calls AudioOutput_Output */
	int ret = error;
	if (ret != NErr_Success)
		return 100000;

	size_t samples = ring_buffer.avail() / sizeof(float);
	return samples * audio_parameters.audio.bytes_per_sample;
}

void AudioSession::AudioOutput_Flush(double seconds)
{
	/* there's a small race condition here
	since we might immediately start writing new data, 
	there's a chance that the AudioThread reads our new data before it is able to process the flush */
	parent->Flush(ring_buffer.write_position());
}

void AudioSession::AudioOutput_Pause(int state)
{
	parent->Pause(state);
}

void AudioSession::OnDone()
{

	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[AudioSession] OnDone");
	done_flag = 1;

	double crossfade_time;
	if (settings && settings->GetCrossfadeTime(&crossfade_time) == NErr_Success)
	{
		fade_samples = (size_t)(crossfade_time * (double)samples_per_second);
		volume_delta = 1.0/(double)fade_samples;
	}
	else
	{
		skip_crossfade=true;
	}

}

void AudioSession::OnTransition(bool transition, size_t *crossfade_samples)
{
	/* this might get called redundantly during crossfade, so expliticly check this */
	if (crossfade_state == PLAYING)
	{
		if (transition)
		{
			volume_adjust=1.0;
			crossfade_state=FADE_OUT;

			size_t samples_left_in_buffer;
			Internal_AdjustRead(ring_buffer.size(), &samples_left_in_buffer);
			samples_left_in_buffer /= sizeof(float);
			if (samples_left_in_buffer < fade_samples)
			{
				fade_samples = samples_left_in_buffer;
				volume_delta = 1.0/(double)fade_samples;
			}

			*crossfade_samples = fade_samples;
			skip_crossfade=false;
		}
		else
		{
			skip_crossfade=true;
		}
	}
}

void AudioSession::OnStart(bool transition, size_t crossfade_samples)
{
	/* this might get called redundantly during crossfade, so expliticly check this */
	if (crossfade_state == IDLE)
	{
		if (transition)
		{
			volume_adjust=0.0;
			crossfade_state=FADE_IN;
			fade_samples = crossfade_samples;
			if (fade_samples)
				volume_delta = 1.0/(double)fade_samples;
		}
		else
		{
			crossfade_state = PLAYING;
		}
	}
}

void AudioSession::AudioOutput_Done()
{
	/* queue this up on to the audio thread.  */
	parent->Done(this);
}

void AudioSession::AudioOutput_Stop()
{
	parent->Stop();
}

double AudioSession::AudioOutput_Latency()
{
	uint32_t samples_in_buffer = ring_buffer.size() / (audio_parameters.audio.number_of_channels * sizeof(float));
	return (double)samples_in_buffer / audio_parameters.audio.sample_rate + (double)latency / 1000.0;
}

int AudioSession::AudioOutput_Playing()
{
	size_t bytes_left_in_buffer;
	Internal_AdjustRead(ring_buffer.size(), &bytes_left_in_buffer);
	if (bytes_left_in_buffer)
		return NErr_True;
	else
		return NErr_False;
}
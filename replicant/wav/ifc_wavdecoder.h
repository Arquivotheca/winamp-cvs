#pragma once
#include "foundation/error.h"
#include "foundation/dispatch.h"
#include "nsiff/nsiff.h"
#include "audio/ifc_audioout.h"

class ifc_wavreader : public Wasabi2::Dispatchable
{
	protected:
	ifc_wavreader() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_wavreader() {}
public:
	ns_error_t ReadBytes(void *data, size_t bytes_requested, size_t *bytes_read) { return WAVReader_ReadBytes(data, bytes_requested, bytes_read); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual ns_error_t WASABICALL WAVReader_ReadBytes(void *data, size_t bytes_requested, size_t *bytes_read)=0;
};

class ifc_wavdecoder : public Wasabi2::Dispatchable
{
protected:
	ifc_wavdecoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_wavdecoder() {}
public:
	ns_error_t GetLengthSeconds(uint64_t data_length, double *length) { return WAVDecoder_GetLengthSeconds(data_length, length); }
	ns_error_t FillAudioParameters(ifc_audioout::Parameters *parameters) { return WAVDecoder_FillAudioParameters(parameters); }
	ns_error_t Decode(ifc_wavreader *reader, const void **output_buffer, size_t *output_buffer_bytes, double *start_position) { return WAVDecoder_Decode(reader, output_buffer, output_buffer_bytes, start_position); }
	ns_error_t SeekSeconds(double *seconds, uint64_t *new_data_offset) { return WAVDecoder_SeekSeconds(seconds, new_data_offset); }
	ns_error_t GetBitrate(double *bitrate) { return WAVDecoder_GetBitrate(bitrate); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual ns_error_t WASABICALL WAVDecoder_GetLengthSeconds(uint64_t data_length, double *length)=0;
	/* sizeof_parameters will already be filled out for you */
	virtual ns_error_t WASABICALL WAVDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)=0;
	virtual ns_error_t WASABICALL WAVDecoder_Decode(ifc_wavreader *reader, const void **output_buffer, size_t *output_buffer_bytes, double *start_position)=0;
	/* fill in with the actual seconds you'll resume playback at */
	virtual ns_error_t WASABICALL WAVDecoder_SeekSeconds(double *seconds, uint64_t *new_data_offset) = 0;	
	virtual ns_error_t WASABICALL WAVDecoder_GetBitrate(double *bitrate) = 0;
};
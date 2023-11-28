#pragma once
#include "nsiff/nsiff.h"
#include "file/svc_fileplayback.h"
#include "nswasabi/ServiceName.h"
#include "ifc_wavdecoder.h"
#include "nswasabi/MetadataChain.h"
#include "WAVParser.h"

// {4A4B273A-A421-4ED8-AE42-69B899AA5609}
static const GUID wav_file_playback_guid = 
{ 0x4a4b273a, 0xa421, 0x4ed8, { 0xae, 0x42, 0x69, 0xb8, 0x99, 0xaa, 0x56, 0x9 } };

class WAVPlaybackService : public svc_fileplayback
{
public:
	WASABI_SERVICE_NAME("WAV Playback Service");
	WASABI_SERVICE_GUID(wav_file_playback_guid);

	ns_error_t WASABICALL FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
};


class WAVPlayback : public ifc_fileplayback, private ifc_wavreader, public WAVParser
{
public:
	WAVPlayback();
	~WAVPlayback();

	int Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
protected:
	nsiff_t iff_object;
	ifc_audioout::Parameters audio_parameters;

	ifc_wavdecoder *decoder;
	bool output_opened;

	ifc_metadata *metadata; // TODO: MetadataChain<WAVMetadata>
	ifc_fileplayback_parent *parent;
	int Init(nx_file_t file, ifc_metadata *parent_metadata);
private:
	

	/* ifc_fileplayback */
	void WASABICALL FilePlayback_Close();
	ns_error_t WASABICALL FilePlayback_Seekable();
	ns_error_t WASABICALL FilePlayback_GetMetadata(ifc_metadata **metadata);
	ns_error_t WASABICALL FilePlayback_GetLength(double *length, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position);
	ns_error_t WASABICALL FilePlayback_DecodeStep();
	ns_error_t WASABICALL FilePlayback_Interrupt(Agave_Seek *resume_information);
	ns_error_t WASABICALL FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata);
	
	/* ifc_wavreader */
	ns_error_t WASABICALL WAVReader_ReadBytes(void *data, size_t bytes_requested, size_t *bytes_read);
	
};
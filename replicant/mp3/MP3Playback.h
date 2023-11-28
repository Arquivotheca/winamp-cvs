#pragma once
#include "nsmp3/mpgadecoder.h"
#include "audio/ifc_audioout.h"
#include "mp3/giofile_crt.h"
#include "file/ifc_fileplayback.h"
#include "nswasabi/MetadataChain.h"

// implements local file playback
class MP3Playback : public ifc_fileplayback
{
public:
	MP3Playback();
	~MP3Playback();

	int Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);

private:
	ifc_fileplayback_parent *parent;
	MetadataChain<GioFile> *giofile;
	nx_uri_t filename;

	double current_bitrate;


	CMpgaDecoder *mpeg;
	
	bool output_opened;
	uint64_t samples_written;
	double samples_per_second;
	double total_bitrate;
	double last_length;
	double start_position;
	unsigned long total_frames;
	
private:
	int Init(nx_file_t file, ifc_metadata *parent_metadata);

	void WASABICALL FilePlayback_Close();
	ns_error_t WASABICALL FilePlayback_Seekable();
	ns_error_t WASABICALL FilePlayback_GetMetadata(ifc_metadata **metadata);
	ns_error_t WASABICALL FilePlayback_GetLength(double *length, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position);
	ns_error_t WASABICALL FilePlayback_DecodeStep();
	ns_error_t WASABICALL FilePlayback_Interrupt(Agave_Seek *resume_information);
	ns_error_t WASABICALL FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata);
};

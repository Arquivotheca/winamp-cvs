#include "ogg/ogg.h"
#include "nu/LockFreeItem.h"
#include "nx/nx.h"
#include <stdio.h>
#include "file/ifc_fileplayback.h"
#include "svc_oggdecoder.h"
#include "ifc_oggaudiodecoder.h"

class OggPlayback : public ifc_fileplayback
{
public:
	OggPlayback();
	~OggPlayback();
	int Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);

private:
	int Sync();
	ifc_fileplayback_parent *parent;
	ogg_sync_state ogg_sync;
	ogg_stream_state ogg_stream;
	nx_file_t file;
	ifc_oggaudiodecoder *audio_decoder;

private:
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
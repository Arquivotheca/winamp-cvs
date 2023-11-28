#include "api.h"
#include "OggPlayback.h"
#include "nx/nxsleep.h"
#include "service/ifc_servicefactory.h"

OggPlayback::OggPlayback()
{
	file=0;
	parent=0;
	audio_decoder=0;
	memset(&ogg_sync, 0, sizeof(ogg_sync));
	memset(&ogg_stream, 0, sizeof(ogg_stream));
}

OggPlayback::~OggPlayback()
{
	NXFileRelease(file);
	file=0;
	if (audio_decoder)
		audio_decoder->Release();
}

int OggPlayback::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	this->parent = parent;
	ogg_sync_init(&ogg_sync);
	ogg_stream_init(&ogg_stream, 0);
	this->file = NXFileRetain(file);
	ogg_sync_reset(&ogg_sync);
	ogg_stream_reset(&ogg_stream);

	// TODO: find stream end to determine length
	// TODO: make sure there aren't two (or more) concatenated streams
	// TODO: find metadata
	return Sync();
}

static ns_error_t FindDecoder(ogg_packet *packet, ifc_oggaudiodecoder **audio_decoder)
{
	GUID ogg_decoder_guid = svc_oggdecoder::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(ogg_decoder_guid, n++))
	{
		svc_oggdecoder *l = (svc_oggdecoder*)sf->GetInterface();
		if (l)
		{
			ifc_oggaudiodecoder *decoder=0;
			int ret = l->CreateAudioDecoder(packet, &decoder);
			l->Release();

			if (ret == NErr_Success)
				return NErr_Success;
			else if (ret != NErr_NoMatchingImplementation)
				return ret;
		}
	}
	return NErr_NoMatchingImplementation;
}

int OggPlayback::Sync()
{
	/* TODO: we're doing this in Init(), but we should do it incrementally during DecodeStep
	otherwise we can't get interrupted with Stop */
	bool first_packet = true;
	bool first_page = true;

	ogg_page ogg_page;
	for (;;)
	{
		if (ogg_sync_pageout(&ogg_sync, &ogg_page) == 1)
		{
			if (first_page)
			{
				int serial=ogg_page_serialno(&ogg_page);
				ogg_stream_init(&ogg_stream,serial); // TODO: error check
				first_page=false;
			}
			ogg_packet ogg_packet;
			ogg_stream_pagein(&ogg_stream, &ogg_page); // TODO: error check
			while (ogg_stream_packetout(&ogg_stream, &ogg_packet) == 1)
			{
				if (first_packet)
				{
					// find a decoder.  TODO: if it's Ogg Skeleton we might want to switch to a different codepath that handles video
					return FindDecoder(&ogg_packet, &audio_decoder);
				}
			}
		}
		else 
		{
			char *file_buffer = ogg_sync_buffer(&ogg_sync, 8192);
			size_t bytes_read =0;
			NXFileRead(file, file_buffer, 8192, &bytes_read);
			// TODO: check for errors/EOF
			ogg_sync_wrote(&ogg_sync, bytes_read);
		}
	}
	return NErr_NotImplemented;
}

void OggPlayback::FilePlayback_Close()
{
	ogg_sync_clear(&ogg_sync); 
	ogg_stream_clear(&ogg_stream);
}

ns_error_t OggPlayback::FilePlayback_Seekable()
{
	return NErr_True;
}

ns_error_t OggPlayback::FilePlayback_GetMetadata(ifc_metadata **metadata)
{
	return NErr_NotImplemented; // TODO!!!!
}

ns_error_t OggPlayback::FilePlayback_GetLength(double *length, ns_error_t *exact)
{
	return NErr_NotImplemented; // TODO!!!!
}

ns_error_t OggPlayback::FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)
{
	return NErr_NotImplemented; // TODO!!!!
}

ns_error_t OggPlayback::FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)
{
	return NErr_Success;
}

ns_error_t OggPlayback::FilePlayback_DecodeStep()
{
	
		ogg_page ogg_page;
		if (ogg_sync_pageout(&ogg_sync, &ogg_page) == 1)
		{
			ogg_packet ogg_packet;
			ogg_stream_pagein(&ogg_stream, &ogg_page); // TODO: error check
			while (ogg_stream_packetout(&ogg_stream, &ogg_packet) == 1)
			{
			}
		}
		else 
		{
			char *file_buffer = ogg_sync_buffer(&ogg_sync, 8192);
			size_t bytes_read;
			NXFileRead(file, file_buffer, 8192, &bytes_read);
			// TODO: check for EOF
			ogg_sync_wrote(&ogg_sync, bytes_read);
		}
	
		return NErr_Success;
}


ns_error_t OggPlayback::FilePlayback_Interrupt(Agave_Seek *resume_information)
{
	return NErr_NotImplemented;
}

	ns_error_t OggPlayback::FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)
	{
	return NErr_NotImplemented;
}


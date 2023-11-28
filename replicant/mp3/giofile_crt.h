#pragma once
#include "GioReplicant.h"
#include <stdio.h>
#include "nsid3v1/nsid3v1.h"
#include "nsapev2/nsapev2.h"
#include "metadata/ifc_metadata.h"
#include "metadata/ifc_metadata_editor.h"
#include "nswasabi/APEv2Metadata.h"
#include "nx/nx.h"

class GioFile : public GioReplicant
{
public:
	GioFile();
	~GioFile();
	void Close();
	int Open(nx_uri_t filename, nx_file_t file);
	int SeekSeconds(double seconds, double average_bitrate);

	/* CGioBase implementation */
	SSC Read(void *buffer, int bytes_to_read, int *bytes_read);
	bool IsEof() const;

	uint64_t GetPosition();
	void SetPosition(uint64_t position);
protected:
	nx_file_t mp3_file;
	bool mpeg_eof;

	int ReadFirstFrame(uint8_t *frame_buffer, size_t buffer_len, size_t *buffer_written);
};


class GioFileWrite : public GioFile, public ifc_metadata_editor
{
protected:
		/* ifc_metadata_editor implementation */
	int WASABICALL MetadataEditor_Save();
	int WASABICALL MetadataEditor_SaveAs(nx_uri_t destination);

	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value);
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);

};

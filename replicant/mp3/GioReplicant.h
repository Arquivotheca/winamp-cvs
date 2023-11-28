#pragma once
#include "foundation/types.h"
#include "nsmp3/giobase.h"
#include "nsid3v2/nsid3v2.h"
#include "mp3/LAMEInfo.h"
#include "mp3/CVbriHeader.h"
#include "mp3/OFL.h"
#include "nswasabi/ID3v2Metadata.h"
#include "metadata/ifc_metadata.h"

template <class tag_t>
class GioTag
{
public:
	GioTag() : tag(0), position(0), length(0) {}
	tag_t tag;
	uint64_t position;
	uint64_t length;
};

class GioReplicant : public CGioBase, public ifc_metadata
{
public:
	GioReplicant();
	~GioReplicant();

	double GetLengthSeconds(double average_bitrate, ns_error_t *exact=0) const;
	int GetSeekPosition(double seconds, double average_bitrate, uint64_t *position);
	int GetBitrate(double *bitrate, ns_error_t *exact=0);

protected:
	uint64_t mpeg_length; // length of mpeg data
	uint64_t mpeg_position; // starting position of MPEG data (past ID3v2 tag, etc)
	uint64_t mpeg_samples; // number of decodable samples in the MPEG stream
	double mpeg_duration; // total duration in seconds of the MPEG stream
	uint32_t mpeg_frames; // total number of MPEG frames

	GioTag<LAMEInfo *> lame;
	GioTag<CVbriHeader *> vbri;
	GioTag<OFL *> ofl;
	
	MPEGHeader first_frame_header;

	/* ID3v2 */
	GioTag<nsid3v2_tag_t> id3v2;
	ID3v2Metadata id3v2_metadata;

	/* ifc_metadata */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	int WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }
};
#pragma once
#include "metadata/ifc_metadata.h"
#include "FLAC/all.h"
#include "nx/nx.h"
#include "nu/PtrList.h"

class FLACMetadata : public ifc_metadata
{
public:
	FLACMetadata();
	~FLACMetadata();
	int GetMetadata(const char *tag, unsigned int index, nx_string_t *value);
	int OwnStreamInfo(FLAC__StreamMetadata *stream_info);
	int OwnMetadataBlock(FLAC__StreamMetadata *metadata_block);
	int OwnPicture(FLAC__StreamMetadata *picture);
	void SetFileStats(nx_file_stat_t stats);
protected:
	nx_file_stat_s file_stats;
	FLAC__StreamMetadata *metadata_block;
	FLAC__StreamMetadata *stream_info;
	nu::PtrList<FLAC__StreamMetadata> pictures;
	bool own_data;
    
	/* ifc_metadata implementation */
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	int WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }

	int GetPosition(const char *tag, unsigned int index, int *pos);
};
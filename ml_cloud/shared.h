#pragma once
#include "metadata/ifc_metadata.h"

ns_error_t ComputeMediaHash(const wchar_t *filename, nx_string_t *out_mediahash);
ns_error_t ComputeArtHash(nx_uri_t filename, int field, nx_string_t *out_arthash);

class MediaHashMetadata : public ifc_metadata
{
public:
	MediaHashMetadata(nx_string_t mediahash);
	~MediaHashMetadata();
	static int MetadataKey_CloudMediaHash;
private:
	/* ifc_metadata implementation */
	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }

private:
	nx_string_t mediahash;
};

class ArtHashMetadata : public ifc_metadata
{
public:
	ArtHashMetadata(nx_string_t arthash_album);
	~ArtHashMetadata();
	static int MetadataKey_CloudArtHashAlbum;
private:
	/* ifc_metadata implementation */
	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }

private:
	nx_string_t arthash_album;
};
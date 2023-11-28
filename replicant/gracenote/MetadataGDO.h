#pragma once
#include "metadata/metadata.h"
#include "gnsdk.h"
#include "ifc_gracenote_results.h"
#include "nx/nxuri.h"

// {CF366BE4-7DC3-4f67-A04F-2315C1A01C3D}
static const GUID gracenote_metadata_identifier_track = 
{ 0xcf366be4, 0x7dc3, 0x4f67, { 0xa0, 0x4f, 0x23, 0x15, 0xc1, 0xa0, 0x1c, 0x3d } };

// {8F02C640-BB27-41cb-8F1B-33BE56B185B2}
static const GUID gracenote_metadata_identifier_album = 
{ 0x8f02c640, 0xbb27, 0x41cb, { 0x8f, 0x1b, 0x33, 0xbe, 0x56, 0xb1, 0x85, 0xb2 } };

// {A2EC9734-A83C-40c5-853E-53FB0748CF50}
static const GUID gracenote_metadata_guid = 
{ 0xa2ec9734, 0xa83c, 0x40c5, { 0x85, 0x3e, 0x53, 0xfb, 0x7, 0x48, 0xcf, 0x50 } };

class GracenoteMetadataService : public svc_metadata
{
public:
	WASABI_SERVICE_NAME("Gracenote Metadata");
	static GUID GetServiceGUID() { return gracenote_metadata_guid; }
private:
	int WASABICALL MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension) { return NErr_NotImplemented; }
	int WASABICALL MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata)  { return NErr_NotImplemented; }
	int WASABICALL MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata)  { return NErr_NotImplemented; }
	int WASABICALL MetadataService_DeserializeMetadata(nx_data_t data, ifc_metadata **metadata);
};

class MetadataGDO_ArtCache : public Wasabi2::Dispatchable /* for reference counting purposes */
{
public:
	MetadataGDO_ArtCache();
	~MetadataGDO_ArtCache();
	
	int Retrieve(gnsdk_gdo_handle_t gdo, unsigned int index, artwork_t *artwork, data_flags_t flags);
private:
	int Open(gnsdk_gdo_handle_t gdo);
	int GetArtDataCache(unsigned int index, artwork_t *artwork, data_flags_t flags);
	int Add(unsigned int index, gnsdk_byte_t *buffer, gnsdk_size_t buffer_size, gnsdk_cstr_t option);
	void SetAttempted(unsigned int index);

	struct data
	{
		int attempted;
		nx_data_t art_data;
	};
	data cache[5];
	gnsdk_link_query_handle_t link_handle;
	bool link_failed;
};

class MetadataGDO : public ifc_gracenote_results
{
public:
	MetadataGDO();
	~MetadataGDO();
	gnsdk_gdo_handle_t album_gdo;
	int SaveTo(nx_uri_t filename, int flags);
protected:
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }
	void WriteMetadata(ifc_metadata_editor *metadata, int key);

	MetadataGDO_ArtCache *art_cache;

};

class MetadataGDO_TrackMatch : public MetadataGDO
{
public:
	MetadataGDO_TrackMatch();
	~MetadataGDO_TrackMatch();

	int Initialize(gnsdk_gdo_handle_t gdo, gnsdk_cstr_t child=GNSDK_GDO_CHILD_TRACK_MATCHED, unsigned int index=1, MetadataGDO_ArtCache *art_cache=0);
	
private:
	
	gnsdk_gdo_handle_t track_gdo;

	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);

	
};

class MetadataGDO_AlbumMatch : public MetadataGDO
{
public:
	MetadataGDO_AlbumMatch();
	~MetadataGDO_AlbumMatch();

	int Initialize(gnsdk_gdo_handle_t gdo);
	
private:

	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetMetadata(int type, unsigned int index, ifc_metadata **metadata);	
	int WASABICALL Metadata_Serialize(nx_data_t *data);
};
#pragma once
#include "file/svc_filemetadata.h"
#include "file/ifc_filemetadata_editor.h"
#include "nswasabi/ServiceName.h"
#include "nswasabi/MetadataChain.h"
#include "giofile_crt.h"
#include "nswasabi/ReferenceCounted.h"

// {28251F66-E783-45FD-8A36-12529DB4F034}
static const GUID mp3_file_metadata_guid = 
{ 0x28251f66, 0xe783, 0x45fd, { 0x8a, 0x36, 0x12, 0x52, 0x9d, 0xb4, 0xf0, 0x34 } };

class MP3MetadataService : public svc_filemetadata
{
public:
	WASABI_SERVICE_NAME("MP3 File Metadata");
	static GUID GetServiceGUID() { return mp3_file_metadata_guid; }
private:
	int WASABICALL FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int WASABICALL FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);	
	int WASABICALL FileMetadataService_CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata);
};

class MP3FileMetadata :	public MetadataChain<GioFile>
{
public:
	ns_error_t Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);

};

class MP3MetadataEditor: public ifc_filemetadata_editor
{
public:
	MP3MetadataEditor();
	~MP3MetadataEditor();
	ns_error_t Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata);
private:
	ns_error_t WASABICALL FileMetadata_GetMetdataObject(ifc_metadata_editor **metadata);
	ns_error_t WASABICALL FileMetadata_Save(nx_file_t file);
	ns_error_t WASABICALL FileMetadata_RequireTempFile();	
	ns_error_t WASABICALL FileMetadata_SaveAs(nx_file_t destination, nx_file_t source);
	ns_error_t WASABICALL FileMetadata_WantID3v2(int *position);
	ns_error_t WASABICALL FileMetadata_WantID3v1();
	ns_error_t WASABICALL FileMetadata_WantAPEv2(int *position);
	ns_error_t WASABICALL FileMetadata_WantLyrics3();

	ifc_metadata_editor *metadata;
};
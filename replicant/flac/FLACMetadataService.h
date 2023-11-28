#pragma once
#include "file/svc_filemetadata.h"
#include "nswasabi/MetadataChain.h"
#include "nswasabi/MetadataEditorChain.h"
#include "FLACMetadata.h"
#include "FLAC/all.h"
#include "nswasabi/ServiceName.h"
#include "metadata/ifc_metadata_editor.h"
#include "nswasabi/ReferenceCounted.h"

class FLACMetadataEditor : public FLACMetadata, public ifc_metadata_editor
{
public:
	FLACMetadataEditor();
	~FLACMetadataEditor();
	REFERENCE_COUNT_AS(FLACMetadata);
	int Initialize(nx_uri_t filename, nx_file_t file, bool optimize);

	
protected:
	bool NeedsTempFile();
	ns_error_t Internal_Save(nx_file_t destination);
	ns_error_t Internal_SaveAs(nx_file_t destination, nx_file_t source);

		int WASABICALL MetadataEditor_Save() { return NErr_NotImplemented; }
	int WASABICALL MetadataEditor_SetField(int field, unsigned int index, nx_string_t value);
	int WASABICALL MetadataEditor_SetInteger(int field, unsigned int index, int64_t value);
	int WASABICALL MetadataEditor_SetReal(int field, unsigned int index, double value);
	int WASABICALL MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
private:
	
	int Internal_Initialize(nx_uri_t filename, FLAC__Metadata_Chain *chain, FLAC__Metadata_Iterator *itr);

	FLAC__Metadata_Chain *chain;
	FLAC__Metadata_Iterator *itr;
	nx_uri_t filename;	



	int SetMetadata(const char *tag, unsigned int index, nx_string_t value);
	int EraseMetadata(const char *tag, unsigned int index, nx_string_t value);
};

// {B14D0DDE-D5CC-41E9-93B4-A3788A75AA06}
static const GUID flac_file_metadata_guid = 
{ 0xb14d0dde, 0xd5cc, 0x41e9, { 0x93, 0xb4, 0xa3, 0x78, 0x8a, 0x75, 0xaa, 0x6 } };


class FLACMetadataService : public svc_filemetadata
{
public:
	WASABI_SERVICE_NAME("FLAC File Metadata");
	static GUID GetServiceGUID() { return flac_file_metadata_guid; }
private:
	int WASABICALL FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension);
	int WASABICALL FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
	int WASABICALL FileMetadataService_CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata);
};

class FLACFileMetadata : public MetadataChain<FLACMetadataEditor>
{
public:
	ns_error_t Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
};

class FLACFileMetadataEditor : public ifc_filemetadata_editor,
	private MetadataEditorChain<FLACMetadataEditor>
{
public:
	REFERENCE_COUNT_AS(ifc_filemetadata_editor);
	
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

};

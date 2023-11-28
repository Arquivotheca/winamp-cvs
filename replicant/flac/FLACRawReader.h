#pragma once
#include "file/svc_filerawreader.h"
#include "decode/ifc_raw_media_reader.h"
#include "FLAC/all.h"
#include "nswasabi/ServiceName.h"
#include "FLACFileCallbacks.h"

// {ECAE005F-966A-4AE7-BE6D-0172CBBD3485}
static const GUID flac_raw_reader_guid = 
{ 0xecae005f, 0x966a, 0x4ae7, { 0xbe, 0x6d, 0x1, 0x72, 0xcb, 0xbd, 0x34, 0x85 } };

class FLACRawReaderService : public svc_filerawreader
{
public:
	WASABI_SERVICE_NAME("FLAC File Raw Reader");	
	static GUID GetServiceGUID() { return flac_raw_reader_guid; } 
	int WASABICALL FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
};

class FLACRawReader : public ifc_raw_media_reader
{
public:
	FLACRawReader();
	~FLACRawReader();
	int Initialize(nx_file_t file, ifc_metadata *parent_metadata);
	int WASABICALL RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read);

private:
	FLAC__StreamDecoder *decoder;
	nx_file_t file;
	FLACClientData client_data;
};

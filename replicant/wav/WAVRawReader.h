#pragma once
#include "file/svc_filerawreader.h"
#include "decode/ifc_raw_media_reader.h"
#include "nsiff/nsiff.h"
#include "nswasabi/ServiceName.h"
#include "WAVParser.h"

// {235C41D5-4D78-437B-ABE5-DE73A413D857}
static const GUID wav_raw_reader_guid = 
{ 0x235c41d5, 0x4d78, 0x437b, { 0xab, 0xe5, 0xde, 0x73, 0xa4, 0x13, 0xd8, 0x57 } };


class WAVRawReaderService : public svc_filerawreader
{
public:
	WASABI_SERVICE_NAME("MAV File Raw Reader");	
	static GUID GetServiceGUID() { return wav_raw_reader_guid; } 
	int WASABICALL FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
};

class WAVRawReader : public ifc_raw_media_reader, public WAVParser
{
public:
	WAVRawReader();
	~WAVRawReader();
	int Initialize(nx_file_t file);
	int WASABICALL RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read);

private:
	nsiff_t iff_object;
};
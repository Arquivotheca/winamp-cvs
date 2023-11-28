#pragma once
#include "file/svc_filerawreader.h"
#include "decode/ifc_raw_media_reader.h"
#include "giofile_crt.h"
#include "nswasabi/ServiceName.h"

// {5EC19CF3-E1ED-4AA5-AFD3-8E93149692BD}
static const GUID mpeg_audio_raw_reader_guid = 
{ 0x5ec19cf3, 0xe1ed, 0x4aa5, { 0xaf, 0xd3, 0x8e, 0x93, 0x14, 0x96, 0x92, 0xbd } };

class MP3RawReaderService : public svc_filerawreader
{
public:
	WASABI_SERVICE_NAME("MP3 File Raw Reader");	
	static GUID GetServiceGUID() { return mpeg_audio_raw_reader_guid; } 
	int WASABICALL FileRawReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata);
};

class MP3RawReader : public ifc_raw_media_reader
{
public:
	MP3RawReader();
	~MP3RawReader();
	int Initialize(GioFile *file);
	int WASABICALL RawMediaReader_Read(void *buffer, size_t buffer_size, size_t *bytes_read);

private:
	GioFile *file;
};
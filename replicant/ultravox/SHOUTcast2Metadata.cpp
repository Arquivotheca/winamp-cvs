#include "api.h"
#include "SHOUTcast2Metadata.h"

SHOUTcast2Metadata::SHOUTcast2Metadata()
{
	metadata_id=0; /* we'll know the first metadata packet because we havn't created xml_parser yet */
	metadata_index=0;
	metadata_span=0;
	xml_parser=0;
	xml_factory = WASABI2_API_SVC->GetServiceByGUID(obj_xml::GetServiceGUID());
}

SHOUTcast2Metadata::~SHOUTcast2Metadata()
{
	if (xml_parser)
	{
		xml_parser->UnregisterCallback(&xml_artist);
		xml_parser->UnregisterCallback(&xml_album);
		xml_parser->UnregisterCallback(&xml_title);
		xml_parser->Close();
	
		xml_parser->Release();
	}
}

int SHOUTcast2Metadata::OnMetadata(ultravox_header_t ultravox_header, jnl_http_t http)
{
	int ret;
	bool new_metadata=false;
	size_t bytes_read;
	/* http is guaranteed to have enough bytes before this is called */

	/* Metadata packet format 
	[Metadata ID] (16bits) - Used to identify a metadata set for when metadata is split across multiple ultravox messages.
	[Metadata Span] (16 bits) - the number of messages comprising the complete metadata package (numeric, minimum 1, maximum 32).
	[Metadata Index] (16 bits) - the ordinal identification of this message within the metadata package (numeric, minimum 1, maximum 32).
	[Metadata] – the metadata information.
	*/

	if (!xml_factory || ultravox_header->uvox_length < 6)
		return NErr_Error;

	/* parse metadata 'header' stuff */
	UltravoxMetadataHeader metadata_header;
	uint8_t buffer[6];

	bytes_read = jnl_http_get_bytes(http, buffer, 6);
	ultravox_header->uvox_length -= bytes_read;

	ret = uvox_metadata_parse(&metadata_header, buffer, bytes_read);
	if (ret != NErr_Success)
		return ret;

	/* see if we have a different metadata block then last time */
	if (metadata_header.metadata_id != metadata_id)
		new_metadata=true;
	
	/* make sure we're starting parsing from the first packet */
	if (new_metadata)
	{
		if (metadata_header.metadata_index != 1)
			return NErr_Error;
	}
	else if (metadata_header.metadata_index != metadata_index + 1) /* make sure we didn't miss a packet */
	{
		return NErr_Error;
	}
	
	/* configure/open XML parser if needed */
	if (!xml_parser)
	{
		new_metadata=true;
		xml_parser = (obj_xml *)xml_factory->GetInterface();
		if (!xml_parser)
		{
			xml_factory = 0; // clear this so we don't ever try again
			return NErr_Error;
		}

		nx_string_t match_artist = NXStringCreateFromUTF8("metadata\fTPE1");
		nx_string_t match_album = NXStringCreateFromUTF8("metadata\fTALB");
		nx_string_t match_title = NXStringCreateFromUTF8("metadata\fTIT2");
		xml_parser->RegisterCallback(match_artist, obj_xml::MATCH_EXACT, &xml_artist);
		xml_parser->RegisterCallback(match_album, obj_xml::MATCH_EXACT, &xml_album);
		xml_parser->RegisterCallback(match_title, obj_xml::MATCH_EXACT, &xml_title);
		xml_parser->Open();
	} 
	else if (new_metadata) /* Reset XML parser if needed */
	{
		xml_artist.Reset();
		xml_album.Reset();
		xml_title.Reset();
		xml_parser->Reset();
	}

	/* Read metadata and give to XML parser */
	void *xml_buffer = xml_parser->GetBuffer(ultravox_header->uvox_length);
	if (!xml_buffer)
			return NErr_Error;

	bytes_read = jnl_http_get_bytes(http, xml_buffer, ultravox_header->uvox_length);
	ultravox_header->uvox_length-=bytes_read;	
	if (ultravox_header->uvox_length != 0)
		return NErr_Error;

	ret = xml_parser->ParseBuffer(bytes_read);
	if (ret != NErr_Success)
		return ret;
	
	metadata_id = metadata_header.metadata_id;
	metadata_index = metadata_header.metadata_id;

	/* check if it's the last packet in a span */
	if (metadata_header.metadata_index == metadata_header.metadata_span)
	{
		/* TODO: make ifc_metadata object! */
		xml_parser->Flush();
		return NErr_Success;
	}
	
	return NErr_NeedMoreData;
}
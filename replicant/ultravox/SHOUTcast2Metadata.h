#pragma once
#include "xml/obj_xml.h"
#include "service/ifc_servicefactory.h"
#include "nswasabi/XMLString.h"
#include "ultravox/UltravoxHeader.h"
#include "jnetlib/jnetlib.h"
#include "metadata/ifc_metadata.h"

class SHOUTcast2Metadata
{
public:
	SHOUTcast2Metadata();
	~SHOUTcast2Metadata();

	/* http is guaranteed to have enough bytes before this is called 
	  after function call, ultravox_header->uvox_length+1 bytes still remaining in the HTTP stream! */

	int OnMetadata(ultravox_header_t ultravox_header, jnl_http_t http);

private:
	obj_xml *xml_parser;
	ifc_serviceFactory *xml_factory; // we'll create this during the constructor, and it will also serve as a way of knowing that there's no XML service available (if it's NULL)

	XMLString xml_title, xml_artist, xml_album;
	uint16_t metadata_id, metadata_index, metadata_span;
};
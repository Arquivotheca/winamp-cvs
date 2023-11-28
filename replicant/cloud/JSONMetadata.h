#pragma once
#include "metadata/ifc_metadata.h"
#include "JSON-Tree.h"

class JSONMetadata : public ifc_metadata
{
public:
	JSONMetadata(const JSON::Value *cmd, const JSON::Value *fields);

private:
	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }
private:
	const JSON::Value *cmd;
	const JSON::Value *fields;
	const JSON::Value *art;
};
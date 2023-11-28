#pragma once
#include "metadata/ifc_metadata.h"
#include "../gen_ml/ml.h"

class ItemRecordMetadata : public ifc_metadata
{
public:
	ItemRecordMetadata(const itemRecordW *record) : record(record) {}
	~ItemRecordMetadata() {}

	ns_error_t WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	ns_error_t WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	ns_error_t WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);

	ns_error_t WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags);
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data);
	ns_error_t WASABICALL Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata);

	ns_error_t WASABICALL Metadata_Serialize(nx_data_t *data);

private:
	const itemRecordW *record;
};
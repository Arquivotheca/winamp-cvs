#include "FileMetadata.h"
#include "nswasabi/ReferenceCounted.h"

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}


/* ifc_metadata */
int FileMetadataRead::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	int ret;
	bool known=false;

	if (field == MetadataKeys::URI)
	{
		if (index > 0)
			return NErr_EndOfEnumeration;

		return NXURIGetNXString(value, filename);
	}

	ret = id3v2_metadata.Metadata_GetField(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = apev2_metadata.Metadata_GetField(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = id3v1_metadata.Metadata_GetField(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	if (known)
		return NErr_Empty;
	else
		return NErr_Unknown;
}

int FileMetadataRead::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	int ret;
	bool known=false;

	switch(field)
	{
	case MetadataKeys::FILE_SIZE:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = file_stat.file_size;
		return NErr_Success;
	case MetadataKeys::FILE_TIME:
		if (index > 0)
			return NErr_EndOfEnumeration;
		*value = file_stat.modified_time;
		return NErr_Success;
	}

	ret = id3v2_metadata.Metadata_GetInteger(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = apev2_metadata.Metadata_GetInteger(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = id3v1_metadata.Metadata_GetInteger(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	if (known)
		return NErr_Empty;
	else
		return NErr_Unknown;
}

int FileMetadataRead::Metadata_GetReal(int field, unsigned int index, double *value)
{
	bool known=false;

	int ret;
	ret = id3v2_metadata.Metadata_GetReal(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = apev2_metadata.Metadata_GetReal(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = id3v1_metadata.Metadata_GetReal(field, index, value);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success || ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else 
		return ret; /* separate else so we can set a breakpoint */

	if (known)
		return NErr_Empty;
	else 
		return NErr_Unknown;
}

int FileMetadataRead::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	bool known=false;
	int ret;

	ret = id3v2_metadata.Metadata_GetArtwork(field, index, artwork, flags);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success)
	{
		if (artwork && artwork->data && TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
		{
			ReferenceCountedNXURI source_uri;
			if (NXDataGetSourceURI(artwork->data, &source_uri) != NErr_Success)
			{
				NXDataSetSourceURI(artwork->data, filename);
				NXDataSetSourceStat(artwork->data, &file_stat);
			}
		}
		return ret;
	}
	else if (ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	ret = apev2_metadata.Metadata_GetArtwork(field, index, artwork, flags);
	if (ret == NErr_Empty)
		known=true;
	else if (ret == NErr_Success)
	{
		if (artwork && artwork->data && TestFlag(flags, DATA_FLAG_SOURCE_INFORMATION))
		{
			ReferenceCountedNXURI source_uri;
			if (NXDataGetSourceURI(artwork->data, &source_uri) != NErr_Success)
			{
				NXDataSetSourceURI(artwork->data, filename);
				NXDataSetSourceStat(artwork->data, &file_stat);
			}
		}
		return ret;
	}
	else if (ret == NErr_EndOfEnumeration) /* NErr_EndOfEnumeration indicates that the key was found, but the index is out-of-bounds */
		return ret;
	else if (ret != NErr_Unknown)
		return ret; /* separate else so we can set a breakpoint */

	if (known)
		return NErr_Empty;
	else
		return NErr_Unknown;
}

int FileMetadataRead::Metadata_GetBinary(int field, unsigned int index, nx_data_t *data)
{
	switch(field)
	{
	case MetadataKeys::STAT:
		return NXDataCreate(data, &file_stat, sizeof(file_stat));
	}
	return NErr_Unknown;
}

int FileMetadataRead::Metadata_GetMetadata(int field, unsigned int index, ifc_metadata **metadata)
{
	return NErr_NotImplemented;
}

int FileMetadataRead::Metadata_Serialize(nx_data_t *data)
{
	return NErr_NotImplemented;
}


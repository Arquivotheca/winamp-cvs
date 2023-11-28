#pragma once
#include "metadata/ifc_metadata.h"

class M3UFileInfo : public ifc_metadata
{
public:
	M3UFileInfo();
	~M3UFileInfo();

	void Reset();
	void SetTitle(const char *buf);
	void SetRootPath(nx_string_t root_path);
	void SetFilename(const char *filename);
	void SetUTF8(bool utf8);
	void SetLength(int length);

private:
	int WASABICALL Metadata_GetField(int field, unsigned int index, nx_string_t *value);
	int WASABICALL Metadata_GetInteger(int field, unsigned int index, int64_t *value);
	int WASABICALL Metadata_GetReal(int field, unsigned int index, double *value);
	int WASABICALL Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags) { return NErr_NotImplemented; }
	ns_error_t WASABICALL Metadata_GetBinary(int field, unsigned int index, nx_data_t *data) { return NErr_NotImplemented; }
	enum Relative
	{
		ABSOLUTE_PATH,
		RELATIVE_PATH,
		DRIVE_RELATIVE_PATH,
	};

		nx_charset_t charset;
	const char *filename; /* can be a pointer because it's assigned right before the OnFile call */
	char title[2048]; /* has to be a buffer because we'll overwrite the read buffer when reading the line with the filename */
	int length;
};
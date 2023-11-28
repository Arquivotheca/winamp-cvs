#include "M3UFileInfo.h"
#include "metadata/MetadataKeys.h"
#include "nu/strsafe.h"

M3UFileInfo::M3UFileInfo()
{
	charset=nx_charset_latin1;
	Reset();
}

M3UFileInfo::~M3UFileInfo()
{
}

void M3UFileInfo::Reset()
{
	filename=0;
	title[0]=0;
	length=-1;
}

void M3UFileInfo::SetTitle(const char *buf)
{
	StringCbCopyA(title, sizeof(title), buf);
}

void M3UFileInfo::SetFilename(const char *filename)
{
	this->filename = filename;
}

void M3UFileInfo::SetUTF8(bool utf8)
{
	if (utf8)
		charset=nx_charset_utf8;
	else
		charset=nx_charset_latin1;
}

void M3UFileInfo::SetLength(int length)
{
	this->length = length;
}

/* ifc_metadata implementations */
int M3UFileInfo::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	if (index != 0)
		return NErr_EndOfEnumeration;

	switch(field)
	{
	case MetadataKeys::URI:
		if (!filename)
			return NErr_Empty;
		return NXStringCreateWithCString(value, filename, charset);
	case MetadataKeys::TITLE:
		if (!title[0])
			return NErr_Empty;
		return NXStringCreateWithCString(value, title, charset);

	}
	return NErr_Unknown;
}

int M3UFileInfo::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	return NErr_Unknown;
}

int M3UFileInfo::Metadata_GetReal(int field, unsigned int index, double *value)
{
	if (index != 0)
		return NErr_EndOfEnumeration;
	switch(field)
	{
	case MetadataKeys::LENGTH:
		if (length == -1)
			return NErr_Empty;

		*value = (double)length;
		return NErr_Success;
	}

	return NErr_Unknown;
}

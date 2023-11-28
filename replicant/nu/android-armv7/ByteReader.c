#include "ByteReader.h"
#include <string.h>
/* generic LITTLE ENDIAN implementation */

size_t bytereader_find_zero(bytereader_t byte_reader)
{
	size_t i=0;
	
	for (i=0;i<byte_reader->byte_length && byte_reader->data_ptr[i];i++)
	{
		// empty loop
	}
	return i;
}

GUID bytereader_read_uuid_be(bytereader_t byte_reader)
{
	GUID guid_value;
	guid_value.Data1 = bytereader_read_u32_be(byte_reader);
	guid_value.Data2 = bytereader_read_u16_be(byte_reader);
	guid_value.Data3 = bytereader_read_u16_be(byte_reader);
	bytereader_read_n(byte_reader, guid_value.Data4, 8);
	return guid_value;
}
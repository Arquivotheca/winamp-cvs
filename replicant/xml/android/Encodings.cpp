#include <expat.h>

int XMLCALL UnknownEncoding(void *data, const XML_Char *name, XML_Encoding *info)
{
	// TODO
	return XML_STATUS_ERROR;
}
#include "nxpath.h"
#include "foundation/error.h"
#include <string.h>


int NXPathMatchExtension(nx_uri_t filename, nx_string_t extension)
{
	size_t a, b;

	if (!filename || !extension)
		return NErr_False;

	a = strlen(filename->string); 
	b = strlen(extension->string);

	if (b >= a)
		return NErr_False;

	if (filename->string[a-b-1] != '.')
		return NErr_False;

	if (strcasecmp(&filename->string[a-b], extension->string) == 0)
		return NErr_True;
	else
		return NErr_False;

}

int NXPathProtocol(nx_uri_t filename, const char *protocol)
{
		size_t a, b;

	if (!filename || !protocol)
		return NErr_False;

	a = strlen(filename->string);
	b = strlen(protocol);

	if (b > a)
		return NErr_False;

	if (filename->string[b] != ':' || filename->string[b+1] != '/' || filename->string[b+2] != '/')
		return NErr_False;

	do
	{
		// TODO: case insensitive
		b--;
		if (filename->string[b] != protocol[b])
			return NErr_False;
	} while (b);
	
	return NErr_Success;
}

int NXPathIsURL(nx_uri_t filename)
{
	if (filename && strstr(filename->string, "://"))
		return NErr_True;

	return NErr_False;
}
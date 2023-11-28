#include "M3UHandler.h"
#include "M3ULoader.h"
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nx.h"
#include "constants.h"
#include <new>

M3UHandler::M3UHandler()
{

}

nx_string_t M3UHandler::PlaylistHandler_EnumerateExtensions(size_t n)
{
	switch(n)
	{
	case 0:
		return extension_m3u;
	case 1:
		return extension_m3u8;
	default:
		return 0;
	}
}

int M3UHandler::PlaylistHandler_SupportedFilename(nx_uri_t filename)
{
	if (NXPathMatchExtension(filename, extension_m3u) == NErr_True
		|| NXPathMatchExtension(filename, extension_m3u8) == NErr_True)
		return NErr_True;

	return NErr_False;
}

int M3UHandler::PlaylistHandler_CreateLoader(nx_uri_t filename, ifc_playlistloader **out_loader)
{
	M3ULoader *loader = new (std::nothrow) ReferenceCounted<M3ULoader>;
	if (!loader)
		return NErr_OutOfMemory;

	*out_loader = loader;
	return NErr_Success;	
}
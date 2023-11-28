#include "nsiff.h"
#include "Parser.h"
#include <new>

ns_error_t nsiff_create_parser_from_file(nsiff_t *out_iff_object, nx_file_t file, nsiff_callbacks_t callbacks, void *context)
{
	NSIFF::Parser *iff_object = 0;
	ns_error_t ret = NSIFF::Parser::CreateParser(&iff_object, file, callbacks, context);
	if (ret != NErr_Success)
		return ret;

	*out_iff_object = (nsiff_t)iff_object;
	return NErr_Success;
}


void nsiff_destroy(nsiff_t iff_object)
{
	delete (NSIFF::Parser *)iff_object;
}

ns_error_t nsiff_parse(nsiff_t iff_object)
{
	return ((NSIFF::Parser *)iff_object)->Parse();
}

ns_error_t nsiff_read_current_chunk(nsiff_t object, void *data, size_t read_size, size_t *bytes_read)
{
	if (!object)
		return NErr_BadParameter;

	NSIFF::Parser *parser = (NSIFF::Parser *)object;
	return parser->ReadCurrentChunk(data, read_size, bytes_read);	
}

ns_error_t nsiff_seek(nsiff_t object, uint64_t position)
{
	if (!object)
		return NErr_BadParameter;

	NSIFF::Parser *parser = (NSIFF::Parser *)object;
	return parser->Seek(position);
}

ns_error_t nsiff_tell(nsiff_t object, uint64_t *position)
{
	if (!object)
		return NErr_BadParameter;

	NSIFF::Parser *parser = (NSIFF::Parser *)object;
	return parser->Tell(position);
}

ns_error_t nsiff_read(nsiff_t object, void *data, size_t bytes_requested, size_t *bytes_read)
{
	if (!object)
		return NErr_BadParameter;

	NSIFF::Parser *parser = (NSIFF::Parser *)object;
	return parser->Read(data, bytes_requested, bytes_read);
}


#include "nx/nxdata.h"
#include "foundation/atomics.h"
#include "foundation/error.h"
#include <stdlib.h>
#include <string.h>

typedef struct nx_data_struct_t
{
	volatile size_t ref_count;
	nx_string_t mime_type;
	nx_string_t description;
	nx_uri_t source_uri;
	nx_file_stat_t source_stats;
	CFDataRef data_ref;
} nx_data_struct_t;

nx_data_t NXDataRetain(nx_data_t data)
{
	if (NULL == data)
		return 0;

	nx_atomic_inc(&data->ref_count);
	return data;

}

void NXDataRelease(nx_data_t data)
{
	if (data)
	{
		if (nx_atomic_dec(&data->ref_count) == 0)
		{
			free(data->source_stats);
			NXURIRelease(data->source_uri);
			NXStringRelease(data->mime_type);
			NXStringRelease(data->description);
			if (NULL != data->data_ref)
				CFRelease(data->data_ref);
			free(data);
		}
	}
}


int
NXDataCreateWithCFData(nx_data_t *out_data, CFDataRef data_ref, nx_string_t mime_type)
{
	if (NULL == out_data)
		return NErr_BadParameter;
	
	nx_data_t data = (nx_data_t)malloc(sizeof(nx_data_struct_t));
	if (!data)
	{
		*out_data = NULL;
		return NErr_OutOfMemory;
	}
		
	if (NULL != data_ref)
		CFRetain(data_ref);

	data->data_ref = data_ref;
	
	data->ref_count = 1;
	data->mime_type = NXStringRetain(mime_type);
	data->source_uri=0;
	data->source_stats=0;
	data->description=0;
	*out_data = data;
	
	return NErr_Success;
}

int NXDataCreateEmpty(nx_data_t *out_data)
{
	return NXDataCreateWithCFData(out_data, 0, 0);
}

int NXDataCreateFromURI(nx_data_t *out_data, nx_uri_t filename)
{
	nx_file_stat_s stat_buffer;
	nx_data_t data;
	size_t data_length;
	size_t bytes_read;
	uint64_t file_length;
	void *bytes;
	int ret;
	int fd;
	
	
	fd = NXFile_open(filename, nx_file_O_BINARY|nx_file_O_RDONLY);
	if (fd == -1)
		return NErr_FileNotFound;
	
	ret = NXFile_fstat(fd, &stat_buffer);
	if (ret != NErr_Success)
	{
		close(fd);
		return ret;
	}
	
	file_length = stat_buffer.file_size;
	
	if (file_length > SIZE_MAX)
	{
		close(fd);
		return NErr_IntegerOverflow;
	}
	
	data_length = (size_t)file_length;
	
	ret = NXDataCreateWithSize(&data, &bytes, data_length);
	if (ret != NErr_Success)
	{
		close(fd);
		return ret;
	}
	
	data->source_stats=(nx_file_stat_t)malloc(sizeof(nx_file_stat_s));
	if (!data->source_stats)
	{
		close(fd);
		NXDataRelease(data);
		return NErr_OutOfMemory;
	}
	
	bytes_read = read(fd, bytes, data_length);
	close(fd);
	if (bytes_read != data_length)
	{
		NXDataRelease(data);
		return NErr_Error;
	}
	
	*data->source_stats=stat_buffer;
	data->source_uri=NXURIRetain(filename);
	*out_data = data;
	return NErr_Success;
}

int NXDataCreate(nx_data_t *out_data, const void *bytes, size_t length)
{
	CFDataRef data_ref;
	
	data_ref = CFDataCreate(kCFAllocatorDefault, bytes, length);
	if (NULL == data_ref)
	{
		*out_data = NULL;
		return NErr_OutOfMemory;
	}
	
	int err = NXDataCreateWithCFData(out_data, data_ref, 0);
	CFRelease(data_ref);
	
	return err;
}

int NXDataCreateWithSize(nx_data_t *out_data, void **bytes, size_t length)
{
	CFDataRef data_ref;
	
	nx_data_t data = (nx_data_t)malloc(sizeof(nx_data_struct_t));
	if (!data)
		return NErr_OutOfMemory;
	
	void *buffer = CFAllocatorAllocate(kCFAllocatorDefault, length, 0);
	if (NULL == buffer)
	{
		*out_data = NULL;
		if (NULL != bytes)
			*bytes = NULL;
		free(data);
		return NErr_OutOfMemory;
	}
	
	data_ref = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, buffer, length, kCFAllocatorDefault);
	if (NULL == data_ref)
	{
		CFAllocatorDeallocate(kCFAllocatorDefault, buffer);
		*out_data = NULL;
		if (NULL != bytes)
			*bytes = NULL;
		free(data);
		CFAllocatorDeallocate(kCFAllocatorDefault, buffer);
		return NErr_OutOfMemory;
	}
	
	int err = NXDataCreateWithCFData(out_data, data_ref, 0);
	CFRelease(data_ref);
	
	if (NULL != bytes)
	{
		if (NErr_Success != err)
			*bytes = NULL;
		else 
			*bytes = buffer;
	}
	
	return err;
}

size_t NXDataSize(nx_data_t data)
{
	if (NULL == data || NULL == data->data_ref)
		return 0;
	
	return CFDataGetLength(data->data_ref);
}

const void * NXDataPointer(nx_data_t data)
{
	if (NULL == data || NULL == data->data_ref)
		return NULL;
	
	return CFDataGetBytePtr(data->data_ref);
}

int NXDataGet(nx_data_t data, const void **bytes, size_t *length)
{
	if (NULL == data || NULL == data->data_ref)
		return NErr_NullPointer;
	
	CFIndex size = CFDataGetLength(data->data_ref);
	
	if (NULL != bytes)
		*bytes = CFDataGetBytePtr(data->data_ref);
	
	if (NULL != length)
		*length = size;
	
	if (0 == size)
		return NErr_Empty;

	return NErr_Success;
}

int NXDataCopyBytes(nx_data_t data, void *buffer, size_t offset, size_t length)
{
	if (NULL == data || NULL == data->data_ref)
		return NErr_NullPointer;
	
	if (NULL == buffer)
		return NErr_BadParameter;
	
	CFRange range = CFRangeMake(offset, length);
	CFDataGetBytes(data->data_ref, range, buffer);
	
	return NErr_Success;
}


int NXDataSetMIME(nx_data_t data, nx_string_t mime_type)
{
	nx_string_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->mime_type;
	data->mime_type = NXStringRetain(mime_type);
	NXStringRelease(old);
	return NErr_Success;
}

int NXDataSetDescription(nx_data_t data, nx_string_t description)
{
	nx_string_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->description;
	data->description = NXStringRetain(description);
	NXStringRelease(old);
	return NErr_Success;
}

int NXDataSetSourceURI(nx_data_t data, nx_uri_t source_uri)
{
		nx_uri_t old;
	if (!data)
		return NErr_BadParameter;

	old = data->source_uri;
	data->source_uri = NXURIRetain(source_uri);
	NXURIRelease(old);
	return NErr_Success;
}

int NXDataSetSourceStat(nx_data_t data, nx_file_stat_t source_stats)
{
	nx_file_stat_t new_stats;
	if (!data)
		return NErr_BadParameter;

	if (source_stats)
	{
		new_stats=(nx_file_stat_t)malloc(sizeof(nx_file_stat_s));
		if (!new_stats)
			return NErr_OutOfMemory;

		*new_stats = *source_stats;
		free(data->source_stats);
		data->source_stats=new_stats;
	}
	else
	{
		free(data->source_stats);
		data->source_stats=0;
	}
	return NErr_Success;
}

int NXDataGetMIME(nx_data_t data, nx_string_t *mime_type)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->mime_type)
		return NErr_Empty;

	*mime_type = NXStringRetain(data->mime_type);
	return NErr_Success;
}

int NXDataGetDescription(nx_data_t data, nx_string_t *description)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->description)
		return NErr_Empty;

	*description = NXStringRetain(data->description);
	return NErr_Success;
}

int NXDataGetSourceURI(nx_data_t data, nx_uri_t *source_uri)
{
	if (!data)
		return NErr_BadParameter;

	if (!data->source_uri)
		return NErr_Empty;

	*source_uri = NXURIRetain(data->source_uri);
	return NErr_Success;
}

int NXDataGetSourceStat(nx_data_t data, nx_file_stat_t *source_stats)
{
	if (!data)
		return NErr_BadParameter;
	
	if (!data->source_stats)
		return NErr_Empty;

	*source_stats = data->source_stats;
	return NErr_Success;
}

int NXDataGetCFData(nx_data_t data, CFDataRef *data_ref)
{
	if (NULL == data_ref)
		return NErr_BadParameter;
	
	if (NULL == data)
		return NErr_NullPointer;
	
	*data_ref = data->data_ref;
	if (NULL == data->data_ref)
		return NErr_Empty;
	
	CFRetain(*data_ref);
	return NErr_Success;
}

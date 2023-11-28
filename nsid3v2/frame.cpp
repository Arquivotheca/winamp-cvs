#include "frame.h"
#include "util.h"
#include "../zlib/zlib.h"
#include "frames.h"
#include <bfc/error.h>

/* === ID3v2 common === */
ID3v2::Frame::Frame()
{
	data = 0;
	data_size = 0;
}

ID3v2::Frame::~Frame()
{
	free(data);
}

int ID3v2::Frame::GetData(const void **_data, size_t *data_len) const
{
	if (data)
	{
		*_data = data;
		*data_len = data_size;
		return NErr_Success;
	}
	else
		return NErr_NullPointer;
}

size_t ID3v2::Frame::GetDataSize() const
{
	return data_size;
}

int ID3v2::Frame::NewData(size_t new_len, void **_data, size_t *_data_len)
{
	// TODO: benski> do we need to update the frame header?
	void *new_data = malloc(new_len);
	if (new_data)
	{
		free(data);
		data = new_data;
		data_size = new_len;
		*_data = data;
		*_data_len = data_size;
		return NErr_Success;
	}
	else
		return NErr_OutOfMemory;
}

static inline void Advance(const void *&data, size_t &len, size_t amount)
{
	data = (const uint8_t *)data + amount;
	len -= amount;
}

static inline void AdvanceBoth(const void *&data, size_t &len, size_t &len2, size_t amount)
{
	data = (const uint8_t *)data + amount;
	len -= amount;
	len2 -= amount;
}

/* === ID3v2.2 === */
ID3v2_2::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_2::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

int ID3v2_2::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	data_size = header.FrameSize(); // size of frame AFTER re-synchronization
	
	/* check to make sure that we have enough input data to read the data */
	if (header.Unsynchronised())
	{
		/* this is tricky, because the stored size reflects after re-synchronization,
		but the incoming data is unsynchronized */
		if (ID3v2::Util::UnsynchronisedSize(_data, data_size) > len)
			return 1;
	}
	else if (data_size > len)
		return 1;

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return 1;

	/* === Read the data === */
	if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return NErr_Success;
}

const int8_t *ID3v2_2::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}

/* === ID3v2.3 === */
ID3v2_3::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_3::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

/* helper function
		reads num_bytes from input into output, dealing with re-synchronization and length checking 
		increments input pointer
		increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
		decrements input_len by bytes read
		decrements output_len by bytes written
		*/
bool ID3v2_3::Frame::ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const
{
	/* verify that we have enough data in the frame */
	if (num_bytes > frame_len)
		return false;

	/* verify that we have enough data in the buffer */
	size_t bytes_to_read;
	if (header.Unsynchronised())
		bytes_to_read = ID3v2::Util::UnsynchronisedSize(input, num_bytes);
	else
		bytes_to_read = num_bytes;

	if (bytes_to_read > input_len)
		return false;

	/* read data */
	if (header.Unsynchronised())
	{
		*bytes_read += ID3v2::Util::SynchroniseTo(&output, input, num_bytes);
	}
	else
	{
		*bytes_read += num_bytes;
		memcpy(output, input, num_bytes);
	}

	/* increment input pointer */
	input = (const uint8_t *)input + bytes_to_read;

	/* decrement sizes */
	frame_len -= num_bytes;
	input_len -= bytes_to_read;
	return true;	
}

/* benski> this function is a bit complex
  we have two things to worry about, and can have any combination of the two
	1) Is the data 'unsynchronized'
	2) Is the data compressed (zlib)

	we keep track of three sizes:
	len - number of bytes in input buffer
	data_size - number of bytes of output data buffer
	frame_size - number of bytes of data in frame AFTER re-synchronization

	frame_size==data_size when compression is OFF
*/
int ID3v2_3::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	size_t frame_size = header.FrameSize(); // size of frame AFTER re-synchronization
	
	if (header.Compressed())
	{
		// read 4 bytes of decompressed size
		uint32_t true_size;
		if (ReadData(&true_size, _data, len, frame_size, 4, read) == false)
			return 1;

		data_size = ID3v2::Util::UInt32RawToUInt32(true_size);
	}

	/* Check for group identity.  If this exists, we'll store it separate from the raw data */
	if (header.Grouped())
	{
		// read 1 byte for group identity
		if (ReadData(&group_identity, _data, len, frame_size, 1, read) == false)
			return 1;
	}

	if (!header.Compressed())
	{
		data_size = frame_size;
	}

	/* check to make sure that we have enough input data to read the data */
	if (!header.Compressed() && header.Unsynchronised())
	{
		/* this is tricky, because the stored size reflects after re-synchronization,
		but the incoming data is unsynchronized */
		if (ID3v2::Util::UnsynchronisedSize(_data, data_size) > len)
			return 1;
	}
	else if (frame_size > len)
		return 1;

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return NErr_OutOfMemory;

	/* === Read the data === */
	if (header.Compressed())
	{
		if (header.Unsynchronised()) // compressed AND unsynchronized.. what a pain!!
		{
			// TODO: combined re-synchronization + inflation
			size_t sync_size = ID3v2::Util::UnsynchronisedSize(_data, frame_size);
			void *temp = malloc(sync_size);
			if (!temp)
				return 1;

			*read += ID3v2::Util::UnsynchroniseTo(temp, _data, frame_size);

			uLongf uncompressedSize = data_size;
			int ret = uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)temp, sync_size);
			free(temp);
			if (ret != Z_OK)
				return 1;
		}
		else
		{
			uLongf uncompressedSize = data_size;
			if (uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)_data, frame_size) != Z_OK)
				return 1;
			*read += frame_size;
		}
	}
	else if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return NErr_Success;
}

const int8_t *ID3v2_3::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}

/* === ID3v2.4 === */
ID3v2_4::Frame::Frame(const ID3v2::Header &_header, const int8_t *id, int flags) : header(_header, id, flags)
{
}

ID3v2_4::Frame::Frame(const FrameHeader &_header) : header(_header)
{
}

/* helper function
		reads num_bytes from input into output, dealing with re-synchronization and length checking 
		increments input pointer
		increments bytes_read value by number of input bytes read (different from num_bytes when data is unsynchronized
		decrements input_len by bytes read
		decrements output_len by bytes written
		*/
bool ID3v2_4::Frame::ReadData(void *output, const void *&input, size_t &input_len, size_t &frame_len, size_t num_bytes, size_t *bytes_read) const
{
	/* verify that we have enough data in the frame */
	if (num_bytes > frame_len)
		return false;

	/* verify that we have enough data in the buffer */
	size_t bytes_to_read;
	if (header.Unsynchronised())
		bytes_to_read = ID3v2::Util::UnsynchronisedSize(input, num_bytes);
	else
		bytes_to_read = num_bytes;

	if (bytes_to_read > input_len)
		return false;

	/* read data */
	if (header.Unsynchronised())
	{
		*bytes_read += ID3v2::Util::SynchroniseTo(&output, input, num_bytes);
	}
	else
	{
		*bytes_read += num_bytes;
		memcpy(output, input, num_bytes);
	}

	/* increment input pointer */
	input = (const uint8_t *)input + bytes_to_read;

	/* decrement sizes */
	frame_len -= num_bytes;
	input_len -= bytes_to_read;
	return true;	
}

/* benski> this function is a bit complex
  we have two things to worry about, and can have any combination of the two
	1) Is the data 'unsynchronized'
	2) Is the data compressed (zlib)

	we keep track of three sizes:
	len - number of bytes in input buffer
	data_size - number of bytes of output data buffer
	frame_size - number of bytes of data in frame AFTER re-synchronization

	frame_size==data_size when compression is OFF
*/
int ID3v2_4::Frame::Parse(const void *_data, size_t len, size_t *read)
{
	*read = 0;
	size_t frame_size = header.FrameSize(); // size of frame AFTER re-synchronization
	
	// TODO: if frame_size >= 128, verify size.  iTunes v2.4 parser bug ...

	
	/* Check for group identity.  If this exists, we'll store it separate from the raw data */
	/* Note: ID3v2.4 puts group identity BEFORE data length indicator, where as v2.3 has it the other way */
	if (header.Grouped())
	{
		// read 1 byte for group identity
		if (ReadData(&group_identity, _data, len, frame_size, 1, read) == false)
			return 1;
	}

	if (header.Compressed() || header.DataLengthIndicated())
	{
		// read 4 bytes of decompressed size
		uint32_t true_size;
		if (ReadData(&true_size, _data, len, frame_size, 4, read) == false)
			return 1;

		data_size = ID3v2::Util::UInt32RawToUInt32(true_size);
	}

	if (!(header.Compressed() || header.DataLengthIndicated()))
	{
		data_size = frame_size;
	}

	/* check to make sure that we have enough input data to read the data */
	if (!header.Compressed() && header.Unsynchronised())
	{
		/* this is tricky, because the stored size reflects after re-synchronization,
		but the incoming data is unsynchronized */
		if (ID3v2::Util::UnsynchronisedSize(_data, data_size) > len)
			return 1;
	}
	else if (frame_size > len)
		return 1;

	/* allocate memory (real data_size) */
	data = malloc(data_size);
	if (!data)
		return 1;

	/* === Read the data === */
	if (header.Compressed())
	{
		if (header.Unsynchronised()) // compressed AND unsynchronized.. what a pain!!
		{
			// TODO: combined re-synchronization + inflation
			size_t sync_size = ID3v2::Util::UnsynchronisedSize(_data, frame_size);
			void *temp = malloc(sync_size);
			if (!temp)
				return 1;

			*read += ID3v2::Util::UnsynchroniseTo(temp, _data, frame_size);

			uLongf uncompressedSize = data_size;
			int ret = uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)temp, sync_size);
			free(temp);
			if (ret != Z_OK)
				return 1;
		}
		else
		{
			uLongf uncompressedSize = data_size;
			if (uncompress((Bytef *)data, &uncompressedSize, (const Bytef *)_data, frame_size) != Z_OK)
				return 1;
			*read += frame_size;
		}
	}
	else if (header.Unsynchronised())
	{
		*read += ID3v2::Util::UnsynchroniseTo(data, _data, data_size);
	}
	else // normal data
	{
		memcpy(data, _data, data_size);
		*read += data_size;
	}

	return 0;
}

const int8_t *ID3v2_4::Frame::GetIdentifier() const
{
	return header.GetIdentifier();
}

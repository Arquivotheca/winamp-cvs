#include "api.h"
#include "giofile_crt.h"
#include "nsid3v2/nsid3v2.h"
#include <assert.h>
#include "nx/nxfile.h"
#include "metadata/MetadataKeys.h"
#include "nswasabi/ReferenceCounted.h"
#include <stdlib.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <new>
#include "nu/AutoBuffer.h"

/* assumption about MP3 metadata layout is as follows:
[ID3v2] ([LAME] || [VBRI]) ([OFL] && MPEG DATA) [APEv2] [Lyrics3] [ID3v2.4] [ID3v1]
*/

GioFile::GioFile()
{
	mp3_file = 0;
	mpeg_eof=false;
}

GioFile::~GioFile()
{
	Close();
}

void GioFile::Close()
{
	NXFileRelease(mp3_file);
	mp3_file = 0;
}

int GioFile::SeekSeconds(double seconds, double average_bitrate)
{
	uint64_t absolute_position;
	GioReplicant::GetSeekPosition(seconds, average_bitrate, &absolute_position);

	mpeg_eof=false;
	NXFileSeek(mp3_file, absolute_position);

	return NErr_Success;
}

int GioFile::ReadFirstFrame(uint8_t *frame_buffer, size_t buffer_len, size_t *buffer_written)
{
	size_t bytes_read;
	
	if (NXFileRead(mp3_file, frame_buffer, 4, &bytes_read) == NErr_Success && bytes_read == 4) // read header
	{
		first_frame_header.ReadBuffer(frame_buffer);
		if (first_frame_header.IsSync())
		{
			if (first_frame_header.IsCRC())
			{
				// TODO: maybe read CRC data and give it to the header object
				NXFileRead(mp3_file, 0, 2, &bytes_read);
			}

			size_t frame_length = first_frame_header.FrameSize()-first_frame_header.HeaderSize();
			if (frame_length <= buffer_len 
				&& NXFileRead(mp3_file, frame_buffer, frame_length, &bytes_read) == NErr_Success && bytes_read == frame_length) // read frame data
			{
				*buffer_written = frame_length;
				return NErr_Success;
			}
		}
	}
	return NErr_Error;
}

int GioFile::Open(nx_uri_t filename, nx_file_t file)
{
	mp3_file = NXFileRetain(file);

	NXFileLength(mp3_file, &mpeg_length);
	mpeg_position = 0;
	NXFileSeek(mp3_file, 0);

	/* If we're lucky, we start right on an MPEG frame */
	uint8_t frame_buffer[1448];
	size_t frame_buffer_length;
	if (ReadFirstFrame(frame_buffer, 1448, &frame_buffer_length) == NErr_Success)
	{
		/* read LAME/Xing header */
		lame.tag = new (std::nothrow) LAMEInfo;
		if (!lame.tag)
			return NErr_OutOfMemory;
		if (lame.tag->Read(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
		{
			size_t vbr_header_length=first_frame_header.FrameSize();
			lame.position = mpeg_position;
			lame.length = vbr_header_length;
			// TODO: validate values and update flags accordingly
			mpeg_position += vbr_header_length; 
			mpeg_length -= vbr_header_length;

			mpeg_duration = lame.tag->GetLengthSeconds();
			mpeg_samples = lame.tag->GetSamples();
			mpeg_frames = lame.tag->GetFrames();
		}
		else
		{
			delete lame.tag;
			lame.tag=0;
			/* read VBRI header */
			vbri.tag = new (std::nothrow) CVbriHeader;
			if (!vbri.tag)
				return NErr_OutOfMemory;
			if (vbri.tag->readVbriHeader(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
			{
				size_t vbr_header_length=first_frame_header.FrameSize();
				vbri.position = mpeg_position;
				vbri.length = vbr_header_length;
				mpeg_position += vbr_header_length;
				mpeg_length -= vbr_header_length;

				mpeg_duration = vbri.tag->GetLengthSeconds();
				mpeg_samples = vbri.tag->GetSamples();
				mpeg_frames = vbri.tag->GetFrames();
				if (mpeg_frames)
					mpeg_frames--;
			}
			else
			{
				delete vbri.tag;
				vbri.tag=0;
				NXFileSeek(mp3_file, mpeg_position);
			}
		}

		/* read ofl data */
		if (ReadFirstFrame(frame_buffer, 1448, &frame_buffer_length) == NErr_Success)
		{
			ofl.tag = new (std::nothrow) OFL;
			if (!ofl.tag)
				return NErr_OutOfMemory;
			if (ofl.tag->Read(first_frame_header, frame_buffer, frame_buffer_length) == NErr_Success)
			{
				ofl.position = mpeg_position;
				ofl.length = first_frame_header.FrameSize();
				mpeg_duration = ofl.tag->GetLengthSeconds();
				mpeg_samples = ofl.tag->GetSamples();
				if (!mpeg_frames)
					mpeg_frames = ofl.tag->GetFrames();
			}
			else
			{
				delete ofl.tag;
				ofl.tag=0;
			}
		}
		NXFileSeek(mp3_file, mpeg_position);
	}
	else
	{
		memset(&first_frame_header, 0, sizeof(first_frame_header));
	}

	return NErr_Success;
}

/* CGioBase */
SSC GioFile::Read(void *buffer, int bytes_to_read, int *bytes_read)
{
	size_t local_bytes_read;
	ns_error_t ret = NXFileRead(mp3_file, buffer, bytes_to_read, &local_bytes_read);
	if (ret == NErr_EndOfFile)
	{
		mpeg_eof=true;
		*bytes_read = 0;
		return SSC_OK;//SSC_W_MPGA_SYNCEOF;
	}
	else if (ret == NErr_Success)
	{
		*bytes_read = local_bytes_read;
		return SSC_OK;
	}
	else
	{
		*bytes_read = 0;
		return SSC_E_IO_READFAILED;
	}
}

bool GioFile::IsEof() const
{
	return mpeg_eof;
}

void GioFile::SetPosition(uint64_t position)
{
	NXFileSeek(mp3_file, mpeg_position+position);
}

uint64_t GioFile::GetPosition() 
{
	uint64_t file_position;
	NXFileTell(mp3_file, &file_position);
	return file_position-mpeg_position; 
}

int GioFileWrite::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	return NErr_NotImplemented;
}

int GioFileWrite::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	return NErr_NotImplemented;
}

int GioFileWrite::MetadataEditor_SetReal(int field, unsigned int index, double value)
{
	return NErr_NotImplemented;
}

int GioFileWrite::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	return NErr_NotImplemented;	
}
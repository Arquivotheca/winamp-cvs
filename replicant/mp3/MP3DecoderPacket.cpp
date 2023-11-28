#include "MP3DecoderPacket.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>
#if 0
MP3DecoderPacket::MP3DecoderPacket()
{
	giofile=0;
	mpeg=0;
	packet_available=false;
	bytes_available=0;
}

MP3DecoderPacket::~MP3DecoderPacket()
{
	if (giofile)
		giofile->Release();
	giofile=0;

	delete mpeg;
	mpeg=0;
}

int MP3DecoderPacket::Initialize(nx_uri_t filename)
{
	giofile = new (std::nothrow) ReferenceCounted<GioFileRead>;
	if (!giofile)
		return NErr_OutOfMemory;

	int ret = giofile->Open(filename);
	if (ret != NErr_Success)
		return ret;

	mpeg = new CMpgaDecoder;
	if (!mpeg)
		return NErr_OutOfMemory;

	mpeg->Connect(giofile);

	/* now we need to decode until we get one succesful frame */
	ret = DecodeNextFrame(true);
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}

int MP3DecoderPacket::DecodeNextFrame(bool first)
{
	for (;;)
	{
		SSC mpeg_ret = mpeg->DecodeFrame(decode_buffer, sizeof(decode_buffer), &bytes_available);
		if (SSC_SUCCESS(mpeg_ret))
		{
			packet_available=true;
			return NErr_Success;
		}
		else switch(mpeg_ret)
		{
		case SSC_W_MPGA_SYNCNEEDDATA:
			if (!mpeg->IsEof())
				break;

		case SSC_W_MPGA_SYNCEOF:
			return NErr_EndOfFile;

		case SSC_E_MPGA_WRONGLAYER:
			if (first)
			{
				mpeg->m_Mbs.Seek(1);
				break;
			}
			else
			{
				return NErr_Error;
			}

		default:
			return NErr_Error;

		}
	}
}
#endif
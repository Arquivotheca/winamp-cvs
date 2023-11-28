#include "gioultravox.h"
#include "foundation/error.h"

GioUltravox::GioUltravox()
{
	mp3http=0;
}

GioUltravox::~GioUltravox()
{
	if (mp3http)
		mp3http->Release();
	mp3http=0;
}

int GioUltravox::Open(ifc_ultravox_reader *reader)
{
	if (!reader)
		return NErr_NullPointer;

	mp3http = reader;
	mp3http->Retain();
	
	return NErr_Success;
}

SSC GioUltravox::Read(void *pBuffer, int cbToRead, int *pcbRead)
{
	size_t bytesRead=0;
	switch(mp3http->Read(pBuffer, cbToRead, &bytesRead))
	{
	case NErr_Error:
		return SSC_E_IO_READFAILED;
	case NErr_Success:
		*pcbRead = bytesRead;
		return SSC_OK;
	default:
		return SSC_E_IO_GENERIC;
	}
}

bool GioUltravox::IsEof() const
{
	return mp3http->IsClosed() != NErr_Success;

}

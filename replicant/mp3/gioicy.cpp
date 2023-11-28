/*
 *  giofile_jnet.cpp
 *  shoutcast_player
 *
 *  Created by Ben Allison on 12/16/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
 *
 */

#include "gioicy.h"

GioICY::GioICY()
{
	mp3http=0;
}

GioICY::~GioICY()
{
	if (mp3http)
		mp3http->Release();
	mp3http=0;
}

int GioICY::Open(ifc_icy_reader *reader,ifc_http *_http_parent)
{
	if (!reader || !_http_parent)
		return NErr_NullPointer;

	mp3http = reader;
	mp3http->Retain();
	http_parent = _http_parent;

	return NErr_Success;
}

SSC GioICY::Read(void *pBuffer, int cbToRead, int *pcbRead)
{
	/* ICYReader::Read doesn't guarantee a full read */

	*pcbRead=0;
	while (cbToRead)
	{
		size_t bytesRead=0;
		int ret = mp3http->Read(pBuffer, cbToRead, &bytesRead);

		switch(ret)
		{
		case NErr_Success:
			break;
		case NErr_Error:
			return SSC_E_IO_READFAILED;
		default:
			return SSC_E_IO_GENERIC;
		}

		if (bytesRead == 0)
		{
			if (mp3http->IsClosed())
				return SSC_W_MPGA_SYNCEOF;
			else
				return SSC_OK;
		}

		*pcbRead += bytesRead;
		pBuffer = (uint8_t *)pBuffer + bytesRead;
		cbToRead -= bytesRead;
		/*if (cbToRead && http_parent->Sleep(0, ifc_http::WAKE_STOP) == ifc_http::WAKE_STOP)
		{
			return SSC_E_IO_GENERIC;
		}*/
	}
	return SSC_OK;
}

bool GioICY::IsEof() const
{
	return mp3http->IsClosed() != NErr_Success;
}

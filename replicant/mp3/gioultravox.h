#pragma once
#include "nsmp3/giobase.h"
#include "ultravox/ifc_ultravox_reader.h"

class GioUltravox : public CGioBase
{
public:
	GioUltravox();
	~GioUltravox();
	int Open(ifc_ultravox_reader *reader);

	/* CGioBase implementation */
	SSC Read(void *pBuffer, int cbToRead, int *pcbRead);
	bool IsEof() const;
		
private:
	ifc_ultravox_reader *mp3http;
};
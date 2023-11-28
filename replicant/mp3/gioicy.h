/*
 *  giofile_jnet.h
 *  shoutcast_player
 *
 *  Created by Ben Allison on 12/16/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
 *
 */

#pragma once
#include "nsmp3/giobase.h"
#include "icy/ifc_icy_reader.h"
#include "http/ifc_http.h"

class GioICY : public CGioBase
{
public:
	GioICY();
	~GioICY();
	int Open(ifc_icy_reader *reader, ifc_http *http_parent);

	/* CGioBase implementation */
	SSC Read(void *pBuffer, int cbToRead, int *pcbRead);
	bool IsEof() const;
		
private:
	ifc_icy_reader *mp3http;
	ifc_http *http_parent;
};
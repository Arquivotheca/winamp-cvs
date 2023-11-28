//------------------------------------------------------------------------
//
// iTunes XML Library Writer
// Copyright (C) 2003-2013 Nullsoft, Inc.
//
//------------------------------------------------------------------------

#ifndef _ITUNESXMLWRITE_H
#define _ITUNESXMLWRITE_H

class plistKey;
class XMLWrite;
class plistData;
#include <bfc/string/stringw.h>
//------------------------------------------------------------------------

class iTunesXmlWrite {
public:
	iTunesXmlWrite();
	virtual ~iTunesXmlWrite();

	int pickFile(HWND hwndDlg, const wchar_t *title=NULL);
	void saveXml(plistKey *rootkey);

	void writeData(XMLWrite *writer, plistData *data);

private:
	StringW file;
};

#endif

//------------------------------------------------------------------------
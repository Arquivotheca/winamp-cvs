#ifndef NULLSOFT_PLAYLIST_B4SWRITER_H
#define NULLSOFT_PLAYLIST_B4SWRITER_H

#include <stdio.h>
#include "PlaylistWriter.h"

class B4SWriter : public PlaylistWriter
{
public:
	B4SWriter();
	int Open(const wchar_t *filename);
	void Write(const wchar_t *filename);
	void Write(const wchar_t *filename, const wchar_t *title, int length);
	void Close();
private:
	FILE *fp;
};

#endif
#ifndef NULLSOFT_M3U8WRITERH
#define NULLSOFT_M3U8WRITERH

#include <stdio.h>
#include "PlaylistWriter.h"
class M3U8Writer : public PlaylistWriter
{
public:
	M3U8Writer();
	int Open(const wchar_t *filename);
		void Write(const wchar_t *filename);
	void Write(const wchar_t *filename, const wchar_t *title, int length);
	void Close();
private:
	FILE *fp;
};

#endif
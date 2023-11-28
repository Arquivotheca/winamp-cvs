#ifndef NULLSOFT_M3UWRITERH
#define NULLSOFT_M3UWRITERH

#include <stdio.h>
#include "PlaylistWriter.h"
class M3UWriter : public PlaylistWriter
{
public:
	M3UWriter();
	int Open(const wchar_t *filename);
	void Write(const wchar_t *filename);
	void Write(const wchar_t *filename, const wchar_t *title, int length);
	void Close();
private:
	FILE *fp;
};

#endif
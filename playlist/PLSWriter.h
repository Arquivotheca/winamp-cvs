#ifndef NULLSOFT_PLSWRITERH
#define NULLSOFT_PLSWRITERH

#include <stdio.h>
#include "PlaylistWriter.h"
class PLSWriter : public PlaylistWriter
{
public:
	PLSWriter();
	int Open(const wchar_t *filename);
	void Write(const wchar_t *filename);
	void Write(const wchar_t *filename, const wchar_t *title, int length);
	void Close();
private:
	size_t numEntries;
	FILE *fp;
};

#endif
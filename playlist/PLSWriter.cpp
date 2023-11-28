#include "PLSWriter.h"
#include "../nu/AutoChar.h"

PLSWriter::PLSWriter() : fp(0), numEntries(0)
{
}

int PLSWriter::Open(const wchar_t *filename)
{
  fp = _wfopen(filename, L"wt");
  if (!fp) 
		return 0;
  
	fprintf(fp, "[playlist]\r\n");

	return 1;
}
void PLSWriter::Write(const wchar_t *filename)
{
	fprintf(fp, "File%d=%s\r\n", ++numEntries, (const char *)AutoChar(filename));
}

void PLSWriter::Write(const wchar_t *filename, const wchar_t *title, int length)
{
	Write(filename);
		fprintf(fp, "Title%d=%s\r\n", numEntries, (const char *)AutoChar(title));
				fprintf(fp, "Length%d=%d\r\n", numEntries, length);
	
}

void PLSWriter::Close()
{
	fprintf(fp, "NumberOfEntries=%d\r\n", numEntries);
	fprintf(fp, "Version=2\r\n");
	fclose(fp);
}
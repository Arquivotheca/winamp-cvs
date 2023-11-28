#include "M3UWriter.h"
#include <shlwapi.h>

static void MakeRelativePathName(const char *filename, char *outFile, const char *path)
{
	char outPath[MAX_PATH];

	int common = PathCommonPrefixA(path, filename, outPath);
	if (common && common == lstrlenA(path))
	{
		PathAddBackslashA(outPath);
		const char *p = filename + lstrlenA(outPath);
		lstrcpyA(outFile, p);
	}
	else if (!PathIsUNCA(filename) && PathIsSameRootA(filename, path))
	{
		lstrcpyA(outFile, filename+2);
	}
}

M3UWriter::M3UWriter() : extended(0), fp(0)
{
}

int M3UWriter::Open(char *filename, int extendedMode)
{
  fp = fopen(filename,"wt");
  if (!fp) 
		return 0;
  
	if (extendedMode) 
		fprintf(fp,"#EXTM3U\n");

	extended=extendedMode;

	lstrcpynA(basePath, filename, MAX_PATH);
	PathRemoveFileSpecA(basePath);
	return 1;
}

int M3UWriter::Open(FILE *_fp, char *filename, int extendedMode)
{
	fp = _fp;
  if (!fp) 
		return 0;
  
	if (extendedMode) 
		fprintf(fp,"#EXTM3U\n");

	extended=extendedMode;

	lstrcpynA(basePath, filename, MAX_PATH);
	PathRemoveFileSpecA(basePath);
	return 1;
}

void M3UWriter::SetFilename(char *filename)
{
	char temp[MAX_PATH];
	MakeRelativePathName(filename, temp, basePath);
	fprintf(fp,"%s\n",temp);
}

void M3UWriter::SetExtended(char *filename, char *title, int length)
{
	if (!extended)
		SetFilename(filename);
	else
	{
		char temp[MAX_PATH];
		MakeRelativePathName(filename, temp, basePath);
		fprintf(fp,"#EXTINF:%d,%s\n%s\n",length,title,temp);
	}
}

void M3UWriter::Close()
{
	fclose(fp);
}
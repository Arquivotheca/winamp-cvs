#ifndef _PP_HACKS_H_
#define _PP_HACKS_H_

#include <windows.h>
#include <stdio.h>

#ifndef _DEBUG
// release optimizations
// /Og (global optimizations), /Os (favor small code), /Oy (no frame pointers)
//#pragma optimize("gsy",on)
#pragma comment(linker,"/RELEASE")
// set the 512-byte alignment (only in VC6+)
#if _MSC_VER >= 1200
#pragma comment(linker,"/opt:nowin98")
#endif
#endif

#define sprintf wsprintf
#define vsprintf wvsprintf

#if 1

FILE * __cdecl wfopen(const char * path, const char *mode);
size_t __cdecl wfread(void * buf, size_t s1, size_t s2, FILE * f);
int __cdecl wfseek(FILE *f, long s, int t);
long __cdecl wftell(FILE * f);
size_t __cdecl wfwrite(const void * buf, size_t s1, size_t s2, FILE * f);
int _cdecl wfclose(FILE* f);
int _cdecl wfflush(FILE* f);
FILE* _cdecl wtmpfile();
int _cdecl wfgetc( FILE *stream );
int _cdecl wfeof(FILE * stream);
int _cdecl wfputc( int c, FILE *stream );

#else

#define wfopen fopen
#define wfread fread
#define wfseek fseek
#define wftell ftell
#define wfwrite fwrite
#define wfclose fclose
#define wfflush fflush
#define wtmpfile tmpfile
#define wfgetc fgetc
#define wfeof feof
#define wfputc fputc

#endif

#endif
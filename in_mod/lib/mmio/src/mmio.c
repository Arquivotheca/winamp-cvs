/*
 
 Mikmod Portable System Management Facilities (the MMIO)

  By Jake Stine of Divine Entertainment (1996-2000) and
     Jean-Paul Mikkers (1993-1996).

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 --------------------------------------------------
 module: mmio.c

 Miscellaneous portable I/O routines.. used to solve some portability
 issues (like big/little endian machines and word alignment in structures).

 Notes:
  Expanded to allow for streaming from a memory block as well as from
  a file.  Also, set up in a manner which allows easy use of packfiles
  without having to overload any functions (faster and easier!).

 Portability:
  All systems - all compilers

 -----------------------------------
 The way this module works - By Jake Stine [Air Richter]


 - _mm_read_I_UWORD and _mm_read_M_UWORD have distinct differences:
   the first is for reading data written by a little endian (intel) machine,
   and the second is for reading big endian (Mac, RISC, Alpha) machine data.

 - _mm_write functions work the same as the _mm_read functions.

 - _mm_read_string is for reading binary strings.  It is basically the same
   as an fread of bytes.
*/                                                                                     

#include "mmio.h"
#include <string.h>

#ifndef COPY_BUFSIZE
#define COPY_BUFSIZE  1024
#endif

static UBYTE  _mm_cpybuf[COPY_BUFSIZE];


// =====================================================================================
static const MMSTREAM_CALLBACK callback_std =
// =====================================================================================
{
	fread,
	fwrite,
	fgetc,
	fputc,
	fseek,
	ftell,
	feof,
	fclose
};


// =====================================================================================
    MMSTREAM *_mmstream_createfp(FILE *fp, int iobase)
// =====================================================================================
// Creates an MMSTREAM structure from the given file pointer and seekposition
{
    MMSTREAM     *mmf;

    mmf = (MMSTREAM *)MikMod_malloc(NULL, sizeof(MMSTREAM));

    mmf->fp      = fp;
    mmf->iobase  = iobase;
    mmf->dp      = NULL;
    mmf->seekpos = 0;

	mmf->cb      = callback_std;

    return mmf;
}


// =====================================================================================
    MMSTREAM *_mmstream_createmem(void *data, int iobase)
// =====================================================================================
// Creates an MMSTREAM structure from the given memory pointer and seekposition
{
    MMSTREAM *mmf;

    mmf = (MMSTREAM *)MikMod_malloc(NULL, sizeof(MMSTREAM));

    mmf->dp        = (UBYTE *)data;
    mmf->iobase    = iobase;
    mmf->fp        = NULL;
    mmf->seekpos   = 0;

	mmf->cb        = callback_std;

    return mmf;
}


MMSTREAM *_mmstream_createfp_callback(FILE *fp, int iobase,const MMSTREAM_CALLBACK * cb)
{
    MMSTREAM     *mmf;

    mmf = (MMSTREAM *)MikMod_malloc(NULL, sizeof(MMSTREAM));

    mmf->fp      = fp;
    mmf->iobase  = iobase;
    mmf->dp      = NULL;
    mmf->seekpos = 0;

	mmf->cb		 = *cb;

	return mmf;
}

// =====================================================================================
	void _mmstream_setapi(MMSTREAM *mmfp,const MMSTREAM_CALLBACK * cb)
// =====================================================================================
{
	if (mmfp) mmfp->cb = *cb;	
}

// =====================================================================================
    void _mmstream_delete(MMSTREAM *mmf)
// =====================================================================================
{
    MikMod_free(NULL, mmf);
}


// =====================================================================================
    CHAR *StringRead(MMSTREAM *fp)
// =====================================================================================
// Reads strings written out by StringWrite above:  a UWORD length followed by length 
//characters.  A NULL is added to the string after loading.
{
    CHAR  *s;
    UWORD len;

    len = _mm_read_I_UWORD(fp);
    if(len==0)
    {   s = MikMod_calloc(NULL, 16, sizeof(CHAR));
    } else
    {   if((s = (CHAR *)MikMod_malloc(NULL, len+1)) == NULL) return NULL;
        _mm_read_UBYTES((UBYTE *)s,len,fp);
        s[len] = 0;
    }

    return s;
}


// =====================================================================================
    MMSTREAM *_mm_fopen(const CHAR *fname, const CHAR *attrib)
// =====================================================================================
{
    FILE     *fp;
    MMSTREAM *mfp;

    if((fp= fopen(fname,attrib)) == NULL)
    {   //CHAR   sbuf[256];
        // this should check attributes and build a more appropriate error!
        
        //sprintf(sbuf,"Error opening file: %s",fname);
        //_mmerr_set(MMERR_OPENING_FILE, sbuf);
        _mmlogd2("Error opening file: %s > %s",fname, _sys_errlist[errno]);
        return NULL;
    }

    mfp           = MikMod_calloc(NULL, 1,sizeof(MMSTREAM));
    mfp->fp       = fp;
    mfp->iobase   = 0;
    mfp->dp       = NULL;

	mfp->cb       = callback_std;

    return mfp;
}


// =====================================================================================
    void _mm_fclose(MMSTREAM *mmfile)
// =====================================================================================
{
    if(mmfile)
    {   if(mmfile->fp) mmfile->cb.p_fclose(mmfile->fp);
        MikMod_free(NULL, mmfile);
    }
}


// =====================================================================================
    int _mm_fseek(MMSTREAM *stream, long offset, int whence)
// =====================================================================================
{
    if(!stream) return 0;

    if(stream->fp)
    {   // file mode...
        return stream->cb.p_fseek(stream->fp,(whence==SEEK_SET) ? offset+stream->iobase : offset, whence);
    } else
    {   long   tpos;
        switch(whence)
        {   case SEEK_SET: tpos = offset;                   break;
            case SEEK_CUR: tpos = stream->seekpos + offset; break;
            case SEEK_END: /*tpos = stream->length + offset;*/  break; // not supported!
        }
        if((tpos < 0) /*|| (stream->length && (tpos > stream->length))*/) return 1; // seek failed
        stream->seekpos = tpos;
    }

    return 0;
}


// =====================================================================================
    long _mm_ftell(MMSTREAM *stream)
// =====================================================================================
{
   if(!stream) return 0;
   return (stream->fp) ? (stream->cb.p_ftell(stream->fp) - stream->iobase) : stream->seekpos;
}

// =====================================================================================
    long _mm_fsize(MMSTREAM *stream)
// =====================================================================================
{
    ULONG pos = _mm_ftell(stream), size;
    // get file size
    _mm_fseek(stream, 0, SEEK_END);
    size = _mm_ftell(stream);
    _mm_fseek(stream, pos, SEEK_SET);

   return size;
}

// =====================================================================================
    BOOL _mm_feof(MMSTREAM *stream)
// =====================================================================================
{
    if(!stream) return 1;
    if (stream->fp) return (stream->cb.p_feof)(stream->fp);

    return 0;
}


// =====================================================================================
    BOOL _mm_fexist(CHAR *fname)
// =====================================================================================
{
   FILE *fp;
   
   if((fp=fopen(fname,"r")) == NULL) return 0;
   fclose(fp);

   return 1;
}



// =====================================================================================
//                              THE INPUT (READ) SECTION
// =====================================================================================

#define fileread_SBYTE(x)         (SBYTE)x->cb.p_fgetc(x->fp)
#define fileread_UBYTE(x)         (UBYTE)x->cb.p_fgetc(x->fp)
#define dataread_SBYTE(y)         y->dp[y->seekpos++]
#define dataread_UBYTE(y)         y->dp[y->seekpos++]

#define fileread_I_SWORD(fp) ((SWORD)(fileread_UBYTE(fp) | (fileread_UBYTE(fp)<<8)))
#define fileread_I_UWORD(fp) ((UWORD)(fileread_UBYTE(fp) | (fileread_UBYTE(fp)<<8)))
#define dataread_I_SWORD(fp) ((SWORD)(dataread_UBYTE(fp) | (dataread_UBYTE(fp)<<8)))
#define dataread_I_UWORD(fp) ((UWORD)(dataread_UBYTE(fp) | (dataread_UBYTE(fp)<<8)))
#define fileread_I_SLONG(fp) ((SLONG)(fileread_I_UWORD(fp) | (fileread_I_UWORD(fp)<<16)))
#define fileread_I_ULONG(fp) ((ULONG)(fileread_I_UWORD(fp) | (fileread_I_UWORD(fp)<<16)))
#define dataread_I_SLONG(fp) ((SLONG)(dataread_I_UWORD(fp) | (dataread_I_UWORD(fp)<<16)))
#define dataread_I_ULONG(fp) ((ULONG)(dataread_I_UWORD(fp) | (dataread_I_UWORD(fp)<<16)))
#define fileread_M_SWORD(fp) ((SWORD)((fileread_UBYTE(fp)<<8) | fileread_UBYTE(fp)))
#define fileread_M_UWORD(fp) ((UWORD)((fileread_UBYTE(fp)<<8) | fileread_UBYTE(fp)))
#define dataread_M_SWORD(fp) ((SWORD)((dataread_UBYTE(fp)<<8) | dataread_UBYTE(fp)))
#define dataread_M_UWORD(fp) ((UWORD)((dataread_UBYTE(fp)<<8) | dataread_UBYTE(fp)))
#define fileread_M_SLONG(fp) ((SLONG)((fileread_M_UWORD(fp)<<16) | fileread_M_UWORD(fp)))
#define fileread_M_ULONG(fp) ((ULONG)((fileread_M_UWORD(fp)<<16) | fileread_M_UWORD(fp)))
#define dataread_M_SLONG(fp) ((SLONG)((dataread_M_UWORD(fp)<<16) | dataread_M_UWORD(fp)))
#define dataread_M_ULONG(fp) ((ULONG)((dataread_M_UWORD(fp)<<16) | dataread_M_UWORD(fp)))


// ============
//   UNSIGNED
// ============

UBYTE _mm_read_UBYTE(MMSTREAM *fp)
{
    return((fp->fp) ? fileread_UBYTE(fp) : dataread_UBYTE(fp));
}

SBYTE _mm_read_SBYTE(MMSTREAM *fp)
{
    return((fp->fp) ? fileread_SBYTE(fp) : dataread_SBYTE(fp));
}


#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

UWORD _mm_read_I_UWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_UWORD(fp) : dataread_I_UWORD(fp);
}

UWORD _mm_read_M_UWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_UWORD(fp) : dataread_M_UWORD(fp);
}

ULONG _mm_read_I_ULONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_ULONG(fp) : dataread_I_ULONG(fp);
}

ULONG _mm_read_M_ULONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_ULONG(fp) : dataread_M_ULONG(fp);
}

SWORD _mm_read_M_SWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_SWORD(fp) : dataread_M_SWORD(fp);
}

SWORD _mm_read_I_SWORD(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_SWORD(fp) : dataread_I_SWORD(fp);
}

SLONG _mm_read_M_SLONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_M_SLONG(fp) : dataread_M_SLONG(fp);
}

SLONG _mm_read_I_SLONG(MMSTREAM *fp)
{
    return (fp->fp) ? fileread_I_SLONG(fp) : dataread_I_SLONG(fp);
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif

#define fileread_SBYTES(x,y,z)  z->cb.p_fread((void *)x,1,y,z->fp)
#define fileread_UBYTES(x,y,z)  z->cb.p_fread((void *)x,1,y,z->fp)
#define dataread_SBYTES(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y)
#define dataread_UBYTES(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y)

#ifdef MM_BIG_ENDIAN
#define fileread_M_SWORDS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(SWORD),z->fp)
#define fileread_M_UWORDS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(UWORD),z->fp)
#define fileread_M_SLONGS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(SLONG),z->fp)
#define fileread_M_ULONGS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(ULONG),z->fp)
#define dataread_M_SWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SWORD))
#define dataread_M_UWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(UWORD))
#define dataread_M_SLONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SLONG))
#define dataread_M_ULONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(ULONG))
#else
#define fileread_I_SWORDS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(SWORD),z->fp)
#define fileread_I_UWORDS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(UWORD),z->fp)
#define fileread_I_SLONGS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(SLONG),z->fp)
#define fileread_I_ULONGS(x,y,z)  z->cb.p_fread((void *)x,1,y*sizeof(ULONG),z->fp)
#define dataread_I_SWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SWORD))
#define dataread_I_UWORDS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(UWORD))
#define dataread_I_SLONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(SLONG))
#define dataread_I_ULONGS(x,y,z)  memcpy((void *)x,&z->dp[z->seekpos],y*sizeof(ULONG))
#endif


#define DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN(type_name, type)    \
int                                                              \
_mm_read_##type_name##S (type *buffer, int number, MMSTREAM *fp) \
{                                                                \
    if(fp->fp)                                                   \
    {   while(number>0)                                          \
        {   *buffer = fileread_##type_name(fp);                  \
            number--;  buffer++;                                 \
        }                                                        \
    } else                                                       \
    {   while(number>0)                                          \
        {   *buffer = dataread_##type_name(fp);                  \
            number--;  buffer++;                                 \
        }                                                        \
    }                                                            \
    return !_mm_feof(fp);                                        \
}


#define DEFINE_MULTIPLE_READ_FUNCTION_NORM(type_name, type)      \
int                                                              \
_mm_read_##type_name##S (type *buffer, int number, MMSTREAM *fp) \
{                                                                \
    if(fp->fp)                                                   \
    {   fileread_##type_name##S(buffer,number,fp);               \
    } else                                                       \
    {   dataread_##type_name##S(buffer,number,fp);               \
        fp->seekpos += number;                                   \
    }                                                            \
    return !_mm_feof(fp);                                        \
}

DEFINE_MULTIPLE_READ_FUNCTION_NORM   (SBYTE, SBYTE)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (UBYTE, UBYTE)

#ifdef _MSC_VER
#pragma optimize( "g", off )
#endif

#ifdef MM_BIG_ENDIAN
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (I_ULONG, ULONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (M_ULONG, ULONG)
#else
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_ENDIAN (M_ULONG, ULONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_SWORD, SWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_SLONG, SLONG)
DEFINE_MULTIPLE_READ_FUNCTION_NORM   (I_ULONG, ULONG)
#endif

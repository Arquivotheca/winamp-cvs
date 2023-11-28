#ifndef _MMIO_H_
#define _MMIO_H_

#ifndef MMEXPORT
#ifdef MM_DLL_EXPORT
#define MMEXPORT  extern __declspec( dllexport )
#elif MIKMOD_STATIC
#define MMEXPORT  extern __declspec( dllimport )
#else
#define MMEXPORT  extern
#endif
#endif


// #ifndef __VECTORC
#define restrict
#define alignvalue(a)
#define __hint__(a)
#define div12 /
#define sqrt12 sqrt

#define align(a) __declspec (align (a))

#define _mm_exchange(x,y)  \
{ \
    uint  __tmp = x; \
    x = y; \
    y = __tmp; \
} \


#include <stdio.h>
#include <stdlib.h>
#include "mmtypes.h"

#ifdef _MSC_VER
#include <assert.h>
#endif

// CPU mode options.  These will force the use of the selected mode, whether that
// cpu is available or not, so make sure you check first!  Speed enhancments are
// present in sample loading and software mixing.

#define CPU_NONE              0
#define CPU_MMX         (1ul<<0)
#define CPU_3DNOW       (1ul<<1)
#define CPU_SIMD        (1ul<<2)
#define CPU_AUTODETECT    (0xff)

MMEXPORT uint _mm_cpudetect(void);

// Miscellaneous Macros 
// --------------------
// I should probably put these somewhere else.

// boundschecker macro.  I do a lot of bounds checking ;)

#define _mm_boundscheck(v,a,b)  ((v > b) ? b : ((v < a) ? a : v))
#define _mm_2hex(a)             (((((a)&0xf0)>>4)*10) + ((a)&0xf))

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif


// 64 bit integer macros.  These allow me to get pointers to the high
// and low portions of a 64 bit integer.  Used mostly by the Mikmod
// software mixer - but could be useful for other things.

#ifdef MM_BIG_ENDIAN

#define _mm_HI_SLONG(x) ((SLONG *)&x)
#define _mm_LO_SLONG(x) ((SLONG *)&x + 1)
#define _mm_HI_ULONG(x) ((ULONG *)&x)
#define _mm_LO_ULONG(x) ((ULONG *)&x + 1)

#else

#define _mm_HI_SLONG(x) ((SLONG *)&x + 1)
#define _mm_LO_SLONG(x) ((SLONG *)&x)
#define _mm_HI_ULONG(x) ((ULONG *)&x + 1)
#define _mm_LO_ULONG(x) ((ULONG *)&x)

#endif


#ifdef __cplusplus
extern "C" {
#endif


// ===============================================
//  Error handling and logging stuffs - MMERROR.C
// ===============================================

// Generic Error Codes
// -------------------
// Notes:
//  - These should always be relatively generalized. (ie, you probably
//    shouldn't have to add any new errors).  More specific error infor-
//    mation is provided via logging and error strings.
//

enum
{   MMERR_NONE = 0,
    MMERR_INVALID_PARAMS,
    MMERR_OPENING_FILE,
    MMERR_UNSUPPORTED_FILE,
    MMERR_OUT_OF_MEMORY,
    MMERR_END_OF_FILE,
    MMERR_DISK_FULL,
    MMERR_DETECTING_DEVICE,
    MMERR_INITIALIZING_DEVICE,
    MMERR_FATAL
};

// Error / Logging prototypes
// --------------------------

#ifdef _DEBUG
#define USE_LOG_SHIZ
#endif

#define _mmlogd         _mmlog
#define _mmlogd1        _mmlog
#define _mmlogd2        _mmlog
#define _mmlogd3        _mmlog

#ifndef USE_LOG_SHIZ

#pragma warning(disable:4002)
#define _mmlog(x)
#define _mmlogv(x)
#define _mmlog_init(x,yf)
#define _mmlog_exit()

#else

MMEXPORT void   _mmlog_init(void (__cdecl *log)(const CHAR *fmt, ... ), void (__cdecl *logv)(const CHAR *fmt, ... ));
MMEXPORT void   _mmlog_exit(void);
MMEXPORT void   (__cdecl *_mmlog)(const CHAR *fmt, ... );
MMEXPORT void   (__cdecl *_mmlogv)(const CHAR *fmt, ... );

#endif

MMEXPORT void   _mmerr_sethandler(void (*func)(int, const CHAR *));
MMEXPORT void   _mmerr_set(int err_is_human, const CHAR *human_is_err);
MMEXPORT void   _mmerr_setsub(int err_is_human, const CHAR *human_is_err);
MMEXPORT int    _mmerr_getinteger(void);
MMEXPORT CHAR  *_mmerr_getstring(void);
MMEXPORT void   _mmerr_handler(void);


// =======================================
//  Input / Output and Streaming - MMIO.C
// =======================================

// =====================================================================================
	typedef struct MMSTREAM_CALLBACK
// =====================================================================================
{
    int (__cdecl *p_fread)(void *buffer, size_t size, size_t count, FILE *stream);
    int (__cdecl *p_fwrite)(const void *buffer, size_t size, size_t count, FILE *stream);

    int (__cdecl *p_fgetc)(FILE *stream);
    int (__cdecl *p_fputc)(int c, FILE *stream);

    int (__cdecl *p_fseek)(FILE *stream, long offset, int origin);
    int (__cdecl *p_ftell)(FILE *stream);

    int (__cdecl *p_feof)(FILE *stream);

	int (__cdecl *p_fclose)(FILE * stream);
} MMSTREAM_CALLBACK;

// =====================================================================================
    typedef struct MMSTREAM
// =====================================================================================
// Mikmod customized module file pointer structure.
// Contains various data that may be required for a module.
{   
    FILE   *fp;      // File pointer
    UBYTE  *dp;

    long   iobase;       // base seek position within the file
    long   seekpos;      // used in data(mem) streaming mode only.

	MMSTREAM_CALLBACK cb;

} MMSTREAM;

MMEXPORT MMSTREAM *_mmstream_createfp(FILE *fp, int iobase);
MMEXPORT MMSTREAM *_mmstream_createfp_callback(FILE *fp, int iobase,const MMSTREAM_CALLBACK * cb);
MMEXPORT MMSTREAM *_mmstream_createmem(void *data, int iobase);
MMEXPORT void      _mmstream_delete(MMSTREAM *mmf);
MMEXPORT void      _mmstream_setapi(MMSTREAM *mmfp,const MMSTREAM_CALLBACK * cb);

// ===================================================
//  Memory allocation with error handling - MMALLOC.C
// ===================================================

// Block Types In-Depth
// --------------------
// By default, a global blocktype is created which is the default block used when the
// user called MikMod_malloc
//

typedef struct MM_ALLOC MM_ALLOC, *mm_alloc_t;


MMEXPORT MM_ALLOC *_mmalloc_create(const char *name, MM_ALLOC *parent);
MMEXPORT void      _mmalloc_close(MM_ALLOC *type);
MMEXPORT void      _mmalloc_setshutdown(MM_ALLOC *type, void (*shutdown)(void *block), void *block);

MMEXPORT void     *MikMod_malloc(MM_ALLOC *type, size_t size);
MMEXPORT void     *MikMod_calloc(MM_ALLOC *type, size_t nitems, size_t size);
MMEXPORT void     *_mm_realloc(MM_ALLOC *type, void *old_blk, size_t size);
MMEXPORT void      MikMod_freex(MM_ALLOC *type, void *d);

#define MikMod_free(a, b) MikMod_freex(a, &b)


#define _MM_ALLOCATE(var, TYPE, parent)                         \
{                                                               \
    MM_ALLOC *allochandle;                                      \
                                                                \
    allochandle = _mmalloc_create(#TYPE, parent);               \
    var = (TYPE*)MikMod_calloc(allochandle, 1, sizeof(TYPE));      \
    if (var == NULL) return NULL;                               \
    var->allochandle = allochandle;                             \
}

#define _MM_ALLOCATE_NAME(var, TYPE, parent, NAME)              \
{                                                               \
    MM_ALLOC *allochandle;                                      \
                                                                \
    allochandle = _mmalloc_create(NAME, parent);               \
    var = (TYPE*)MikMod_calloc(allochandle, 1, sizeof(TYPE));      \
    if (var == NULL) return NULL;                               \
    var->allochandle = allochandle;                             \
}

MMEXPORT void _mm_RegisterErrorHandler(void (*proc)(int, CHAR *));
MMEXPORT BOOL _mm_fexist(CHAR *fname);

MMEXPORT CHAR *StringRead(MMSTREAM *fp);


//  MikMod/DivEnt style file input / output -
//    Solves several portability issues.
//    Notably little vs. big endian machine complications.

#define _mm_rewind(x) _mm_fseek(x,0,SEEK_SET)

MMEXPORT int      _mm_fseek(MMSTREAM *stream, long offset, int whence);
MMEXPORT long     _mm_ftell(MMSTREAM *stream);
MMEXPORT long     _mm_fsize(MMSTREAM *stream);
MMEXPORT BOOL     _mm_feof(MMSTREAM *stream);
MMEXPORT MMSTREAM *_mm_fopen(const CHAR *fname, const CHAR *attrib);
MMEXPORT void     _mm_fclose(MMSTREAM *mmfile);


#define _mm_read_string(b,n,f)  _mm_read_SBYTES(b, n, f)

MMEXPORT long     _mm_flength(FILE *stream);
MMEXPORT BOOL     _mm_copyfile(FILE *fpi, FILE *fpo, uint len);


MMEXPORT SBYTE _mm_read_SBYTE (MMSTREAM *fp);
MMEXPORT UBYTE _mm_read_UBYTE (MMSTREAM *fp);

MMEXPORT SWORD _mm_read_M_SWORD (MMSTREAM *fp);
MMEXPORT SWORD _mm_read_I_SWORD (MMSTREAM *fp);

MMEXPORT UWORD _mm_read_M_UWORD (MMSTREAM *fp);
MMEXPORT UWORD _mm_read_I_UWORD (MMSTREAM *fp);

MMEXPORT SLONG _mm_read_M_SLONG (MMSTREAM *fp);
MMEXPORT SLONG _mm_read_I_SLONG (MMSTREAM *fp);

MMEXPORT ULONG _mm_read_M_ULONG (MMSTREAM *fp);
MMEXPORT ULONG _mm_read_I_ULONG (MMSTREAM *fp);


MMEXPORT int _mm_read_SBYTES    (SBYTE *buffer, int number, MMSTREAM *fp);
MMEXPORT int _mm_read_UBYTES    (UBYTE *buffer, int number, MMSTREAM *fp);

MMEXPORT int _mm_read_M_SWORDS  (SWORD *buffer, int number, MMSTREAM *fp);
MMEXPORT int _mm_read_I_SWORDS  (SWORD *buffer, int number, MMSTREAM *fp);

MMEXPORT int _mm_read_M_UWORDS  (UWORD *buffer, int number, MMSTREAM *fp);
MMEXPORT int _mm_read_I_UWORDS  (UWORD *buffer, int number, MMSTREAM *fp);

MMEXPORT int _mm_read_M_SLONGS  (SLONG *buffer, int number, MMSTREAM *fp);
MMEXPORT int _mm_read_I_SLONGS  (SLONG *buffer, int number, MMSTREAM *fp);

MMEXPORT int _mm_read_M_ULONGS  (ULONG *buffer, int number, MMSTREAM *fp);
MMEXPORT int _mm_read_I_ULONGS  (ULONG *buffer, int number, MMSTREAM *fp);



#ifdef __WATCOMC__
#pragma aux _mm_fseek      parm nomemory modify nomemory
#pragma aux _mm_ftell      parm nomemory modify nomemory
#pragma aux _mm_flength    parm nomemory modify nomemory
#pragma aux _mm_fopen      parm nomemory modify nomemory
#pragma aux _mm_fputs      parm nomemory modify nomemory
#pragma aux _mm_copyfile   parm nomemory modify nomemory
#pragma aux _mm_iobase_get parm nomemory modify nomemory
#pragma aux _mm_iobase_set parm nomemory modify nomemory
#pragma aux _mm_iobase_setcur parm nomemory modify nomemory
#pragma aux _mm_iobase_revert parm nomemory modify nomemory
#pragma aux _mm_read_string   parm nomemory modify nomemory

#pragma aux _mm_read_M_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_M_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_I_UWORD parm nomemory modify nomemory; 
#pragma aux _mm_read_M_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SLONG parm nomemory modify nomemory; 
#pragma aux _mm_read_M_ULONG parm nomemory modify nomemory; 
#pragma aux _mm_read_I_ULONG parm nomemory modify nomemory; 

#pragma aux _mm_read_M_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_UWORDS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_SLONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_M_ULONGS parm nomemory modify nomemory; 
#pragma aux _mm_read_I_ULONGS parm nomemory modify nomemory; 

#endif


MMEXPORT CHAR *_mm_strdup(MM_ALLOC *allochandle, const CHAR *src);

#ifdef __cplusplus
};
#endif

#endif

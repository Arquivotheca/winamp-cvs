/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_MOD.C

  Generic MOD loader (Protracker, StarTracker, FastTracker, etc).  Personally
  my favorite loader, because it loads most of my favorite tunes! :)

 Portability:
  All systems - all compilers (hopefully)

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "..\..\..\..\mikamp\resource.h"


/*************************************************************************
*************************************************************************/

static const CHAR protracker[]   = "Protracker";
static const CHAR sndtracker[]   = "Soundtracker 31";
static const CHAR startrekker[]  = "Startrekker";
static const CHAR fasttracker[]  = "FastTracker";
static const CHAR oktalyzer[]    = "Oktalyzer";
static const CHAR taketracker[]  = "TakeTracker";
static const CHAR noisetracker[]  = "Noisetracker";

#define FLAG_PAIRED     1

static const struct MODTYPE
{   CHAR        id[5];
    UBYTE       channels;
    UBYTE       flags;
    const CHAR  *name;
}
modtypes[] =
{  { "M.K.", 4,  0, protracker   },   // protracker 4 channel
   { "M!K!", 4,  0, protracker   },   // protracker 4 channel
   { "M&K!", 4,  0, noisetracker },   // nousetracker 4 channel
   { "N.T.", 4,  0, noisetracker },   // nousetracker 4 channel
   { "NORI", 4,  0, noisetracker },   // nousetracker 4 channel
   { "NSMS", 4,  0, sndtracker   },   // soundtracker31 4 channel
   { "FLT4", 4,  0, startrekker  },   // startracker 4 channel
   { "EXO4", 4,  0, startrekker  },   // startracker 4 channel
   { "2CHN", 2,  0, fasttracker  },   // fasttracker 2 channel
   { "4CHN", 4,  0, fasttracker  },   // fasttracker 4 channel
   { "6CHN", 6,  0, fasttracker  },   // fasttracker 6 channel
   { "8CHN", 8,  0, fasttracker  },   // fasttracker 8 channel
   { "10CH", 10, 0, fasttracker  },   // fasttracker 10 channel
   { "12CH", 12, 0, fasttracker  },   // fasttracker 12 channel
   { "14CH", 14, 0, fasttracker  },   // fasttracker 14 channel
   { "16CH", 16, 0, fasttracker  },   // fasttracker 16 channel
   { "18CH", 18, 0, fasttracker  },   // fasttracker 18 channel
   { "20CH", 20, 0, fasttracker  },   // fasttracker 20 channel
   { "22CH", 22, 0, fasttracker  },   // fasttracker 22 channel
   { "24CH", 24, 0, fasttracker  },   // fasttracker 24 channel
   { "26CH", 26, 0, fasttracker  },   // fasttracker 26 channel
   { "28CH", 28, 0, fasttracker  },   // fasttracker 28 channel
   { "30CH", 30, 0, fasttracker  },   // fasttracker 30 channel
   { "32CH", 32, 0, fasttracker  },   // fasttracker 32 channel
   { "CD81", 8,  0, oktalyzer    },   // atari oktalyzer 8 channel
   { "OCTA", 8,  0, oktalyzer    },   // atari oktalyzer 8 channel
   { "16CN", 16, 0, taketracker  },   // taketracker 16 channel
   { "32CN", 32, 0, taketracker  },   // taketracker 32 channel
};

#define TYPES_COUNT     (sizeof(modtypes)/sizeof(modtypes[0]))

/*************************************************************************
*************************************************************************/


typedef struct MSAMPINFO                  // sample header as it appears in a module
{   CHAR  samplename[22];
    UWORD length;
    UBYTE finetune;
    UBYTE volume;
    UWORD reppos;
    UWORD replen;
} MSAMPINFO;


typedef struct MODULEHEADER              // verbatim module header
{   CHAR       songname[20];             // the songname..
    MSAMPINFO  samples[31];              // all sampleinfo
    UBYTE      songlength;               // number of patterns used
    UBYTE      magic1;                   // should be 127
    UBYTE      positions[128];           // which pattern to play at pos
    UBYTE      magic2[4];                // string "M.K." or "FLT4" or "FLT8"
} MODULEHEADER;

#define MODULEHEADERSIZE 1084


typedef struct MODNOTE
{   UBYTE a,b,c,d;
} MODNOTE;


// =====================================================================================
static BOOL MOD_Test(MMSTREAM *mmfile)
// =====================================================================================
{
    int   modtype;
    UBYTE id[4];

    _mm_fseek(mmfile,MODULEHEADERSIZE-4,SEEK_SET);
    if(!_mm_read_UBYTES(id,4,mmfile)) return 0;

    // find out which ID string

    for(modtype=0; modtype<TYPES_COUNT; modtype++)
        if(!memcmp(id,modtypes[modtype].id,4)) return 1;

    return 0;
}


// =====================================================================================
static void *MOD_Init(void)
// =====================================================================================
{
    return MikMod_calloc(NULL, 1,sizeof(MODULEHEADER));
}


// =====================================================================================
static void MOD_Cleanup(void *handle)
// =====================================================================================
{
    MikMod_free(NULL, handle);
}


/*
Old (amiga) noteinfo:

 _____byte 1_____   byte2_    _____byte 3_____   byte4_
/                \ /      \  /                \ /      \
0000          0000-00000000  0000          0000-00000000

Upper four    12 bits for    Lower four    Effect command.
bits of sam-  note period.   bits of sam-
ple number.                  ple number.

*/


// =====================================================================================
static void ConvertNote(UTRK_WRITER *ut, MODNOTE *n)
// =====================================================================================
{
    uint   instrument,effect,effdat,note;
    UWORD  period;

    // extract the various information from the 4 bytes that
    // make up a single note

    instrument = (n->a&0x10)|(n->c>>4);
    period     = (((UWORD)n->a&0xf)<<8) | n->b;
    effect     = n->c&0xf;
    effdat     = n->d;

    // Convert the period to a note number

    note = PeriodToNote(period);

    if (instrument) utrk_write_inst(ut, instrument);
    if (note)       utrk_write_note(ut, note);

    // volslide up has priority
	if (effect==0xa && effdat&0xf0)
		effdat &= 0xf0;

    // Convert pattern jump from Dec to Hex
    if (effect == 0xd)
        effdat = _mm_2hex(effdat);

    pt_write_effect(ut, effect,effdat);
}


// =====================================================================================
static BOOL ML_LoadPatterns(MODULEHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
// =====================================================================================
//  Loads all patterns of a modfile and converts them into the
//  3 byte format.
{
    uint     t, s, ch;
    MODNOTE  n;

    if(!AllocTracks(of))   return 0;
    if(!AllocPatterns(of)) return 0;

    for (t=0; t<of->numpat; t++)
    {
        utrk_reset(of->ut);
        for (s=64; s; s--)
        {   
            for (ch=0; ch<of->numchn; ch++)
            {   
                n.a = _mm_read_UBYTE(mmfile);
                n.b = _mm_read_UBYTE(mmfile);
                n.c = _mm_read_UBYTE(mmfile);
                n.d = _mm_read_UBYTE(mmfile);
                utrk_settrack(of->ut, ch);
                ConvertNote(of->ut,&n);
            }
            utrk_newline(of->ut);
        }
        if(!utrk_dup_pattern(of->ut, of)) return 0;
    }

    return 1;
}


// =====================================================================================
static BOOL MOD_Load(MODULEHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
// =====================================================================================
{
    uint        t, modtype;
    UNISAMPLE  *q;
    MSAMPINFO  *s;           // old module sampleinfo

    // try to read module header

    _mm_read_string((CHAR *)mh->songname,20,mmfile);

    for(t=0; t<31; t++)
    {   s = &mh->samples[t];
        _mm_read_string(s->samplename,22,mmfile);
        s->length   =_mm_read_M_UWORD(mmfile);
        s->finetune =_mm_read_UBYTE(mmfile);
        s->volume   =_mm_read_UBYTE(mmfile);
        s->reppos   =_mm_read_M_UWORD(mmfile);
        s->replen   =_mm_read_M_UWORD(mmfile);
    }

    mh->songlength  =_mm_read_UBYTE(mmfile);
    mh->magic1      =_mm_read_UBYTE(mmfile);

    // Fix for FREAKY mods..  Not sure if it works for all of 'em tho.. ?
    if(mh->songlength >= 129)  mh->songlength = mh->magic1;

    _mm_read_UBYTES(mh->positions,128,mmfile);
    _mm_read_UBYTES(mh->magic2,4,mmfile);

    if(_mm_feof(mmfile))
	{
        _mmlog("load_mod> unexpected end of file (in header)");
        return 0;
    }

    // find out which ID string

    for(modtype=0; modtype<TYPES_COUNT; modtype++)
        if(!memcmp(mh->magic2,modtypes[modtype].id,4)) break;

    // set module variables

    of->initspeed = 6;
    of->inittempo = 125;
    of->numchn    = modtypes[modtype].channels;        // get number of channels
    of->modtype   = _mm_strdup(of->allochandle, modtypes[modtype].name);// get ascii type of mod
    of->songname  = DupStrTrim(of->allochandle, mh->songname, 20);
    of->numpos    = mh->songlength;                    // copy the songlength

    if(!AllocPositions(of, of->numpos)) return 0;

    for (t=0; t<of->numpos; t++)
        of->positions[t] = mh->positions[t];

    // Count the number of patterns

    of->numpat = 0;

    for(t=0; t<128; t++)
    {   if(mh->positions[t] > of->numpat)
            of->numpat = mh->positions[t];
    }
    of->numpat++;
    of->numtrk = of->numpat*of->numchn;

    // Finally, init the sampleinfo structures 
    of->numsmp = 31;

    if(!AllocSamples(of, 0))     return 0;

    s = mh->samples;       // init source pointer 
    q = of->samples;
    
    for(t=0; t<of->numsmp; t++)
    {   
        // convert the samplename
        q->samplename = DupStr(of->allochandle, s->samplename, 22);

        // init the sampleinfo variables and
        // convert the size pointers to longword format

        q->speed     = finetune[s->finetune & 0xf];
        q->volume    = s->volume * 2;
        q->loopstart = (ULONG)s->reppos << 1;
        q->loopend   = q->loopstart + ((ULONG)s->replen << 1);
        q->length    = (ULONG)s->length << 1;

        q->format    = SF_SIGNED;

        if(s->replen > 1) q->flags |= SL_LOOP;

        // Enable aggressive declicking for songs that do not loop and that
        // are long enough that they won't be adversely affected.

        if(!(q->flags & (SL_LOOP | SL_SUSTAIN_LOOP)) && (q->length > 5000))
            q->flags |= SL_DECLICK;

        // fix replen if repend > length
        if(q->loopend > q->length) q->loopend = q->length;

        s++;    // point to next source sampleinfo
        q++;
    }

    of->ut = utrk_init(of->numchn, of->allochandle);

    utrk_memory_reset(of->ut);
    utrk_local_memflag(of->ut, PTMEM_PORTAMENTO, UMF_EFFECT);

    if(!ML_LoadPatterns(mh, of, mmfile)) return 0;

    {
        // ADPCM Support Explaination
        // --------------------------
        // The ADPCM headers are attached to the head of each module sample, so we must
        // seek to the head of each sample header and check for the presence of ADPCM.
        // ADPCM rules : 16 + (samplelen / 2) ROUNDED UP 

        long  seekpos = _mm_ftell(mmfile);

        q = of->samples;
        for(t=0; t<of->numsmp; t++, q++)
        {   CHAR   adpcm[8];

            q->seekpos = seekpos;

            _mm_fseek(mmfile, q->seekpos, SEEK_SET);
            _mm_read_UBYTES(adpcm, 5, mmfile);
            if(!memcmp(adpcm, "ADPCM", 5))
            {   q->seekpos += 5;
                seekpos    += ((q->length+1)/2)+16 + 5;
				q->format|=SF_SIGNED|SF_16BITS;
                q->compress = DECOMPRESS_ADPCM;
            } else
                seekpos    += q->length;

        }
    }

    return 1;
}


// =====================================================================================
static CHAR *MOD_LoadTitle(MMSTREAM *mmfile)
// =====================================================================================
{
   CHAR s[20];

   _mm_fseek(mmfile,0,SEEK_SET);
   if(!_mm_read_UBYTES(s,20,mmfile)) return NULL;
   
   return DupStrTrim(NULL, s, 20);
}


MLOADER load_mod =
{
    "MOD",
    IDS_FAMILY_STRING_PROTRACKER,//"Protracker",
    0,                              // default FULL STEREO panning
    NULL,
    MOD_Test,
    MOD_Init,
    MOD_Cleanup,
    MOD_Load,
#ifndef _MM_WINAMP_
    MOD_LoadTitle
#endif
};
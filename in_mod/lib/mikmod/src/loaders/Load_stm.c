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
 Module: LOAD_STM.C

  ScreamTracker 2 (STM) module Loader - Version 1.oOo Release 2 
  A Coding Nightmare by Rao and Air Richter of HaRDCoDE
  You can now play all of those wonderful old C.C. Catch STM's!

 Portability:
  All systems - all compilers (hopefully)

*/

#include <string.h>
#include <ctype.h>
#include "mikmod.h"
#include "itshare.h"
#include "..\..\..\..\mikamp\resource.h"


typedef struct STMNOTE
{   UBYTE note,insvol,volcmd,cmdinf;
} STMNOTE;


// Raw STM sampleinfo struct:

typedef struct STMSAMPLE
{  CHAR  filename[12];   // Can't have long comments - just filename comments :)
   UBYTE unused;         // 0x00
   UBYTE instdisk;       // Instrument disk
   UWORD reserved;       // ISA in memory when in ST 2
   UWORD length;         // Sample length
   UWORD loopbeg;        // Loop start point
   UWORD loopend;        // Loop end point
   UBYTE volume;         // Volume
   UBYTE reserved2;      // More reserved crap
   UWORD c2spd;          // Good old c2spd
   UBYTE reserved3[4];   // Yet more of PSi's reserved crap
   UWORD isa;            // Internal Segment Address ->
                         //    contrary to the tech specs, this is NOT actually
                         //    written to the stm file.
} STMSAMPLE;

// Raw STM header struct:

typedef struct STMHEADER
{  CHAR  songname[20];
   CHAR  trackername[8];   // !SCREAM! for ST 2.xx 
   UBYTE unused;           // 0x1A 
   UBYTE filetype;         // 1=song, 2=module (only 2 is supported, of course) :) 
   UBYTE ver_major;        // Like 2 
   UBYTE ver_minor;        // "ditto" 
   UBYTE inittempo;        // initspeed= stm inittempo>>4 
   UBYTE  numpat;          // number of patterns 
   UBYTE   globalvol;      // <- WoW! a RiGHT TRiANGLE =8*) 
   UBYTE    reserved[13];  // More of PSi's internal crap 
   STMSAMPLE sample[31];   // STM sample data
   UBYTE patorder[128];    // Docs say 64 - actually 128
} STMHEADER;

static const struct
{
    CHAR *sign;
    CHAR *name;
}
stm_id[] =
{
    { "!Scream!", "Screamtracker x.xx" },
    { "BMOD2STM", "BMOD2STM x.xx" },
    { "WUZAMOD!", "Wuzamod x.xx" },
};

#define STMID_COUNT     (sizeof(stm_id)/sizeof(stm_id)[0])

// =====================================================================================
static BOOL STM_Test(MMSTREAM *mmfile)
// =====================================================================================
{
   CHAR  id[8];
   int   i;

   _mm_fseek(mmfile, 20, SEEK_SET);
   _mm_read_UBYTES(id, 8, mmfile);
   if (_mm_read_UBYTE(mmfile)!=0x1A ||
       _mm_read_UBYTE(mmfile)!=2)
       return 0;

   for (i=0; i<STMID_COUNT; i++)
       if (!memcmp(id, stm_id[i].sign, 8))
           return 1;

   return 0;
}


// =====================================================================================
static void *STM_Init(void)
// =====================================================================================
{
    return MikMod_calloc(NULL, 1,sizeof(STMHEADER));
}

// =====================================================================================
static void STM_Cleanup(void *handle)
// =====================================================================================
{
    MikMod_free(NULL, handle);
}


// =====================================================================================
static void STM_ConvertNote(UTRK_WRITER *ut, STMNOTE *n)
// =====================================================================================
{
    uint note, inst, vol, cmd, inf;

    // extract the various information from the 4 bytes that
    //  make up a single note

    note = n->note;
    inst = n->insvol >> 3;
    vol  = (n->insvol&7) + ((n->volcmd&0xF0)>>1);
    cmd  = n->volcmd & 15;
    inf  = n->cmdinf;

    if (inst)       utrk_write_inst(ut, inst);
    if (note < 251) utrk_write_note(ut, (((note>>4)+2)*12)+(note&0xf) + 1); // <- normal note and up the octave by two
    if (vol < 65)   pt_write_effect(ut, 0xc, vol);

    if (cmd == 1)
        pt_write_effect(ut, 0xf, inf>>4);
    else S3MIT_ProcessCmd(ut, NULL, cmd, inf, 3, PTMEM_PORTAMENTO, 0);
}


// =====================================================================================
static BOOL STM_LoadPatterns(UNIMOD *of, MMSTREAM *mmfile)
// =====================================================================================
{
    uint    t, s;
    STMNOTE n;

    if (!AllocTracks(of)) return 0;
    if (!AllocPatterns(of)) return 0;

    // prepare writer
    of->ut = utrk_init(4, of->allochandle);
    utrk_memory_reset(of->ut);
    S3MIT_SetMemDefaults(of);

    // convert patterns
    for (t=0; t<of->numpat; t++)
    {
        utrk_reset(of->ut);

        if (_mm_feof(mmfile))
        {
            _mmlog("load_stm > Failure: Unexpected end of file reading pattern %d",t);
            return 0;
        }

        for (s=0; s<64*4; s++)
        {
            utrk_settrack(of->ut, s & 3);

            n.note = _mm_read_UBYTE(mmfile);
            // decode first byte
            switch (n.note)
            {
            // note off
            case 252:
                pt_write_effect(of->ut, 0xc, 0);
                break;
#ifdef _BLOW_UP
            case 254:
                _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_stm> Unsupported effect.");
                return 0;
#endif
            // nothing, inc.
            case 251:
            case 253:
                break;
            // other bytes follow
            default:
                n.insvol = _mm_read_UBYTE(mmfile);
                n.volcmd = _mm_read_UBYTE(mmfile);
                n.cmdinf = _mm_read_UBYTE(mmfile);
                STM_ConvertNote(of->ut,&n);
            }

            if ((s&3) == 3) utrk_newline(of->ut);
        }
  
        if (!utrk_dup_pattern(of->ut, of)) return 0;
    }

    return 1;
}


// =====================================================================================
static BOOL STM_Load(STMHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
// =====================================================================================
{
    uint       t; 
    ULONG      MikMod_ISA; // We MUST generate our own ISA - NOT stored in the stm
    UNISAMPLE *q;

    // try to read stm header

    _mm_read_string(mh->songname,20,mmfile);
    _mm_read_string(mh->trackername,8,mmfile);
    mh->unused      =_mm_read_UBYTE(mmfile);
    mh->filetype    =_mm_read_UBYTE(mmfile);
    mh->ver_major   =_mm_read_UBYTE(mmfile);
    mh->ver_minor   =_mm_read_UBYTE(mmfile);
    mh->inittempo   =_mm_read_UBYTE(mmfile);
    mh->numpat      =_mm_read_UBYTE(mmfile);
    mh->globalvol   =_mm_read_UBYTE(mmfile);
    _mm_read_UBYTES(mh->reserved,13,mmfile);

#ifdef _BLOW_UP
    if (!mh->inittempo || mh->numpat>99)
        return 0;
#endif

    for(t=0;t<31;t++)
    {   STMSAMPLE *s = &mh->sample[t];  // STM sample data

        _mm_read_string(s->filename,12,mmfile);
        s->unused    =_mm_read_UBYTE(mmfile);
        s->instdisk  =_mm_read_UBYTE(mmfile);
        s->reserved  =_mm_read_I_UWORD(mmfile);
        s->length    =_mm_read_I_UWORD(mmfile);
        s->loopbeg   =_mm_read_I_UWORD(mmfile);
        s->loopend   =_mm_read_I_UWORD(mmfile);
        s->volume    =_mm_read_UBYTE(mmfile);
        s->reserved2 =_mm_read_UBYTE(mmfile);
        s->c2spd     =_mm_read_I_UWORD(mmfile);
        _mm_read_UBYTES(s->reserved3,4,mmfile);
        s->isa       =_mm_read_I_UWORD(mmfile);
    }
    _mm_read_UBYTES(mh->patorder,128,mmfile);

    if(_mm_feof(mmfile))
    {   _mmlog("load_stm > Failure: Unexpected end of file reading module header");
        return 0;
    }

    // set module variables

    for (t=0; t<STMID_COUNT; t++)
        if (!memcmp(mh->trackername, stm_id[t].sign, 8))
        {
            CHAR *ver;

            of->modtype = _mm_strdup(of->allochandle, stm_id[t].name);
            ver = of->modtype + strlen(of->modtype) - 4;
            ver[0] = mh->ver_major + '0';
            ver[2] = (mh->ver_minor)/10 + '0';
            ver[3] = (mh->ver_minor)%10 + '0';
            break;
        }

    of->songname  = DupStrTrim(of->allochandle, mh->songname, 20);
    of->numpat    = mh->numpat;
    of->inittempo = 125;
    of->initspeed = mh->inittempo >> 4;
    of->numchn    = 4;                       // get number of channels

    if (!AllocPositions(of, 0x80)) return 0;

    // 99 terminates the patorder list (but we also look for invalid values)
    for (t=0; t<128 && mh->patorder[t]<of->numpat; t++)
        of->positions[t] = mh->patorder[t];

    of->numpos = t;
    of->numtrk = of->numpat*of->numchn;

    // Finally, init the sampleinfo structures
    of->numsmp = 31;

    if(!AllocSamples(of, 0))        return 0;
    if(!STM_LoadPatterns(of,mmfile)) return 0;

    q = of->samples;

    MikMod_ISA = _mm_ftell(mmfile);
    MikMod_ISA = (MikMod_ISA+15) & 0xfffffff0;

    for(t=0; t<of->numsmp; t++)
    {   // load sample info

        q->samplename   = DupStr(of->allochandle, mh->sample[t].filename,12);
        q->seekpos      = MikMod_ISA;

        q->speed      = mh->sample[t].c2spd;
        q->volume     = mh->sample[t].volume * 2;
        q->length     = mh->sample[t].length;
        if (/*!mh->sample[t].volume || */q->length==1) q->length = 0; // if vol = 0 or length = 1, then no sample
        q->loopstart  = mh->sample[t].loopbeg;
        q->loopend    = mh->sample[t].loopend;

        MikMod_ISA += q->length;
        MikMod_ISA  = (MikMod_ISA+15) & 0xfffffff0;

        // Once again, contrary to the STM specs, all the sample data is
        // actually SIGNED! Sheesh

        q->format  = SF_SIGNED;

        if((q->loopstart<q->length) && (q->loopend>0) && (q->loopend != 0xffff)) q->flags |= SL_LOOP;

        // fix replen if repend>length

        if(q->loopend > q->length) q->loopend = q->length;

        // Enable aggressive declicking for songs that do not loop and that
        // are long enough that they won't be adversely affected.
        
        if(!(q->flags & (SL_LOOP | SL_SUSTAIN_LOOP)) && (q->length > 5000))
            q->flags |= SL_DECLICK;

        q++;
    }

    return 1;
}


// =====================================================================================
static CHAR *STM_LoadTitle(MMSTREAM *mmfile)
// =====================================================================================
{
    CHAR s[20];

    _mm_fseek(mmfile,0,SEEK_SET);
    if(!_mm_read_UBYTES(s,20,mmfile)) return NULL;

    return DupStrTrim(NULL, s, 20);
}


MLOADER load_stm =
{   "STM",
    IDS_FAMILY_STRING_SCREAMTRACKER2,//"Screamtracker 2",
    0,                              // default FULL STEREO panning
    NULL,
    STM_Test,
    STM_Init,
    STM_Cleanup,
    STM_Load,
#ifndef _MM_WINAMP_
    STM_LoadTitle
#endif
};
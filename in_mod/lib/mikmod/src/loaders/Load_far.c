/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-1999)

 Support:
  If you find problems with THIS code, send mail to:
    air@divent.simplenet.com
  or x-fixer@narod.ru

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: LOAD_FAR.C

  Farandole (FAR) module loader.  As I recall this one works well (cept I
  haven't had a .FAR module to test it with for years).

  X-Fixer: found that this loader is totally broken. Bringing it to life.
  (though, writting it from scratch could be faster =) Some effects are
  not implemented, because Daniel Potter has very strange (contradictory,
  you can say) ideas about effects processing.

  If you have some cool tunes in this format - send them to me!

 Portability:
  All systems - all compilers (hopefully ;-)
*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "..\..\..\..\mikamp\resource.h"


typedef struct FARSAMPLE
{   CHAR  samplename[32];
    ULONG length;
    UBYTE finetune;
    UBYTE volume;
    ULONG reppos;
    ULONG repend;
    UBYTE type;
    UBYTE loop;
} FARSAMPLE;

typedef struct FARHEADER
{   UBYTE id[4];                        // file magic
    CHAR  songname[40];                 // songname
    CHAR  blah[3];                      // 13,10,26
    UWORD headerlen;                    // length of header in bytes
    UBYTE version;
    UBYTE onoff[16];
    UBYTE edit1[9];
    UBYTE speed;
    UBYTE panning[16];
    UBYTE edit2[4];
    UWORD stlen;
    // comment field here
    UBYTE orders[256];
    UBYTE numpat;
    UBYTE snglen;
    UBYTE loopto;
    UWORD patsiz[256];
} FARHEADER;


static const CHAR FAR_Version[] = "Farandole";


static BOOL FAR_Test(MMSTREAM *mmfile)
{
    UBYTE id[4];

    if (!_mm_read_UBYTES(id, 4, mmfile)) return 0;
    return !memcmp(id, "FAR\xFE", 4);
}


static void *FAR_Init(void)
{
    return MikMod_calloc(NULL, 1, sizeof(FARHEADER));
}


static void FAR_Cleanup(void *handle)
{
    MikMod_free(NULL, handle);
}

//
//  The Loader
//
//  Farandole actually has fixed number of samples (64) and patterns (256),
//  but some of them (any) can be empty, and so not stored. The phylosiphy
//  of this loader is "set count to the number of highest non-empty item and
//  fill holes with dummies".
//

static void SetMemDefaults(UNIMOD *of)
{
    UNITRK_EFFECT eff;
    // vibrato
    eff.effect   = UNI_VIBRATO_DEPTH;
    eff.framedly = 0;
    eff.param.u  = 4*16;
    utrk_memdef_add(of, &eff, PTMEM_VIBRATO_DEPTH);
    utrk_local_memflag(of->ut, PTMEM_VIBRATO_DEPTH, UMF_FRAMEDLY);
    utrk_local_memflag(of->ut, PTMEM_PORTAMENTO, UMF_EFFECT);
}

static BOOL LoadPatterns(FARHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint t;
    // alloc track and pattern structures
    if (!AllocTracks(of)) return 0;
    if (!AllocPatterns(of)) return 0;

    of->ut = utrk_init(of->numchn, of->allochandle);
    utrk_memory_reset(of->ut);
    SetMemDefaults(of);
    
    // read patterns
    for (t=0; t<of->numpat; t++)
    {
        if (mh->patsiz[t])
        {
            uint rows = (mh->patsiz[t]-2) >> 6;
            uint breakpos;
            uint u;

            // check for eof
            if (_mm_feof(mmfile))
            {
                _mmlog("load_far> unexpected end of file (in patterns)");
                return 0;
            }
            // read header
            breakpos  = _mm_read_UBYTE(mmfile);
            _mm_read_UBYTE(mmfile);                         // skip unused byte

            if (rows < breakpos+2)
                return 0;
            // read data
            of->pattrows[t] = breakpos + 2;
            utrk_reset(of->ut);
            for (u=rows; u; u--)
            {
                int ch;
                for (ch=0; ch<16; ch++)
                {
                    UBYTE  note, ins, vol, eff, dat;

                    utrk_settrack(of->ut, ch);
                    // read
                    note = _mm_read_UBYTE(mmfile);
                    ins  = _mm_read_UBYTE(mmfile);
                    vol  = _mm_read_UBYTE(mmfile);
                    eff  = _mm_read_UBYTE(mmfile);
                    // put note
                    if (note)
                    {
                        utrk_write_note(of->ut, note + 24);
                        utrk_write_inst(of->ut, ins + 1);
                    }
                    // set volume
                    if (vol && vol<16)
                    {
                        if (vol == 1)
                            vol = 0;
                        else vol <<= 2;
                        pt_write_effect(of->ut, 0xc, vol);
                    }
                    // write effect
                    dat = eff & 0xf;
                    eff >>= 4;

                    switch (eff)
                    {
                    // porta up
                    case 0x1:
                        pt_write_effect(of->ut, 0x1, dat);
                        break;
                    // porta down
                    case 0x2:
                        pt_write_effect(of->ut, 0x2, dat);
                        break;
                    // retrigger
                    case 0x4:
                        if (dat)
                        {
                            UNITRK_EFFECT  effdat;

                            effdat.effect         = UNI_RETRIG;
                            effdat.param.loword.s = -dat;
                            effdat.param.hiword.u = 0;
                            effdat.framedly       = 0;
                            utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                        }
                        break;
                    // vibrato depth
                    case 0x5:
                        // zero IS a legal value ;-)
                        {
                            UNITRK_EFFECT  effdat;

                            effdat.effect    = UNI_VIBRATO_DEPTH;
                            effdat.param.u   = dat*16;
                            effdat.framedly  = UFD_RUNONCE;     // prevent vibrato (makes small change, though)
                            utrk_write_local(of->ut, &effdat, PTMEM_VIBRATO_DEPTH);
                        }
                        break;
                    // vibrato
                    case 0x6:
                        {
                            UNITRK_EFFECT  effdat;
                            // restore depth
                            effdat.framedly  = 0;
                            utrk_memory_local(of->ut, &effdat, PTMEM_VIBRATO_DEPTH, 0);
                            // speed
                            effdat.effect    = UNI_VIBRATO_SPEED;
                            effdat.param.u   = dat*16;
                            effdat.framedly  = 1;
                            utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                        }
                        break;
                    // volslide up
                    case 0x7:
                        pt_write_exx(of->ut, 0xa, dat<<2);
                        break;
                    // volslide down
                    case 0x8:
                        pt_write_exx(of->ut, 0xb, dat<<2);
                        break;
                    // panning
                    case 0xb:
                        pt_write_exx(of->ut, 0x8, dat);
                        break;
                    // fine tempo up/down/cancel
                    case 0xe:
                    case 0xd:
                        // fancy range checking is not supported
                        if (dat)
                        {
                            UNITRK_EFFECT  effdat;

                            effdat.effect    = UNI_GLOB_TEMPOSLIDE;
                            effdat.param.s   = dat*80/128;
                            if (eff == 0xd)
                                effdat.param.s = -effdat.param.s;
                            effdat.framedly  = UFD_RUNONCE;
                            if (effdat.param.s)             // do not write empty effect
                                utrk_write_global(of->ut, &effdat, UNIMEM_NONE);
                        }
                        else pt_write_effect(of->ut, 0xf, 80);
                        break;
                    // speed
                    case 0xf:
                        // check for super-fast tempo
                        if (!dat)
#ifdef _BLOW_UP
                        {
                            _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_far> Unsupported effect.");
                            return 0;
                        }
#else
                            dat++;
#endif
                        pt_write_effect(of->ut, 0xf, dat);
                        break;
                    // global
                    case 0:
                        if (!dat) break;
                    // others not yet implemented
#ifdef _BLOW_UP
                    default:
                        _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_far> Unsupported effect.");
                        return 0;
#endif
                    }
                }
                utrk_newline(of->ut);
            }
        }
        else
        {
            of->pattrows[t] = 64;                           // make 64-rows dummy (in case it's used in orders
            utrk_reset(of->ut);                             // this will make at least some sence)
        }
        // save pattern
        if (!utrk_dup_pattern(of->ut, of))
            return 0;
    }
        
    return 1;
}

static BOOL LoadSamples(UNIMOD *of, MMSTREAM *mmfile)
{
    UBYTE      smap[8];
    UNISAMPLE  *q;
    uint       t;
    // read sample map
    if (!_mm_read_UBYTES(smap, 8, mmfile))
    {
       _mmlog("load_far> unexpected end of file (in samples)");
       return 0;
    }
    // count samples
    of->numsmp = 0;
    for (t=0; t<64; t++)
        if (smap[t>>3] & (1 << (t&7)))
            of->numsmp = t + 1;
    // alloc sample structs
    if (!AllocSamples(of, 0))
        return 0;
    // read samples headers
    q = of->samples;
    for (t=0; t<of->numsmp; t++, q++)
        if (smap[t>>3] & (1 << (t&7)))
        {
            FARSAMPLE  s;
            // read sample header
            _mm_read_SBYTES(s.samplename, 32, mmfile);
            s.length   = _mm_read_I_ULONG(mmfile);
            s.finetune = _mm_read_UBYTE(mmfile);
            s.volume   = _mm_read_UBYTE(mmfile);
            s.reppos   = _mm_read_I_ULONG(mmfile);
            s.repend   = _mm_read_I_ULONG(mmfile);
            s.type     = _mm_read_UBYTE(mmfile);
            s.loop     = _mm_read_UBYTE(mmfile);
            // convert it
            q->samplename = DupStr(of->allochandle, s.samplename, 32);
            q->length     = s.length;
            q->loopstart  = s.reppos;
            q->loopend    = s.repend;
            q->volume     = s.volume << 3;
            q->speed      = 16726;
            
            q->format = SF_SIGNED;
            if (s.type & 1)
                q->format |= SF_16BITS;
            if (s.loop)
                q->flags |= SL_LOOP;
            // check for eof
            if (s.length && _mm_feof(mmfile))
            {
                _mmlog("load_far> unexpected end of file (in samples)");
                return 0;
            }
            // store data position
            q->seekpos = _mm_ftell(mmfile);
            // seek to next sample
            _mm_fseek(mmfile, q->length, SEEK_CUR);
        }

    return 1;
}

static BOOL FAR_Load(FARHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint t;
    
    // try to read module header (first part)
    _mm_read_UBYTES(mh->id, 4, mmfile);
    _mm_read_SBYTES(mh->songname, 40, mmfile);
    _mm_read_SBYTES(mh->blah, 3, mmfile);

    if (mh->blah[0]!=13 || mh->blah[1]!=10 || mh->blah[2]!=26)
        return 0;

    mh->headerlen = _mm_read_I_UWORD (mmfile);
    mh->version   = _mm_read_UBYTE (mmfile);
    _mm_read_UBYTES(mh->onoff, 16, mmfile);
    _mm_read_UBYTES(mh->edit1, 9, mmfile);
    mh->speed     = _mm_read_UBYTE(mmfile);
    _mm_read_UBYTES(mh->panning, 16, mmfile);
    _mm_read_UBYTES(mh->edit2, 4, mmfile);
    mh->stlen     = _mm_read_I_UWORD (mmfile);
    // read songtext into comment field
    of->comment = ReadComment(of->allochandle, mh->stlen, 132, mmfile);
    // try to read module header (second part)
    _mm_read_UBYTES(mh->orders, 256, mmfile);
    mh->numpat         = _mm_read_UBYTE(mmfile);
    mh->snglen         = _mm_read_UBYTE(mmfile);
    mh->loopto         = _mm_read_UBYTE(mmfile);
    _mm_read_I_UWORDS(mh->patsiz, 256, mmfile);
    // check for eof
    if (_mm_feof(mmfile))
    {
       _mmlog("load_far> unexpected end of file (in header)");
        return 0;
    }
    // seek across eventual new data
    _mm_fseek(mmfile, mh->headerlen - (869 + mh->stlen), SEEK_CUR);
    
    // init modfile data
    of->modtype   = _mm_strdup(of->allochandle, FAR_Version);
    of->songname  = DupStrTrim(of->allochandle, mh->songname, 40);
    of->numchn    = 16;
    of->initspeed = mh->speed;
    of->inittempo = 80;
    of->reppos    = mh->loopto;
    of->flags    |= UF_LINEAR;

    for (t=0; t<16; t++)
    {
        of->panning[t] = PAN_LEFT + (mh->panning[t]<<4);
        if (mh->panning[t] > 8)
            of->panning[t] += 16;
    }

    for (t=0; t<16; t++)
        of->muted[t] = !mh->onoff[t];

    // count number of patterns stored in file
    of->numpat = 0;
    for (t=0; t<256; t++)
        if (mh->patsiz[t])
            of->numpat = t + 1;

    // handle positions
    of->numpos = mh->snglen;
    if (!AllocPositions(of, of->numpos))
        return 0;
    // copy order list
    for (t=0; t<of->numpos; t++)
        of->positions[t] = mh->orders[t];
    
    of->numtrk = of->numpat * of->numchn;

    // load patterns    
    if (!LoadPatterns(mh, of, mmfile))
        return 0;
    // load samples
    if (!LoadSamples(of, mmfile))
        return 0;

    return 1;
}


static CHAR *FAR_LoadTitle(MMSTREAM *mmfile)
{
   CHAR s[40];

   _mm_fseek(mmfile, 4, SEEK_SET);
   if (!_mm_read_UBYTES(s, 40, mmfile))
       return NULL;
   
   return DupStrTrim(NULL, s, 40);
}


MLOADER load_far =
{
    "FAR",
    IDS_FAMILY_STRING_FARANDOLE_COMPOSER,//"Farandole Composer",
    0,
    NULL,
    FAR_Test,
    FAR_Init,
    FAR_Cleanup,
    FAR_Load,
#ifndef _MM_WINAMP_
    FAR_LoadTitle
#endif
};
/*

 Module: LOAD_OKT.C

 By X-Fixer, 2002

 Support:
  If you find problems with THIS code, send mail to:
    x-fixer@narod.ru

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------

  Amiga Oktalyzer modules loader. Mostly based on (somewhat incorrect)
  specification by Harald Zappe (was he drunk?). Some ideas are taken from
  (incomplete) code in XMP.

  I have some really good tunes in OKT, so I like the loader :)

 Portability:

  I don't know.

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "..\..\..\..\mikamp\resource.h"


static const CHAR OKT_Version[] = "Oktalyzer";

typedef struct _CHUNK_HEADER
{
    CHAR  id[4];
    ULONG length;
} CHUNK_HEADER;


typedef struct _OKT_SAMPLE
{
    CHAR   samplename[20];
    ULONG  len;
    UWORD  repstart;
    UWORD  replen;
    UBYTE  pad1;
    UBYTE  volume;
    SWORD  pad2;
} OKT_SAMPLE;


static __inline BOOL ReadChunkHeader(CHUNK_HEADER *chunk, char id[4], ULONG len, MMSTREAM *mmfile)
{
    // read id
    _mm_read_UBYTES(chunk->id, 4, mmfile);
    if (memcmp(chunk->id, id, 4))
        return 0;
    // read length
    chunk->length = _mm_read_M_ULONG(mmfile);
    if (len && chunk->length!=len)
        return 0;

    return 1;
}


static BOOL OKT_Test(MMSTREAM *mmfile)
{
    UBYTE id[8];

    if (!_mm_read_UBYTES(id, 8, mmfile)) return 0;
    return !memcmp(id, "OKTASONG", 8);
}


static void *OKT_Init(void)
{
    return (void*)1;
}


static void OKT_Cleanup(void *handle)
{
}

//
//  The Loader
//
//  This format is very chunky - everything is stored in separate
//  chunks (even song speed). Strange, but some common features are
//  missing (like panning and even song title). Usually IFF-like formats
//  are ment to be extensible, but AFAIK here order of chunks is fixed
//  (this, of course, kills all extensibility).
//

static __inline BOOL ReadChannels(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;
    SWORD        flags[4];
    uint         i;

    if (!ReadChunkHeader(&head, "CMOD", 8, mmfile))
        return 0;
    // read channels flags
    _mm_read_M_SWORDS(flags, 4, mmfile);

    of->numchn = 0;
    for (i=0; i<4; i++)
    {
        int pan = (i+1)&2 ? PAN_RIGHT-load_okt.defpan: PAN_LEFT+load_okt.defpan;

        of->panning[of->numchn++] = pan;
        if (flags[i])
        {
            if (flags[i] > 1)                               // unknown
                return 0;
            of->panning[of->numchn++] = pan;
        }
    }

    return 1;
}

static BOOL ReadSamples(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;
    uint         i;
    UNISAMPLE    *q;

    if (!ReadChunkHeader(&head, "SAMP", 0, mmfile))
        return 0;
    // alloc sample structs
    of->numsmp = head.length / 32;
    if (!AllocSamples(of, 0))
        return 0;
    // read samples
    for (q=of->samples, i=of->numsmp; i; --i, q++)
    {
        OKT_SAMPLE s;
        // read sample header
        _mm_read_UBYTES(s.samplename, 20, mmfile);
        s.len         = _mm_read_M_ULONG(mmfile);
        s.repstart    = _mm_read_M_UWORD(mmfile);
        s.replen      = _mm_read_M_UWORD(mmfile);
        s.pad1        = _mm_read_UBYTE(mmfile);
        s.volume      = _mm_read_UBYTE(mmfile);
        s.pad2        = _mm_read_M_UWORD(mmfile);
        // convert
        q->samplename = DupStr(of->allochandle, s.samplename, 20);
        q->length     = s.len & ~1;                         // length should be rounded down
        if (s.replen > 2)
        {
            q->loopstart  = s.repstart * 2;
            q->loopend    = (s.repstart + s.replen) * 2;
            q->flags     |= SL_LOOP;
        }
        q->volume     = s.volume * 2;
        q->speed      = 8363;       //DIG 8287?
        q->format     = SF_SIGNED;
    }
    // check for eof
    if (_mm_feof(mmfile))
    {
        _mmlog("load_okt> unexpected end of file (in samples)");
        return 0;
    }

    return 1;
}

static __inline BOOL ReadSpeed(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;

    if (!ReadChunkHeader(&head, "SPEE", 2, mmfile))
        return 0;
    of->inittempo = 125;
    of->initspeed = _mm_read_M_UWORD(mmfile);

    return 1;
}

static __inline BOOL ReadNumPats(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;

    if (!ReadChunkHeader(&head, "SLEN", 2, mmfile))
        return 0;
    of->numpat = _mm_read_M_UWORD(mmfile);
    of->numtrk = of->numpat * of->numchn;

    return AllocPatterns(of) && AllocTracks(of);
}

static __inline BOOL ReadNumPos(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;

    if (!ReadChunkHeader(&head, "PLEN", 2, mmfile))
        return 0;
    of->numpos = _mm_read_M_UWORD(mmfile);

    return 1;
}

static __inline BOOL ReadPositions(UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;
    uint         i;

    if (!ReadChunkHeader(&head, "PATT", 0, mmfile))
        return 0;
    if (head.length < of->numpos)
        return 0;
    // read positions
    if (!AllocPositions(of, head.length)) return 0;

    for (i=0; i<head.length; i++)
        of->positions[i] = _mm_read_UBYTE(mmfile);

    return 1;
}

static BOOL ReadPattern(uint num, UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;
    uint         i;

    if (!ReadChunkHeader(&head, "PBOD", 0, mmfile))
        return 0;
    // read rows count
    of->pattrows[num] = _mm_read_M_UWORD(mmfile);

    utrk_reset(of->ut);
    for (i=of->pattrows[num]; i; --i)
    {
        uint j;

        for (j=0; j<of->numchn; j++)
        {
            UNITRK_EFFECT effdat;
            UBYTE note;
            UBYTE inst;
            UBYTE effect;
            UBYTE data;

            utrk_settrack(of->ut, j);
            // read note
            note   = _mm_read_UBYTE(mmfile);
            inst   = _mm_read_UBYTE(mmfile);
            effect = _mm_read_UBYTE(mmfile);
            data   = _mm_read_UBYTE(mmfile);
            // convert
            if (note)
            {
                utrk_write_note(of->ut, note + 36);
                utrk_write_inst(of->ut, inst + 1);
            }
            // effect
            switch (effect)
            {
            // no effect
            case 0:
                break;
            // position jump
            case 25:
                pt_write_effect(of->ut, 0xb, _mm_2hex(data));
                break;
            // set speed
            case 28:
                pt_write_effect(of->ut, 0xf, data);
                break;
            // portamento up / down
            case 1:
            case 2:
                pt_write_effect(of->ut, effect, data);
                break;
            // arpeggio
            case 10:
            case 12:
                if (data)
                {
                    effdat.param.byte_a = data & 0xf;
                    effdat.param.byte_b = effect==10 ? 0-(SBYTE)(data>>4) : data&0xf;
                    effdat.param.byte_d = 3;
                    effdat.effect  = UNI_ARPEGGIO;
                    effdat.framedly = 0;
                    utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                }
                break;
            // arpeggio, too
            case 11:
                if (data)
                {
                    effdat.param.byte_a = data & 0xf;
                    effdat.param.byte_b = 0;
                    effdat.param.byte_c = 0 - (SBYTE)(data>>4);
                    effdat.param.byte_d = 4;
                    effdat.effect  = UNI_ARPEGGIO;
                    effdat.framedly = 0;
                    utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                }
                break;
            // pitch slide down
            case 13:
            case 21:
                if (data)
                {
                    effdat.param.loword.s = 0 - data;
                    effdat.param.hiword.u = 1;
                    effdat.effect         = UNI_NOTESLIDE;
                    effdat.framedly       = effect==13 ? 0 : UFD_RUNONCE;
                    utrk_write_local(of->ut, &effdat, PTMEM_NOTESLIDE);
                }
                else
                    utrk_memory_local(of->ut, NULL, PTMEM_NOTESLIDE, 0);
                break;
            // pitch slide up
            case 30:
            case 17:
                if (data)
                {
                    effdat.param.loword.s = data;
                    effdat.param.hiword.u = 1;
                    effdat.effect         = UNI_NOTESLIDE;
                    effdat.framedly       = effect==30 ? 0 : UFD_RUNONCE;
                    utrk_write_local(of->ut, &effdat, PTMEM_NOTESLIDE);
                }
                else
                    utrk_memory_local(of->ut, NULL, PTMEM_NOTESLIDE, 0);
                break;
            // Amiga filter control - ignored
            case 15:
                break;
            // release key
            case 27:
                effdat.effect   = UNI_KEYOFF;
                effdat.param.u  = 0;
                effdat.framedly = 0;
                utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                break;
            // volume control
            case 31:
                // set volume
                if (data <= 0x40)
                    pt_write_effect(of->ut, 0xc, data);
                else
                {
                    effdat.effect   = UNI_VOLSLIDE;
                    // fast fade out
                    if (data <= 0x50)
                    {
                        effdat.param.s  = 0x40 - (long)data;
                        effdat.framedly = 0;
                    }
                    // fast fade in
                    else if (data <= 0x60)
                    {
                        effdat.param.s  = data - 0x50;
                        effdat.framedly = 0;
                    }
                    // slow fade out
                    else if (data <= 0x70)
                    {
                        effdat.param.s  = 0x60 - (long)data;
                        effdat.framedly = UFD_RUNONCE;
                    }
                    // slow fade in
                    else if (data <= 0x80)
                    {
                        effdat.param.s  = data - 0x70;
                        effdat.framedly = UFD_RUNONCE;
                    }
                    else break;
                    utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                }
                break;
#ifdef _BLOW_UP
            //DIG effects 4 and 9 appear in some songs
            default:
                _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_okt> Unsupported effect.");
                return 0;
#endif
            }
        }
        utrk_newline(of->ut);
    }
    // save pattern
    if (!utrk_dup_pattern(of->ut, of))
        return 0;
    // check for eof
    if (_mm_feof(mmfile))
    {
        _mmlog("load_okt> unexpected end of file (in patterns)");
        return 0;
    }

    return 1;
}

static __inline BOOL ReadSampleBody(uint num, UNIMOD *of, MMSTREAM *mmfile)
{
    CHUNK_HEADER head;

    if (!of->samples[num].length) return 1;                 // skip empty sample
    if (!ReadChunkHeader(&head, "SBOD", 0, mmfile))
        return 0;
    // check sample length
    if (of->samples[num].length != head.length)
        return 0;
    // store sample offset
    of->samples[num].seekpos = _mm_ftell(mmfile);
    _mm_fseek(mmfile, head.length, SEEK_CUR);

    return 1;
}

static BOOL OKT_Load(void *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint    i;

    // init module data
    of->modtype   = _mm_strdup(of->allochandle, OKT_Version);

    // read headers
    _mm_fseek(mmfile, 8, SEEK_SET);                         // skip id

    if (!ReadChannels(of, mmfile) ||
        !ReadSamples(of, mmfile) ||
        !ReadSpeed(of, mmfile) ||
        !ReadNumPats(of, mmfile) ||
        !ReadNumPos(of, mmfile) ||
        !ReadPositions(of, mmfile))
        return 0;

    // read patterns
    of->ut = utrk_init(of->numchn, of->allochandle);
    utrk_memory_reset(of->ut);
    utrk_local_memflag(of->ut, PTMEM_PORTAMENTO, UMF_EFFECT);

    for (i=0; i<of->numpat; i++)
        if (!ReadPattern(i, of, mmfile))
            return 0;

    // read samples offsets
    for (i=0; i<of->numsmp; i++)
        if (!ReadSampleBody(i, of, mmfile))
            return 0;

    return 1;
}


static CHAR *OKT_LoadTitle(MMSTREAM *mmfile)
{
   return NULL;
}


MLOADER load_okt =
{
    "OKT",
    IDS_FAMILY_STRING_AMIGA_OKTALYZER,//"Amiga Oktalyzer",
    0,
    NULL,
    OKT_Test,
    OKT_Init,
    OKT_Cleanup,
    OKT_Load,
#ifndef _MM_WINAMP_
    OKT_LoadTitle
#endif
};
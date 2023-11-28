/*

 Module: LOAD_PTM.C

 By X-Fixer, 2002

 Support:
  If you find problems with THIS code, send mail to:
    x-fixer@narod.ru

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------

  PolyTracker is a ScreamTracker 3 like tracker written by Lone Ranger / AcmE.
  (In fact it's a mixture of ScreamTracker and Protracker). Loader is based
  on a great format specification by JAL / Nostalgia. Many thanks to him.

  Thanks to JAL and Lone Ranger for help in writting this loader!

  Yay for Netherlands music! REW rules!

 Portability:

  I don't know.

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "itshare.h"
#include "..\..\..\..\mikamp\resource.h"


typedef struct
{
    CHAR   songname[28];
    UBYTE  eof;
    UWORD  version;
    UBYTE  reserved1;
    UWORD  numpos;
    UWORD  numsmp;
    UWORD  numpat;
    UWORD  numchn;
    UWORD  flags;
    UWORD  reserved2;
    CHAR   id[4];
    UBYTE  reserved3[16];
    UBYTE  pan[32];
    UBYTE  orders[256];
    UWORD  patoffs[129];                                    // the last one is for internal use
} PTMHEADER;

typedef struct
{
    UBYTE  type;
    CHAR   filename[12];
    UBYTE  volume;
    UWORD  speed;
    UWORD  seg;
    ULONG  offset;
    ULONG  length;
    ULONG  loopstart;
    ULONG  loopend;
    ULONG  gusBeg, gusStart, gusEnd;
    UBYTE  gusLoop;
    UBYTE  reserved;
    CHAR   samplename[28];
    UBYTE  id[4];
} PTMINSTR;

static const CHAR PTM_Version[] = "PolyTracker x.xx";

// PolyTracker's oddball volume table
static const int volume_table[129] =
{
      0,   1,   3,   5,   6,   7,   9,  10,
     11,  12,  14,  15,  16,  17,  19,  20,
     21,  22,  23,  24,  25,  26,  27,  28,
     29,  30,  31,  32,  34,  35,  37,  38,
     39,  40,  41,  42,  44,  45,  46,  47,
     48,  49,  50,  50,  51,  52,  53,  54,
     55,  55,  56,  57,  58,  58,  59,  60,
     61,  62,  63,  65,  66,  67,  69,  70,
     71,  72,  73,  74,  76,  77,  78,  79,
     80,  81,  82,  83,  84,  85,  86,  87,
     88,  89,  90,  91,  92,  93,  94,  95,
     96,  96,  97,  98,  99, 100, 101, 101,
    102, 103, 104, 105, 106, 106, 107, 108,
    108, 109, 110, 111, 112, 112, 113, 114,
    114, 115, 116, 116, 117, 118, 118, 119,
    120, 120, 121, 122, 122, 124, 125, 126,
    128
};


static BOOL PTM_Test(MMSTREAM *mmfile)
{
    UBYTE id[4];

    _mm_fseek(mmfile, 44, SEEK_SET);
    if (!_mm_read_UBYTES(id, 4, mmfile)) return 0;
    return !memcmp(id, "PTMF", 4);
}


static void *PTM_Init(void)
{
    return MikMod_calloc(NULL, 1, sizeof(PTMHEADER));
}


static void PTM_Cleanup(HANDLE *handle)
{
    MikMod_free(NULL, handle);
}

//
//  The Loader
//
//  Nice format, nice tracker, nice documentation => nice loader
//

static BOOL LoadPatterns(PTMHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint i;
    // prepare
    if (!AllocTracks(of)) return 0;
    if (!AllocPatterns(of)) return 0;

    of->ut = utrk_init(of->numchn, of->allochandle);
    utrk_memory_reset(of->ut);
    S3MIT_SetMemDefaults(of);

    for (i=0; i<of->numpat; i++)
    {
        SLONG datasize = (mh->patoffs[i+1] - mh->patoffs[i])*16;
        if (datasize < 0)
            return 0;

        // check for eof
        if (datasize && _mm_feof(mmfile))
        {
            _mmlog("load_ptm> unexpected end of file (in patterns)");
            return 0;
        }
        // seek to start
        _mm_fseek(mmfile, mh->patoffs[i]*16, SEEK_SET);
        utrk_reset(of->ut);
        // read pattern
        while (datasize-- > 0)
        {
            UBYTE flag = _mm_read_UBYTE(mmfile);

            if (flag)
            {
                UBYTE chn = flag & 0x1F;
                // set channel
                if (chn >= of->numchn)
                    return 0;
                utrk_settrack(of->ut, chn);
                // read note
                if (flag & 32)
                {
                    UBYTE note = _mm_read_UBYTE(mmfile);
                    UBYTE inst = _mm_read_UBYTE(mmfile);
                    datasize -= 2;

                    if (note)
                    {
                        // note
                        if (note < 254)
                            utrk_write_note(of->ut, note);  // yay! the same notes scale!
                        // note off
                        else
                        {
                            UNITRK_EFFECT eff = {0, UNI_NOTEKILL, UFD_RUNONCE};
                            utrk_write_local(of->ut, &eff, UNIMEM_NONE);
                        }
                    }
                    if (inst)
                        utrk_write_inst(of->ut, inst);
                }
                // read effect
                if (flag & 64)
                {
                    UBYTE effect = _mm_read_UBYTE(mmfile);
                    UBYTE data   = _mm_read_UBYTE(mmfile);
                    datasize -= 2;

                    switch (effect)
                    {
                    // slide down/up
                    case 1:
                    case 2:
                        S3MIT_ProcessCmd(of->ut, NULL, 4+effect, data, 7, PTMEM_PORTAMENTO, 0);
                        break;
                    // volslide down/up
                    case 0xA:
                        S3MIT_ProcessCmd(of->ut, NULL, 4, data, 7, PTMEM_PORTAMENTO, 0);
                        break;
                    // global volume
                    case 0x10:
                        S3MIT_ProcessCmd(of->ut, NULL, 0x16, data, 7, PTMEM_PORTAMENTO, 0);
                        break;
                    // retrigger
                    case 0x11:
                        S3MIT_ProcessCmd(of->ut, NULL, 0x11, data, 7, PTMEM_PORTAMENTO, 0);
                        break;
                    // fine vibrato
                    case 0x12:
                        S3MIT_ProcessCmd(of->ut, NULL, 0x15, data, 7, PTMEM_PORTAMENTO, 0);
                        break;
                    // note slide + retrigger
                    case 0x15:
                    case 0x16:
                        pt_write_exx(of->ut, 0x9, data>>4);
                        /* no break */
                    // note slide
                    case 0x13:
                    case 0x14:
                        if (data)
                        {
                            UNITRK_EFFECT eff;
                            UBYTE lo = data&0xf, hi = data>>4;

                            eff.param.loword.s = lo;
                            eff.param.hiword.u = hi;
                            if (effect & 1) eff.param.s = -eff.param.s;
                            // both present
                            if (lo && hi)
                            {
                                eff.effect   = UNI_NOTESLIDE;
                                eff.framedly = 0;
                                utrk_write_local(of->ut, &eff, PTMEM_NOTESLIDE);
                            }
                            // value only
                            else if (lo)
                                utrk_memory_local_ex(of->ut, &eff, PTMEM_NOTESLIDE, UMF_LOWORD, 0);
                            // speed only
                            else if (hi)
                                utrk_memory_local_ex(of->ut, &eff, PTMEM_NOTESLIDE, UMF_HIWORD, 0);
                        }
                        else utrk_memory_local_ex(of->ut, NULL, PTMEM_NOTESLIDE, 0, 0);
                        break;
                    // reverse sample
                    case 0x17:
                        {
                            UNITRK_EFFECT eff;

                            eff.effect   = UNI_REVERSE;
                            eff.framedly = UFD_RUNONCE;
                            eff.param.u  = data*256;
                            utrk_write_local(of->ut, &eff, UNIMEM_NONE);
                        }
                        break;
                    // other effects are handled here
                    default:
#ifdef _BLOW_UP
                        if (effect>15 &&
                            effect!=25 &&      // 25=='P' was seen in shotgun.ptm
                            effect!=29)        // 29=='T' was seen in bugfixed.ptm
                        {
                            _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_ptm> Unsupported effect.");
                            return 0;
                        }
#endif
                        // some notes:
                        // format spec says E1x=down and E2x=up, but indeed
                        // it's vice versa - like in protracker.

                        pt_write_effect(of->ut, effect, data);
                    }
                }
                // read volume
                if (flag & 128)
                {
                    UBYTE volume = _mm_read_UBYTE(mmfile);
                    datasize--;

                    pt_write_effect(of->ut, 0xc, volume);
                }
            }
            else utrk_newline(of->ut);
        }
        // save pattern
        if (!utrk_dup_pattern(of->ut, of))
            return 0;
    }

    return 1;
}

static BOOL LoadSamples(PTMHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    UNISAMPLE  *q;
    uint        i;

    // check for eof
    if (_mm_feof(mmfile))
    {
        _mmlog("load_ptm> unexpected end of file (in sample headers)");
        return 0;
    }
    // alloc sample structs
    if (!AllocSamples(of, 0))
        return 0;
    // read samples headers
    q = of->samples;

    for (i=0; i<of->numsmp; i++, q++)
    {
        PTMINSTR    s;
        // read sample header
        s.type      = _mm_read_UBYTE(mmfile);
        _mm_read_UBYTES(s.filename, 12, mmfile);
        s.volume    = _mm_read_UBYTE(mmfile);
        s.speed     = _mm_read_I_UWORD(mmfile);
        _mm_read_I_UWORD(mmfile);
        s.offset    = _mm_read_I_ULONG(mmfile);
        s.length    = _mm_read_I_ULONG(mmfile);
        s.loopstart = _mm_read_I_ULONG(mmfile);
        s.loopend   = _mm_read_I_ULONG(mmfile);
        _mm_read_I_ULONG(mmfile);
        _mm_read_I_ULONG(mmfile);
        _mm_read_I_ULONG(mmfile);
        _mm_read_I_UWORD(mmfile);
        _mm_read_UBYTES(s.samplename, 28, mmfile);
        _mm_read_UBYTES(s.id, 4, mmfile);
        // convert it
        if (!(s.type & 3))                                  // no data
        {
            s.length = 0;
            s.offset = 0;
        }

        // I do not used DupStr here, because PolyTracker really used this
        // as a null terminated string, and many sample names contain garbage
        // after the first NULL
        q->samplename = _mm_strdup(of->allochandle, s.samplename);
        q->seekpos    = s.offset;
        q->length     = s.length;
        q->speed      = s.speed;
        q->volume     = s.volume << 1;
        q->loopstart  = s.loopstart;
        q->loopend    = s.loopend;
        q->format     = SF_SIGNED|SF_DELTA;

        if (s.type & 4) q->flags |= SL_LOOP;
        if (s.type & 8) q->flags |= SL_BIDI;

        if (s.type & 16)
        {
            q->format     |= SF_16BITS|SF_DELTA_BYTES;      // oddball byte-oriented delta compression
            q->loopstart >>= 1;
            q->loopend   >>= 1;
            q->length    >>= 1;
        }
        // store offset of the first not empty sample
        // to get the size of the last pattern
        if (!mh->patoffs[mh->numpat] && q->seekpos)
            mh->patoffs[mh->numpat] = q->seekpos / 16;
    }

    return 1;
}

static BOOL PTM_Load(PTMHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint    i;

    // try to read module header
    _mm_read_UBYTES(mh->songname, 28, mmfile);
    _mm_read_UBYTE(mmfile);
    mh->version = _mm_read_I_UWORD(mmfile);
    _mm_read_UBYTE(mmfile);
    mh->numpos  = _mm_read_I_UWORD(mmfile);
    mh->numsmp  = _mm_read_I_UWORD(mmfile);
    mh->numpat  = _mm_read_I_UWORD(mmfile);
    mh->numchn  = _mm_read_I_UWORD(mmfile);
    mh->flags   = _mm_read_I_UWORD(mmfile);
    _mm_read_I_UWORD(mmfile);
    _mm_read_UBYTES(mh->id, 4, mmfile);
    _mm_read_UBYTES(mh->reserved3, 16, mmfile);
    _mm_read_UBYTES(mh->pan, 32, mmfile);
    _mm_read_UBYTES(mh->orders, 256, mmfile);
    _mm_read_I_UWORDS(mh->patoffs, 128, mmfile);

    if (mh->numpat>128 || mh->numpos>256 || mh->numchn>32)
        return 0;

    // init modfile data
    of->modtype     = _mm_strdup(of->allochandle, PTM_Version);
    of->modtype[12] = ((mh->version>>8)&0xf) + '0';
    of->modtype[14] = ((mh->version>>4)&0xf) + '0';
    of->modtype[15] = ((mh->version)&0xf)    + '0';
    of->songname    = DupStrTrim(of->allochandle, mh->songname, 28);
    of->numpos      = mh->numpos;
    of->numsmp      = mh->numsmp;
    of->numpat      = mh->numpat;
    of->numchn      = mh->numchn;
    of->numtrk      = of->numpat * of->numchn;
    of->inittempo   = 125;
    of->initspeed   = 6;
    of->volume_table = volume_table;

    // panning
    for (i=0; i<of->numchn; i++)
        of->panning[i] = mh->pan[i]<7 ? PAN_LEFT+mh->pan[i]*18 : mh->pan[i]>7 ? PAN_RIGHT - (15-mh->pan[i])*17 : 0;

    // orders
    if (!AllocPositions(of, of->numpos))
        return 0;

    for (i=0; i<of->numpos; i++)
        of->positions[i] = mh->orders[i];

    // load samples
    if (!LoadSamples(mh, of, mmfile))
        return 0;

    if (!mh->patoffs[mh->numpat])                           // all samples are empty, or some unknown shit
    {
        // set offset to file size
        long oldpos = _mm_ftell(mmfile);
        _mm_fseek(mmfile, 0, SEEK_END);
        mh->patoffs[mh->numpat] = _mm_ftell(mmfile)/16;
        _mm_fseek(mmfile, oldpos, SEEK_SET);
    }

    // load patterns
    if (!LoadPatterns(mh, of, mmfile))
        return 0;

    return 1;
}


static CHAR *PTM_LoadTitle(MMSTREAM *mmfile)
{
   CHAR s[28];

   _mm_fseek(mmfile, 0, SEEK_SET);
   if (!_mm_read_UBYTES(s, 28, mmfile))
       return NULL;
   
   return DupStrTrim(NULL, s, 28);
}


MLOADER load_ptm =
{
    "PTM",
    IDS_FAMILY_STRING_POLYTRACKER,//"PolyTracker",
    0,
    NULL,
    PTM_Test,
    PTM_Init,
    PTM_Cleanup,
    PTM_Load,
#ifndef _MM_WINAMP_
    PTM_LoadTitle
#endif
};
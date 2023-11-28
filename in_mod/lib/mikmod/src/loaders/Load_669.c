/*

 MikMod Sound System

  By Jake Stine of Divine Entertainment (1996-1999)

 Support:
  If you find problems with this code, send mail to:
    air@divent.simplenet.com

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------

 Module: LOAD_669.C
 
  Tran's 669 module loader.  It doesn't work very well because Tran's brain
  has a method of twisted logic beyond the extraordinary, and a lacking a-
  bility to document it.

  X-Fixer: added few effects and made some fixes (not much work to code,
  but much work to find 8-).

 Portability:
   All systems - all compilers (hopefully)


*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "..\..\..\..\mikamp\resource.h"

// Raw 669 header struct:

typedef struct S69HEADER
{
	UBYTE  marker[2];
    CHAR   message[108];
    UBYTE  nos;
    UBYTE  nop;
    UBYTE  looporder;
    UBYTE  orders[0x80];
    UBYTE  tempos[0x80];
    UBYTE  breaks[0x80];
} S69HEADER;


// Raw 669 sampleinfo struct:

typedef struct S69SAMPLE
{   CHAR   filename[13];
    SLONG  length;
    SLONG  loopbeg;
    SLONG  loopend;
} S69SAMPLE;


static CHAR *S69_Version[] =
{
    "Composer 669",
    "Extended 669"
};


static BOOL S69_Test(MMSTREAM *mmfile)
{
    UBYTE id[2];

    if(!_mm_read_UBYTES(id,2,mmfile)) return 0;
    if(!memcmp(id,"if",2) || !memcmp(id,"JN",2))
    {   _mm_fseek(mmfile,108,SEEK_CUR);
        if(_mm_read_UBYTE(mmfile) > 64) return 0;
        if(_mm_read_UBYTE(mmfile) > 128) return 0;
        if(_mm_read_UBYTE(mmfile) > 120) return 0;
        return 1;
    }
    return 0;
}


static void *S69_Init(void)
{
    return MikMod_calloc(NULL, 1, sizeof(S69HEADER));
}


static void S69_Cleanup(void *handle)
{
    MikMod_free(NULL, handle);
}

static BOOL S69_LoadPatterns(MMSTREAM *mmfile, S69HEADER *mh, UNIMOD *of)
{
    uint    t;
    int     t2,t3;


    if(!AllocPatterns(of)) return 0;
    if(!AllocTracks(of)) return 0;

    of->ut = utrk_init(of->numchn, of->allochandle);

    for(t=0; t<of->numpat; t++)
    {
        of->pattrows[t] = mh->breaks[t] + 1;
        utrk_reset(of->ut);

        pt_write_effect(of->ut, 0xf, mh->tempos[t]);        // the man who wrote previous code was really drunk -XF

        for(t2=64; t2; t2--)
        {   for(t3=0; t3<8; t3++)
            {   
                UBYTE         a, b, c;
                uint          note,inst,vol,lo;
                UNITRK_EFFECT effdat;
            
                a = _mm_read_UBYTE(mmfile);
                b = _mm_read_UBYTE(mmfile);
                c = _mm_read_UBYTE(mmfile);

                note = a >> 2;
                inst = ((a & 0x3) << 4) | ((b & 0xf0) >> 4);
                vol  = b & 0xf;

                utrk_settrack(of->ut, t3);
                // this was incorrect, too -XF
                if (a < 0xFF)
                {
                    if (a < 0xFE)
                    {
                        utrk_write_inst(of->ut, inst + 1);
                        utrk_write_note(of->ut, note + 25);
                    }
                    pt_write_effect(of->ut, 0xc, vol<<2);
                }

                lo = c & 0xf;
                switch(c >> 4)
                {
                // portamento up
                case 0:
                    pt_write_effect(of->ut, 0x1, lo);
                    break;
                // portamento down
                case 1:
                    pt_write_effect(of->ut, 0x2, lo);
                    break;
                // port to note
                case 2:
                    pt_write_effect(of->ut, 0x3, lo);
                    break;
                // freq adjust
                case 3:
                    pt_write_exx(of->ut, 0x1, lo << 2);
                    break;
                // vibrato
                case 4:
                    pt_write_effect(of->ut, 0x4, lo);
                    break;
                // G commands
                case 6:
                    effdat.effect  = UNI_PANSLIDE;
                    effdat.framedly = 0;
                    effdat.param.s = (lo*2 - 1) * 8;        //DIG not sure about scale
                    utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                    break;
                // retrig
                case 7:
                    pt_write_exx(of->ut, 0x9, lo);
                    break;
                // no command
                case 15:
                    break;
                // tempo
                case 5:
                    // check for super-fast tempo
                    if (!lo)
#ifdef _BLOW_UP
                    {
                        _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_669> Unsupported effect.");
                        return 0;
                    }
#else
                        lo++;
#endif
                    pt_write_effect(of->ut, 0xf, lo);
                    break;
#ifdef _BLOW_UP
                default:
                    _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_669> Unsupported effect.");
                    return 0;
#endif
                }
            }
            utrk_newline(of->ut);
        }

        if (_mm_feof(mmfile))
        {
            _mmlog("load_669> unexpected end of file (in patterns)");
            return 0;
        }

        utrk_dup_pattern(of->ut, of);
    }
    return 1;
}

static BOOL S69_LoadSamples(MMSTREAM *mmfile, S69HEADER *mh, UNIMOD *of)
{
    UNISAMPLE   *q;
    uint        t;
    long        seekpos;

    if(!AllocSamples(of, 0)) return 0;

    // calculate position of sample data
    // (we should skip sample headers and patterns)
    seekpos = _mm_ftell(mmfile) + mh->nos*25 + mh->nop*64*8*3;

    for (t=0, q=of->samples; t<of->numsmp; t++, q++)
    {   
        S69SAMPLE   s;
        // try to read sample info
        _mm_read_UBYTES((UBYTE *)s.filename,13,mmfile);
        s.length   = _mm_read_I_SLONG(mmfile);
        s.loopbeg  = _mm_read_I_SLONG(mmfile);
        s.loopend  = _mm_read_I_SLONG(mmfile);

        if (s.length<0 || s.loopbeg<-1 || s.loopend<-1)
        {   
            _mmlog("load_669> illegal value in sample header");
            return 0;
        }
        // fill info
        q->samplename = DupStr(of->allochandle, s.filename, 13);
        q->seekpos    = seekpos;
        q->speed      = 8363;           //DIG 8740 ?
        q->length     = s.length;
        q->volume     = 128;
        if (s.loopend < s.length)
        {
            q->loopstart = s.loopbeg;
            q->loopend   = s.loopend;
            q->flags    |= SL_LOOP;
        }
        // advance
        seekpos += s.length;
    }

    return 1;
}

static BOOL S69_Load(S69HEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    uint        t;

    // try to read module header       
    _mm_read_UBYTES(mh->marker,2,mmfile);
    _mm_read_UBYTES((UBYTE *)mh->message,108,mmfile);
    mh->nos = _mm_read_UBYTE(mmfile);
    mh->nop = _mm_read_UBYTE(mmfile);
    mh->looporder = _mm_read_UBYTE(mmfile);
    _mm_read_UBYTES(mh->orders,0x80,mmfile);
    _mm_read_UBYTES(mh->tempos,0x80,mmfile);
    _mm_read_UBYTES(mh->breaks,0x80,mmfile);

    // set module variables
    of->initspeed = 6;
    of->inittempo = 80;
    of->songname  = DupStrTrim(of->allochandle, mh->message, 36);
    of->comment   = ProcessComment(of->allochandle, 108, 36, mh->message);
    of->modtype   = _mm_strdup(of->allochandle, S69_Version[memcmp(mh->marker,"JN",2)==0]);
    of->numchn    = 8;
    of->numpat    = mh->nop;
    of->numsmp    = mh->nos;
    of->numtrk    = of->numchn*of->numpat;
    of->reppos    = mh->looporder;
    of->flags    |= UF_LINEAR;

    if(!AllocPositions(of, 0x80)) return 0;

    for (t=0; t<0x80 && mh->orders[t]!=0xff; t++)
        of->positions[t] = mh->orders[t];

    of->numpos = t;

    if(!S69_LoadSamples(mmfile, mh, of)) return 0;

    if(!S69_LoadPatterns(mmfile, mh, of)) return 0;

    return 1;
}

static CHAR *S69_LoadTitle(MMSTREAM *mmfile)
{
   CHAR s[36];

   _mm_fseek(mmfile, 2, SEEK_SET);
   if (!_mm_read_UBYTES(s, 36, mmfile))
       return NULL;
   
   return DupStrTrim(NULL, s, 36);
}


MLOADER load_669 =
{   "669",
    IDS_FAMILY_STRING_COMPOSER_669,//"Composer 669",
    0,                              // default FULL STEREO panning
    NULL,
    S69_Test,
    S69_Init,
    S69_Cleanup,
    S69_Load,
#ifndef _MM_WINAMP_
    S69_LoadTitle
#endif
};
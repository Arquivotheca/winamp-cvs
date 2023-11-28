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
 Module:  MLOADER.C

  Frontend interface for the dozen or so mikmod loaders.

 Portability:
  All systems - all compilers

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "mplayer.h"


MLOADER *firstloader = NULL;

// =====================================================================================
    int MikMod_GetNumLoaders(void)
// =====================================================================================
{
    int      t;
    MLOADER *l;

    for(t=1,l=firstloader; l; l=l->next, t++);

    return t;
}


// =====================================================================================
    void ML_RegisterLoader(MLOADER *ldr)
// =====================================================================================
{
    MLOADER *cruise = firstloader;

    if(!ldr) return;
    
    if(cruise!=NULL)
    {   while(cruise->next!=NULL)  cruise = cruise->next;
        cruise->next = ldr;
    } else
        firstloader = ldr; 

    ldr->enabled   = TRUE;
    ldr->nopaneff  = FALSE;
    ldr->noreseff  = FALSE;
}


// =====================================================================================
    CHAR *DupStr(MM_ALLOC *allochandle, CHAR *s, UWORD len)
// =====================================================================================
//  Creates a CSTR out of a character buffer of 'len' bytes, but strips
//  any terminating non-printing characters like 0, spaces etc.
{
    UWORD t;
    CHAR  *d = NULL;

    if (!s) return NULL;

    // Scan for first printing char in buffer [includes high ascii up to 254 ?]
    while(len)
    {   
        if((UBYTE)s[len-1] > 32u) break;
        len--;
    }

    if (!len)
        return NULL;

    // When the buffer wasn't completely empty, allocate
    // a cstring and copy the buffer into that string, except
    // for any control-chars

#ifdef __GNUC__
    if(len<16) len = 16;
#endif

    if ((d = (CHAR*)MikMod_malloc(allochandle, len+1)) != NULL)
    {
        for (t=0; t<len; t++)
            d[t] = (UBYTE)s[t]<32u ? ' ' : s[t];
        d[t] = 0;

//        OemToChar(d, d);
    }

    return d;
}


// =====================================================================================
    CHAR *DupStrTrim(MM_ALLOC *allochandle, CHAR *s, UWORD len)
// =====================================================================================
//  Creates a CSTR out of a character buffer of 'len' bytes, but strips
//  any starting & terminating non-printing characters like 0, spaces etc.
{
    while (len && (UBYTE)*s<=32u)
    {
        s++;
        len--;
    }

    return len ? DupStr(allochandle, s, len) : NULL;
}


// =====================================================================================
    CHAR *ReadComment(MM_ALLOC *allochandle, uint length, uint linelen, MMSTREAM *file)
// =====================================================================================
{
    CHAR  *buf, *result;
    // init
    if (!length) return NULL;
    if ((buf=(CHAR *)MikMod_malloc(allochandle, length)) == NULL)
        return NULL;
    // read
    _mm_read_UBYTES(buf, length, file);
    result = ProcessComment(allochandle, length, linelen, buf);
    MikMod_free(allochandle, buf);

    return result;
}


// =====================================================================================
    CHAR *ProcessComment(MM_ALLOC *allochandle, uint length, uint linelen, CHAR *string)
// =====================================================================================
{
#define is_space(c)         (c==' ' || c=='\t')
#define is_space_nl(c)      (is_space(c) || c==0xD)
#define trim_line()         while (q>output && is_space(*(q-1))) *q--;
#define trim_line_nl()      while (q>output && is_space_nl(*(q-1))) *q--;
#define put_newline()       { trim_line(); *q++ = 0xD; allocated_length--; line = 0; }

    CHAR  *p, *output, *q;
    uint i, j, line;
		size_t allocated_length;
    // init
    if (!length) return NULL;
    if (!linelen) linelen = (uint)-1;
		 
		allocated_length = length + length/linelen;

    if ((output=(CHAR *)MikMod_malloc(allochandle, allocated_length + 1))==NULL)
        return NULL;
    // process buffer
    for (i=0, line=0, p=string, q=output; i<length && allocated_length; i++, p++)
    {
        switch (*p)
        {
        // tab
        case 0x9:
            j = 8;
            while (j-- && allocated_length)
            {
                *q++ = ' ';
                line++;
								allocated_length--;
            }
            break;
        // LF
        case 0xA:
            if (i && *(p-1)!=0xD)
                put_newline();
            break;
        // CR
        case 0xD:
            put_newline();
            break;
        // others
        default:
            if ((UBYTE)*p < 20u)
                *q++ = ' ';
            else *q++ = *p;
						allocated_length--;
            line++;
            if (line >= linelen)
                put_newline();
        }
    }
    // finish up
    trim_line_nl();
    if (q == output)                                        // empty comment
    {
        MikMod_free(allochandle, output);
        output = NULL;
    }
    else *q = 0;

    return output;

#undef is_space
#undef is_space_nl
#undef trim_line
#undef trim_line_nl
#undef put_newline
}


// =====================================================================================
    BOOL AllocPositions(UNIMOD *of, uint32_t total)
// =====================================================================================
{
    assert(of != NULL);
//    assert(total);

    if((of->positions=(UWORD*)MikMod_calloc(of->allochandle, total, sizeof(UWORD))) == NULL)
        return 0;
    return 1;
}


// =====================================================================================
    BOOL AllocPatterns(UNIMOD *of)
// =====================================================================================
{
    uint t;
    assert(of != NULL);
    assert(of->numpat && of->numchn);

    // Allocate track sequencing array

    if ((of->patterns  =(UWORD*)MikMod_calloc(of->allochandle, (ULONG)(of->numpat+2)*of->numchn, sizeof(UWORD))) == NULL)
        return 0;
    if ((of->pattrows  =(UWORD*)MikMod_calloc(of->allochandle, of->numpat+2, sizeof(UWORD))) == NULL)
        return 0;
    if ((of->globtracks=(UBYTE**)MikMod_calloc(of->allochandle, of->numpat+2, sizeof(UBYTE*))) == NULL)
        return 0;

    for (t=0; t<of->numpat+1; t++)
        of->pattrows[t] = 64;

    return 1;
}


// =====================================================================================
    BOOL AllocTracks(UNIMOD *of)
// =====================================================================================
{
    assert(of != NULL);
    assert(of->numtrk);

    if((of->tracks=(UBYTE**)MikMod_calloc(of->allochandle, of->numtrk+2, sizeof(UBYTE*))) == NULL)
        return 0;
    return 1;
}


// =====================================================================================
    BOOL AllocInstruments(UNIMOD *of)
// =====================================================================================
{
    uint t;
    assert(of != NULL);
    assert(of->numins);

    if ((of->instruments=(INSTRUMENT*)MikMod_calloc(of->allochandle, of->numins, sizeof(INSTRUMENT)))==NULL)
        return 0;

    for (t=0; t<of->numins; t++)
    {
        uint n;
        // init note / sample lookup table
        for (n=0; n<120; n++)
        {
            of->instruments[t].samplenote[n]   = n;
            of->instruments[t].samplenumber[n] = t;
        }
        // other default values
        of->instruments[t].globvol = 64;
    }

    return 1;
}


// =====================================================================================
    BOOL AllocSamples(UNIMOD *of, BOOL ext)
// =====================================================================================
// Allocates memory for of->numsmp number of samples.  if 'ext' is true, additional extended
// sample information is allocated.
{
    UWORD u;
    assert(of != NULL);
    assert(of->numsmp);

    if (ext)
    {
        if ((of->extsamples=(EXTSAMPLE*)MikMod_calloc(of->allochandle, of->numsmp, sizeof(EXTSAMPLE))) == NULL)
            return 0;
//CUT         of->flags |= UF_EXTSAMPLES;
    }

    if ((of->samples=(UNISAMPLE*)MikMod_calloc(of->allochandle, of->numsmp, sizeof(UNISAMPLE))) == NULL)
        return 0;

    for (u=0; u<of->numsmp; u++)
    {
        of->samples[u].volume  = 128;
        of->samples[u].panning = PAN_CENTER;
        of->samples[u].handle  = -1;
//TEST        of->samples[u].cutoff     = 64;
        if(ext) of->extsamples[u].globvol = 64;
    }

    return 1;
}


// =====================================================================================
    void pt_write_exx(UTRK_WRITER *ut, uint eff, uint dat)
// =====================================================================================
// ** Protracker Standard Effects Processor **
// These are used by almost every loader since every module at least bears
// some similarity to PT in one effect or another.
{
    UNITRK_EFFECT  effdat;
    BOOL globeffect = 0;

    if (eff == 0) return;
    effdat.effect   = 0;
    effdat.framedly = UFD_RUNONCE;

    switch(eff)
    {   case 0x1:                  // Fineslide up
            effdat.param.s  = dat << 3;
            effdat.effect   = UNI_PITCHSLIDE;
        break;

        case 0x2:                  // Fineslide down
            effdat.param.s  = 0 - (dat << 3);
            effdat.effect   = UNI_PITCHSLIDE;
        break;

        //case 0x3:                  // Glissando control (not supported).
            //effdat.param.u = dat;
            //effdat.effect  = UNI_GLISSANDO_CTRL;
        //break;

        case 0x4:                  // set vibrato waveform
            effdat.param.u  = dat;
            effdat.effect   = UNI_VIBRATO_WAVEFORM;
        break;

        case 0x5:                  // Set finetune
            effdat.param.u = finetune[dat];
            effdat.effect  = UNI_SETSPEED;
        break;

        case 0x6:                  // Pattern loop (now global effect)
            effdat.param.u  = dat | LOOP_PATTERNSCOPE;
            effdat.effect   = dat ? UNI_GLOB_LOOP : UNI_GLOB_LOOPSET;
            globeffect      = 1;
        break;

        case 0x7:                  // Set Tremolo waveform
            effdat.param.u  = dat;
            effdat.effect   = UNI_TREMOLO_WAVEFORM;
        break;

        case 0x8:                  // channel panning (dmp?)
            effdat.param.s  = (dat<=8 ? dat*16 : dat*17) + PAN_LEFT;
            effdat.effect   = UNI_PANNING;
        break;

        case 0x9:                  // Retrigger with no volume modifier.
            effdat.param.loword.s  = dat;
            effdat.param.hiword.u  = 0;
            effdat.effect   = UNI_RETRIG;
            effdat.framedly = 0;
        break;

        case 0xa:                  // fine volume slide up
            effdat.param.s  = dat*2;
            effdat.effect   = UNI_VOLSLIDE;
        break;

        case 0xb:                  // fine volume slide dn
            effdat.param.s  = 0 - (dat*2);
            effdat.effect   = UNI_VOLSLIDE;
        break;

        case 0xc:                  // Note Cut
            effdat.framedly = dat;
            effdat.effect   = UNI_VOLUME;
            effdat.param.u  = 0;
        break;

        case 0xd:                  // note delay
            if (dat)
            {
                // on
                effdat.param.u  = 1;
                effdat.effect   = UNI_NOTEDELAY;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);
                // off
                effdat.param.u  = 0;
                effdat.framedly = UFD_RUNONCE | dat;
            }
        break;

        case 0xe:                  // pattern delay
            effdat.param.hiword.u = dat;
            effdat.param.loword.u = 0;
            effdat.effect         = UNI_GLOB_DELAY;
            globeffect = 1;
        break;
    }

    if (effdat.effect)
    {
        if (globeffect)
            utrk_write_global(ut, &effdat, UNIMEM_NONE);
        else
            utrk_write_local(ut, &effdat, UNIMEM_NONE);
    }
}


// =====================================================================================
    void pt_write_effect(UTRK_WRITER *ut, uint eff, uint dat)
// =====================================================================================
// Translates Protracker effects into Unimod effects.  Works for all standard PT effects
// 0 through f (hex) except Exx extended commands.  For those, use pt_write_exx (above).
{
    UNITRK_EFFECT  effdat;
    uint           hi = dat>>4, lo = dat & 15;

    effdat.framedly = 0;
    effdat.param.u  = 0;
    
    if(eff!=0 || dat!=0)                // don't write empty effect
    {   // global effects get special treatments now.
        switch(eff)
        {   case 0:                     // arpeggio!
                effdat.param.byte_a = lo;
                effdat.param.byte_b = hi;
                effdat.param.byte_d = 3;
                effdat.effect  = UNI_ARPEGGIO;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);
            break;

            case 0x1:                   // pitch slide up
                if(dat)
                {   effdat.param.s  = dat<<3;
                    effdat.effect   = UNI_PITCHSLIDE;
                    effdat.framedly = 1;
                    utrk_write_local(ut, &effdat, PTMEM_PITCHSLIDEUP);
                } else utrk_memory_local(ut, &effdat, PTMEM_PITCHSLIDEUP, 0);
            break;

            case 0x2:                   // pitch slide down
                if(dat)
                {   effdat.param.s  = 0 - (dat<<3);
                    effdat.effect   = UNI_PITCHSLIDE;
                    effdat.framedly = 1;
                    utrk_write_local(ut, &effdat, PTMEM_PITCHSLIDEDN);
                } else utrk_memory_local(ut, &effdat, PTMEM_PITCHSLIDEDN, 0);
            break;

            case 0x3:                   // Portamento to Note
                effdat.effect   = UNI_PORTAMENTO_LEGACY;
                if(dat)
                {   effdat.param.u  = dat*8;
                    utrk_write_local(ut, &effdat, PTMEM_PORTAMENTO);
                } else utrk_memory_local(ut, &effdat, PTMEM_PORTAMENTO, 0);
            break;

            case 0x4:                    // Vibrato
                if(lo)
                {   effdat.param.u   = lo*16;
                    effdat.effect    = UNI_VIBRATO_DEPTH;
                    utrk_write_local(ut, &effdat, PTMEM_VIBRATO_DEPTH);
                } else utrk_memory_local(ut, NULL, PTMEM_VIBRATO_DEPTH, 0);

                if(hi)
                {   effdat.param.u   = hi*4;
                    effdat.framedly  = 1;
                    effdat.effect    = UNI_VIBRATO_SPEED;
                    utrk_write_local(ut, &effdat, PTMEM_VIBRATO_SPEED);
                } else utrk_memory_local(ut, NULL, PTMEM_VIBRATO_SPEED, 0);
            break;
            
            case 0x5:                    // Portamento + Volume Slide
                // Note: Make sure we use the 'net' slide speed
                // (hi - lo) because protracker works that way.
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);

                effdat.effect    = UNI_PORTAMENTO_LEGACY;
                utrk_memory_local(ut, &effdat, PTMEM_PORTAMENTO, 0);
            break;

            case 0x6:                    // Vibrato + Volume Slide
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);

                utrk_memory_local(ut, NULL, PTMEM_VIBRATO_DEPTH, 0);
                utrk_memory_local(ut, NULL, PTMEM_VIBRATO_SPEED, 0);
            break;

            case 0x7:                    // Tremolo!
                if(lo)
                {   effdat.param.u  = lo * 8;
                    effdat.effect   = UNI_TREMOLO_DEPTH;
                    utrk_write_local(ut, &effdat, PTMEM_TREMOLO_DEPTH);
                } else utrk_memory_local(ut, NULL, PTMEM_TREMOLO_SPEED, 0);

                if(hi)
                {   effdat.param.u  = hi * 4;
                    effdat.effect   = UNI_TREMOLO_SPEED;
                    effdat.framedly = 1;
                    utrk_write_local(ut, &effdat, PTMEM_TREMOLO_SPEED);
                } else utrk_memory_local(ut, NULL, PTMEM_TREMOLO_SPEED, 0);
            break;

            // this effect was introduced by DMP and has introduced
            // more problems, of course. DMP panning range is 00-80
            // (and A4 for "surround"), but FT2 uses 00-FF range for
            // both XMs and MODs. Other players do whatever they want.
            case 0x8:                    // OttO Panning!
                effdat.param.s  = dat + PAN_LEFT;
                if (effdat.param.s > PAN_RIGHT)
                    effdat.param.s = PAN_SURROUND;
                effdat.effect   = UNI_PANNING;
                effdat.framedly = UFD_RUNONCE;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);
            break;

            case 0x9:                    // Sample Offset
                if(dat)
                {   effdat.effect          = UNI_OFFSET_LEGACY;
                    effdat.framedly        = UFD_RUNONCE;
                    effdat.param.loword.u  = dat*256;
                     utrk_write_local(ut, &effdat, PTMEM_OFFSET);
                } else utrk_memory_local(ut, &effdat, PTMEM_OFFSET, 0);
            break;

            case 0xa:                    // Volume Slide
                // Note: Make sure we use the 'net' slide speed
                // (hi - lo) because protracker works that way.
                effdat.effect    = UNI_VOLSLIDE;
                effdat.framedly  = 1;
                effdat.param.s   = (hi - lo) << 1;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);
            break;

            case 0xb:                    // Pattern jump
                effdat.param.u = dat;
                effdat.effect  = UNI_GLOB_PATJUMP;
                utrk_write_global(ut, &effdat, UNIMEM_NONE);
            break;

            case 0xc:                     // set volume
                effdat.param.u  = (dat > 64) ? VOLUME_FULL : (dat << 1);
                effdat.effect   = UNI_VOLUME;
                effdat.framedly = UFD_RUNONCE;
                utrk_write_local(ut, &effdat, UNIMEM_NONE);
            break;

            case 0xd:                     // Pattern break
                effdat.param.u = dat;
                effdat.effect  = UNI_GLOB_PATBREAK;
                utrk_write_global(ut, &effdat, UNIMEM_NONE);
            break;

            case 0xe:
                pt_write_exx(ut,hi,lo);
            break;

            case 0xf:                     // Set Speed / BPM
                if(dat)
                {
                    effdat.param.u  = dat;
                    effdat.framedly = UFD_RUNONCE;
                    effdat.effect   = dat > 0x20 ? UNI_GLOB_TEMPO : UNI_GLOB_SPEED;
                    utrk_write_global(ut, &effdat, PTMEM_TEMPO);
                }
                else utrk_memory_global(ut, &effdat, PTMEM_TEMPO);
            break;
        }
    }
}


// =====================================================================================
    void pt_global_consolidate(UNIMOD *of, UBYTE **globtrack)
// =====================================================================================
// Copies the effects from the current read global track to the current write global
// track.  Used for modules that store their formats in track format (not pattern-format
// like mods/it/s3m).
{
    uint         u;
    uint         t;
    UBYTE        *urow[64];

    // build the row indexes for this pattern

    int   temp = of->ut->unichn;
    of->ut->unichn = 0;

    for(t=0; t<of->numpat; t++)
    {   uint sigh;
        utrk_reset(of->ut);
        for(u=0; u<of->numchn; u++)
            urow[u] = globtrack[of->patterns[(t*of->numchn)+u]];

        // now consolidate all global tracks into the main one.
        for(u=0; u<64; u++)
        {   for(sigh=0; sigh<of->numchn; sigh++)
                urow[sigh] = utrk_global_copy(of->ut, urow[sigh], sigh);
            utrk_newline(of->ut);
        }
        of->globtracks[t] = utrk_dup_global(of->ut, of->allochandle);
    }
    of->ut->unichn = temp;
}



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
 Module: unimod.c
 
  Structure handling and manipulation.  Includes loading, freeing, and track
  manipulation functions for the UNIMOD format.

 Portability:
  All compilers - All Systems.

*/

#include <string.h>
#include <stdarg.h>
#include "mikmod.h"
#include "uniform.h"


#define MAX_MEM_DEFS    64          // maximum memory slots defaults



// =====================================================================================
    static BOOL loadsamples(MDRIVER *md, UNIMOD *of, MMSTREAM *fp)
// =====================================================================================
{
    UNISAMPLE  *s;
    int         u;

    for(u=of->numsmp, s=of->samples; u; u--, s++)
        if(s->length)
        {   
            // if the seekpos is 0, then we need to set it automatically, assuming that each
            // sample follows suit from the current file seek position

            SL_RegisterSample(md, &s->handle, s->format, s->length, s->compress, fp, s->seekpos);
        }

    return 1;
}


static void pt_set_memdefs(UNIMOD *mf)
{
    mf->memsize = PTMEM_LAST;
}

/******************************************

    Next are the user-callable functions

******************************************/


// =====================================================================================
    void Unimod_UnloadSamples(UNIMOD *mf)
// =====================================================================================
// Unloads all samples,a nd sets the sample handles to -1.
{
    if(mf && mf->md && mf->samples)
    {
        uint t;

        for (t=0; t<mf->numsmp; t++)
        {
            if (mf->samples[t].handle >= 0)
            {
                MD_SampleUnload(mf->md, mf->samples[t].handle);
                mf->samples[t].handle = -1;
            }
        }
    }
}


// =====================================================================================
    static void ML_Shutdown(UNIMOD *mf)
// =====================================================================================
// Before a proper shutdown of a loaded module, we have to unload the samples from the
// driver.  They are not allocated as part of our unimod allocation tree, hence:
{
    if(mf && mf->samples)
    {   uint t;
        for(t=0; t<mf->numsmp; t++)
            MD_SampleUnload(mf->md, mf->samples[t].handle);
    }
}


// =====================================================================================
    void Unimod_Free(UNIMOD *mf)
// =====================================================================================
{
    if(!mf) return;
    _mmlog("Unimod > Unloading module \'%s\'", mf->songname);
    _mmalloc_close(mf->allochandle);
}

extern MLOADER *firstloader;

#ifndef _MM_WINAMP_

// =====================================================================================
    CHAR *Unimod_LoadTitle(CHAR *filename)
// =====================================================================================
{
    MLOADER  *l;
    CHAR     *retval;
    MMSTREAM *fp;

    if ((fp=_mm_fopen(filename, "rb")) == NULL)
        return NULL;

    // Try to find a loader that recognizes the module
    for (l=firstloader; l; l=l->next)
    {
        _mm_rewind(fp);
        if (l->enabled && l->Test(fp)) break;
    }

    if (fp == NULL)
    {
        _mmerr_set(MMERR_UNSUPPORTED_FILE, "Corrupt file or unsupported module type.");
        return NULL;
    }

    retval = l->LoadTitle(fp);
    _mm_fclose(fp);
    return retval;
}

#endif

// =====================================================================================

static void SetModuleDefaults(UNIMOD *mf, MLOADER *l)
{
    uint t;

    mf->initvolume = 128;

    // init panning array
    for(t=0; t<64; t++)
        mf->panning[t] = ((t+1)&2) ? (PAN_RIGHT-l->defpan) : (PAN_LEFT+l->defpan);

    // default channel volume set to max...
    for(t=0; t<64; t++)
        mf->chanvol[t] = 64;

    // all channels are unmuted.
    memset(mf->muted, 0, sizeof(mf->muted));

    // memslots defaults
    mf->memdefs = MikMod_calloc(mf->allochandle, MAX_MEM_DEFS, sizeof(UE_EFFECT));
    pt_set_memdefs(mf);
}

#define CheckEnvelope(envl)                                     \
if (i->envl##pts)                                               \
{                                                               \
    int j;                                                      \
                                                                \
    for (j=1; j<i->envl##pts; j++)                              \
        if (i->envl##env[j].pos <= i->envl##env[j-1].pos)       \
        {                                                       \
            i->envl##pts = j;                                   \
            break;                                              \
        }                                                       \
                                                                \
    if (i->envl##beg>i->envl##end || i->envl##beg>i->envl##pts) \
        i->envl##flg &= ~EF_LOOP;                               \
    if (i->envl##end > i->envl##pts)                            \
        i->envl##end = i->envl##pts;                            \
                                                                \
    if (i->envl##susbeg>i->envl##susend || i->envl##susbeg>i->envl##pts)    \
        i->envl##flg &= ~EF_SUSTAIN;                            \
    if (i->envl##susend > i->envl##pts)                         \
        i->envl##susend = i->envl##pts;                         \
}

//
//  We do not want our player thingie to crash, do we?
//  so we check module's data here. we try to fix everything
//  we can (and give up in very rare cases).
//

static BOOL CheckModule(UNIMOD *mf, MMSTREAM *smpfp)
{
    // Check the number of positions (song order list).  If there are
    // none, then we generate a 'dud' list which comprises of all the
    // patterns of the song, in order.

    if (!mf->numpos)
    {
        uint   k;

        AllocPositions(mf, mf->numpos = mf->numpat);

        for (k=0; k<mf->numpat; k++)
            mf->positions[k] = k;
    }
    else
    {
        // Make sure there aren't any illegal positions.
        // if anything invalid will be found, the song will be stopped.
        // if more smart processing is required, it should be done in
        // the particular loader.
        uint  k;        
        
        for (k=0; k<mf->numpos; k++)
            if (mf->positions[k] >= mf->numpat)
            {
                mf->numpos = k;
                break;
            }
    }

    //
    //  check for incorrect tracks
    //  and replace them with the empty one
    //

    {
        const uint trcks = mf->numtrk;
        int        dummy = -1;
        uint       t;

        for (t=0; t<mf->numpat*mf->numchn; t++)
            if (mf->patterns[t] >= trcks)
            {
                // find empty track
                if (dummy < 0)
                {
                    uint i;
                    // search in already-created tracks
                    for (i=0; i<trcks; i++)
                        if (mf->tracks[i] == utrk_blanktrack)
                            break;
                    // not found - create new
                    if (i == trcks)
                    {
                        UBYTE **data = _mm_realloc(mf->allochandle, mf->tracks, (mf->numtrk+1)*sizeof(mf->tracks[0]));
                        if (data == NULL)
                            return FALSE;

                        mf->tracks = data;
                        mf->tracks[i] = utrk_blanktrack;
                        mf->numtrk++;
                    }
                    // remember
                    dummy = i;
                }
                // replace
                mf->patterns[t] = dummy;
            }
    }

    //
    //  check samples
    //

    if (mf->numsmp)
    {
        UNISAMPLE *s = mf->samples;
        ULONG      fsize = smpfp ? _mm_fsize(smpfp) : -1;
        uint       t;

        for (t=0; t<mf->numsmp; t++, s++)
        {
            // totally invalid samples
            if ((ULONG)s->seekpos >= fsize ||                   // out of file
                (!s->compress && s->seekpos+s->length>fsize) || // out of file (again)
                !(mf->flags&UF_XMPERIODS || s->speed))          // speed, eh?
            {
                s->length  = 0;
                s->seekpos = 0;
                continue;
            }
            // invalid values
            if (s->length)
            {
                // loops
                if (s->flags & SL_LOOP)
                {
                    if (s->loopstart>=s->length || s->loopstart>=s->loopend)
                        s->flags &= ~SL_LOOP;
                    else if (s->loopend > s->length)
                        s->loopend = s->length;
                }
                if (s->flags & SL_SUSTAIN_LOOP)
                {
                    if (s->susbegin>=s->length || s->susbegin>=s->susend)
                        s->flags &= ~SL_SUSTAIN_LOOP;
                    else if (s->susend > s->length)
                        s->susend = s->length;
                }
                // others
                s->volume  = _mm_boundscheck(s->volume, 0, VOLUME_FULL);
                s->panning = _mm_boundscheck(s->panning, PAN_LEFT, PAN_RIGHT);
            }
        }
    }

    //
    //  check instruments stuff
    //

    if (mf->numins)
    {
        INSTRUMENT *i = mf->instruments;
        uint       t;

        for (t=0; t<mf->numins; t++, i++)
        {
            CheckEnvelope(vol);
            CheckEnvelope(pan);
            CheckEnvelope(pit);
            CheckEnvelope(res);
        }
    }

    return TRUE;
}

#undef CheckEnvelope

// =====================================================================================
    static UNIMOD *Unimod_Load_Internal(MDRIVER *md, MMSTREAM *modfp, MMSTREAM *smpfp, int mode)
// =====================================================================================
// Loads a module given a file pointer.  Useable for situations that involve packed file
// formats - a single file that contains all module data, or UniMod modules which use the
// sample library feature.
//
// - Songs and samples are loaded from the file positions specified.
// - mode specifies the module as MM_STATIC or MM_DYNAMIC
{
    MLOADER     *l;
    BOOL         ok = FALSE;
    UNIMOD      *mf;
    ML_HANDLE   *lh;

    _MM_ALLOCATE(mf, UNIMOD, NULL);
    _mmalloc_setshutdown(mf->allochandle, ML_Shutdown, mf);

    // Try to find a loader that recognizes the module
    for (l=firstloader; l; l=l->next)
    {
        _mm_rewind(modfp);
        if (l->enabled && l->Test(modfp)) break;
    }

    if (l == NULL)
    {
        Unimod_Free(mf);
        return NULL;
    }

    // set UNIMOD defaults
    SetModuleDefaults(mf, l);

    // init module loader and load the header / patterns
    if (lh = l->Init())
    {
        mf->filesize = _mm_fsize(modfp);
        _mm_rewind(modfp);
        ok = l->Load(lh, mf, modfp);
        // free loader and unitrk allocations
        mf->numtrk = utrk_cleanup(mf->ut);
        l->Cleanup(lh);
    }

    // check
    if (!ok || !CheckModule(mf, smpfp))
    {
        Unimod_Free(mf);
        return NULL;
    }

    mf->md = md;
    if (l->nopaneff) mf->flags |= UF_NO_PANNING;
    if (l->noreseff) mf->flags |= UF_NO_RESONANCE;

    return mf;
}


// =====================================================================================
    void Unimod_SetDefaultPan(int defpan)
// =====================================================================================
// Sets the default panning position for all registered loaders.
{
    MLOADER *l;

    for (l=firstloader; l; l=l->next)
        l->defpan = defpan;
}


// =====================================================================================
    BOOL Unimod_LoadSamples(UNIMOD *mf, MDRIVER *md, MMSTREAM *smpfp)
// =====================================================================================
// Returns FALSE on error, TRUE on success!
{
    if (mf && md && smpfp)
    {
        if (!loadsamples(md, mf, smpfp))
            return FALSE;
        if (!SL_LoadSamples(md))
            return FALSE;
        mf->md = md;
    }

    return TRUE;
}

// =====================================================================================
MMEXPORT UNIMOD  *Unimod_Load_FP(MDRIVER *md, const CHAR *filename,MMSTREAM * fp)
// =====================================================================================
{
    UNIMOD  *mf;
    
    _mmlogd1("Mikmod > unimod_load > Loading Module : %s", filename);

    if ((mf = Unimod_Load_Internal(md, fp, fp, MM_STATIC)) != NULL)
    {
        if (!Unimod_LoadSamples(mf, md, fp))
        {
            Unimod_Free(mf);
            mf = NULL;
        }
        else
        {
            mf->filename = _mm_strdup(mf->allochandle, filename);
        }
    }

    if (!mf)
    {
        _mmlog("Unimod > Module load failed : %s",filename);
        _mmerr_set(MMERR_UNSUPPORTED_FILE, "Corrupt file or unsupported module type.");
    }

    return mf;
}

// =====================================================================================
MMEXPORT UNIMOD  *Unimod_LoadInfo_FP(const CHAR *filename,MMSTREAM * fp)
// =====================================================================================
{
	UNIMOD    *mf;
    if (mf = Unimod_Load_Internal(NULL, fp, NULL, MM_STATIC))
        mf->filename = _mm_strdup(mf->allochandle, filename);
	return mf;
}

// =====================================================================================
    UNIMOD *Unimod_LoadInfo(const CHAR *filename)
// =====================================================================================
// Open a module via it's filename and loads only the header information.
{
    MMSTREAM  *fp;
    UNIMOD    *mf;
    
    if ((fp = _mm_fopen(filename,"rb")) == NULL)
        return NULL;

	mf = Unimod_LoadInfo_FP(filename, fp);

    _mm_fclose(fp);
    
    return mf;
}


// =====================================================================================
    UNIMOD *Unimod_Load(MDRIVER *md, const CHAR *filename)
// =====================================================================================
// Open a module via it's filename.  This is fairly automated song loader,
// which does not support fancy things like sample libraries (for shared
// samples and instruments).  See song_loadfp for those.
//
// In addition, this puppy will also load the samples automatically, and it
// sets the filesize.
{
    MMSTREAM *fp;
    UNIMOD  *mf;
    
    if ((fp = _mm_fopen(filename,"rb")) == NULL)
        return NULL;

	mf = Unimod_Load_FP(md,filename,fp);

    _mm_fclose(fp);

    return mf;
}



// =====================================================================================
    BOOL __inline MF_PatternInuse(UNIMOD *mf, uint pattern)
// =====================================================================================
// Returns TRUE for a pattern that has any sort of pattern data in it.  Global track 
// contents are considered during the check.
// TODO: add a flag so that I can optionally include the global track in the check.
{

    if(pattern < mf->numpat)
    {
        uint   i = (pattern*mf->numchn),
               t = i+mf->numchn;

        uint   longlen;

        longlen = 0;
        for(; i<t; i++)
        {   // find the length of the longest track, in rows.
            uint  res = utrk_local_getlength(mf->tracks[mf->patterns[i]]);
            if(res > longlen) longlen = res;
        }
        if(longlen)
        {   if(longlen<mf->pattrows[pattern])
                mf->patpos_silence = longlen;
            return TRUE;
        }
    }

    return FALSE;
}


// =====================================================================================
    void Unimod_StripSilence(UNIMOD *mf, long threshold)
// =====================================================================================
// strips trailing silence from the end of a song, by searching backwards through the
// order list and decrementing the order count as long as they are completely empty.
// Threshold is the time, in milliseconds.  IF ZERO, no stripping will occur!
{

    uint   i;
    uint   numpos = mf->numpos;

    for (i=mf->numpos; i; i--)
        if (!MF_PatternInuse(mf, mf->positions[i-1]))
            numpos--;
        else break;

    if (numpos < mf->numpos)
    {
        mf->sngpos_silence  = numpos;
        mf->strip_threshold = threshold;
    }
}

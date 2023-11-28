/*

 MikMod for Winamp

  By Jake Stine of Divine Entertainment (1998-2000)

 Support:
  If you find problems with this code, send mail to:
    air@divent.org

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------
 Module: snglen.c

  Preprocesses a song and gets the length.  Also can preprocess a song and
  build a lookup table to be used for advanced seeking.

  This is a Mikmod 'Module Player Extension.'   It is aware of and requires
  the MPLAYER.C module.  When calling the array of procedures contained herein,
  pass them the MPLAYER handle struct that was given to you via Player_InitSong.

*/

#include "mikmod.h"
#include "mplayer.h"
#include <memory.h>
#include "mpforbid.h"


// =====================================================================================
    static void SaveState(MPLAYER *destps, const MPLAYER *ps)
// =====================================================================================
{
    assert(ps && destps);

    _mmlogd1("Preprocess Saving State : %d",ps->state.sngpos);

    // check buffer length

    if(destps->statecount >= destps->pms_alloc)
    {
        // reallocate for more statecounts (+16 at each realloc).
        destps->statelist = (MP_STATE *)_mm_realloc(destps->allochandle, destps->statelist, (destps->pms_alloc+16) * sizeof(MP_STATE));
        memset(&destps->statelist[destps->pms_alloc],0,16*sizeof(MP_STATE));
        destps->pms_alloc += 16;
    }

    destps->statelist[destps->statecount]         = ps->state;
    destps->statelist[destps->statecount].voice   = NULL;
    destps->statelist[destps->statecount].control = NULL;

    // spawn a copy of the posplayed array.

    if((destps->statelist[destps->statecount].pos_played = (ULONG **)MikMod_calloc(destps->allochandle, ps->mf->numpos,sizeof(ULONG *))) == NULL) return;
    
    {
        uint    i;
        uint    bufsize = 0;
        ULONG  *thebuffer;
        
        for (i=0; i<ps->mf->numpos; i++)
            bufsize += (ps->mf->pattrows[ps->mf->positions[i]] / 32) + 1;

        if ((thebuffer = (ULONG *)MikMod_calloc(destps->allochandle, bufsize, sizeof(ULONG *))) == NULL)
            return;
        
        bufsize = 0;
        for(i=0; i<ps->mf->numpos; i++)
        {
            uint  t;

            destps->statelist[destps->statecount].pos_played[i] = &thebuffer[bufsize];
            t = ((ps->mf->pattrows[ps->mf->positions[i]]/32)+1);
            memcpy(destps->statelist[destps->statecount].pos_played[i], ps->state.pos_played[i], t * sizeof(ULONG));
            bufsize += t;
        }
    }

    destps->statecount++;
}


// =====================================================================================
    void Player_PredictSongLength(MPLAYER *ps)
// =====================================================================================
{
    MPLAYER  *preps;
    
    if(ps->mf->numpos == 0) { ps->songlen = 0; return; }
    
    // Concepts of the Code (By Jake Stine, the Coder!):
    // We create a MPLAYER struct based on the given song, and run a quick
    // and dirty 'simulation play' of it to calculate the length of the song.
    // Only global effects are processed, since those are the only effects
    // that can have an impact on the time it takes a song to play.  This
    // speeds up the calculation time drastically (to nearly instantaneous!)

    if ((preps = Player_Dup(ps)) == NULL)
        return;
    Player_Cleaner(preps);

    while(!preps->ended) Player_PreProcessRow(preps, NULL);

    ps->songlen = preps->state.curtime/64;
    Player_Free(preps);
}


// =====================================================================================
    uint Player_BuildQuickLookups(MPLAYER *ps)
// =====================================================================================
{
    MPLAYER  *preps;

    if (!ps->mf->numpos)
    {
        ps->songlen = 0;
        return 0;
    }

    // Saved-playerstate-related Hoopla and Jazz.
    // ------------------------------------------

    MikMod_free(ps->allochandle, ps->statelist);

    ps->statecount  = 0;
    ps->pms_alloc   = 0;

    // Concepts of the Code (By Jake Stine, the Coder!):
    // The only difference between this function and PredictSongLength is
    // that this procedure asks PreProcessRow to save fast-seek states.
    // Fast-seek states are instances of the MPLAYER struct saved at the
    // start of every pattern, indexed by the time when it is played.

    if ((preps = Player_Dup(ps)) == NULL)
        return 1;
    Player_Cleaner(preps);

    while(!preps->ended) Player_PreProcessRow(preps, ps);

    ps->songlen = preps->state.curtime/64;
    Player_Free(preps);

    return 0;
}


// =====================================================================================
    void Player_Restore(MPLAYER *ps, uint stateidx)
// =====================================================================================
{    
    MP_VOICE    *oldvoice;
    MP_CONTROL  *oldcontrol;
    MP_STATE    *state;
    ULONG      **oldposplay;

    oldvoice   = ps->state.voice;
    oldcontrol = ps->state.control;
    oldposplay = ps->state.pos_played;

    state     = &ps->statelist[stateidx];
    ps->state = *state;

    ps->state.voice      = oldvoice;
    ps->state.control    = oldcontrol;
    ps->state.pos_played = oldposplay;

    // Copy the state's channel and voice control arrays over, if they happen to be in use.

    if(state->control)
        memcpy(ps->state.control, state->control, sizeof(MP_CONTROL) * ps->mf->numchn);

    if(state->voice)
        memcpy(ps->state.voice, state->voice, sizeof(MP_VOICE) * ps->numvoices);

    if(state->pos_played);
    {   uint  i;
        for(i=0; i<ps->mf->numpos; i++)
            memcpy(ps->state.pos_played[i], state->pos_played[i], ((ps->mf->pattrows[ps->mf->positions[i]]/32)+1) * sizeof(ULONG));
    }
}


// =====================================================================================
    void Player_FastSeekFromState(MPLAYER *ps, long time)
// =====================================================================================
// Seeks to a specific 'time' in the given player.  Due to the nature of modules, the
// seek will never be 100% true to the time requested; it will always be slightly more
// (depending on the speed and bpm of the song and the time given).
//
// Parameters:
//  - time is given in milliseconds.
//  - ps is the seek state to start seeking from.  This can be any position, although
//    ones *before* the time work better, and the pattern which contains the actual
//    target row works best.
{
    while(!ps->ended && (ps->state.posjmp || ps->state.curtime<time))
        Player_PreProcessRow(ps, NULL);

    if (ps->ended)
        return;

    MP_UnsetPosPlayed(ps);
    ps->state.posjmp = 3;
    ps->state.sngpos--;
    ps->state.patbrk = ps->state.patpos;
}


// =====================================================================================
    void Player_SetPosTime(MPLAYER *ps, long time)
// =====================================================================================
{
    ps_forbid();

    time *= 64;
    Player_WipeVoices(ps);

    if (time <= 0)
    {
        Player_Cleaner(ps);
        ps_unforbid();
        return;
    }

    if (ps->statelist != NULL)
    {
        int   t = 0;

        while (t<ps->statecount &&
               ps->statelist[t].curtime &&
               time>=ps->statelist[t].curtime)
               t++;

        if (t)
            Player_Restore(ps, t-1);
        else
        {
            Player_Cleaner(ps);
            ps_unforbid();
            return;
        }
    }
    else Player_Cleaner(ps);
    
    Player_FastSeekFromState(ps, time);

    ps_unforbid();
}

// =====================================================================================
    long Player_GetPosTime(MPLAYER *ps)
// =====================================================================================
{
    if (ps->statelist != NULL)
    {
        SWORD    pos = ps->state.sngpos;
        UWORD    patpos = ps->state.patpos;
        MPLAYER *tmp;
        int      t = 0;
        long     time;

        while (t<ps->statecount &&
               ps->statelist[t].curtime &&
               ps->statelist[t].sngpos<=pos)
               t++;

        if (!t) return 0;

        // duplicate player
        if ((tmp = Player_Dup(ps)) == NULL)
            return 0;
        tmp->statelist  = ps->statelist;
        tmp->statecount = ps->statecount;

        // seek
        Player_Restore(tmp, t-1);
        while(!tmp->ended && (tmp->state.sngpos<=pos && tmp->state.patpos<patpos))
            Player_PreProcessRow(tmp, NULL);

        // clean up
        time = tmp->state.curtime;
        Player_Free(tmp);
        return time;
    }
    else return 0;
}

extern UNITRK_EFFECT *global_geteffect(MPLAYER *ps, UNITRK_EFFECT *effdat, MP_CONTROL **a);

// =====================================================================================
    static void process_global_effects(MPLAYER *ps)
// =====================================================================================
{
    MP_CONTROL      *a;      // this puppy gets modified by global_geteffect

    UNITRK_EFFECT   *eff, reteff;
    INT_MOB          dat;
    int              lo, numframes;

    while(eff = global_geteffect(ps, &reteff, &a))
    {   // Universal Framedelay!  If the RUNONCE flag is set, then the command is
        // executed once on the specified tick, otherwise, the command is simply
        // delayed for the number of ticks specified.

        if(eff->framedly & UFD_RUNONCE)
            numframes = 1;
        else
        {   int  woops;
            woops = eff->framedly & UFD_TICKMASK;
            if(woops >= ps->state.sngspd)
                numframes = 1;
            else
                numframes = ps->state.sngspd - woops;
        }

        dat = eff->param;

        switch(eff->effect)
        {   case UNI_GLOB_VOLUME:
                ps->state.volume = dat.u;
            break;

            case UNI_GLOB_VOLSLIDE:
                ps->state.volume += dat.s * numframes;
                ps->state.volume = _mm_boundscheck(ps->state.volume,0,GLOBVOL_FULL);
            break;

            case UNI_GLOB_TEMPO:
                if(ps->state.patdly2 || (dat.byte_a < MIN_TEMPO)) break;
                ps->state.bpm = dat.byte_a;
            break;

            case UNI_GLOB_TEMPOSLIDE:
                lo  = ps->state.bpm;
                lo += (dat.s * numframes);
                ps->state.bpm = _mm_boundscheck(lo, MIN_TEMPO, 0xff);
            break;

            case UNI_GLOB_SPEED:
                if(ps->state.patdly2) break;
                ps->state.sngspd = dat.byte_a;
            break;

            case UNI_GLOB_LOOPSET:       // set loop
                a->rep_patpos = ps->state.patpos;    // set reppos
                a->rep_sngpos = ps->state.sngpos;
            break;

            case UNI_GLOB_LOOP:          // execute loop
                // check if repcnt is already set...
                // Then set patloop flag to indicate to the patjump code it's time to loop.

                if(!a->pat_repcnt)
                {   a->pat_repcnt = dat.byte_a + 1;      // not yet looping, so set repcnt
                    ps->state.patloop++;
                    if(dat.u & LOOP_PATTERNSCOPE)
                    {   if(a->rep_sngpos != ps->state.sngpos)
                        {   a->rep_sngpos = ps->state.sngpos;
                            a->rep_patpos = 0;
                        }
                    }
                }

                if(--a->pat_repcnt)
                {   ps->state.patbrk = a->rep_patpos;
                    ps->state.posjmp = 3;
                    if(a->rep_sngpos != -1)
                        ps->state.posjmp += a->rep_sngpos - ps->state.sngpos;
                    ps->state.sngpos--;
                } else
                {   // Loop has ended, so decrement the patloop flag
                    if(ps->state.patloop) ps->state.patloop--;
                }
            break;

            case UNI_GLOB_DELAY:       // pattern delay
                if(!ps->state.patdly2)
                {   if(dat.hiword.u) ps->state.patdly   = dat.hiword.u + 1;
                    if(dat.loword.u) ps->state.framedly = dat.loword.u;
                }
            break;

            case UNI_GLOB_PATJUMP:
                // FT2, IT give preference to commands in higher channels.
                // Should PT do the same?  I hope so...

                if (ps->state.patdly2)  break;
                if (dat.loword.u > ps->mf->numpos)
                    dat.loword.u = ps->mf->numpos;

                ps->state.posjmp = 2;                       // 2 means we set the new position manually
                if (ps->state.prev_sngpos > ps->state.sngpos)   // remember minimal value
                    ps->state.prev_sngpos = ps->state.sngpos;
                ps->state.sngpos = dat.loword.u;
            break;

            case UNI_GLOB_PATBREAK:
                if (ps->state.patbrk || ps->state.patdly2) break;

                if (ps->state.sngpos+1u>=ps->mf->numpos || dat.u>=ps->mf->pattrows[ps->mf->positions[ps->state.sngpos+1]])
                    ps->state.patbrk = 0;
                else ps->state.patbrk = dat.u;

                if(!ps->state.posjmp) ps->state.posjmp = 3;
            break;
        }
    }
}


// =====================================================================================
    void Player_PreProcessRow(MPLAYER *ps, MPLAYER *destps)
// =====================================================================================
{
    static UNITRK_ROW grow;
    BOOL   saveState = FALSE;

    ps->state.patpos++;

    // process pattern-delay.  pf->patdly2 is the counter and pf->patdly
    // is the command memory.

    if(ps->state.patdly)
    {   ps->state.patdly2 = ps->state.patdly;
        ps->state.patdly  = 0;
    }

    if(ps->state.patdly2)
    {   // patterndelay active
        if(--ps->state.patdly2) ps->state.patpos--;    // so turn back pf->patpos by 1
    }

    // Do we have to get a new patternpointer ?
    //  (when pf->patpos reaches the pattern length or when
    //  a patternbreak is active)

    if(!ps->state.posjmp && (ps->state.patpos == ps->state.numrow)) ps->state.posjmp = 3;

    if(ps->state.posjmp)
    {
        int old_sngpos = ps->state.sngpos;

        ps->state.sngpos += ps->state.posjmp - 2;

        if (ps->state.sngpos >= (int)ps->mf->numpos)
        {   
            if(!(ps->flags & PF_LOOP))
            {
                ps->ended = TRUE;
                return;
            }

            MP_LoopSong(ps, ps->mf);
        }

        ps->state.patpos  = ps->state.patbrk;
        ps->state.patbrk  = ps->state.posjmp = 0;
        ps->state.numrow  = ps->mf->pattrows[ps->mf->positions[ps->state.sngpos]];

        if(ps->state.patpos > ps->state.numrow) ps->state.patpos = ps->state.numrow;
        if(ps->state.sngpos < 0)                ps->state.sngpos = ps->mf->numpos-1;

        // set our global track to the new position.

        {
            UBYTE *track = ps->mf->globtracks[ps->mf->positions[ps->state.sngpos]];
            uint   t = 0;

            while(*track && t<ps->state.patpos)
            {   track += track[0]*2;
                t++;
            }
            ps->state.globtrk_row = track;
            ps->state.globtrk_pos = 0;
        }

        // save the state AFTER each pattern or pattern jump
        // (as long as it's jumping to a new pattern)
        
        if (!ps->state.patloop && old_sngpos!=ps->state.sngpos)
            saveState = destps!=NULL;

    } else
    {
        // no position jump, so go to the next row in the current pattern.

        if(!ps->state.patdly2)
        {
            ps->state.globtrk_row += ps->state.globtrk_row[0]*2;
            ps->state.globtrk_pos  = 0;
        }
    }

    if(!ps->state.patdly2)
    {
        if(!ps->state.patloop)
        {
            if (MP_PosPlayed(ps))
            {
                // Woops, we have played this row before: 
                // Decrement loop counter and reset posplayed arrays.

                ps->state.looping--;

                if (ps->state.looping <= 0)
                {
                    // use our advanced "continue after loop" logic
                    if (ps->flags&PF_CONT_LOOP &&
                        ps->state.prev_sngpos<(int)ps->mf->numpos-1)
                    {
                        // go to next subsong and loop
                        ps->state.sngpos  = ps->state.prev_sngpos + 1;
                        ps->state.patpos  = 0;
                        ps->state.looping = ps->loopcount + 1;
                        ps->state.prev_sngpos = ps->mf->numpos;

                        if (MP_PosPlayed(ps))
                            goto _NO_LUCK_;
                        // reset internal fields
                        ps->state.patbrk  = 0;
                        ps->state.numrow  = ps->mf->pattrows[ps->mf->positions[ps->state.sngpos]];
                        // set our global track to the new position
                        ps->state.globtrk_row = ps->mf->globtracks[ps->mf->positions[ps->state.sngpos]];
                        ps->state.globtrk_pos = 0;
                    }
                    else
                    {
_NO_LUCK_:
                        ps->ended = TRUE;
                        return;
                    }
                }
                else MP_WipePosPlayed(ps);
            }
            
            MP_SetPosPlayed(ps);
        }
    }

    process_global_effects(ps);
    ps->state.curtime += (12500L * ps->state.sngspd * 64) / (ps->state.bpm * 5L);

    if (saveState)
        SaveState(destps, ps);

    if(ps->mf->strip_threshold)
    {
        // end-song stripping logic
        // ------------------------
        // removes the trailing silence from the end of a song.  This section of
        // code is dependant on variables having been set by Unimod_StripSilence().
        // Once we've gone past the 'patpos_silence' and 'sngpos_silence' variables,
        // then we only continue to play until either the song ends or the threshold
        // time is exceeded.

        if((ps->state.patpos >= ps->mf->patpos_silence) && (ps->state.sngpos >= ps->mf->sngpos_silence))
        {   ps->state.strip_timeout -= (12500L * ps->state.sngspd) / (ps->state.bpm * 5L);
            if(ps->state.strip_timeout <= 0)
            {
                // End the Song Early!

                MP_WipePosPlayed(ps);
                ps->state.looping--;
                if(!(ps->flags & PF_LOOP) || (ps->state.looping <= 0)) { ps->ended = TRUE; return; }
                MP_LoopSong(ps, ps->mf);
                ps->state.posjmp = 2;
            }
        }
    }

    // failsafe, for songs that don't loop properly:

    if(ps->state.curtime >= MAX_TIME)
    {
        _mmlog("We broke our booboo: Song reached 100 minutes!");
        ps->ended = TRUE;
    }
}

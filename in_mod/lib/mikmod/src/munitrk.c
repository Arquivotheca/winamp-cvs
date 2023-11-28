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
Module: MUNITRK.C

Air's new unitrk builder/player system.  I had to make it a lot faster,
and make it more usable for the purpose of tracking.  So I made the for-
mat less compressed, and hence less code is required to seek through the
stream.  That made it faster.

Portability:
All systems - all compilers

*/

#include "mikmod.h"
#include "uniform.h"
#include <string.h>


// The blank track.  Don't change this unless you want to amuse people.

UBYTE utrk_blanktrack[5] = { 0, 0, 0, 0, 0 };

/**************************************************************************
>>>>>>>>>>> Next are the routines for PLAYING UNITRK streams: <<<<<<<<<<<<<
**************************************************************************/


// =====================================================================================
uint utrk_local_getlength(UBYTE *track)
// =====================================================================================
// gets the length of the given track, in rows.
{
	int   i=0;

	while (track[0] || track[1])
	{
		track += local_track_length_bytes(track);
		i++;
	}

	return i;
}


/***************************************************************************
>>>>>>>>>>> Next are the routines for CREATING UNITRK streams: <<<<<<<<<<<<<
***************************************************************************/

#define ROWSIZE        128          // max. size of the row buffer (determined by bitmasks)
#define GLOB_ROWSIZE  510          // max. size of the global row (10 byes, 64 chn)

#define BUFPAGE       128          // initial size for the track buffer
#define THRESHOLD     128          // threshold increse size for track buffer

// =====================================================================================
static BOOL write_threshold_check(UTRK_WRITER *ut, UNI_EFFTRK *trk, int size)
// =====================================================================================
{
	if ((trk->writepos+size) >= trk->writesize)
	{
		UBYTE *newbuf;

		// We've reached the end of the buffer, so expand
		// the buffer by BUFPAGE bytes

		newbuf = (UBYTE *)_mm_realloc(ut->allochandle, trk->buffer, trk->writesize+BUFPAGE+size);

		// Check if realloc succeeded

		if (newbuf)
		{
			trk->buffer    = newbuf;
			trk->writesize  += BUFPAGE+size;
		}
		else return 0;
	}
	return 1;
}

// =====================================================================================
static BOOL write_threshold_check2(UTRK_WRITER *ut, UNI_GLOBTRK *trk, int size)
// =====================================================================================
{
	if ((trk->writepos+size) >= trk->writesize)
	{
		UBYTE *newbuf;

		// We've reached the end of the buffer, so expand
		// the buffer by BUFPAGE bytes

		newbuf = (UBYTE *)_mm_realloc(ut->allochandle, trk->buffer, trk->writesize+BUFPAGE+size);

		// Check if realloc succeeded

		if (newbuf)
		{
			trk->buffer    = newbuf;
			trk->writesize  += BUFPAGE+size;
		}
		else return 0;
	}
	return 1;
}
// =====================================================================================
void utrk_reset(UTRK_WRITER *ut)
// =====================================================================================
// Resets index-pointers to create a new track.
{
	int  i;

	for (i=0; i<ut->unichn; i++)
	{
		ut->unitrk[i].buffer[0] = ut->unitrk[i].output[0] = 0; // Row Length/Flag clear
		ut->unitrk[i].buffer[1] = ut->unitrk[i].output[1] = 0; // Row Length/Flag clear
		ut->unitrk[i].writepos  = ut->unitrk[i].outpos    = 0; // commence writing at the first byte.
		ut->unitrk[i].note      = ut->unitrk[i].inst      = 0;
		ut->unitrk[i].reallen   = 0;
	}
	ut->globtrk.buffer[0] = ut->globtrk.output[0] = 0;
	ut->globtrk.writepos  = ut->globtrk.outpos    = 0;
	ut->globtrk.reallen   = 0;
}


// =====================================================================================
void utrk_settrack(UTRK_WRITER *ut, int trk)
// =====================================================================================
{
	ut->curtrk = &ut->unitrk[trk];
	ut->curchn = trk;
}


// =====================================================================================
void utrk_write_global(UTRK_WRITER *ut, const UNITRK_EFFECT *data, int memslot)
// =====================================================================================
// Appends an effect to the global effects column
{
	int   eflag = ut->globtrk.writepos++;

	if (!write_threshold_check2(ut, &ut->globtrk, 3+sizeof(UNITRK_EFFECT)))
		return;

	ut->globtrk.buffer[eflag] = TFLG_EFFECT_INFO;
	ut->globtrk.buffer[ut->globtrk.writepos++] = ut->curchn;

	if (memslot != UNIMEM_NONE)
	{
		ut->globtrk.buffer[ut->globtrk.writepos++] = memslot;
		ut->globtrk.buffer[eflag] |= TFLG_EFFECT_MEMORY;
	}

	memcpy(&ut->globtrk.buffer[ut->globtrk.writepos], data, sizeof(UNITRK_EFFECT));
	ut->globtrk.writepos += sizeof(UNITRK_EFFECT);
}


// =====================================================================================
void utrk_memory_global(UTRK_WRITER *ut, const UNITRK_EFFECT *data, int memslot)
// =====================================================================================
// Appends an effect memory request to the global effects column.
{
	int   eflag = ut->globtrk.writepos++;

	if (!write_threshold_check2(ut, &ut->globtrk, 3+sizeof(UNITRK_EFFECT)))
		return;

	ut->globtrk.buffer[eflag] = TFLG_EFFECT_MEMORY;
	ut->globtrk.buffer[ut->globtrk.writepos++] = ut->curchn;
	ut->globtrk.buffer[ut->globtrk.writepos++] = memslot;

	/*if(gmem_flags[memslot] & UMF_EFFECT)
	{   globtrk.buffer[eflag] |= TFLG_OVERRIDE_EFFECT;
	globtrk.buffer[globtrk.writepos++] = data->effect;
	}

	if(gmem_flags[memslot] & UMF_FRAMEDLY)
	{   globtrk.buffer[eflag] |= TFLG_OVERRIDE_FRAMEDLY;
	globtrk.buffer[globtrk.writepos++] = data->framedly;
	}
	globtrk.buffer[eflag] |= (signs == 0) ? 0 : ((signs > 0) ? TFLG_PARAM_POSITIVE : TFLG_PARAM_NEGATIVE;
	*/
}


// =====================================================================================
void utrk_write_local(UTRK_WRITER *ut, const UNITRK_EFFECT *data, int memslot)
// =====================================================================================
// write effect data to current position and update
{
	int eflag = ut->curtrk->writepos++;
	if (!data->effect) return;

	if (!write_threshold_check(ut, ut->curtrk, 2+sizeof(UNITRK_EFFECT)))
		return;

	ut->curtrk->buffer[eflag] = TFLG_EFFECT_INFO;

	if (memslot != UNIMEM_NONE)
	{
		ut->curtrk->buffer[eflag] |= TFLG_EFFECT_MEMORY;
		ut->curtrk->buffer[ut->curtrk->writepos++] = memslot;
	}

	memcpy(&ut->curtrk->buffer[ut->curtrk->writepos], data, sizeof(UNITRK_EFFECT));
	ut->curtrk->writepos += sizeof(UNITRK_EFFECT);
}


// =====================================================================================
void utrk_memory_local(UTRK_WRITER *ut, const UNITRK_EFFECT *data, int memslot, int signs)
// =====================================================================================
// Appends an effect memory request to the global effects column.
{
	utrk_memory_local_ex(ut, data, memslot, ut->lmem_flag[memslot], signs);
}


// =====================================================================================
void utrk_memory_local_ex(UTRK_WRITER *ut, const UNITRK_EFFECT *data, int memslot, uint flags, int signs)
// =====================================================================================
// Appends an effect memory request to the global effects column.
{
	int eflag = ut->curtrk->writepos++;

	if (!write_threshold_check(ut, ut->curtrk, 9))
		return;

	ut->curtrk->buffer[eflag] = TFLG_EFFECT_MEMORY;
	ut->curtrk->buffer[ut->curtrk->writepos++] = memslot;

	if (data)
	{
		// override effect
		if (flags & UMF_EFFECT)
		{
			ut->curtrk->buffer[eflag] |= TFLG_OVERRIDE_EFFECT;
			ut->curtrk->buffer[ut->curtrk->writepos++] = data->effect;
		}
		// override other data
		if (flags & UMF_OTHERS)
		{
			// set flags
			ut->curtrk->buffer[eflag] |= TFLG_OVERRIDE;
			ut->curtrk->buffer[ut->curtrk->writepos++] = flags;
			// write data
			if (flags & UMF_LOWORD)
			{
				*(UWORD*)&ut->curtrk->buffer[ut->curtrk->writepos] = data->param.loword.u;
				ut->curtrk->writepos += 2;
			}
			if (flags & UMF_HIWORD)
			{
				*(UWORD*)&ut->curtrk->buffer[ut->curtrk->writepos] = data->param.hiword.u;
				ut->curtrk->writepos += 2;
			}
			if (flags & UMF_FRAMEDLY)
				ut->curtrk->buffer[ut->curtrk->writepos++] = data->framedly;
		}
	}

	ut->curtrk->buffer[eflag] |= (signs == 0) ? 0 : ((signs > 0) ? TFLG_PARAM_POSITIVE : TFLG_PARAM_NEGATIVE);
}


// =====================================================================================
static BOOL buffer_threshold_check(UTRK_WRITER *ut, UNI_EFFTRK *trk, int size)
// =====================================================================================
{
	size += THRESHOLD;
	if ((trk->outpos+size) >= trk->outsize)
	{
		UBYTE *newbuf;

		// We've reached the end of the buffer, so expand
		// the buffer by BUFPAGE bytes

		newbuf = (UBYTE *)_mm_realloc(ut->allochandle, trk->output, trk->outsize+BUFPAGE+size);

		// Check if realloc succeeded

		if (newbuf)
		{
			trk->output    = newbuf;
			trk->outsize  += BUFPAGE+size;
		}
		else return 0;
	}
	return 1;
}


// =====================================================================================
static BOOL buffer_threshold_check2(UTRK_WRITER *ut, UNI_GLOBTRK *glob, int size)
// =====================================================================================
{
	size += THRESHOLD;
	if ((glob->outpos+size) >= glob->outsize)
	{
		UBYTE *newbuf;

		// We've reached the end of the buffer, so expand
		// the buffer by BUFPAGE bytes

		newbuf = (UBYTE *)_mm_realloc(ut->allochandle, glob->output, glob->outsize+BUFPAGE+size);

		// Check if realloc succeeded

		if (newbuf)
		{
			glob->output    = newbuf;
			glob->outsize  += BUFPAGE+size;
		}
		else return 0;
	}
	return 1;
}



// =====================================================================================
void utrk_write_inst(UTRK_WRITER *ut, unsigned int ins)
// =====================================================================================
// Appends UNI_INSTRUMENT opcode to the unitrk stream.
{
	ut->curtrk->inst = ins;
}


void utrk_write_note_ex(UTRK_WRITER *ut, unsigned int noteex)
{
	ut->curtrk->noteflags = noteex;
}

// =====================================================================================
void utrk_write_note(UTRK_WRITER *ut, unsigned int note)
// =====================================================================================
// Appends UNI_NOTE opcode to the unitrk stream.
{
	ut->curtrk->note = note;
}


// =====================================================================================
void utrk_newline(UTRK_WRITER *ut)
// =====================================================================================
// Closes the current row of a unitrk stream and sets pointers to start a new row.
// Crunches the current track data into a more stream-like format.
{
	UNI_EFFTRK     *trk = ut->unitrk;
	int            fbyte, lbyte, len, i;

	for (i=ut->unichn; i; i--, trk++)
	{
		if (!buffer_threshold_check(ut,trk,6))
			return;
		trk->output[lbyte = trk->outpos++] = 0;
		trk->output[trk->outpos++] = 0;
		trk->output[fbyte = trk->outpos++] = 0;		
		len = 0;
		if (trk->note || trk->inst)
		{
			trk->output[fbyte]       |= TFLG_NOTE;
			trk->output[trk->outpos++] = trk->note;
			trk->output[trk->outpos++] = trk->inst;
			trk->output[trk->outpos++] = trk->noteflags;
			len += 3;
		}

		if (trk->writepos)
		{
			if (!write_threshold_check(ut, trk, 1))
				return;

			trk->buffer[trk->writepos++] = 0;  // effects terminator
			trk->buffer[trk->writepos++] = 0;  // double terminate
			if (!buffer_threshold_check(ut, trk, trk->writepos))
				return;
			memcpy(&trk->output[trk->outpos], trk->buffer, trk->writepos);
			trk->outpos += trk->writepos;
			len         += trk->writepos;
			trk->output[fbyte] |= TFLG_EFFECT;
		}

		if (len)
			trk->reallen = trk->outpos;

		assert(len < 65533);
		*(uint16_t *)(&trk->output[lbyte]) = len+3;

		// reset temp buffer and index
		trk->buffer[0] = 0;  
		trk->buffer[1] = 0;
		trk->writepos = 0;
		trk->note = trk->inst =trk->noteflags= 0;
	}

	// Now do the global track.

	if (ut->globtrk.writepos)
	{
		uint   twos;

		if (!write_threshold_check2(ut, &ut->globtrk, 1))
			return;

		ut->globtrk.buffer[ut->globtrk.writepos++] = 0;  // effects terminator
		if (!buffer_threshold_check2(ut, &ut->globtrk, ut->globtrk.writepos+2)) return;
		memcpy(&ut->globtrk.output[ut->globtrk.outpos+1], ut->globtrk.buffer, ut->globtrk.writepos);

		twos = ((ut->globtrk.writepos) / 2) + 1;
		ut->globtrk.output[ut->globtrk.outpos] = twos;

		assert(twos);
		ut->globtrk.outpos += twos*2;
		ut->globtrk.reallen = ut->globtrk.outpos;
	}
	else
	{
		if (!buffer_threshold_check2(ut, &ut->globtrk, 3)) return;
		ut->globtrk.output[ut->globtrk.outpos] = 1;
		ut->globtrk.outpos+=2;
	}

	// reset temp buffer and index
	ut->globtrk.buffer[0] = 0;
	ut->globtrk.writepos  = 0;
}


// =====================================================================================
UBYTE *utrk_dup_track(UTRK_WRITER *ut, int chn, MM_ALLOC *allochandle)
// =====================================================================================
// Dups a single track, returns NULL if allocation failed.
{
	UBYTE       *d   = utrk_blanktrack;
	UNI_EFFTRK  *trk = &ut->unitrk[chn];

	if (trk->reallen)
	{
		trk->output[trk->reallen++] = 0;    // track terminator

		if ((d=(UBYTE *)MikMod_malloc(allochandle, trk->reallen))==NULL) return NULL;
		memcpy(d,trk->output,trk->reallen);
	}
	ut->trkwrite++;

	return d;
}


// =====================================================================================
UBYTE *utrk_dup_global(UTRK_WRITER *ut, MM_ALLOC *allochandle)
// =====================================================================================
// Dups the global track
{
	UBYTE   *d = utrk_blanktrack;

	if (ut->globtrk.reallen)
	{
		ut->globtrk.output[ut->globtrk.reallen++] = 0;
		if ((d=(UBYTE *)MikMod_malloc(allochandle, ut->globtrk.reallen))==NULL) return NULL;
		memcpy(d, ut->globtrk.output, ut->globtrk.reallen);
	}

	return d;
}


// =====================================================================================
BOOL utrk_dup_pattern(UTRK_WRITER *ut, UNIMOD *mf)
// =====================================================================================
// Terminates the current array of unitrk streams and assigns it into the
// track array.
{
	UBYTE       *d;
	int          i;
	UNI_EFFTRK  *trk = ut->unitrk;

	if (ut->trkwrite==0)
	{
		mf->tracks[0] = utrk_blanktrack;
		ut->trkwrite++;
	}

	for (i=0; i<ut->unichn; i++, trk++)
	{
		if (trk->reallen)
		{
			trk->output[trk->reallen++] = 0;  // track termination character
			trk->output[trk->reallen++] = 0;  // track termination character
			if ((d=(UBYTE *)MikMod_malloc(mf->allochandle, trk->reallen))==NULL) return 0;
			memcpy(d,trk->output,trk->reallen);
			mf->tracks[ut->trkwrite] = d;
			mf->patterns[(ut->patwrite * mf->numchn) + i] = ut->trkwrite++;
		}
		else
		{
			// blank track, so write ptr to track 0 (always blank)
			mf->patterns[(ut->patwrite * mf->numchn) + i] = 0;
		}

	}

	mf->globtracks[ut->patwrite++] = utrk_dup_global(ut, mf->allochandle);

	return 1;
}


// =====================================================================================
UTRK_WRITER *utrk_init(int nc, MM_ALLOC *ahparent)
// =====================================================================================
{
	UTRK_WRITER *ut;

	_MM_ALLOCATE(ut, UTRK_WRITER, ahparent);

	if (nc)
	{
		int   i;
		ut->unitrk = (UNI_EFFTRK *)MikMod_calloc(ut->allochandle, sizeof(UNI_EFFTRK),nc);
		for (i=0; i<nc; i++)
		{
			ut->unitrk[i].buffer = (UBYTE *)MikMod_malloc(ut->allochandle, ut->unitrk[i].writesize = ROWSIZE);
			ut->unitrk[i].output = (UBYTE *)MikMod_malloc(ut->allochandle, ut->unitrk[i].outsize = BUFPAGE);
		}
	}
	else ut->unitrk = NULL;

	ut->globtrk.buffer = (UBYTE *)MikMod_malloc(ut->allochandle, ut->globtrk.writesize = GLOB_ROWSIZE);
	ut->globtrk.output = (UBYTE *)MikMod_malloc(ut->allochandle, ut->globtrk.outsize = BUFPAGE);

	ut->unichn   = nc;
	ut->curtrk   = ut->unitrk;
	ut->trkwrite = 0;
	ut->patwrite = 0;
	ut->curchn   = 0;

	return ut;
}


// =====================================================================================
void utrk_memory_reset(UTRK_WRITER *ut)
// =====================================================================================
// Clears the given array of memory flags to the default unimod setting.  Note that the
// array is assumed to be 64 elements in length.
{
	memset(ut->lmem_flag,0,64);
}


// =====================================================================================
void utrk_local_memflag(UTRK_WRITER *ut, int memslot, uint flags)
// =====================================================================================
{
	ut->lmem_flag[memslot] = flags;
}


// =====================================================================================
void utrk_memdef_add(UNIMOD *mf, const UNITRK_EFFECT *data, int memslot)
// =====================================================================================
{
	//    assert(mf->numdefs < MAX_MEM_DEFS);
	mf->memdefs[mf->numdefs].effect = *data;
	mf->memdefs[mf->numdefs].memslot = memslot;
	mf->numdefs++;
}

// =====================================================================================
void utrk_memdef_clear(UNIMOD *mf)
// =====================================================================================
{
	mf->numdefs = 0;
}


// =====================================================================================
uint utrk_cleanup(UTRK_WRITER *ut)
// =====================================================================================
{
	uint   retval = 0;

	if (ut)
	{
		retval = ut->trkwrite;
		_mmalloc_close(ut->allochandle);
	}

	return retval;
}


// =====================================================================================
UBYTE *utrk_global_copy(UTRK_WRITER *ut, UBYTE *track, int chn)
// =====================================================================================
{
	int   pos = 1;

	if (track && *track)
	{
		if (*track > 1)
		{
			UBYTE ch;

			ut->curchn = chn;

			while (ch = track[pos])
			{
				pos += 2;           // ignore channel byte
				if (ch & TFLG_EFFECT_MEMORY)
				{
					int memslot = track[pos++];
					if (ch & TFLG_EFFECT_INFO)
					{
						utrk_write_global(ut, (UNITRK_EFFECT *)(&track[pos]), memslot);
						pos += sizeof(UNITRK_EFFECT);
					}
					else
						utrk_memory_global(ut, NULL, memslot);
				}
				else
				{
					utrk_write_global(ut, (UNITRK_EFFECT *)(&track[pos]), UNIMEM_NONE);
					pos += sizeof(UNITRK_EFFECT);
				}
			}
		}
		track += track[0] * 2;
		if (!track[0]) track = NULL;    // Set it to NULL if end-of-track.
	}

	return track;
}


// =====================================================================================
void utrk_optimizetracks(UNIMOD *mf)
// =====================================================================================
{
	uint      i, numtrk;
	uint     *remap, *trklen;

	trklen = MikMod_calloc(mf->allochandle, mf->numtrk, sizeof(int));
	remap  = MikMod_calloc(mf->allochandle, mf->numtrk, sizeof(int));

	for (i=0; i<mf->numtrk; i++)
	{
		UBYTE  *track = mf->tracks[i];

		// Calculate the length of the track
		// ---------------------------------

		while (track[0] || track[1])
		{
			trklen[i] += local_track_length_bytes(track);
			track     += local_track_length_bytes(track);
		}

		remap[i] = i;
	}

	for (i=0, numtrk=0; i<mf->numtrk; i++)
	{
		if (mf->tracks[i])
		{
			UBYTE  *trka = mf->tracks[i];
			uint    t;

			for (t=i+1; t<mf->numtrk; t++)
			{
				UBYTE  *trkb = mf->tracks[t];

				if (trkb && (remap[t] == t))
				{
					if ((trklen[i] == trklen[t]) && !memcmp(trka, trkb, trklen[t]))
					{
						// These tracks are the same, and haven't already been remapped.

						remap[t] = numtrk;
						if (mf->tracks[t] == utrk_blanktrack)
							mf->tracks[t] = NULL;
						else
							MikMod_free(mf->allochandle, mf->tracks[t]);
					}
				}
			}
			remap[i] = numtrk;
			numtrk++;
		}
	}

	// Remap the pattern data!
	// -----------------------

	for (i=0; i<mf->numpat*mf->numchn; i++)
		mf->patterns[i] = remap[mf->patterns[i]];


	{
		// Rebuild the Tracks table
		// ------------------------

		UBYTE **newtrk = (UBYTE **)MikMod_malloc(mf->allochandle, numtrk * sizeof(UBYTE *));
		uint    cnt = 0;

		for (i=0; i<mf->numtrk; i++)
			if (mf->tracks[i]) newtrk[cnt++] = mf->tracks[i];

		MikMod_free(mf->allochandle, mf->tracks);
		mf->tracks = newtrk;
	}

	mf->numtrk = numtrk;

	MikMod_free(mf->allochandle, trklen);
	MikMod_free(mf->allochandle, remap);

}



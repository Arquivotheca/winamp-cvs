/*

 Module: LOAD_AMF.C

 By X-Fixer, 2002

 Support:
  If you find problems with THIS code, send mail to:
    x-fixer@narod.ru

 Distribution / Code rights:
  Use this source code in any fashion you see fit.  Giving me credit where
  credit is due is optional, depending on your own levels of integrity and
  honesty.

 -----------------------------------------

  Digital Sound and Music Interface (created by Otto Chrons) AMF modules loader.
  Headers structures are taken from XMP and fixed (Claudio, if you read this you
  can use this loader freely ;-). The rest was reverse engineered.
  If someone has any docs on this format, I'd really love to see them.

  Tested on few modules. It did not crash :-P

 Portability:

  I don't know.

*/

#include <string.h>
#include "mikmod.h"
#include "uniform.h"
#include "itshare.h"
#include "..\..\..\..\mikamp\resource.h"


typedef struct AMF_HEADER
{
    CHAR   magic[3];        // signature "AMF"
    UBYTE  version;         // 0A for 1.0, 0B for 1.1, etc
    CHAR   title[32];       // song title (20 bytes in 0.8)
    UBYTE  numins;          // number of samples
    UBYTE  len;             // number of orders
    UWORD  numtrk;          // number of tracks
    // 1.0+
    UBYTE  numchn;          // number of channels
    // 1.0  = 16B channel remap table
    // 1.1  = 16B panning table
    // 1.2+ = 32B panning table
    SBYTE  table[32];
    // 1.3+
//  UBYTE  bpm;             // initial tempo
//  UBYTE  tempo;           // initial speed
    // internal stuff for remapping
    MM_ALLOC   *allochandle;
    UNISAMPLE **samplemap;
} AMFHEADER;


typedef struct AMF_INSTR
{
    UBYTE  type;            // sample type: 1 = PCM sample
    CHAR   name[32];        // sample name
    CHAR   filename[13];    // sample filename
    ULONG  ptr;             // sample index in the file
    ULONG  len;             // sample length in bytes (UWORD in 0.8)
    UWORD  speed;           // rounded to nearest multiple of 8
    UBYTE  vol;             // sample volume in range 0-64
    ULONG  lps;             // sample loop start in bytes (UWORD in 0.8)
    ULONG  lpe;             // sample loop end in bytes (UWORD in 0.8)
} AMFINSTR;

static const CHAR AMF_Version[] = "DSMI AMF x.x";


static BOOL AMF_Test(MMSTREAM *mmfile)
{
    UBYTE id[4];

    if (!_mm_read_UBYTES(id, 4, mmfile)) return 0;
    return !memcmp(id, "AMF", 3) && id[3]>=8;
}


static void *AMF_Init(void)
{
    AMFHEADER *handle;

    _MM_ALLOCATE(handle, AMFHEADER, NULL);
    return handle;
}


static void AMF_Cleanup(AMFHEADER *mh)
{
    _mmalloc_close(mh->allochandle);
}

//
//  The Loader
//
//  ...In this format everything in remappable. Even more: tracks are
//  mapped twice :) Channel remap (AMF 1.0) is done for completness,
//  it's useless imho and seems to be ignored by DMP 4.0. Also, m2amf
//  is a funky thing and sometimes replaces memory commands.
//
//  ... one day I will definitely add a popup message that will show
//  up before playing each AMF file and suggest user to get the real
//  version of the song, instead of using AMF crap...
//

static __inline UBYTE ConvertSlide(int data)
{
    if (data < 0)
        return (-data) & 0xF;
    else return (data & 0xF) << 4;
}

static __inline UBYTE ConvertFineSlide(int data)
{
    if (data < 0)
        return (-data)&0xF | 0xF0;
    else return (data & 0xF)<<4 | 0xF;
}

static BOOL LoadPatterns(AMFHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    UWORD   *ptr;
    UWORD   *trackmap;
    UBYTE   **globtrack;
    uint    i, numtrk;

    // read 2nd tracks map and count actual number of tracks (crazy OttO)
    trackmap = MikMod_calloc(mh->allochandle, mh->numtrk + 1, 2);
    ptr = trackmap + 1;                                     // 0 is always mapped to 0

    numtrk = 0;
    for (i=0; i<mh->numtrk; i++, ptr++)
    {
        *ptr =_mm_read_I_UWORD(mmfile);                     // 1-based track index
        if (numtrk < *ptr)
            numtrk = *ptr;
    }

    numtrk++;
    of->numtrk = numtrk;

    // remap tracks
    ptr = of->patterns;

    for (i=0; i<of->numpat*of->numchn; i++, ptr++)
        *ptr = *ptr<=mh->numtrk ? trackmap[*ptr] : 0;       // fix for freaky AMFs

    if (!AllocTracks(of)) return 0;

    //
    //  AMF stores data in tracks (not patterns), like unimod does.
    //  This requires some additional stuff to extract global effects
    //  and put them in proper place. Look into load_mtm for details.
    //

    of->ut = utrk_init(1, mh->allochandle);
    utrk_memory_reset(of->ut);
    S3MIT_SetMemDefaults(of);

    if(!(globtrack = (UBYTE**)MikMod_calloc(mh->allochandle, numtrk, sizeof(UBYTE*))))
        return 0;

    // 0-th track is an empty one
    of->tracks[0] = utrk_blanktrack;
    of->ut->trkwrite++;

    // read tracks
    for (i=1; i<numtrk; i++)
    {
        ULONG len, oldpos;
        uint  j, inst = 0;
        int   volume = -1;
        // check for eof
        if (_mm_feof(mmfile))
        {
            _mmlog("load_amf> unexpected end of file (in track)");
            return 0;
        }
        // read length
        len = _mm_read_I_UWORD(mmfile);                     // wicked 24-bit integer
        len += _mm_read_UBYTE(mmfile) << 16;                // beware! parsing order is not defined!
        oldpos = 0;
        j = len;

        utrk_reset(of->ut);
        while (j--)
        {
            UBYTE pos, cmd, data;
            SBYTE inf;
            // read command
            pos  = _mm_read_UBYTE(mmfile);
            cmd  = _mm_read_UBYTE(mmfile);
            inf = data = _mm_read_UBYTE(mmfile);
            // set volume for previous row
            // this is done to avoid multiply volume setting
            // commands per row (optimization/workaround)
            if (oldpos<pos && volume!=-1)
            {
                pt_write_effect(of->ut, 0xc, volume);
                volume = -1;
            }
            // end of track
            if (!j)
            {
                // The last triplet is usually FF FF FF, but may be not.
                // This has something to do with song looping (or m2amf
                // bugs), I guess.
                break;
            }
            // set position
            // DMP actually does not support pos<oldpos, but it does
            // not show errors or something and silently ignores it.
            // Anyway, I have not seen such a broken module.
            while (oldpos < pos)
            {
                utrk_newline(of->ut);
                oldpos++;
            }
            // put command
            if (cmd >= 0x7F)
            {
                switch (cmd & 0x7F)
                {
                // set sample
                case 0x00:
                    inst = data + 1;
                    break;
                // speed
                case 0x01:
                    S3MIT_ProcessCmd(of->ut, NULL, 1, data, 7, PTMEM_PORTAMENTO, 0);
                    break;
                // volslide
                case 0x02:
                    S3MIT_ProcessCmd(of->ut, NULL, 4, ConvertSlide(inf), 7, PTMEM_PORTAMENTO, 0);
                    break;
                // set volume
                case 0x03:
                    volume = data;
                    break;
                // portamento down/up
                case 0x04:
                    if (inf >= 0)
                        S3MIT_ProcessCmd(of->ut, NULL, 5, inf, 7, PTMEM_PORTAMENTO, 0);
                    else
                        S3MIT_ProcessCmd(of->ut, NULL, 6, -inf, 7, PTMEM_PORTAMENTO, 0);
                    break;
                // portamento to note
                case 0x06:
                    pt_write_effect(of->ut, 0x3, data);
                    break;
                // tremor and tremolo (!)
                // Seems that Otto does not know the difference between these
                // 2 commands and decided to convert them to one (without even
                // parameters convertion). I convert it to tremolo, because it
                // sounds more like in DMP (though DMP says "tremor") and tremolo
                // instead of tremor is less noticeable, then tremor instead of
                // tremolo. What a mess!
                case 0x07:
                    S3MIT_ProcessCmd(of->ut, NULL, 0x12, data, 7, PTMEM_PORTAMENTO, 0);
                    break;
                // arpeggio
                case 0x08:
                    pt_write_effect(of->ut, 0x0, data);
                    break;
                // vibrato
                case 0x09:
                    pt_write_effect(of->ut, 0x4, data);
                    break;
                // portamento + volslide
                case 0x0A:
                    S3MIT_ProcessCmd(of->ut, NULL, 0xc, ConvertSlide(inf), 7, PTMEM_PORTAMENTO, 0);
                    break;
                // vibrato + volslide
                case 0x0B:
                    S3MIT_ProcessCmd(of->ut, NULL, 0xb, ConvertSlide(inf), 7, PTMEM_PORTAMENTO, 0);
                    break;
                // pattern break
                // m2amf always writes 0 as parameter, so DMP always jumps
                // to 0-th row in the next pattern.
                case 0x0C:
                    pt_write_effect(of->ut, 0xd, data);
                    break;
                // position jump
                case 0x0D:
                    pt_write_effect(of->ut, 0xb, data);
                    break;
                // retrigger
                // S3M retrigger also has volume slide parameter (hi nibble),
                // but m2amf ignores it. Should I mask it out, to prevent volume
                // slide, or leave it as is, hoping new m2amf versions will
                // implement it someday?
                case 0x0F:
                    S3MIT_ProcessCmd(of->ut, NULL, 0x11, data, 7, PTMEM_PORTAMENTO, 0);
                    break;
                // sample offset
                case 0x10:
                    pt_write_effect(of->ut, 0x9, data);
                    break;
                // fine volslide
                case 0x11:
                    S3MIT_ProcessCmd(of->ut, NULL, 4, ConvertFineSlide(inf), 7, PTMEM_PORTAMENTO, 0);
                    break;
                // extrafine and fine portamento
                case 0x16:
                    inf /= 4;                               // what for does he scale this up?
                case 0x12:
                    if (inf >= 0)
                    {
                        inf &= 0xf;
                        inf |= cmd==0x92 ? 0xf0 : 0xe0;
                        S3MIT_ProcessCmd(of->ut, NULL, 5, (UBYTE)inf, 7, PTMEM_PORTAMENTO, 0);
                    }
                    else
                    {
                        inf = -inf & 0xf;
                        inf |= cmd==0x92 ? 0xf0 : 0xe0;
                        S3MIT_ProcessCmd(of->ut, NULL, 6, (UBYTE)inf, 7, PTMEM_PORTAMENTO, 0);
                    }
                    break;
                // note delay
                case 0x13:
                    pt_write_exx(of->ut, 0xd, data);
                    break;
                // note cut
                case 0x14:
                    S3MIT_ProcessCmd(of->ut, NULL, 0x13, 0xC0|(data&0xf), 7, PTMEM_PORTAMENTO, 0);
                    break;
                // tempo
                // Otto does not know, that S3M ignores Txx with values lower
                // than 32, so DMP can set tempo to these low values.
                case 0x15:
                    S3MIT_ProcessCmd(of->ut, NULL, 0x14, data, 7, PTMEM_PORTAMENTO, 0);
                    break;
                // panning
                case 0x0E:                                  // when converted from PT's E8x
                    pt_write_exx(of->ut, 0x8, data);
                    break;
                case 0x17:
                    {
                        UNITRK_EFFECT effdat;

                        effdat.effect   = UNI_PANNING;
                        effdat.param.s  = inf<=64 ? inf*2 : PAN_SURROUND;
                        effdat.framedly = UFD_RUNONCE;
                        utrk_write_local(of->ut, &effdat, UNIMEM_NONE);
                    }
                    break;
                // write sample / note off
                // kudos to me for supporting the command (FF), that DMP itself
                // does not support (but m2amf uses)!
                case 0x7F:
                    utrk_write_inst(of->ut, inst);
                    if (data != 255) volume = data;
                    break;
#ifdef _BLOW_UP
                default:
                    _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_amf> Unsupported effect.");
                    return 0;
#endif
                }
            }
            // put note
            else if (cmd)
            {
#ifdef _BLOW_UP
                if (cmd<12 || cmd>0x70 || (data!=255 && data>64))
                {
                    _mmerr_set(MMERR_UNSUPPORTED_FILE, "load_amf> Unsupported effect.");
                    return 0;
                }
#endif

                utrk_write_note(of->ut, cmd - 11);
                // volume
                if (data != 255)                            // Damn, I'm good.
                {
                    utrk_write_inst(of->ut, inst);
                    volume = data;
                }
            }
            // note cut
            else
            {
                UNITRK_EFFECT eff = {0, UNI_NOTEKILL, UFD_RUNONCE};
                utrk_write_local(of->ut, &eff, UNIMEM_NONE);
            }
        }
        utrk_newline(of->ut);                               // new line to force setting "inuse" of global track

        if(!(of->tracks[i] = utrk_dup_track(of->ut, 0, of->allochandle)))
            return 0;
        globtrack[i] = utrk_dup_global(of->ut, mh->allochandle);
    }

    // process global tracks
    pt_global_consolidate(of, globtrack);

    return 1;
}

static BOOL LoadSamples(AMFHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    UNISAMPLE  *q;
    uint        i;

    // check for eof
    if (_mm_feof(mmfile))
    {
        _mmlog("load_amf> unexpected end of file (in samples)");
        return 0;
    }
    // alloc sample structs
    of->numsmp = mh->numins;
    if (!AllocSamples(of, 0))
        return 0;
    // read samples headers
    mh->samplemap = MikMod_calloc(mh->allochandle, of->numsmp, sizeof(UNISAMPLE*));
    q = of->samples;

    for (i=0; i<of->numsmp; i++, q++)
    {
        AMFINSTR    s;
        // read sample header
        s.type     = _mm_read_UBYTE(mmfile);
        _mm_read_UBYTES(s.name, 32, mmfile);
        _mm_read_UBYTES(s.filename, 13, mmfile);
        s.ptr      = _mm_read_I_ULONG(mmfile);

        if (mh->version >= 10)
            s.len  = _mm_read_I_ULONG(mmfile);
        else s.len = _mm_read_I_UWORD(mmfile);

        s.speed    = _mm_read_I_UWORD(mmfile);
        s.vol      = _mm_read_UBYTE(mmfile);

        if (mh->version >= 10)
        {
            s.lps  = _mm_read_I_ULONG(mmfile);
            s.lpe  = _mm_read_I_ULONG(mmfile);
        }
        else
        {
            s.lps  = _mm_read_I_UWORD(mmfile);
            s.lpe  = _mm_read_I_UWORD(mmfile);
            if (s.lpe == 0xFFFF) s.lpe = 0;
        }

        // convert it
        if (s.ptr>of->numsmp || s.type>1)
            return 0;
        if (s.ptr)
            mh->samplemap[s.ptr - 1] = q;
        q->samplename = DupStr(of->allochandle, s.name, 32);
        q->length     = s.len;
        q->speed      = s.speed;
        q->volume     = s.vol << 1;
        q->loopstart  = s.lps;
        q->loopend    = s.lpe;

        if (s.lps || s.lpe)
            q->flags |= SL_LOOP;
    }

    return 1;
}

static BOOL AMF_Load(AMFHEADER *mh, UNIMOD *of, MMSTREAM *mmfile)
{
    UWORD   *ptr;
    uint    i, j;

    // try to read module header
    _mm_read_UBYTES(mh->magic, 3, mmfile);
    mh->version   = _mm_read_UBYTE(mmfile);
    _mm_read_SBYTES(mh->title, 32, mmfile);
    mh->numins    = _mm_read_UBYTE(mmfile);
    mh->len       = _mm_read_UBYTE(mmfile);
    mh->numtrk    = _mm_read_I_UWORD(mmfile);
    if (mh->version >= 10)
    {
        mh->numchn = _mm_read_UBYTE(mmfile);
        _mm_read_SBYTES(mh->table, mh->version>=12 ? 32: 16 , mmfile);
    }
    else
    {
        mh->numchn = 4;
        // I do not know, what these 12 byte contain. It looks
        // like garbage, almost always the same for all modules
        // (00 C3 1F 51 E8 63 01 59...) and I'm bored with AMF
        // already. Anyway, it seems it's not critical info ;-)
        memset(mh->title + 20, 0, 12);
    }

		if (mh->numchn > 32)
			return 0;

    // init modfile data
    of->modtype     = _mm_strdup(of->allochandle, AMF_Version);
    of->modtype[9]  = mh->version/10 + '0';
    of->modtype[11] = mh->version%10 + '0';
    of->songname    = DupStrTrim(of->allochandle, mh->title, 32);
    of->numchn      = mh->numchn;
    // new stuff
    if (mh->version >= 13)
    {
        of->inittempo = _mm_read_UBYTE(mmfile);
        of->initspeed = _mm_read_UBYTE(mmfile);
    }
    else
    {
        of->inittempo = 125;
        of->initspeed = 6;

        if (mh->version == 10)
        {
            uint u;

            for (u=0; u<of->numchn; u++)
                if ((UBYTE)mh->table[u] >= of->numchn)
                    mh->table[u] = u;
        }
    }
    // check for eof
    if (_mm_feof(mmfile))
    {
        _mmlog("load_amf> unexpected end of file (in header)");
        return 0;
    }
    // set panning
    if (mh->version >= 11)
    {
        int *ptr = of->panning;
        
        for (i=0; i<mh->numchn; i++, ptr++)
        {
					if (mh->table[i] > 64)
						*ptr = PAN_SURROUND;
					else
						*ptr = mh->table[i] * 2;
        }
    }
    // fill orders
    of->numpos = mh->len;
    if (!AllocPositions(of, of->numpos))
        return 0;

    for (i=0; i<of->numpos; i++)
        of->positions[i] = i;

    // alloc pattern structures
    of->numpat = of->numpos;
    if (!AllocPatterns(of)) return 0;
    // read tracks table (set patterns-tracks relationships)
    // and setup patterns
    ptr = of->patterns;

    for (i=0; i < of->numpat; i++)
    {
        if (mh->version >= 14)
            of->pattrows[i] = _mm_read_I_UWORD(mmfile);     // pattern length

        for (j=0; j<of->numchn; j++)
        {
            // 1-based track index
            if (mh->version != 10)
                *ptr++ = _mm_read_I_UWORD(mmfile);
            // channels remapping for 1.0 files
            // seems that DMP 4.0 ignores it
            else *(ptr + mh->table[j]) = _mm_read_I_UWORD(mmfile);
        }

        if (mh->version == 10)
            ptr += of->numchn;
    }

    // load samples
    if (!LoadSamples(mh, of, mmfile))
        return 0;
    // load patterns    
    if (!LoadPatterns(mh, of, mmfile))
        return 0;
    // set seekpos for samples
    {
        long  seekpos = _mm_ftell(mmfile);

        for (i=0; i<of->numsmp; i++)
        {
            UNISAMPLE *q = mh->samplemap[i];
            if (!q) break;
            q->seekpos = seekpos;
            seekpos += q->length;
        }
    }

    return 1;
}


static CHAR *AMF_LoadTitle(MMSTREAM *mmfile)
{
   CHAR s[32];

   _mm_fseek(mmfile, 4, SEEK_SET);
   if (!_mm_read_UBYTES(s, 32, mmfile))
       return NULL;
   
   return DupStrTrim(NULL, s, 32);
}


MLOADER load_amf =
{
    "AMF",
    IDS_FAMILY_STRING_DSMI_AMF,//"DSMI AMF",
    0,
    NULL,
    AMF_Test,
    AMF_Init,
    AMF_Cleanup,
    AMF_Load,
#ifndef _MM_WINAMP_
    AMF_LoadTitle
#endif
};
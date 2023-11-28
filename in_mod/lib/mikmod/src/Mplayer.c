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
 Module: mplayer.c
 
  Player Module - Processes the UniMod stream.  Contains execution code for
  all effects.  BIG!  This is the 'core' of mikmod.  The heart of the beast,
  you might even say.

 Portability:
  All compilers - All Systems.

*/

#ifndef _DEBUG
#define DEBUG(X)
#endif

#include <string.h>
#include <stdarg.h>
#include "mikmod.h"
#include "mpforbid.h"

#define PF_START_BEGINNING  -32

#define KICK_NOTE       1
#define KICK_NNA        2
#define KICK_FADES      4
#define KICK_REVERSE  128               // this might be bad (it should be a flag), but it's to make "minimal change"

#define KICK_VOLENV     8
#define KICK_PANENV    16
#define KICK_PITENV    32
#define KICK_RESENV    64

#define KICK_ENV      (KICK_VOLENV | KICK_PANENV | KICK_PITENV | KICK_RESENV | KICK_FADES)
#define KICK_ALL      (KICK_NOTE | KICK_NNA | KICK_ENV)

#define CHANVOL_FULL   64

size_t local_track_length_bytes(const UBYTE *track)
{
	return *(uint16_t *)track; // & TFLG_LENGTH_MASK;
}
// =====================================================================================
    static void pseek(MPLAYER *ps)
// =====================================================================================
// Seeks all channels in the module to the current row (ps->patpos).
// Sets a->row to the current position within the track, or to NULL if
// the end of the track has been found.
{
    const UNIMOD     *mf = ps->mf;
    uint       t = 0;
    int        postmp = (mf->positions[ps->state.sngpos]*mf->numchn);
    UBYTE      *track;
    MP_CONTROL *a;

    for(a=ps->state.control; t<mf->numchn; t++, a++)
    {   
        uint i = mf->patterns[postmp+t];
        track = mf->tracks[i];

        i = 0;
        while((track[0] || track[1]) && i<ps->state.patpos)
        {   
					track += local_track_length_bytes(track);
            i++;
        }
        a->row = track;
        a->pos = 0;
    }

    // Now seek the global track too!

    track = mf->globtracks[mf->positions[ps->state.sngpos]];
    t = 0;
    while(*track && t<ps->state.patpos)
    {   track += track[0]*2;
        t++;
    }
    ps->state.globtrk_row = track;
    ps->state.globtrk_pos = 0;
}


// =====================================================================================
    static void nextrow(MPLAYER *ps)
// =====================================================================================
// jumps to the next row in the track for each channel in the specified module.
{
    int         t = ps->mf->numchn;
    MP_CONTROL *a = ps->state.control;

    for(; t; t--, a++)
    {  
			a->row += local_track_length_bytes(a->row);
        a->pos = 0;
    }

    // Increment the globaltrack position too!
    ps->state.globtrk_row += ps->state.globtrk_row[0] * 2;
    ps->state.globtrk_pos = 0;
}


// =====================================================================================
    static void __inline getnote(MP_CONTROL *a, UNITRK_NOTE *retnote)
// =====================================================================================
{
	if((a->row[0] || a->row[1]) && (a->row[2] & TFLG_NOTE))
	{  
		retnote->note = a->row[3];
		retnote->inst = a->row[4];
		retnote->noteex = a->row[5];
	}
	else 
		retnote->note = retnote->inst = 0;
}


// =====================================================================================
    static UNITRK_EFFECT *local_geteffect(MP_CONTROL *a, UNITRK_EFFECT *effdat)
// =====================================================================================
// Gets the next effect from the current row
{
    UBYTE ch;
    
    if(a->pos == 0)
    {  
			ch = (a->row[0] || a->row[1])?a->row[2]:0;
			if(!(ch & TFLG_EFFECT)) 
				return NULL;
        a->pos += (ch & TFLG_NOTE) ? 6 : 3;
    }

		// TODO: benski> merge with effect/note flag
    if(ch = a->row[a->pos])
    {   a->pos++;
        if(ch & TFLG_EFFECT_MEMORY)
        {   // note: when memory is present, we always return the address to
            // the actual memory slot, since that is more efficient than loading
            // the data into effdat.

            int             memslot = a->row[a->pos++];
            UNITRK_EFFECT   *effmem  = &a->memory[memslot].effect;

            if(ch & TFLG_EFFECT_INFO)
            {   // Copy the new effect data to memory
                memcpy(effmem, &a->row[a->pos], sizeof(UNITRK_EFFECT));
                a->pos += sizeof(UNITRK_EFFECT);
                a->memory[memslot].flag = MEMFLAG_LOCAL;
                return effmem;
            } else 
            {   if(ch & (TFLG_PARAM_POSITIVE|TFLG_PARAM_NEGATIVE|TFLG_OVERRIDE_EFFECT|TFLG_OVERRIDE))
                {
                    // correct data
                    if(((ch & TFLG_PARAM_POSITIVE) && (effmem->param.s < 0)) || ((ch & TFLG_PARAM_NEGATIVE) && (effmem->param.s > 0)))
                        effmem->param.s = -effmem->param.s;
                    // override effect
                    if(ch & TFLG_OVERRIDE_EFFECT)
                        effmem->effect = a->row[a->pos++];
                    // override other data
                    if(ch & TFLG_OVERRIDE)
                    {
                        UBYTE flag = a->row[a->pos++];
                        // lo param
                        if (flag & UMF_LOWORD)
                        {
                            effmem->param.loword.u = *(UWORD*)&a->row[a->pos];
                            a->pos += 2;
                        }
                        // hi param
                        if (flag & UMF_HIWORD)
                        {
                            effmem->param.hiword.u = *(UWORD*)&a->row[a->pos];
                            a->pos += 2;
                        }
                        // framedly
                        if (flag & UMF_FRAMEDLY)
                            effmem->framedly = a->row[a->pos++];
                    }

                    a->memory[memslot].flag = MEMFLAG_LOCAL;
                    return effmem;
                }
                return (!(a->memory[memslot].flag & MEMFLAG_GLOBAL)) ? effmem : NULL;
            }
        } else
        {   // copy the effect data to effdat for return
            memcpy(effdat, &a->row[a->pos], sizeof(UNITRK_EFFECT));
            a->pos += sizeof(UNITRK_EFFECT);
        }
    } else return NULL;

    return effdat;
}


// =====================================================================================
    UNITRK_EFFECT *global_geteffect(MPLAYER *ps, UNITRK_EFFECT *effdat, MP_CONTROL **a)
// =====================================================================================
// Gets the next effect from the current row
{
    UBYTE      *row = ps->state.globtrk_row, ch;

    if(ps->state.globtrk_pos == 0)
    {   if(*row <= 1) return NULL;
        ps->state.globtrk_pos++;
    }

    if(ch = row[ps->state.globtrk_pos])
    {   ps->state.globtrk_pos++;
        *a = &ps->state.control[row[ps->state.globtrk_pos++]];

        if(ch & TFLG_EFFECT_MEMORY)
        {   // note: when memory is present, we always return the address to
            // the actual memory slot, since that is more efficient than loading
            // the data into effdat.

            int         memslot = row[ps->state.globtrk_pos++];

            if(ch & TFLG_EFFECT_INFO)
            {   // Copy the new effect data to memory
                memcpy(&(*a)->memory[memslot].effect, &row[ps->state.globtrk_pos], sizeof(UNITRK_EFFECT));
                (*a)->memory[memslot].flag = MEMFLAG_GLOBAL;
                ps->state.globtrk_pos += sizeof(UNITRK_EFFECT);
            }
            return (!((*a)->memory[memslot].flag & MEMFLAG_LOCAL)) ? &(*a)->memory[memslot].effect : NULL;
        } else
        {   // copy the effect data to effdat for return
            memcpy(effdat, &row[ps->state.globtrk_pos], sizeof(UNITRK_EFFECT));
            ps->state.globtrk_pos += sizeof(UNITRK_EFFECT);
        }
    } else return NULL;

    return effdat;
}

// ** Triton's linear periods to frequency translation table (for
// ** Fast Tracker 2 [XM] modules):

// =====================================================================================
    static const UWORD lintab[1536] =
// =====================================================================================
{
    16384, 16376, 16369, 16361, 16354, 16347, 16339, 16332, 16324, 16317, 16310, 16302,
    16295, 16288, 16280, 16273, 16266, 16258, 16251, 16244, 16236, 16229, 16222, 16214,
    16207, 16200, 16192, 16185, 16178, 16170, 16163, 16156, 16149, 16141, 16134, 16127,
    16119, 16112, 16105, 16098, 16090, 16083, 16076, 16069, 16061, 16054, 16047, 16040,
    16032, 16025, 16018, 16011, 16004, 15996, 15989, 15982, 15975, 15967, 15960, 15953,
    15946, 15939, 15931, 15924, 15917, 15910, 15903, 15896, 15888, 15881, 15874, 15867,
    15860, 15853, 15845, 15838, 15831, 15824, 15817, 15810, 15803, 15795, 15788, 15781,
    15774, 15767, 15760, 15753, 15746, 15739, 15731, 15724, 15717, 15710, 15703, 15696,
    15689, 15682, 15675, 15668, 15661, 15654, 15646, 15639, 15632, 15625, 15618, 15611,
    15604, 15597, 15590, 15583, 15576, 15569, 15562, 15555, 15548, 15541, 15534, 15527,
    15520, 15513, 15506, 15499, 15492, 15485, 15478, 15471, 15464, 15457, 15450, 15443,
    15436, 15429, 15422, 15415, 15408, 15401, 15394, 15387, 15380, 15373, 15367, 15360,
    15353, 15346, 15339, 15332, 15325, 15318, 15311, 15304, 15297, 15290, 15284, 15277,
    15270, 15263, 15256, 15249, 15242, 15235, 15228, 15222, 15215, 15208, 15201, 15194,
    15187, 15180, 15174, 15167, 15160, 15153, 15146, 15139, 15133, 15126, 15119, 15112,
    15105, 15098, 15092, 15085, 15078, 15071, 15064, 15058, 15051, 15044, 15037, 15030,
    15024, 15017, 15010, 15003, 14997, 14990, 14983, 14976, 14970, 14963, 14956, 14949,
    14943, 14936, 14929, 14922, 14916, 14909, 14902, 14895, 14889, 14882, 14875, 14869,
    14862, 14855, 14848, 14842, 14835, 14828, 14822, 14815, 14808, 14802, 14795, 14788,
    14782, 14775, 14768, 14762, 14755, 14748, 14742, 14735, 14728, 14722, 14715, 14708,
    14702, 14695, 14688, 14682, 14675, 14669, 14662, 14655, 14649, 14642, 14636, 14629,
    14622, 14616, 14609, 14603, 14596, 14589, 14583, 14576, 14570, 14563, 14557, 14550,
    14543, 14537, 14530, 14524, 14517, 14511, 14504, 14498, 14491, 14484, 14478, 14471,
    14465, 14458, 14452, 14445, 14439, 14432, 14426, 14419, 14413, 14406, 14400, 14393,
    14387, 14380, 14374, 14367, 14361, 14354, 14348, 14341, 14335, 14328, 14322, 14315,
    14309, 14303, 14296, 14290, 14283, 14277, 14270, 14264, 14257, 14251, 14245, 14238,
    14232, 14225, 14219, 14212, 14206, 14200, 14193, 14187, 14180, 14174, 14168, 14161,
    14155, 14148, 14142, 14136, 14129, 14123, 14117, 14110, 14104, 14098, 14091, 14085,
    14078, 14072, 14066, 14059, 14053, 14047, 14040, 14034, 14028, 14021, 14015, 14009,
    14002, 13996, 13990, 13983, 13977, 13971, 13965, 13958, 13952, 13946, 13939, 13933,
    13927, 13920, 13914, 13908, 13902, 13895, 13889, 13883, 13877, 13870, 13864, 13858,
    13852, 13845, 13839, 13833, 13827, 13820, 13814, 13808, 13802, 13795, 13789, 13783,
    13777, 13771, 13764, 13758, 13752, 13746, 13739, 13733, 13727, 13721, 13715, 13709,
    13702, 13696, 13690, 13684, 13678, 13671, 13665, 13659, 13653, 13647, 13641, 13634,
    13628, 13622, 13616, 13610, 13604, 13598, 13591, 13585, 13579, 13573, 13567, 13561,
    13555, 13549, 13543, 13536, 13530, 13524, 13518, 13512, 13506, 13500, 13494, 13488,
    13482, 13475, 13469, 13463, 13457, 13451, 13445, 13439, 13433, 13427, 13421, 13415,
    13409, 13403, 13397, 13391, 13385, 13378, 13372, 13366, 13360, 13354, 13348, 13342,
    13336, 13330, 13324, 13318, 13312, 13306, 13300, 13294, 13288, 13282, 13276, 13270,
    13264, 13258, 13252, 13246, 13240, 13234, 13228, 13222, 13216, 13211, 13205, 13199,
    13193, 13187, 13181, 13175, 13169, 13163, 13157, 13151, 13145, 13139, 13133, 13127,
    13121, 13115, 13110, 13104, 13098, 13092, 13086, 13080, 13074, 13068, 13062, 13056,
    13051, 13045, 13039, 13033, 13027, 13021, 13015, 13009, 13003, 12998, 12992, 12986,
    12980, 12974, 12968, 12962, 12957, 12951, 12945, 12939, 12933, 12927, 12922, 12916,
    12910, 12904, 12898, 12892, 12887, 12881, 12875, 12869, 12863, 12858, 12852, 12846,
    12840, 12834, 12829, 12823, 12817, 12811, 12805, 12800, 12794, 12788, 12782, 12777,
    12771, 12765, 12759, 12754, 12748, 12742, 12736, 12731, 12725, 12719, 12713, 12708,
    12702, 12696, 12690, 12685, 12679, 12673, 12668, 12662, 12656, 12650, 12645, 12639,
    12633, 12628, 12622, 12616, 12611, 12605, 12599, 12593, 12588, 12582, 12576, 12571,
    12565, 12559, 12554, 12548, 12542, 12537, 12531, 12525, 12520, 12514, 12508, 12503,
    12497, 12492, 12486, 12480, 12475, 12469, 12463, 12458, 12452, 12447, 12441, 12435,
    12430, 12424, 12418, 12413, 12407, 12402, 12396, 12391, 12385, 12379, 12374, 12368,
    12363, 12357, 12351, 12346, 12340, 12335, 12329, 12324, 12318, 12312, 12307, 12301,
    12296, 12290, 12285, 12279, 12274, 12268, 12263, 12257, 12251, 12246, 12240, 12235,
    12229, 12224, 12218, 12213, 12207, 12202, 12196, 12191, 12185, 12180, 12174, 12169,
    12163, 12158, 12152, 12147, 12141, 12136, 12130, 12125, 12120, 12114, 12109, 12103,
    12098, 12092, 12087, 12081, 12076, 12070, 12065, 12060, 12054, 12049, 12043, 12038,
    12032, 12027, 12021, 12016, 12011, 12005, 12000, 11994, 11989, 11984, 11978, 11973,
    11967, 11962, 11957, 11951, 11946, 11940, 11935, 11930, 11924, 11919, 11913, 11908,
    11903, 11897, 11892, 11887, 11881, 11876, 11871, 11865, 11860, 11854, 11849, 11844,
    11838, 11833, 11828, 11822, 11817, 11812, 11806, 11801, 11796, 11790, 11785, 11780,
    11774, 11769, 11764, 11759, 11753, 11748, 11743, 11737, 11732, 11727, 11721, 11716,
    11711, 11706, 11700, 11695, 11690, 11684, 11679, 11674, 11669, 11663, 11658, 11653,
    11648, 11642, 11637, 11632, 11627, 11621, 11616, 11611, 11606, 11600, 11595, 11590,
    11585, 11580, 11574, 11569, 11564, 11559, 11553, 11548, 11543, 11538, 11533, 11527,
    11522, 11517, 11512, 11507, 11501, 11496, 11491, 11486, 11481, 11475, 11470, 11465,
    11460, 11455, 11450, 11444, 11439, 11434, 11429, 11424, 11419, 11413, 11408, 11403,
    11398, 11393, 11388, 11383, 11377, 11372, 11367, 11362, 11357, 11352, 11347, 11342,
    11336, 11331, 11326, 11321, 11316, 11311, 11306, 11301, 11296, 11291, 11285, 11280,
    11275, 11270, 11265, 11260, 11255, 11250, 11245, 11240, 11235, 11230, 11224, 11219,
    11214, 11209, 11204, 11199, 11194, 11189, 11184, 11179, 11174, 11169, 11164, 11159,
    11154, 11149, 11144, 11139, 11134, 11129, 11124, 11119, 11114, 11109, 11104, 11099,
    11094, 11089, 11084, 11079, 11074, 11069, 11064, 11059, 11054, 11049, 11044, 11039,
    11034, 11029, 11024, 11019, 11014, 11009, 11004, 10999, 10994, 10989, 10984, 10979,
    10974, 10969, 10964, 10959, 10954, 10949, 10944, 10939, 10935, 10930, 10925, 10920,
    10915, 10910, 10905, 10900, 10895, 10890, 10885, 10880, 10875, 10871, 10866, 10861,
    10856, 10851, 10846, 10841, 10836, 10831, 10826, 10822, 10817, 10812, 10807, 10802,
    10797, 10792, 10787, 10783, 10778, 10773, 10768, 10763, 10758, 10753, 10749, 10744,
    10739, 10734, 10729, 10724, 10720, 10715, 10710, 10705, 10700, 10695, 10691, 10686,
    10681, 10676, 10671, 10666, 10662, 10657, 10652, 10647, 10642, 10638, 10633, 10628,
    10623, 10618, 10614, 10609, 10604, 10599, 10594, 10590, 10585, 10580, 10575, 10571,
    10566, 10561, 10556, 10552, 10547, 10542, 10537, 10533, 10528, 10523, 10518, 10514,
    10509, 10504, 10499, 10495, 10490, 10485, 10480, 10476, 10471, 10466, 10461, 10457,
    10452, 10447, 10443, 10438, 10433, 10428, 10424, 10419, 10414, 10410, 10405, 10400,
    10396, 10391, 10386, 10382, 10377, 10372, 10367, 10363, 10358, 10353, 10349, 10344,
    10339, 10335, 10330, 10325, 10321, 10316, 10311, 10307, 10302, 10298, 10293, 10288,
    10284, 10279, 10274, 10270, 10265, 10260, 10256, 10251, 10247, 10242, 10237, 10233,
    10228, 10223, 10219, 10214, 10210, 10205, 10200, 10196, 10191, 10187, 10182, 10177,
    10173, 10168, 10164, 10159, 10154, 10150, 10145, 10141, 10136, 10132, 10127, 10122,
    10118, 10113, 10109, 10104, 10100, 10095, 10090, 10086, 10081, 10077, 10072, 10068,
    10063, 10059, 10054, 10050, 10045, 10041, 10036, 10031, 10027, 10022, 10018, 10013,
    10009, 10004, 10000, 9995, 9991, 9986, 9982, 9977, 9973, 9968, 9964, 9959,
    9955, 9950, 9946, 9941, 9937, 9932, 9928, 9923, 9919, 9914, 9910, 9906,
    9901, 9897, 9892, 9888, 9883, 9879, 9874, 9870, 9865, 9861, 9856, 9852,
    9848, 9843, 9839, 9834, 9830, 9825, 9821, 9817, 9812, 9808, 9803, 9799,
    9794, 9790, 9786, 9781, 9777, 9772, 9768, 9763, 9759, 9755, 9750, 9746,
    9741, 9737, 9733, 9728, 9724, 9720, 9715, 9711, 9706, 9702, 9698, 9693,
    9689, 9685, 9680, 9676, 9671, 9667, 9663, 9658, 9654, 9650, 9645, 9641,
    9637, 9632, 9628, 9624, 9619, 9615, 9610, 9606, 9602, 9597, 9593, 9589,
    9584, 9580, 9576, 9572, 9567, 9563, 9559, 9554, 9550, 9546, 9541, 9537,
    9533, 9528, 9524, 9520, 9516, 9511, 9507, 9503, 9498, 9494, 9490, 9486,
    9481, 9477, 9473, 9468, 9464, 9460, 9456, 9451, 9447, 9443, 9439, 9434,
    9430, 9426, 9422, 9417, 9413, 9409, 9405, 9400, 9396, 9392, 9388, 9383,
    9379, 9375, 9371, 9366, 9362, 9358, 9354, 9350, 9345, 9341, 9337, 9333,
    9328, 9324, 9320, 9316, 9312, 9307, 9303, 9299, 9295, 9291, 9286, 9282,
    9278, 9274, 9270, 9266, 9261, 9257, 9253, 9249, 9245, 9240, 9236, 9232,
    9228, 9224, 9220, 9215, 9211, 9207, 9203, 9199, 9195, 9191, 9186, 9182,
    9178, 9174, 9170, 9166, 9162, 9157, 9153, 9149, 9145, 9141, 9137, 9133,
    9129, 9124, 9120, 9116, 9112, 9108, 9104, 9100, 9096, 9092, 9087, 9083,
    9079, 9075, 9071, 9067, 9063, 9059, 9055, 9051, 9047, 9042, 9038, 9034,
    9030, 9026, 9022, 9018, 9014, 9010, 9006, 9002, 8998, 8994, 8990, 8986,
    8981, 8977, 8973, 8969, 8965, 8961, 8957, 8953, 8949, 8945, 8941, 8937,
    8933, 8929, 8925, 8921, 8917, 8913, 8909, 8905, 8901, 8897, 8893, 8889,
    8885, 8881, 8877, 8873, 8869, 8865, 8861, 8857, 8853, 8849, 8845, 8841,
    8837, 8833, 8829, 8825, 8821, 8817, 8813, 8809, 8805, 8801, 8797, 8793,
    8789, 8785, 8781, 8777, 8773, 8769, 8765, 8761, 8757, 8753, 8749, 8745,
    8742, 8738, 8734, 8730, 8726, 8722, 8718, 8714, 8710, 8706, 8702, 8698,
    8694, 8690, 8686, 8683, 8679, 8675, 8671, 8667, 8663, 8659, 8655, 8651,
    8647, 8643, 8640, 8636, 8632, 8628, 8624, 8620, 8616, 8612, 8608, 8605,
    8601, 8597, 8593, 8589, 8585, 8581, 8577, 8574, 8570, 8566, 8562, 8558,
    8554, 8550, 8546, 8543, 8539, 8535, 8531, 8527, 8523, 8520, 8516, 8512,
    8508, 8504, 8500, 8496, 8493, 8489, 8485, 8481, 8477, 8474, 8470, 8466,
    8462, 8458, 8454, 8451, 8447, 8443, 8439, 8435, 8432, 8428, 8424, 8420,
    8416, 8413, 8409, 8405, 8401, 8397, 8394, 8390, 8386, 8382, 8378, 8375,
    8371, 8367, 8363, 8360, 8356, 8352, 8348, 8344, 8341, 8337, 8333, 8329,
    8326, 8322, 8318, 8314, 8311, 8307, 8303, 8299, 8296, 8292, 8288, 8284,
    8281, 8277, 8273, 8270, 8266, 8262, 8258, 8255, 8251, 8247, 8243, 8240,
    8236, 8232, 8229, 8225, 8221, 8217, 8214, 8210, 8206, 8203, 8199, 8195,
};

/*
#define LOGFAC 4*16

// =====================================================================================
    static const UWORD logtab[105] =
// =====================================================================================
{
    LOGFAC*907,LOGFAC*900,LOGFAC*894,LOGFAC*887,LOGFAC*881,LOGFAC*875,LOGFAC*868,LOGFAC*862,
    LOGFAC*856,LOGFAC*850,LOGFAC*844,LOGFAC*838,LOGFAC*832,LOGFAC*826,LOGFAC*820,LOGFAC*814,
    LOGFAC*808,LOGFAC*802,LOGFAC*796,LOGFAC*791,LOGFAC*785,LOGFAC*779,LOGFAC*774,LOGFAC*768,
    LOGFAC*762,LOGFAC*757,LOGFAC*752,LOGFAC*746,LOGFAC*741,LOGFAC*736,LOGFAC*730,LOGFAC*725,
    LOGFAC*720,LOGFAC*715,LOGFAC*709,LOGFAC*704,LOGFAC*699,LOGFAC*694,LOGFAC*689,LOGFAC*684,
    LOGFAC*678,LOGFAC*675,LOGFAC*670,LOGFAC*665,LOGFAC*660,LOGFAC*655,LOGFAC*651,LOGFAC*646,
    LOGFAC*640,LOGFAC*636,LOGFAC*632,LOGFAC*628,LOGFAC*623,LOGFAC*619,LOGFAC*614,LOGFAC*610,
    LOGFAC*604,LOGFAC*601,LOGFAC*597,LOGFAC*592,LOGFAC*588,LOGFAC*584,LOGFAC*580,LOGFAC*575,
    LOGFAC*570,LOGFAC*567,LOGFAC*563,LOGFAC*559,LOGFAC*555,LOGFAC*551,LOGFAC*547,LOGFAC*543,
    LOGFAC*538,LOGFAC*535,LOGFAC*532,LOGFAC*528,LOGFAC*524,LOGFAC*520,LOGFAC*516,LOGFAC*513,
    LOGFAC*508,LOGFAC*505,LOGFAC*502,LOGFAC*498,LOGFAC*494,LOGFAC*491,LOGFAC*487,LOGFAC*484,
    LOGFAC*480,LOGFAC*477,LOGFAC*474,LOGFAC*470,LOGFAC*467,LOGFAC*463,LOGFAC*460,LOGFAC*457,
    LOGFAC*453,LOGFAC*450,LOGFAC*447,LOGFAC*443,LOGFAC*440,LOGFAC*437,LOGFAC*434,LOGFAC*431,
    LOGFAC*428,                                     // this one is needed for interpolation
};
*/

#define TABC 32

// =====================================================================================
    static const UWORD amigatab[13] =
// =====================================================================================
{   
    1812*TABC, 1712*TABC, 1616*TABC, 1524*TABC, 1440*TABC, 1356*TABC, 1280*TABC,
    1208*TABC, 1140*TABC, 1076*TABC, 1016*TABC, 960*TABC, 906*TABC,
};


// =====================================================================================
    static const UBYTE VibratoTable[32] =
// =====================================================================================
{   
    0,24,49,74,97,120,141,161,
    180,197,212,224,235,244,250,253,
    255,253,250,244,235,224,212,197,
    180,161,141,120,97,74,49,24
};


// =====================================================================================
    static const UBYTE avibtab[128] =
// =====================================================================================
{
    0,1,3,4,6,7,9,10,12,14,15,17,18,20,21,23,
    24,25,27,28,30,31,32,34,35,36,38,39,40,41,42,44,
    45,46,47,48,49,50,51,52,53,54,54,55,56,57,57,58,
    59,59,60,60,61,61,62,62,62,63,63,63,63,63,63,63,
    64,63,63,63,63,63,63,63,62,62,62,61,61,60,60,59,
    59,58,57,57,56,55,54,54,53,52,51,50,49,48,47,46,
    45,44,42,41,40,39,38,36,35,34,32,31,30,28,27,25,
    24,23,21,20,18,17,15,14,12,10,9,7,6,4,3,1
};


// =====================================================================================
    static const UWORD LinearSlideTable[32] =
// =====================================================================================
// Tricky Air -
//  The first value is actually the value for 32.  To use this table, you
//  must go like this: val = index ? LinearSlideTable[index & 31] : 0x10000ul
{
    63670, 65476, 65417, 65358, 65299, 65240, 65182, 65123, 65064, 65005, 64947,
    64888, 64830, 64771, 64713, 64654, 64596, 64538, 64479, 64421, 64363, 64305, 
    64247, 64189, 64131, 64073, 64016, 63958, 63900, 63842, 63785, 63727
};


// =====================================================================================
    static const SBYTE PanbrelloTable[128] =
// =====================================================================================
{
    0,2,3,5,6,8,9,11,12,14,16,17,19,20,22,23,
    24,26,27,29,30,32,33,34,36,37,38,39,41,42,43,44,
    45,46,47,48,49,50,51,52,53,54,55,56,56,57,58,59,
    59,60,60,61,61,62,62,62,63,63,63,64,64,64,64,64,
    64,64,64,64,64,64,63,63,63,62,62,62,61,61,60,60,
    59,59,58,57,56,56,55,54,53,52,51,50,49,48,47,46,
    45,44,43,42,41,39,38,37,36,34,33,32,30,29,27,26,
    24,23,22,20,19,17,16,14,12,11,9,8,6,5,3,2
};


// =====================================================================================
    static int find_empty_voice(MPLAYER *ps, int curchan)
// =====================================================================================
// returns mp_control index of free channel
//
// New Note Action Scoring System:
//  ---------------------------------
//   1) total-volume (fadevol, chanvol, volume) is the main scorer.
//   2) a looping sample is a bonus x2
//   3) a forground channel is a bonus x4
//   4) an active envelope with keyoff is a handicap -x2
{
    MP_VOICE *a;
    int       t, k;
    uint      tvol, p, pp;
    
    for (t=0; t<ps->numvoices; t++)
        if (!(ps->state.voice[t].shared.kick) && Voice_Stopped(ps->vs,t))
            return t;
        
    tvol = 0xffffffUL;
    t = 0;
    p = 0;
    a = ps->state.voice;

    for (k=0; k<ps->numvoices; k++, a++)
        if (!a->shared.kick)
        {
            // Note: a->shared.s should never be null, because if it is, the voice should
            // have properly flagged as stopped above.  If not, then something bad is amiss!
            assert(a->shared.s != NULL);
            pp = a->totalvol << ((a->shared.s->flags & SL_LOOP) ? 1 : 0);

            if (a->master && a==a->master->slave)
                pp <<= 2;
        
            if (pp < tvol)
            {
                tvol = pp;
                t    = k;
            }
        }
    
    if (tvol > 8000*7) return -1;
    return t;
}


// =====================================================================================
    static __inline int Interpolate(int p, int p1, int p2, int v1, int v2)
// =====================================================================================
// Takes the given endpoints and values, and finds the value of the point represented
// by 'p'.  
{
		if ((p1==p2)||(p==p1)) return v1;
    assert(p>=p1 && p<=p2);

    return v1 + ((SLONG)((p - p1) * (v2 - v1)) / (p2 - p1));
}


// =====================================================================================
    static __inline uint getoldperiod(UBYTE note, ULONG speed)
// =====================================================================================
{
    UBYTE n, o;

    assert(speed);
    if (!speed) return 4242;            // <- prevent divide overflow.. (42 eheh)

    n = note % 12;
    o = note / 12;

    return ((8363ul*(ULONG)amigatab[n+1]) >> o ) / speed;
}

// =====================================================================================
    static __inline uint getnewperiod(UBYTE note)
// =====================================================================================
{
    return ((12L*12*16*8)-((ULONG)note*16*8));
}

// =====================================================================================
    static __inline uint getlogperiod(UBYTE note, ULONG fine)
// =====================================================================================
{
    UBYTE n, o;

    //
    //  we use finetune = 0..255 and zero-based note (as opposed to FT2!).
    //  every 128 finetune units is a half-tone. lower 7 bits of finetune
    //  (fine MOD 128) is used for linear interpolation within Amiga
    //  periods table.
    //

    note += fine >> 7;

    n = note % 12;
    o = note / 12;

    return Interpolate(fine&0x7F, 0, 128, amigatab[n], amigatab[n+1]) >> o;
}

// =====================================================================================
    static __inline uint getlinearperiod(UBYTE note, ULONG fine)
// =====================================================================================
{
    // here the situation is different, because we use our
    // own tables. this formula works fine.
    return ((12L*12*16*8)-((ULONG)note*16*8)) - fine + 128;
}

// =====================================================================================
    static __inline uint GetPeriod(uint flags, UBYTE note, ULONG speed)
// =====================================================================================
{
    if (flags & PF_XMPERIODS)
        return (flags & PF_LINEAR) ? getlinearperiod(note,speed) : getlogperiod(note,speed);
    else return (flags & PF_LINEAR) ? getnewperiod(note) : getoldperiod(note,speed);
}


// =====================================================================================
    static __inline ULONG getfrequency(uint flags, ULONG period, ULONG speed)
// =====================================================================================
// XM linear period to frequency conversion
{

    // Original XM equation:
    // return((10L*12*16*4)-((ULONG)note*16*4)-(fine/2)+64);
    // maxscales * notebase * (steps-per-note) - note * (steps-per-note)
    //   This gives us a number which represents our note within its scale.  The steps
    //   per note represents the resolution of the period table.  In this case, 64
    //   periods per step in our note.
    //  
    // My equation, which improves the resolution for higher frequencies by offsetting
    // the scale to be more 'centered.'  Then I improved accuracy all-around by doubling
    // the number of period steps between notes.
    //
    // return( ((12L*12*16*8)-((ULONG)note*16*8)) * 2) -(fine)+128);
    
    // Frequency = 8363*2^((6*12*16*4 - Period) / (12*16*4));
    //
    //  - the '6' is the middle-c octave.  I changed it to 7.

    if (!(flags & PF_LINEAR))
        return ((8363ul*1712ul)*2ul) / period;
    else 
    {
        // Notes:
        //  - XMs use the default 8363 speed and a finetune, just like modules.
        //  - I add 6 to the shifter because that's 8-2.  the -2 is there to re-
        //    center the middle C (thanks to a 12 octave scale now)
        
        ULONG result;
        result = (flags&PF_XMPERIODS ? 8363ul : speed) * lintab[period % 1536];
        result >>= (period / 1536) + 6;
        return result;
    }
}


// =====================================================================================
    static __inline int DoPan(int envpan, int pan)
// =====================================================================================
{
    //DIG

    // the old XM compatable way?  or is it wrong for XMs too?!
    //return(pan + ((envpan * (128-abs(pan-128))) / 256));

    // Air's Official Mikmod Panning thingie:
    //  And just what does a Mikmod Panning thingie do?  It modifies the panning
    //  position (pan) by the panning envelope value.  We take the 

    return envpan*(256 - abs(pan))/256 + pan;
}


// =====================================================================================
    static void StartEnvelope(ENVPR *t, UBYTE flg, UBYTE pts, UBYTE susbeg, UBYTE susend, UBYTE beg, UBYTE end, ENVPT *p)
// =====================================================================================
{
    t->pts    = pts;
    t->susbeg = susbeg;
    t->susend = susend;
    t->beg    = beg;
    t->end    = end;
    t->env    = p;

    t->p = 0;
    t->a = 0;
    t->b = (flg & EF_SUSTAIN) ? 0 : 1;       //((flg & EF_SUSTAIN) && !(keyoff & KEY_OFF)) ? 0 : 1;
}


// =====================================================================================
    static __inline int InterpolateEnv(int p, ENVPT *a, ENVPT *b)
// =====================================================================================
{
    return Interpolate(p, a->pos, b->pos, a->val, b->val);
}


// =====================================================================================
    static int ProcessEnvelope(MPLAYER *ps, ENVPR *t, UBYTE flg, SWORD v)
// =====================================================================================
// This procedure processes all envelope types, include volume, pitch, and
// panning.  Envelopes are defined by a set of points, each with a magnitude
// [relating either to volume, panning position, or pitch modifier] and a
// tick position.
//
//  Envelopes work in the following manner:
//
// (a) Each tick the envelope is moved a point further in its progression.
//   1. For an accurate progression, magnitudes between two envelope points
//      are interpolated.
//
// (b) When progression reaches a defined point on the envelope, values
//     are shifted to interpolate between this point and the next,
//     and checks for loops or envelope end are done.
//
// Misc:
//   Sustain loops are loops that are only active as long as the keyoff
//   flag is clear.  When a volume envelope terminates, so does the current
//   fadeout.
{
    if (!(flg & EF_ON)) return v;

    if (t->pts == 1) return t->env[0].val;
        
    if (t->pts)
    {
        int    a, b;        // actual points in the envelope
        UWORD  p;           // the 'tick counter' - real point being played

        a = t->a;
        b = t->b;
        p = t->p;

        // compute the current envelope value between points a and b

        if (a == b)
            v = t->env[a].val;
        else v = InterpolateEnv(p, &t->env[a], &t->env[b]);

        p++;

        // pointer reached point b?
        // ITs are inclusive, XMs are exclusive. I hate them both. :)
        
        if (flg&EF_INCLUSIVE ? p>t->env[b].pos :  p>=t->env[b].pos)
        {
            a = b++;            // shift points a and b

            // check for loops, sustain loops, or end of envelope
            if (flg & EF_SUSTAIN)
            {
                if (b > t->susend)
                {
                    a = t->susbeg;
                    if (t->susbeg == t->susend)
                        b = a;
                    else b = a + 1;
                    p = t->env[a].pos;
                }
            }
            else if (flg&EF_LOOP && b>t->end)
            {
                a = t->beg;
                if (t->beg == t->end)
                    b = a;
                else b = a + 1;
                p = t->env[a].pos;
            }
            else
            {
                if (b >= t->pts)
                {
                    if (flg&EF_VOLENV && ps->state.channel!=-1)
                    {
                        ps->state.voice[ps->state.channel].keyoff |= KEY_FADE;
                        if (v == 0)
                            ps->state.voice[ps->state.channel].shared.fadevol = 0;
                    }
                    b--;
                    p--;
                }
            }
        }
        t->a = a;
        t->b = b;
        t->p = p;
    }

    return v;
}


// =====================================================================================
    static __inline void DoEnvelopeControl(INT_MOB dat, UBYTE *flag, unsigned int envlen, ENVPR *env)
// =====================================================================================
{
    if (dat.byte_b)
        *flag |= EF_ON;
    else *flag &= ~EF_ON;

    // no need to check for i being null?
    if (dat.hiword.u)
        env->p = env->env[(dat.hiword.u>envlen) ? envlen : dat.hiword.u].pos;
}


// =====================================================================================
    static void DoVibrato(MP_CONTROL *a, uint depth)
// =====================================================================================
{
    UBYTE q;
    UWORD temp;

    q = (a->vibpos>>2) & 0x1f;

    switch(a->wavecontrol & 3)
    {   case 0:     // sine
            temp = VibratoTable[q];
        break;

        case 1:     // ramp down
            q<<=3;
            if(a->vibpos < 0) q = 255-q;
            temp = q;
        break;

        case 2:     // square wave
            temp = 255;
        break;

        case 3:     // Evil random wave
           temp = rand() & 255;
        break;
    }

    temp *=  depth;
    temp >>= 8;

    if(a->vibpos>=0)
        a->shared.period = a->tmpperiod+temp;
    else
        a->shared.period = a->tmpperiod-temp;
}


// =====================================================================================
    static void DoTremolo(MP_CONTROL *a, uint depth)
// =====================================================================================
{
    UBYTE q;
    UWORD temp;

    q = (a->trmpos>>2) & 0x1f;

    switch((a->wavecontrol>>4) & 3)
    {   case 0:    // sine
            temp = VibratoTable[q];
        break;

        case 1:    // ramp down
            q<<=3;
            if(a->trmpos < 0) q = 255-q;
            temp = q;
        break;

        case 2:    // square wave
            temp = 255;
        break;

        case 3:     // Evil random wave
           temp = rand() & 255;
        break;
    }

    temp *=  depth;
    temp >>= 8;

    if(a->trmpos >= 0)
    {   a->volume = a->tmpvolume + temp;
        if(a->volume > VOLUME_FULL) a->volume = VOLUME_FULL;
    } else
    {   a->volume = a->tmpvolume - temp;
        if(a->volume < 0) a->volume = 0;
    }
}


// =====================================================================================
    static void DoToneSlide(MP_CONTROL *a, int portspeed)
// =====================================================================================
{
    int dist;

    // We have to slide a->shared.period towards a->wantedperiod, so
    // compute the difference between those two values

    dist = a->shared.period - a->wantedperiod;

    if( dist==0 || (portspeed > abs(dist)) ) // if they are equal or if portamentospeed is too big
        a->shared.period = a->wantedperiod;        // make tmpperiod equal tperiod
    else if(dist>0)                         // dist>0 ?
        a->shared.period -= portspeed;             // then slide up
    else a->shared.period += portspeed;             // dist<0 -> slide down

    a->tmpperiod = a->shared.period;
}


// =====================================================================================
    static void DoPanbrello(MPLAYER *ps, MP_CONTROL *a, int depth)
// =====================================================================================
{
    UBYTE q;
    SLONG temp;

    q = a->panbpos & 0x7f;

    switch(a->panbwave)
    {   case 0: // sine
           temp = PanbrelloTable[q];
        break;

        // only sinewave is correctly supported right now

        case 1: // ramp down
           q /= 2;
           if(a->panbpos < 0) q = 64-q;
           temp = q;
        break;

        case 2: // square wave
           temp = 64;
        break;

        case 3: // evil random
           temp = rand() & 255;
        break;
    }

    temp *= depth;
    temp /= 8;

    if(a->panbpos >= 0)
    {   a->shared.panning  = ((ps->state.panning[ps->state.channel] == PAN_SURROUND) ? PAN_CENTER : ps->state.panning[ps->state.channel]) + temp;
        if(a->shared.panning > PAN_RIGHT) a->shared.panning = PAN_RIGHT;
    } else
    {   a->shared.panning  = ((ps->state.panning[ps->state.channel] == PAN_SURROUND) ? PAN_CENTER : ps->state.panning[ps->state.channel]) - temp;
        if(a->shared.panning < PAN_LEFT) a->shared.panning = PAN_LEFT;
    }
}

// =====================================================================================
#define DoKeyOff(voice)                                                         \
{                                                                               \
    (voice)->keyoff |= KEY_OFF;                                                 \
                                                                                \
    (voice)->shared.volflg &= ~EF_SUSTAIN;                                      \
    (voice)->shared.panflg &= ~EF_SUSTAIN;                                      \
    (voice)->shared.pitflg &= ~EF_SUSTAIN;                                      \
    (voice)->shared.resflg &= ~EF_SUSTAIN;                                      \
                                                                                \
    if(!((voice)->shared.volflg&EF_ON) || (voice)->shared.volflg&EF_LOOP)       \
        (voice)->keyoff = KEY_KILL;                           /* fade out */    \
}
// =====================================================================================



// =====================================================================================
    static void process_global_effects(MPLAYER *ps)
// =====================================================================================
{
    MP_CONTROL      *a;      // this puppy gets modified by global_geteffect

    UNITRK_EFFECT   *eff, reteff;
    INT_MOB          dat;
    int              lo;

    while(eff = global_geteffect(ps, &reteff, &a))
    {   // Universal Framedelay!  If the RUNONCE flag is set, then the command is
        // executed once on the specified tick, otherwise, the command is simply
        // delayed for the number of ticks specified.
/*		UBYTE framedly;

		if (eff->framedly == UFD_FXXHACK)
			framedly = ps->last_fxx_hack;
		else framedly=ps->last_fxx_hack=eff->framedly;*/

        lo = eff->framedly & UFD_TICKMASK;
        if(lo >= ps->state.sngspd)
            lo = ps->state.sngspd - 1;
        if(eff->framedly & UFD_RUNONCE)
        {   // we only check vbtick if sngspd is not 0, otherwise we just run
            // the effect (or else it would never get run!)
            if(ps->state.sngspd && (lo != ps->state.vbtick)) continue;
        } else if(ps->state.vbtick < lo) continue;

        dat = eff->param;

        switch(eff->effect)
        {   case UNI_GLOB_VOLUME:
                ps->state.volume = dat.u;
            break;

            case UNI_GLOB_VOLSLIDE:
                ps->state.volume += dat.s;
                ps->state.volume = _mm_boundscheck(ps->state.volume,0,GLOBVOL_FULL);
            break;

            case UNI_GLOB_TEMPO:
                if(ps->state.patdly2 || (dat.byte_a < MIN_TEMPO)) break;
                ps->state.bpm = dat.byte_a;
            break;

            case UNI_GLOB_TEMPOSLIDE:
                //if(!ps->state.vbtick) break;
                lo  = ps->state.bpm;
                lo += dat.s;
                ps->state.bpm = _mm_boundscheck(lo, MIN_TEMPO, 0xff);
            break;

            case UNI_GLOB_SPEED:
                if(ps->state.patdly2) break;
                ps->state.sngspd = dat.byte_a;
                ps->state.vbtick = 0;
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
                {   // Loop has ended, so decrememnt the patloop flag
                    if(ps->state.patloop) ps->state.patloop--;
                }
            break;

            case UNI_GLOB_DELAY:       // pattern delay
                if(!ps->state.patdly2)
                {   if(dat.hiword.u) ps->state.patdly   = dat.hiword.u + 1;
                }
                if(dat.loword.u) ps->state.framedly += dat.loword.u;
            break;

            case UNI_GLOB_PATJUMP:
                // FT2, IT give preference to commands in higher channels.
                // Should PT do the same?  I hope so...

                if (ps->state.patdly2)  break;

                if (dat.loword.u > ps->mf->numpos)
                    dat.loword.u = ps->mf->numpos;

                if (ps->state.vbtick == ps->state.sngspd-1)
                {
                    ps->state.posjmp = 2;                           // 2 means we set the new position manually

                    if (ps->state.prev_sngpos > ps->state.sngpos)   // remember minimal value
                        ps->state.prev_sngpos = ps->state.sngpos;

                    ps->state.sngpos = dat.loword.u;
                }
            break;

            case UNI_GLOB_PATBREAK:
                if (ps->state.patbrk || ps->state.patdly2)
                    break;

                if (ps->state.sngpos+1u>=ps->mf->numpos || dat.u>=ps->mf->pattrows[ps->mf->positions[ps->state.sngpos+1]])
                    ps->state.patbrk = 0;
                else ps->state.patbrk = dat.u;

                if (!ps->state.posjmp)
                    ps->state.posjmp = 3;
            break;
        }
    }
}


// --------------------------------
// --> General Player Functions <--
// --------------------------------

// =====================================================================================
    static void pt_playeffects(MPLAYER *ps, MP_CONTROL *a)
// =====================================================================================
{
    UNITRK_EFFECT  reteff, *eff;
    INT_MOB        dat;
    int            lo;

    while(eff = local_geteffect(a, &reteff))
    {   // Universal Framedelay!  If the RUNONCE flag is set, then the command is
        // executed once on the specified tick, otherwise, the command is simply
        // delayed for the number of ticks specified.

		UBYTE framedly;

		if (eff->framedly == UFD_FXXHACK)
			framedly = a->last_fxx_hack;
		else
		{
			framedly=eff->framedly;
			if (eff->effect == UNI_PITCHSLIDE) a->last_fxx_hack=framedly;
		}

        lo = framedly & UFD_TICKMASK;
        if(lo >= ps->state.sngspd)
            lo = ps->state.sngspd - 1;
        if(framedly & UFD_RUNONCE)
        {   // we only check vbtick if sngspd is not 0, otherwise we just run
            // the effect (or else it would never get run!)
            if(ps->state.sngspd && (lo != ps->state.vbtick)) continue;
        } else if(ps->state.vbtick < lo) continue;

        dat = eff->param;

        switch(eff->effect)
        {   case 0:         // No effect!
            break;

            case UNI_ARPEGGIO:
                lo = a->shared.note;

                switch(ps->state.vbtick % dat.byte_d)
                {
                case 1:  lo += (SBYTE)dat.byte_a;  break;
                case 2:  lo += (SBYTE)dat.byte_b;  break;
                case 3:  lo += (SBYTE)dat.byte_c;  break;
                }
                a->shared.period = GetPeriod(ps->flags, lo, a->speed);
                a->ownper = 1;
            break;

            case UNI_VOLUME:
                a->tmpvolume = dat.u;
            break;

            case UNI_CHANVOLUME:
                a->shared.chanvol = dat.u;
            break;

            case UNI_PANNING:
                if (ps->flags & PF_NO_PANNING) break;
                a->shared.panning = dat.s;
                ps->state.panning[ps->state.channel] = dat.s;
            break;

            // ----- Volume Slides -----
    
            case UNI_VOLSLIDE:
                a->tmpvolume += dat.s;
                a->tmpvolume = _mm_boundscheck(a->tmpvolume,0,VOLUME_FULL);
            break;

            case UNI_CHANVOLSLIDE:
                a->shared.chanvol += dat.s;
                a->shared.chanvol  = _mm_boundscheck(a->shared.chanvol,0,CHANVOL_FULL);
            break;
            
            case UNI_TREMOLO_SPEED:
                a->trmpos += dat.u;
            break;

            case UNI_TREMOLO_DEPTH:
                DoTremolo(a, dat.u);
                a->ownvol = 1;
            break;
    
            case UNI_TREMOR:
                if(!dat.u) return;

                a->tremor %= (dat.hiword.u + dat.loword.u);
                a->volume  = (a->tremor < dat.hiword.u ) ? a->tmpvolume : 0;
                a->tremor++;
                a->ownvol  = 1;
            break;

            // ----- Pitch Slides -----

            case UNI_PITCHSLIDE:
				a->tmpperiod -= dat.s;
            break;

            case UNI_NOTESLIDE:
                assert(dat.hiword.u);
                if (dat.hiword.u == 0)
                    break;
                // slide when counter reaches 0
                if (!a->noteslide)
                {
                    a->noteslide = dat.hiword.u;
                    a->anote    += dat.loword.s;
                    a->tmpperiod = GetPeriod(ps->flags, a->anote, a->speed);
                }
                a->noteslide--;
            break;

            case UNI_VIBRATO_SPEED:
                a->vibpos += dat.u;
            break;

            case UNI_VIBRATO_DEPTH:
                DoVibrato(a, dat.u);
                a->ownper = 1;
            break;
    
            case UNI_PORTAMENTO_LEGACY:
                // Old mod-style portamento.  Simple, sweet. Yeah!

                if(a->shared.period)
                {   if(!ps->state.vbtick)
                    {
                        if(a->newsamp && a->shared.s)
                        {
                            // In instrument mode, we override the sample - no changes!
                            // In samples mode we rekick the note (amiga style)

                            if(ps->flags&PF_INST && a->old_s)
                            {
                                a->shared.s      = a->old_s;
                                a->shared.kick  &= ~KICK_NOTE;
                            }/* else
                                a->shared.kick  |= KICK_NOTE;*/
							// ^^ eh?

                        } else a->shared.kick = 0;

                        a->shared.kick  |= KICK_ENV;
                        a->shared.kick  &= ~KICK_NNA;     // don't kick NNAs
                        a->tmpperiod     = a->shared.period;
                    } else
                        DoToneSlide(a, dat.u);

                    a->ownper = 1;
                }
            break;

            case UNI_PORTAMENTO:
                if(a->shared.period)
                {   if(!ps->state.vbtick)
                    {   if(a->newsamp && a->shared.s)
                        {   // some sort of funky frequency setting is supposed
                            // to go here.. maybe I'll fuck with it later.
                            //a->shared.period = (a->shared.period * a->speed) / a->newsamp;
                            //a->shared.period = GetPeriod(ps->flags, (5*12), getfrequency(ps->flags,a->shared.period,a->speed));
                            //a->shared.start  = SF_START_CURRENT;
                        } else a->shared.kick  &= ~KICK_NOTE;

                        // NOTE: Standard mode doesn't rekick envelopes!

                        a->shared.kick  &= ~KICK_NNA;
                        a->tmpperiod     = a->shared.period;
                    } else DoToneSlide(a, dat.u);

                    a->ownper = 1;
                }
            break;

    
            // ----- Pan Slides -----

            case UNI_PANSLIDE:
                lo =  (a->shared.panning == PAN_SURROUND) ? PAN_CENTER : a->shared.panning;
                lo += dat.s;
                lo =  _mm_boundscheck(lo,PAN_LEFT,PAN_RIGHT);
                a->shared.panning = ps->state.panning[ps->state.channel] = lo;
            break;

            case UNI_PANBRELLO_SPEED:
                a->panbpos += dat.u;
            break;

            case UNI_PANBRELLO_DEPTH:
                DoPanbrello(ps, a, dat.u);
            break;
    
            // ----- Effects Waveform Controls -----
    
            case UNI_VIBRATO_WAVEFORM:       // set vibrato waveform
                a->wavecontrol &= 0xf0;
                a->wavecontrol |= dat.u & 0x0f;
            break;

            case UNI_TREMOLO_WAVEFORM:       // set tremolo waveform
                a->wavecontrol &= 0x0f;
                a->wavecontrol |= (dat.u & 0x0f) << 4;
            break;

            case UNI_PANBRELLO_WAVEFORM:
                a->panbwave &= 0x0f;
                a->panbwave |= (dat.u & 0x0f) << 4;
            break;

            // ----- Note Maipulations -----
    
            case UNI_NOTEKILL:       // Kills a note (stops playing completely)
                //if(a->slave) Voice_Stop(a->slavechn);
                a->tmpvolume = a->shared.fadevol = 0;
            break;
    
            case UNI_NOTEDELAY:       // note delay
                a->notedelay = dat.u;
            break;

            case UNI_RETRIG:
                assert(dat.loword.s);
                if (dat.loword.s == 0)
                    break;
                if (a->retrig == 0)
                {
                    // when retrig counter reaches 0,
                    // reset counter and restart the sample
                    if (dat.loword.s > 0)
                        a->retrig = dat.loword.s;
                    else a->retrig = ps->state.sngspd / -dat.loword.s;

                    a->shared.kick |= KICK_NOTE;

                    if(ps->state.vbtick)                     // don't slide on first retrig
                    {   switch(dat.hiword.u)
                        {   case 1:
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                                a->tmpvolume -= 1 << dat.hiword.u;
                            break;
        
                            case 6:
                                a->tmpvolume = (2*a->tmpvolume)/3;
                            break;

                            case 7:
                                a->tmpvolume = a->tmpvolume>>1;
                            break;

                            case 9:
                            case 0xa:
                            case 0xb:
                            case 0xc:
                            case 0xd:
                                a->tmpvolume += 1 << (dat.hiword.u-8);
                            break;

                            case 0xe:
                                a->tmpvolume = (3*a->tmpvolume)/2;
                            break;

                            case 0xf:
                                a->tmpvolume = a->tmpvolume<<1;
                            break;
                        }
                        a->tmpvolume = _mm_boundscheck(a->tmpvolume, 0, VOLUME_FULL);
                    }
                }
                a->retrig--; // countdown 
            break;
    
            case UNI_OFFSET_LEGACY:
                // this effect is provided for compatability to IT/S3M modules only,
                // since they have that awkward style of hiworld loword crap.

                if (dat.loword.u) a->offset_lo = dat.loword.u;
				if (dat.hiword.u) a->offset_hi = dat.hiword.u;
				
                a->shared.start = (int)(a->offset_hi<<16) + a->offset_lo;
                if(a->shared.s && (a->shared.s->flags & SL_LOOP) && (a->shared.start > (SLONG)a->shared.s->loopend)) a->shared.start = a->shared.s->loopstart;
            break;
			case UNI_OFFSET_HI:
				a->offset_hi = dat.hiword.u;
                a->shared.start = (int)(a->offset_hi<<16) + a->offset_lo;
                if(a->shared.s && (a->shared.s->flags & SL_LOOP) && (a->shared.start > (SLONG)a->shared.s->loopend)) a->shared.start = a->shared.s->loopstart;
				break;
            case UNI_OFFSET:
                a->shared.setpos = dat.u;
                if(a->shared.s && (a->shared.s->flags & SL_LOOP) && (a->shared.setpos > (SLONG)a->shared.s->loopend)) a->shared.setpos = a->shared.s->loopstart;
            break;

            case UNI_REVERSE:
                //todo process effect without note
                if (a->shared.kick&KICK_NOTE && a->shared.s)
                {
                    a->shared.kick |= KICK_REVERSE;
                    a->shared.start = a->shared.s->length - dat.u;
                    
                    if (a->shared.s->flags&SL_LOOP && a->shared.start<(SLONG)a->shared.s->loopstart)
                        a->shared.start = a->shared.s->loopend;
                    else if (a->shared.start < 0)
                        a->shared.start = 0;
                }
            break;

            case UNI_KEYOFF:
                a->keyoff = KEY_KILL;
                if (!(a->shared.volflg & EF_ON))
                    a->shared.fadevol = 0;
    
            case UNI_KEYFADE:
                DoKeyOff(a);
            break;

            // Instrument Features Controls (Envelopes and NNAs)

            case UNI_ENVELOPE_CONTROL:
                if(!a->slave || !a->shared.i) break;

                switch (dat.byte_a)
                {   
                    // Note to self:  Should these reset only the envelope which is changed or
                    // all of the envelopes together (which is old behavior)?  Well, I should
                    // take the time to find out!  someday....
                    
                    case 0:
                        DoEnvelopeControl(dat, &a->shared.volflg, a->shared.i->volenv[a->shared.i->volpts-1].pos, &a->slave->venv);
                        a->shared.kick |= KICK_VOLENV;
                    break;

                    case 1:
                        DoEnvelopeControl(dat, &a->shared.panflg, a->shared.i->panenv[a->shared.i->panpts-1].pos, &a->slave->penv);
                        a->shared.kick |= KICK_PANENV;
                    break;
                    
                    case 2:
                        DoEnvelopeControl(dat, &a->shared.pitflg, a->shared.i->pitenv[a->shared.i->pitpts-1].pos, &a->slave->cenv);
                        a->shared.kick |= KICK_PITENV;
                    break;

                    case 3:
                        DoEnvelopeControl(dat, &a->shared.resflg, a->shared.i->resenv[a->shared.i->respts-1].pos, &a->slave->renv);
                        a->shared.kick |= KICK_RESENV;
                    break;
                }
            break;

            case UNI_NNA_CONTROL:
                a->shared.nna = (dat.byte_a & NNA_MASK) | NNA_OWN;
                if (a->slave) a->slave->shared.nna = a->shared.nna;
            break;

            case UNI_NNA_CHILDREN:
                for (lo=0; lo<ps->numvoices; lo++)
                {
                    if(ps->state.voice[lo].master == a)
                    {
                        switch(dat.u)
                        {
                            case NNA_CUT:
                                ps->state.voice[lo].shared.fadevol = 0;
                            break;

                            case NNA_OFF:
                                DoKeyOff(ps->state.voice + lo);
                            break;

                            case NNA_FADE:
                                ps->state.voice[lo].keyoff |= KEY_FADE;
                            break;
                        }
                    }
                }           
            break;

            case UNI_FILTER_CUTOFF:
//TEST                if(!(ps->flags & PF_NO_RESONANCE))
//TEST                    a->shared.cutoff    = dat.s;
            break;

            case UNI_FILTER_RESONANCE:
//TEST                if(!(ps->flags & PF_NO_RESONANCE))
//TEST                    a->shared.resonance = dat.u;
            break;

            case UNI_SETSPEED:
                a->speed = dat.u;
                a->wantedperiod = a->tmpperiod = GetPeriod(ps->flags, a->shared.note, a->speed);
            break;

        }
    }
}


// =====================================================================================
    uint Player_HandleTick(MD_VOICESET *vs, MPLAYER *ps)
// =====================================================================================
{
    const UNIMOD *pf = ps->mf;
    UNISAMPLE    *s;

    MP_CONTROL   *a;
    EXTSAMPLE    *es;
    INSTRUMENT   *i;

    if (ps->ended) return 0;

    ps_forbid();

    if(vs)
    {   ps->vs        = vs;
        ps->numvoices = vs->voices;
    }
    
    if(++ps->state.vbtick >= (ps->state.sngspd + ps->state.framedly))
    {   ps->state.patpos++;
        ps->state.vbtick = ps->state.framedly = 0;

        // process pattern-delay.  ps->state.patdly2 is the counter and ps->state.patdly
        // is the command memory.

        if(ps->state.patdly)
        {   ps->state.patdly2 = ps->state.patdly;
            ps->state.patdly  = 0;
        }

        if(ps->state.patdly2)
        {   // patterndelay active
            if(--ps->state.patdly2) ps->state.patpos--;    // so turn back ps->state.patpos by 1
        }

        // Do we have to get a new patternpointer ?
        //  (when ps->state.patpos reaches the pattern length or when
        //  a patternbreak is active)

        if(!ps->state.posjmp && (ps->state.patpos == ps->state.numrow)) ps->state.posjmp = 3;

        if(ps->state.posjmp)
        {
            // Jump to the specified pattern
            // -----------------------------
            // when posjmp == 2 then we don't change.
            // also: check to make sure the position information is legal, by doing
            // a while-loop until a valid position is found.

            ps->state.sngpos += ps->state.posjmp - 2;

            if (ps->state.sngpos >= (int)ps->mf->numpos)
            {   
                if (!(ps->flags & PF_LOOP))
                {
                    ps->ended = TRUE;
                    ps_unforbid();
                    return 0;
                }
                MP_LoopSong(ps, pf);
            }

            ps->state.patpos = ps->state.patbrk;
            ps->state.patbrk = ps->state.posjmp = 0;
            ps->state.numrow = pf->pattrows[pf->positions[ps->state.sngpos]];

            if(ps->state.sngpos < 0)                 ps->state.sngpos = pf->numpos-1;
            if(ps->state.patpos > ps->state.numrow)  ps->state.patpos = ps->state.numrow;

            // reset a->row track stuff to new song position.
            pseek(ps);
            _mmlogd1("\nSONG POSITION CHANGE: %d\n",ps->state.sngpos);
        } else
            // no position jump, so go to the next row in the current pattern.
            if(!ps->state.patdly2) nextrow(ps);


        if (!ps->state.patdly2)
        {
            if (!ps->state.patloop)
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
                            // seek
                            pseek(ps);
                            // reset all voices
                            Player_WipeVoices(ps);
                        }
                        else
                        {
_NO_LUCK_:
                            ps->ended = TRUE;
                            ps_unforbid();
                            return 0;
                        }
                    }
                    else MP_WipePosPlayed(ps);
                }
                MP_SetPosPlayed(ps);
            }
        }

        if (!ps->state.patdly2)
        {
            uint t;

            for (t=0; t<pf->numchn; t++)
            {
                int          funky;
                UNITRK_NOTE  note;
				BOOL         hack_notefade = 0;

                ps->state.channel = t; a = &ps->state.control[t];

                if (a->row == NULL) continue;               // end of track, so skip it!

                // make sure a few things are zero'd
                a->old_s         = NULL;
                a->newsamp       = 0;
                a->shared.setpos = SF_START_CURRENT;
                funky            = 0;

                // Get the note data for this row and process it
                getnote(a, &note);
				if (note.noteex)
					hack_notefade = 1;

                // process the instrument first, because the note code block uses data
                // that the instrument code block has to set.

                if (note.inst)
                {
                    note.inst--;

                    if (ps->flags & PF_INST)
                    {
                        if (note.inst >= pf->numins)
                        {
                            a->shared.kick |= KICK_NNA;
                            a->shared.i = NULL;
                            continue;
                        }
                        if (a->shared.i != &pf->instruments[note.inst])
                        {
                            // instrument chage - force rekick of envelopes (no carry!)
                            a->shared.i = &pf->instruments[note.inst];
                            funky |= 8;
                        }
                    }
                    else if (note.inst >= pf->numsmp)
                    {
                        a->shared.i = NULL;     //DIG ???
                        continue;
                    }

					a->ins_set = 1;

                    a->retrig  = 0;
                    a->tremor  = 0;

                    // Protracker / ImpulseTracker Logic
                    // ---------------------------------
                    // Reset volumes (but not fadeout or envelopes!)
                    // Kick sample on sample-change only (and reset all).

                    // Fasttracker 2 Logic
                    // --------------------
                    // Never kick sample (only note kicks samples), so old note plays
                    // even on sample change.  Reset volumes, fades, envelopes!
                    
                    if (!(ps->flags & PF_XM_INST))
                    {
                        if (a->shared.sample != note.inst)
                        {
                            a->shared.kick   = KICK_ALL;
                            a->shared.sample = note.inst;

                            // special case, when instrument specification
                            // resets current period to the initial value
                            if (ps->flags & PF_OVR_PERIOD)
                                funky |= 1;
                        }
                        funky |= 2;
                    }
                    else
                    {
                        if (a->shared.sample == note.inst)
                            funky |= 2;
                        else funky |= 4;

                        a->shared.kick |= KICK_FADES | KICK_ENV;
                    }
                }

                if (note.note && a->ins_set)
                {
                    a->anote        = note.note - 1;
					a->shared.kick  = hack_notefade ? 0 : KICK_ALL;
                    a->shared.start = PF_START_BEGINNING;

                    // retrig tremolo and vibrato waves ?

                    if (!(a->wavecontrol & 0x80)) a->trmpos = 0;
                    if (!(a->wavecontrol & 0x08)) a->vibpos = 0;
                    if (!a->panbwave) a->panbpos = 0;

                    funky |= 1;
                    if (ps->flags & PF_XM_INST) funky |= 8; // FT2 doesn't carry when a note is specified!
                }

                if (funky & ~4)
                {
                    // - FT2 Fix - 
                    // -----------
                    // FT2 doesn't actually take the inst specification into consideration
                    // unless it is paired with a note!

                    if ((funky & 5) == 5)
                    {
                        a->shared.sample = note.inst;
                        funky |= 2;
                    }

                    i = a->shared.i;

                    if (ps->flags & PF_INST)
                    {
                        if (!i || i->samplenumber[a->anote]>=pf->numsmp)
                        {
                            // illegal sample value, so don't kick it!
                            // Note: we can't set s to null : proper behaviour is to ignore the
                            // sample change completely (ie, continue playing old sample)
                            a->shared.kick = 0;
                            a->old_s       = NULL;
                            a->newsamp     = 0;
                            continue;
                        }
                        s  = &pf->samples[i->samplenumber[a->anote]];
                        es = pf->extsamples==NULL ? NULL : &pf->extsamples[i->samplenumber[a->anote]];
                        a->shared.note = i->samplenote[a->anote];

//CUT                        ps->usesmp[i->samplenumber[a->anote]] = TRUE;
//CUT                        ps->useins[a->shared.sample]          = TRUE;
                    }
                    else
                    {
                        a->shared.note = a->anote;
                        s  = &pf->samples[a->shared.sample];
                        es = pf->extsamples==NULL ? NULL : &pf->extsamples[a->shared.sample];

//CUT                        ps->usesmp[a->shared.sample] = TRUE;
                    }

                    if (a->shared.s != s)
                    {
                        a->old_s     = a->shared.s;
                        a->shared.s  = s;
                        a->shared.es = es;
                        a->newsamp   = 1;
                    }

                    // Channel or Sample-determined default panning?
                    // (Note that the instrument panning can override below!)

                    a->shared.panning = ps->state.panning[t];
                    if (s->flags & PSF_OWNPAN)
						a->shared.panning = s->panning;
					else if (i && i->flags&IF_OWNPAN)
                        a->shared.panning = i->panning;

                    a->shared.handle  = s->handle;
                    a->speed          = s->speed;

                    // Don't reset resonance/cutoff.  If the user changes them with Zxx in
                    // samples-mode, those should stick!

                    if (i)
                    {
                        if (i->pitpansep)
                        {
                            if (i->flags & IF_OWNPAN)       //DIG should we do it?
                                a->shared.panning = i->panning;

                            if (a->shared.panning == PAN_SURROUND)
                                a->shared.panning = PAN_CENTER;

                            a->shared.panning += (a->anote - i->pitpancenter) * i->pitpansep / 8;
                            a->shared.panning  = _mm_boundscheck(a->shared.panning, PAN_LEFT, PAN_RIGHT);
                        }
                        a->shared.pitflg = i->pitflg;
                        a->shared.volflg = i->volflg;
                        a->shared.panflg = i->panflg;
                        a->shared.resflg = i->resflg;  
                        if (!(a->shared.nna & NNA_OWN))
                            a->shared.nna = i->nnatype;
                        a->dca           = i->dca;
                        a->dct           = i->dct;

//TEST                        if (i->flags & IF_USE_CUTOFF)    a->shared.cutoff    = i->cutoff;
//TEST                        if (i->flags & IF_USE_RESONANCE) a->shared.resonance = i->resonance;
                    }
                    else
                    {
                        a->shared.pitflg  = 0;  a->shared.volflg  = 0;
                        a->shared.panflg  = 0;  a->shared.resflg  = 0;
                        a->shared.nna     = 0;
                        a->dca            = 0;  a->dct            = 0;
                    }

                    // Funky block only executed if the instrument was specified and valid.
                    // Addition: FT2 is stupid, and needs to be force run when newnst==inst
                    //  (see inst section above for more shitty logic).

                    if (funky & 2)
                    {
                        // IT's random volume variations:  0:8 bit fixed, and one bit for sign.
                        a->volume    = a->shared.s->volume;
                        a->tmpvolume = a->shared.s->volume;

                        if (i)
                        {
                            if (i->rvolvar)
                            {
                                a->tmpvolume += (a->tmpvolume * ((SLONG)i->rvolvar * (SLONG)((rand() & 511)-255))) / 25600;
                                a->tmpvolume  = _mm_boundscheck(a->tmpvolume, 0, VOLUME_FULL);
                                a->volume     = a->tmpvolume;
                            }

                            if (i->rpanvar && a->shared.panning!=PAN_SURROUND)
                            {
                                a->shared.panning += (a->shared.panning * ((SLONG)i->rpanvar * (SLONG)((rand() & 511)-255))) / 25600;
                                a->shared.panning  = _mm_boundscheck(a->shared.panning, PAN_LEFT, PAN_RIGHT);
                            }

                            if (!(funky & 8))
                            {
                                if (i->volflg & EF_CARRY) a->shared.kick &= ~KICK_VOLENV;
                                if (i->panflg & EF_CARRY) a->shared.kick &= ~KICK_PANENV;
                                if (i->pitflg & EF_CARRY) a->shared.kick &= ~KICK_PITENV;
                                if (i->resflg & EF_CARRY) a->shared.kick &= ~KICK_RESENV;
                            }
                        }
                    }

                    if (funky & 1)
                    {
                        a->wantedperiod = a->tmpperiod = GetPeriod(ps->flags, a->shared.note, a->speed);
					    a->shared.org_note = a->anote;
                    }

                    a->keyoff = KEY_KICK;
                }
            }
        }
    }

    // -- Process Global Channel Effects --

    ps->state.globtrk_pos = 0;
    process_global_effects(ps);

    // -- Process Local Channel Effects --

    if(ps->state.voice)
    {

    uint          t;
    MP_VOICE     *aout;     // current audout (slave of audtmp) it's working on

    for(t=0; t<pf->numchn; t++)
    {
		ps->state.channel = t;
        a = &ps->state.control[t];
        
        if((aout = a->slave) != NULL)
            a->shared.fadevol = aout->shared.fadevol;

        if(!a->row) continue;

        a->ownper = a->ownvol = 0;
        a->pos    = 0;                    // make sure we start form the beginning of the effects.

        pt_playeffects(ps, a);
        if(!a->ownper) a->shared.period = a->tmpperiod;
        if(!a->ownvol) a->volume        = a->tmpvolume;

        if(a->shared.s)
        {   if(a->shared.i)
                a->shared.volume = (a->volume * (a->shared.es ? a->shared.es->globvol : 64) * a->shared.i->globvol) / 2048;  // max val: 256
            else
                a->shared.volume = (a->volume * (a->shared.es ? a->shared.es->globvol : 64)) / 32;  // max val: 256
            if(a->shared.volume > 256) a->shared.volume = 256;
        }
    }

    a = ps->state.control;
    if (ps->flags & PF_NNA)
    {
        for (t=0; t<pf->numchn; t++, a++)
        {
			if (a->notedelay)
				continue;

            if (a->shared.kick & KICK_NNA)
            {
                if (a->slave)
                {
                    a->shared.nna &= NNA_MASK;
                    aout = a->slave;

                    if (aout->shared.nna &= NNA_MASK)
                    {
                        // oh boy, we have to do an NNA
                        // Make sure the old MP_VOICE channel knows it has no master now!

                        a->slave = NULL;                    // assume the channel is taken by NNA
                        aout->has_master = FALSE;
    
                        switch (aout->shared.nna)
                        {
                            case  NNA_CONTINUE:             // continue note, do nothing
                            break;
    
                            case  NNA_OFF:                  // note off
                                DoKeyOff(aout);
                            break;

                            case  NNA_FADE:
                               aout->keyoff |= KEY_FADE;
                            break;
                        }
                    }
                }
    
                if (a->dct != DCT_OFF)
                {
                    uint t2;

                    for (t2=0; t2<vs->voices; t2++)
                    {
                        if(!Voice_Stopped(vs, t2) && ps->state.voice[t2].masterchn==(int)t &&
                           a->shared.sample==ps->state.voice[t2].shared.sample)
                        {
                            BOOL kill = FALSE;

                            switch (a->dct)
                            {
                                case DCT_NOTE:
                                   if(a->anote == ps->state.voice[t2].shared.org_note)
                                       kill = TRUE;
                                break;
                
                                case DCT_SAMPLE:
                                   if(a->shared.handle == ps->state.voice[t2].shared.handle)
                                       kill = TRUE;
                                break;
    
                                case DCT_INST:
                                   kill = TRUE;
                                break;
                            }
    
                            if (kill)
                            {
                                switch (a->dca)
                                {
                                    case DCA_CUT:
                                       ps->state.voice[t2].shared.fadevol = 0;
                                       a->slave = &ps->state.voice[a->slavechn=t2];
                                    break;
    
                                    case DCA_OFF:
                                       DoKeyOff(ps->state.voice + t2);
                                    break;
    
                                    case DCA_FADE:
                                       ps->state.voice[t2].keyoff |= KEY_FADE;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    a = ps->state.control;
    for (t=0; t<pf->numchn; t++, a++)
    {
        if (a->notedelay || !a->shared.s)
            continue;

        if (a->shared.kick & KICK_NNA)
        {
            // If no channel was cut above, find an empty or quiet channel here
            if (ps->flags & PF_NNA)
            {
                int newchn;

                if (a->slave==NULL && (newchn=find_empty_voice(ps, t))!=-1)
                    a->slave = &ps->state.voice[a->slavechn = newchn];
            }
            else a->slave = &ps->state.voice[a->slavechn = t];

            // Assign parts of MP_VOICE only done for a KICK!

            if ((aout = a->slave) != NULL)
            {
                if (aout->has_master && aout->master)
                    aout->master->slave = NULL;     //DIG unset has_master?
                a->slave = aout;
                aout->master     = a;
                aout->masterchn  = t;
                aout->has_master = 1;
            }
        }
        else aout = a->slave;

        if (aout)
        {
            aout->shared  = a->shared;
            aout->keyoff |= a->keyoff;
        }
        a->shared.kick = 0;
    }

    // Now set up the actual hardware channel playback information

    for(t=0; t<vs->voices; t++)
    {   int   envpan = 0, envvol = 256, envpit = 0, envres = 128;
        long  tmpvol;
        SLONG vibval, vibdpt;

        aout = &ps->state.voice[ps->state.channel = t];
        i  = aout->shared.i;
        s  = aout->shared.s;
        es = aout->shared.es;

        if(!s) continue;

        if(s->length == 0)
        {   Voice_Stop(vs,t);
            continue;
        }

        if(aout->shared.period < 2)
        {
            aout->shared.period = 2;
            _mmlogd("mplayer > Warning: Period clipped at 2");
        }
        
        
        if(aout->shared.kick & KICK_NOTE)
        {
            uint   flags = s->flags;

//TEST            if(aout->shared.resflg&EF_ON || aout->shared.cutoff!=128 || aout->shared.resonance)
//TEST                flags |= SL_RESONANCE_FILTER;

            if (aout->shared.kick & KICK_REVERSE)
                flags |= SL_REVERSE;

            Voice_Play(vs, t, s->handle,
                       aout->shared.start==PF_START_BEGINNING ? (s->flags&PSF_UST_LOOP ? s->loopstart : 0) : aout->shared.start,
                       s->length, s->loopstart, s->loopend, s->susbegin, s->susend, flags);

            aout->keyoff = KEY_KICK;
        }

        if(aout->shared.kick & KICK_FADES)
        {   
            aout->shared.fadevol  = 0x10000;
            aout->aswppos         = 0;
        }

        if (i)
        {
            if (aout->shared.kick & KICK_VOLENV) StartEnvelope(&aout->venv, aout->shared.volflg, i->volpts, i->volsusbeg, i->volsusend, i->volbeg, i->volend, i->volenv);
            envvol = ProcessEnvelope(ps, &aout->venv, aout->shared.volflg, 256);
            
            if (aout->shared.kick & KICK_PANENV) StartEnvelope(&aout->penv, aout->shared.panflg, i->panpts, i->pansusbeg, i->pansusend, i->panbeg, i->panend, i->panenv);
            envpan = ProcessEnvelope(ps, &aout->penv, aout->shared.panflg, 0);
            
            if (aout->shared.kick & KICK_PITENV) StartEnvelope(&aout->cenv, aout->shared.pitflg, i->pitpts, i->pitsusbeg, i->pitsusend, i->pitbeg, i->pitend, i->pitenv);
            envpit = ProcessEnvelope(ps, &aout->cenv, aout->shared.pitflg, 0);

            if (aout->shared.kick & KICK_RESENV) StartEnvelope(&aout->renv, aout->shared.resflg, i->respts, i->ressusbeg, i->ressusend, i->resbeg, i->resend, i->resenv);
            envres = ProcessEnvelope(ps, &aout->renv, aout->shared.resflg, 128);
        }

        aout->shared.kick = 0;

        tmpvol  = aout->shared.fadevol;      // max 65536
        tmpvol *= aout->shared.chanvol;      // * max 64
        tmpvol *= aout->shared.volume;       // * max 256
        tmpvol /= 32768L;                    // tmpvol is max 32768
        aout->totalvol = tmpvol>>2;          // totalvolume used to determine samplevolume
        tmpvol *= envvol;                    // * max 256
        tmpvol *= ps->state.volume;          // * max 128
        tmpvol /= 4194304UL * 2;             // max 128

        if (ps->volume_table)                // volume translation
            tmpvol = ps->volume_table[tmpvol];

        if (aout->masterchn!=-1 && ps->state.control[aout->masterchn].muted)   // Channel Muting Line
            Voice_SetVolume(vs, t, 0);
        else Voice_SetVolume(vs, t, tmpvol);

        if (aout->shared.panning == PAN_SURROUND)
            Voice_SetPanning(vs, t, PAN_SURROUND, 0);
        else
        {
            if (aout->shared.panflg & EF_ON)
                Voice_SetPanning(vs, t, DoPan(envpan, aout->shared.panning), 0);
            else Voice_SetPanning(vs, t, aout->shared.panning, 0);
        }

        if (aout->shared.setpos != SF_START_CURRENT)
        {
            //FIXME the whole thing should be changed
            Voice_SetPosition(vs,t,aout->shared.setpos);
            ps->state.control[t].shared.setpos = SF_START_CURRENT;
        }


        // Resonant Filter logic.
        // ----------------------
        // a sample's resonant filter, when enabled, works as a modifier on top of the 
        // instrument's resonant filter, if enabled.

//TEST
        /*
        if(aout->shared.resflg & EF_ON)
            Voice_SetResonance(vs,t, envres*(aout->shared.cutoff==128 ? 128 : aout->shared.cutoff)/128, aout->shared.resonance);
        else
            Voice_SetResonance(vs,t, aout->shared.cutoff, aout->shared.resonance);
        */


        // Sample/Instrument auto-vibrato logic!
        // -------------------------------------

        if(aout->shared.period)
        {   if(es && es->vibdepth)
            {   switch(es->vibtype)
                {   case 0:     // sine wave
                        vibval = avibtab[es->avibpos & 127];
                        if(es->avibpos & 0x80) vibval = -vibval;
                    break;
    
                    case 1:     // Square wave
                        vibval = 64;
                        if(es->avibpos & 0x80) vibval = -vibval;
                    break;
    
                    case 2:     // ramp down
                        vibval = 63-(((es->avibpos + 128) & 255) >> 1);
                    break;
    
                    case 3:     // ramp up
                        vibval = (((es->avibpos + 128) & 255) >> 1) - 64;
                    break;
    
                    case 4:     // random
                        vibval = rand() & 63;
                    break;
                }
    
                if(es->vibflags & AV_IT)
                {   if((aout->aswppos >> 8) < es->vibdepth)
                    {   aout->aswppos += es->vibsweep;
                        vibdpt = aout->aswppos;
                    } else
                        vibdpt = es->vibdepth << 8;
                    vibval = (vibval*vibdpt) / 32768;
                } else   // do XM style auto-vibrato
                {   if(!(aout->keyoff & KEY_OFF))
                    {   if(aout->aswppos < es->vibsweep)
                        {   vibdpt = (aout->aswppos * es->vibdepth) / es->vibsweep;
                            aout->aswppos++;
                        } else
                            vibdpt = es->vibdepth;
                    } else
                    {   // key-off -> depth becomes 0 if final depth wasn't reached
                        // or stays at final level if depth WAS reached

                        if(aout->aswppos >= es->vibsweep)
                           vibdpt = es->vibdepth;
                        else
                           vibdpt = 0;
                     }
                     vibval = (vibval*vibdpt) / 256;
                 }

                 // update vibrato position
                 es->avibpos = (es->avibpos + (es->vibrate/2)) & 0xff;
            } else vibval = 0;

            if (!(ps->flags&PF_LINEAR) && ps->flags&PF_LINEAR_PITENV)
            {
                unsigned long   freqboy;
                register INT64U freqout;
                int             teepee, tp = (envpit*32)+vibval, plop;

                // add envpit with vibval's 32 factor, then add the vibval remainder
                // later, using the 32 entry linear slide table!
                //     63670 = slide down by 32 units.
                //   0x10780 = slide up by 32 units.

                if (tp)
                {
                    plop = abs(tp);
                    freqboy = plop ? LinearSlideTable[plop & 31] : 0x10000ul;
                    if (plop /= 32)
                    {
                        for (teepee = plop-1; teepee; teepee--)
                        {
                            freqboy *= 63670ul; freqboy >>= 16;
                        }
                    }

                    freqout = tp>0 ? 0x100000000UL/freqboy : freqboy;
                    Voice_SetFrequency(vs,t,((INT64U)getfrequency(pf->flags,aout->shared.period,s->speed) * freqout) >> 16);
                }
                else Voice_SetFrequency(vs, t, getfrequency(pf->flags, aout->shared.period, s->speed));
            }
            else Voice_SetFrequency(vs, t, getfrequency(pf->flags, aout->shared.period - vibval - envpit*64, s->speed));
        }

        if (aout->shared.fadevol == 0)     // check for a dead note (fadevol = 0)
        {   
            // Kill the voice in the mixer, and remove the sample association from
            // the player.  This way, when a new note plays, if it is something dependant
            // on the prev note (porta), it won't be stupid.

            Voice_Stop(vs,t);
			
			if (aout->has_master && aout->master && !aout->master->notedelay)
                aout->master->shared.s = NULL;
	        aout->shared.s   = NULL;
        }
        else
        {   // if keyfade, start substracting
            // fadeoutspeed from fadevol:

            if (i && aout->keyoff&KEY_FADE)
            {
                if(aout->shared.fadevol > i->volfade)
                    aout->shared.fadevol -= i->volfade;
                else aout->shared.fadevol = 0;
            }
        }

        if(!(aout->keyoff & KEY_OFF))  Voice_ReleaseSustain(vs, t);

        #ifdef MM_DEBUG_DISPLAY
        // The way this stupid thing works is that you hard-code which channel you want to
        // show, and it shows it.  Pretty damn simple.
        if(aout->masterchn == 0)
        {
            ulong  pan = DoPan(envpan,aout->shared.panning);
            
            _mmlog("sp:%-3d pp:%-4d freq:%-7d pan:%-4d vol:%-4d pit:%d",
               ps->state.sngpos,ps->state.patpos, getfrequency(pf->flags,aout->shared.period,s->speed),
               (aout->shared.panning == PAN_SURROUND) ? 512 : ((aout->shared.panflg & EF_ON) ? pan : aout->shared.panning),
               envvol, envpit);

            //_mmlog("row: %4d  sngspd: %4d  tick: %4d  framedly: %4d",ps->state.sngpos, ps->state.sngspd, ps->state.vbtick, ps->state.framedly);
        }
        #endif

    }
    }  // [if(ps->state.voice)]

    ps->state.curtime += (12500l * 64l) / (ps->state.bpm * 5l);

    // failsafe, for songs that don't loop properly:

    if(ps->state.curtime >= MAX_TIME)
    {
        _mmlog("We broke our booboo: Song reached 100 minutes!");
        ps->ended = TRUE;
    }

    if(ps->mf->strip_threshold)
    {
        // end-song stripping logic
        // ------------------------
        // removes the trailing silence from the end of a song.  This section of
        // code is dependant on variables having been set by Unimod_StripSilence().
        // Once we've gone past the 

        if((ps->state.patpos >= pf->patpos_silence) && (ps->state.sngpos >= pf->sngpos_silence))
        {
            ps->state.strip_timeout -= (12500L * ps->state.sngspd) / (ps->state.bpm * 5L);
            if (ps->state.strip_timeout <= 0)
            {
                // End the Song Early!

                MP_WipePosPlayed(ps);
                ps->state.looping--;
                if(!(ps->flags&PF_LOOP) || ps->state.looping<=0)
                {
                    ps->ended = TRUE;
                    ps_unforbid();
                    return 0;
                }
                MP_LoopSong(ps, pf);
                ps->state.posjmp = 2;
            }
        }
    }

    if (ps->fadespeed)
    {   
        // simple-but-effective fast fadeout
        int k = vs->volume + ps->fadespeed;

        if ((ps->fadespeed>0 && k>=ps->fadedest) || (ps->fadespeed<0 && k<=ps->fadedest))
        {
            k = ps->fadedest;
            ps->fadespeed = 0;
        }

        Voiceset_SetVolume(vs,  k);
    }
    else if(ps->volfade.differential && ps->state.curtime>ps->volfade.starttime && vs->volume!=ps->volfade.destvol)
    {
        // Complex-but-also-effective timed fadeout.
        // determine the current distance between start, current, and dest times, and calculate
        // the volume accordingly!

        long past = ps->state.curtime - ps->volfade.starttime;
        int  newvol;
        
        if (ps->volfade.startvol == MP_VOLUME_CUR)
        {
            ps->volfade.startvol = vs->volume;
            ps->volfade.differential = (ps->volfade.desttime-ps->volfade.starttime) / (ps->volfade.startvol-ps->volfade.destvol);
        }

        if (past > (ps->volfade.desttime-ps->volfade.starttime)*64) // past desttime, so end fade!
            Voiceset_SetVolume(vs, ps->volfade.destvol);
        else
        {
            newvol = past / ps->volfade.differential;
            Voiceset_SetVolume(vs, ps->volfade.startvol - newvol);
        }
    }

    ps_unforbid();

    return 1250000l / (ps->state.bpm * 5l);
}

// MPLAYER Cleaners ---  Used to reset the UNIMOD structure to an 'initial' state.
// Made QuickClean (below) for the Winamp postion lookup (module seeking) stuffs.

// =====================================================================================
    void Player_WipeVoices(MPLAYER *ps)
// =====================================================================================
{
    uint t;

    for (t=0; t<ps->vs->voices; t++)
    {
        Voice_Stop(ps->vs,t);
        ps->state.voice[t].shared.i = NULL;
        ps->state.voice[t].shared.s = NULL;
    }

    for (t=0; t<ps->mf->numchn; t++)
    {
        ps->state.control[t].shared.i = NULL;
        ps->state.control[t].shared.s = NULL;
    }
}


// =====================================================================================
    void Player_Cleaner(MPLAYER *ps)
// =====================================================================================
{
    uint          t;
    
    if (ps)
    {
        const UNIMOD *mf  = ps->mf;

        ps->ended         = 0;

        ps->state.sngpos  = ps->start_pos;
        ps->state.prev_sngpos = ps->mf->numpos;
        ps->state.sngspd  = mf->initspeed;
        ps->state.volume  = mf->initvolume;

        ps->state.vbtick  = ps->state.sngspd;
        ps->state.patdly  = 0;
        ps->state.patdly2 = 0;
        ps->state.bpm     = mf->inittempo;

        ps->state.patpos  = 0;
        ps->state.posjmp  = 2;        // <- make sure the player fetches the first note
        ps->state.patbrk  = 0;

        ps->state.curtime = 0;
        ps->state.patloop = 0;
        ps->state.strip_timeout = ps->mf->strip_threshold;

        ps->state.looping = ps->loopcount + 1;

        if (ps->state.panning)
            memcpy(ps->state.panning, mf->panning, sizeof(ps->state.panning));
        if (ps->state.chanvol)
            memcpy(ps->state.chanvol, mf->chanvol, sizeof(ps->state.chanvol));

        MP_WipePosPlayed(ps);

        for(t=0; t<mf->numchn; t++)
        {
            ps->state.control[t].shared.chanvol = ps->state.chanvol[t];
            ps->state.control[t].shared.panning = ps->state.panning[t];

//TEST            ps->state.control[t].shared.cutoff    = 128;
//TEST            ps->state.control[t].shared.resonance = 0;
            ps->state.control[t].rep_sngpos       = -1;   // default loop to current pattern unless otherwise set.

            memset(ps->state.control[t].memory, 0, mf->memsize * sizeof(MP_EFFMEM));
            // set memslots defaults
            if (mf->numdefs)
            {
                uint num = mf->numdefs;
                UE_EFFECT *eff = mf->memdefs;

                while (num--)
                {
                    ps->state.control[t].memory[eff->memslot].effect = eff->effect;
                    eff++;
                }
            }

            ps->state.control[t].muted = mf->muted[t];
        }
    }
}


// =====================================================================================
    static void player_shutdown(MPLAYER *ps)
// =====================================================================================
// Called by the mmalloc code whenever the player allochandle has been shut down.
{
    Voiceset_Free(ps->vs);
}


// =====================================================================================
    MPLAYER *Player_Create(const UNIMOD *mf, uint flags)
// =====================================================================================
// creates an mplayer structure from the given unimod song and flags.
// Note that this does not initialize the player struct completely - the ps->state.voice element
// is not allocated until someone does that manually (or use Player_InitSong)
{
    MPLAYER *ps;
    uint     i;

    assert(mf != NULL);

    _MM_ALLOCATE(ps, MPLAYER, mf->allochandle);
    _mmalloc_setshutdown(ps->allochandle, player_shutdown, ps);

    // set module and flags
    ps->mf    = mf;
    ps->flags = mf->flags | flags;

    // Allocate memory used by the player ---
    if ((ps->state.control=(MP_CONTROL*)MikMod_calloc(ps->allochandle, mf->numchn, sizeof(MP_CONTROL))) == NULL)
        return NULL;

    // Allocate pos-played array if timeseek enabled or looping turned off...
    if (ps->flags&PF_TIMESEEK || !(ps->flags&PF_LOOP))
    {
        if ((ps->state.pos_played=(ULONG**)MikMod_calloc(ps->allochandle, mf->numpos,sizeof(ULONG *))) == NULL)
            return NULL;

        for (i=0; i<mf->numpos; i++)
            if ((ps->state.pos_played[i]=(ULONG*)MikMod_calloc(ps->allochandle, (mf->pattrows[mf->positions[i]]/32)+1, sizeof(ULONG))) == NULL)
                return NULL;
    }

    // Allocate the per-channel effects memory space.
    for (i=0; i<mf->numchn; i++)
    {
        if ((ps->state.control[i].memory=(MP_EFFMEM*)MikMod_calloc(ps->allochandle, mf->memsize, sizeof(MP_EFFMEM))) == NULL)
            return NULL;
    }

//CUT    if(mf->numins)
//CUT        ps->useins = (BOOL *)MikMod_calloc(ps->allochandle, mf->numins, sizeof(BOOL));

//CUT    if(mf->numsmp)
//CUT        ps->usesmp = (BOOL *)MikMod_calloc(ps->allochandle, mf->numsmp, sizeof(BOOL));

    ps->volume_table = mf->volume_table;

    ps->mutex = ps_forbid_init();

    return ps;
}

// =====================================================================================
    MPLAYER *Player_Dup(MPLAYER *ps)
// =====================================================================================
// Duplicates specified player. Duplicates the original state,
// not the current one!
{
    MPLAYER *res = Player_Create(ps->mf, ps->flags);

    if (res)
    {
        if (ps->state.pos_played != NULL)
            Player_SetLoopStatus(res, ps->flags&PF_LOOP, ps->loopcount);
        Player_SetStartPosition(res, ps->start_pos);

        res->songlen = ps->songlen;
        //todo more stuff
    }

    return res;
}

// =====================================================================================
    MPLAYER *Player_InitSong(const UNIMOD *mf, MD_VOICESET *owner, uint flags, uint maxvoices)
// =====================================================================================
// Handy dandy high level prep-for-playing function.  This takes a song (UNIMOD), player
// flag options, and a maxvoice value and creates an MPLAYER struct and a MD_VOICESET
// struct, all ready to go.
// Notes:
//   maxchan   - specifies the maximum number of voices the player is allowed to
//               allocate into the voiceset it creates.
{
    MPLAYER      *ps;
    MD_VOICESET  *vs;

    if(!mf) return NULL;
    
    ps = Player_Create(mf, flags);

    vs = Voiceset_Create(mf->md, owner, 0, 0);
    Player_Register(vs, ps, maxvoices);

    return ps;
}

// =====================================================================================
    BOOL Player_SetLoopStatus(MPLAYER *ps, BOOL loop, int loopcount)
// =====================================================================================
// Turns on Mikmod's smart looping system, and forces the song to loop the sepcific
// number of times.  Note that the behavior of the loopcount varies dependingon if PF_LOOP
// is set or not: when set, all songs will loop x times.  If unset, only songs with have
// specific looping information will loop.
{
    
    if (ps)
    {
        const UNIMOD  *mf = ps->mf;
    
        if (mf)
        {
            if (ps->state.pos_played == NULL)
            {
                uint   i;

                if ((ps->state.pos_played=(ULONG **)MikMod_calloc(ps->allochandle, mf->numpos,sizeof(ULONG *))) == NULL)
                    return FALSE;

                for (i=0; i<mf->numpos; i++)
                    if((ps->state.pos_played[i]=(ULONG *)MikMod_calloc(ps->allochandle, (mf->pattrows[mf->positions[i]]/32)+1,sizeof(ULONG))) == NULL)
                        return FALSE;
            }
        }

        ps->loopcount     = loopcount;
        ps->state.looping = loopcount + 1;
        if (loop)
            ps->flags |= PF_LOOP;
        else ps->flags &= ~PF_LOOP;
    }
    return TRUE;
}

// =====================================================================================
    void Player_SetStartPosition(MPLAYER *ps, int pos)
// =====================================================================================
{
    ps->start_pos    = pos;
    ps->state.sngpos = pos;
}

// =====================================================================================
    void Player_Register(MD_VOICESET *vs, MPLAYER *ps, uint maxchan)
// =====================================================================================
// Registers a song into the given VoiceSet.  This will override any other song
// or player that may already be registered to use the specified voiceset.
{
    const UNIMOD       *mf = ps->mf;

    // set up the module channels and jazz...
    // a) determine the actual number of channels needed for this song:

    if (!(mf->flags&UF_NNA) && mf->numchn<maxchan)
        maxchan = mf->numchn;
    else if (mf->numvoices && mf->numvoices<maxchan)
        maxchan = mf->numvoices;

    if (mf->flags&UF_NNA || maxchan<mf->numchn)
        ps->flags |= PF_NNA;

    Voiceset_SetNumVoices(vs, maxchan);
    Voiceset_SetPlayer(vs, Player_HandleTick, ps);

    ps->vs = vs;

    // Only allocate voices if we have voices to allocate
    // (fix for possible nosound driver)

    if ((ps->numvoices=vs->voices) &&
        (ps->state.voice=(MP_VOICE*)MikMod_calloc(ps->allochandle, ps->numvoices, sizeof(MP_VOICE))) == NULL)
        return;
    
    Player_Cleaner(ps);
}


// =====================================================================================
    void Player_Free(MPLAYER *ps)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid_deinit();
    ps->mutex = NULL;

    _mmalloc_close(ps->allochandle);
}


// =====================================================================================
    void Player_SetVolume(MPLAYER *ps, int volume)
// =====================================================================================
// This just passes the volume command onto the player's allocated voiceset.  Also,
// this will stop any active QuickFade or VolumeFade (but not a VolumeFadeEx).
{
    if(!ps) return;

    Voiceset_SetVolume(ps->vs, volume);
    ps->fadespeed = 0;
}


// =====================================================================================
    int Player_GetVolume(MPLAYER *ps)
// =====================================================================================
// Pass the voiceset's current volume back to the caller...
{
    if(!ps) return 0;

    return(Voiceset_GetVolume(ps->vs));
}


// =====================================================================================
    void Player_QuickFade(MPLAYER *ps, int start, int dest, int speed)
// =====================================================================================
// This is a quick and dirty player fading feature, intended for use in video games.  
// This fade is *not* suitable for module players since it does not have a wide range of
// fade speeds, and it is timed to the BPM of a song (end-user players would want to be 
// time-based).
{
    if(!ps) return;

    ps_forbid();

    Voiceset_SetVolume(ps->vs, start);
    ps->fadedest  = dest;
    ps->fadespeed = speed;

    ps_unforbid();
}


// =====================================================================================
    void Player_VolumeFadeEx(MPLAYER *ps, int startvol, int destvol, long starttime, int whence, long duration)
// =====================================================================================
// The big and bad advanced volume fader for Mikmod.  It fades evenly, reguardless of
// the BPM rate of the song, plus, you can set a start time!  The start time uses the
// same system as the ANSI C FILE thingie: MP_SEEK_SET, MP_SEEK_CUR, MP_SEEK_END.
{
    if(!ps) return;

    ps_forbid();

    ps->volfade.startvol     = startvol;
    ps->volfade.destvol      = destvol;

    ps->volfade.differential = duration / (startvol - destvol);

    switch(whence)
    {
        case MP_SEEK_SET:
            ps->volfade.starttime  = starttime*64;
        break;

        case MP_SEEK_CUR:
            ps->volfade.starttime  = ps->state.curtime + (starttime*64);
        break;

        case MP_SEEK_END:
            ps->volfade.starttime  = (ps->songlen - starttime)*64;
        break;
    }

    ps->volfade.desttime = ps->volfade.starttime + duration*64;

    ps_unforbid();
}


// =====================================================================================
    void Player_Start(MPLAYER *ps)
// =====================================================================================
// Releases the no-update lock on the player so that it continues to update.
{
    if(!ps) return;
    
    // no orders, no playing!
    if (!ps->mf->numpos)
    {
        ps->state.sngpos = 0;
        return;
    }

    ps_forbid();

    Voiceset_EnableOutput(ps->vs);
    Voiceset_PlayStart(ps->vs);

    ps_unforbid();
}


// =====================================================================================
    void Player_Stop(MPLAYER *ps)
// =====================================================================================
// Stops the player and reinitializes all player information.
{
    if(!ps) return;

    ps_forbid();

    Voiceset_PlayStop(ps->vs);
    Voiceset_Reset(ps->vs);                                 // wipes all voices, and resets player timing

    // Clean up the entire player set, effectively restarting the module.
    Player_Cleaner(ps);

    ps_unforbid();
}

// =====================================================================================
    void Player_Restart(MPLAYER *ps, BOOL loop)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid();

    Voiceset_PlayStop(ps->vs);
    Voiceset_Reset(ps->vs);                                 // wipes all voices, and resets player timing

    // restart player
    // if loop is TRUE restarts from the current position
    // (as was stopped by the looping system). if loop is
    // FALSE restarts from the very beginning.

    if (loop)
    {
        ps->ended         = 0;

        ps->state.vbtick  = ps->state.sngspd;
        ps->state.posjmp  = 2;
        ps->state.curtime = Player_GetPosTime(ps);

        ps->state.strip_timeout = ps->mf->strip_threshold;
        ps->state.looping = ps->loopcount + 1;

        MP_WipePosPlayed(ps);
    }
    else Player_Cleaner(ps);

    Voiceset_PlayStart(ps->vs);

    ps_unforbid();
}

// =====================================================================================
    BOOL Player_Active(MPLAYER *ps)
// =====================================================================================
{
    if(!ps) return 0;
    return ((!ps->ended && (ps->vs->flags & MDVS_PLAYER)) ? TRUE : FALSE);
}


// =====================================================================================
    void Player_Pause(MPLAYER *ps, BOOL nosilence)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid();

    // if silence is set, freeze the voiceset until the user reactivates
    // the player.  Note, a frozen voiceset does no mixing and no updates
    // of the voices within, so it picks up on the very sample it left off.

    Voiceset_PlayStop(ps->vs);
    if(!nosilence) Voiceset_DisableOutput(ps->vs);

    ps_unforbid();
}


// =====================================================================================
    void Player_TogglePause(MPLAYER *ps, BOOL nosilence)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid();

    if(ps->vs->flags & MDVS_PLAYER)
    {   Voiceset_PlayStop(ps->vs);
        if(!nosilence) Voiceset_DisableOutput(ps->vs);
    } else
    {   // Always re-enable, because if things weren't disabled, this
        // won't hurt, and if they were - it helps :)
        Voiceset_EnableOutput(ps->vs);
        Voiceset_PlayStart(ps->vs);
    }

    ps_unforbid();
}


// =====================================================================================
    BOOL Player_Paused(MPLAYER *ps)
// =====================================================================================
{
    // if the voiceset is active then we aren't paused.
    return (ps->vs->flags & MDVS_PLAYER) ? 0 : 1;
}

// Old School nextpos/prevpos functions.
// ========================================================
// These functions are considered relatively out-of-date (or even useless)
// except for a few oldschool situations.  They cut sound to all channels on
// pattern skip so to avoid hanging notes on NNAs.
//
// Generally, you will either want to use player_setposition to jump to a par-
// ticular point of a song, or use player_setpostime to jump to a set timeframe.

void Player_NextPosition(MPLAYER *ps)
{
    if(!ps) return;

    ps_forbid();
    ps->state.posjmp = 3;
    ps->state.patbrk = 0;
    ps->state.vbtick = ps->state.sngspd;

    Player_WipeVoices(ps);
    ps_unforbid();
}


// =====================================================================================
    void Player_PrevPosition(MPLAYER *ps)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid();
    ps->state.posjmp = 1;
    ps->state.patbrk = 0;
    ps->state.vbtick = ps->state.sngspd;

    Player_WipeVoices(ps);

    ps_unforbid();
}


// =====================================================================================
    void Player_SetPosition(MPLAYER *ps, uint pos, BOOL wipe)
// =====================================================================================
{
    if(!ps) return;

    ps_forbid();

    Player_WipeVoices(ps);

    if (pos >= ps->mf->numpos)
        pos = ps->mf->numpos;
    ps->state.posjmp = 2;
    ps->state.patbrk = 0;
    ps->state.sngpos = pos;
    ps->state.vbtick = ps->state.sngspd;

    if (wipe)
        MP_WipePosPlayed(ps);

    ps_unforbid();
}    

// =====================================================================================
    void __cdecl Player_Unmute(MPLAYER *ps, uint arg1, ...)
// =====================================================================================
{
    va_list ap;
    uint   arg2, arg3, t;
    const UNIMOD  *mf = ps->mf;

    va_start(ap, arg1);

    if(mf)
    {   switch(arg1)
        {   case MUTE_INCLUSIVE:
               if(((!(arg2 = va_arg(ap,SLONG))) && (!(arg3 = va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
               {   va_end(ap);
                   return;
               }
               for(; (arg2 < mf->numchn) && (arg2 <= arg3); arg2++)
                   ps->state.control[arg2].muted = 0;
            break;
    
            case MUTE_EXCLUSIVE:
               if(((!(arg2=va_arg(ap,SLONG))) && (!(arg3=va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
               {   va_end(ap);
                   return;
               }
               for(t=0;t<mf->numchn;t++)
               {   if ((t>=arg2) && (t<=arg3)) continue;
                   ps->state.control[t].muted = 0;
               }
            break;
    
            default:
               if(arg1 < mf->numchn) ps->state.control[arg1].muted = 0;
            break;
        }
    }
    va_end(ap);

    return;
}


// =====================================================================================
    void __cdecl Player_Mute(MPLAYER *ps, uint arg1, ...)
// =====================================================================================
{
    va_list ap;
    uint    arg2, arg3, t;
    const UNIMOD  *mf = ps->mf;

    va_start(ap,arg1);

    if(mf)
    {   switch (arg1)
        {   case MUTE_INCLUSIVE:
               if (((!(arg2=va_arg(ap,SLONG))) && (!(arg3=va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
               {   va_end(ap);
                   return;
               }
               for(; (arg2 < mf->numchn) && (arg2 <= arg3); arg2++)
                   ps->state.control[arg2].muted = 1;
            break;
    
            case MUTE_EXCLUSIVE:
               if (((!(arg2=va_arg(ap,SLONG))) && (!(arg3=va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
               {   va_end(ap);
                   return;
               }
               for (t=0; t<mf->numchn; t++)
               {   if ((t >= arg2) && (t <= arg3)) continue;
                   ps->state.control[t].muted = 1;
               }
            break;
    
            default:
               if(arg1 < mf->numchn)
                   ps->state.control[arg1].muted = 1;
            break;
        }
    }
    va_end(ap);

    return;
}


// =====================================================================================
    void __cdecl Player_ToggleMute(MPLAYER *ps, uint arg1, ...)
// =====================================================================================
{
    va_list ap;
    uint    arg2, arg3, t;
    const UNIMOD  *mf = ps->mf;
       
    va_start(ap,arg1);

    if(mf)
    {   switch (arg1)
        {   case MUTE_INCLUSIVE:
              if (((!(arg2=va_arg(ap,SLONG))) && (!(arg3=va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
              {   va_end(ap);
                  return;
              }
              for(; (arg2 < mf->numchn) && (arg2 <= arg3); arg2++)
                  ps->state.control[arg2].muted = (ps->state.control[arg2].muted) ? 0 : 1;
         
            break;
    
            case MUTE_EXCLUSIVE:
               if (((!(arg2 = va_arg(ap,SLONG))) && (!(arg3 = va_arg(ap,SLONG)))) || (arg2 > arg3) || (arg3 >= mf->numchn))
               {   va_end(ap);
                   return;
               }    
               for (t=0;t<mf->numchn;t++)
               {   if((t >= arg2) && (t <= arg3)) continue;
                   ps->state.control[t].muted = (ps->state.control[t].muted) ? 0 : 1;
               }
            break;
    
            default:
               if(arg1 < mf->numchn)
                   ps->state.control[arg1].muted = (ps->state.control[arg1].muted) ? 0 : 1;
            break;
        }
    }
    va_end(ap);

    return;
}


// =====================================================================================
    BOOL Player_Muted(MPLAYER *ps, uint chan)
// =====================================================================================
{
    return (chan < ps->mf->numchn) ? ps->state.control[chan].muted : 1;
}


// =====================================================================================
    int Player_GetChanVoice(MPLAYER *ps, uint chan)
// =====================================================================================
{
    return ps->state.control[chan].slavechn;
}

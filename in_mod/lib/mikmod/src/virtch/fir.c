#include "mikmod.h"
#include "wrap16.h"
#include "wrap8.h"

#include "fir_table.h"

VOLINFO v16, v8;

#define SCAST SWORD
#define _SCAST word
#define MAKENAME(X) Mix16##X
#define vxx v16

#include "fir_inc2.h"

#undef SCAST
#undef _SCAST
#undef MAKENAME
#undef vxx


#define SCAST SBYTE
#define _SCAST byte
#define MAKENAME(X) Mix8##X
#define vxx v8
#define FIR_8BIT

#include "fir_inc2.h"

#undef FIR_8BIT
#undef SCAST
#undef _SCAST
#undef MAKENAME
#undef vxx

#pragma warning(disable: 4113)

VMIXER M16_MONO_FIR =
{   NULL,

    BLAH("FIR mixer mono16"),

    nc16_Check_Mono,

    NULL,
    NULL,

    VC_Volcalc16_Mono,
    VC_Volramp16_Mono,
    Mix16MonoFIR_NoClick,
    Mix16MonoFIR_NoClick,
    Mix16MonoFIR,
    Mix16MonoFIR,
};

VMIXER M16_STEREO_FIR =
{   NULL,

    BLAH("FIR mixer stereo16"),

    nc16_Check_Stereo,

    NULL,
    NULL,

    VC_Volcalc16_Stereo,
    VC_Volramp16_Stereo,
    Mix16StereoFIR_NoClick,
    Mix16SurroundFIR_NoClick,
    Mix16StereoFIR,
    Mix16SurroundFIR,
};

VMIXER M8_MONO_FIR =
{   NULL,

    BLAH("FIR mixer mono8"),

    nc8_Check_Mono,

    NULL,
    NULL,

    VC_Volcalc8_Mono,
    VC_Volramp8_Mono,
    Mix8MonoFIR_NoClick,
    Mix8MonoFIR_NoClick,
    Mix8MonoFIR,
    Mix8MonoFIR,
};

VMIXER M8_STEREO_FIR =
{   NULL,

    BLAH("FIR mixer stereo8"),

    nc8_Check_Stereo,

    NULL,
    NULL,

    VC_Volcalc8_Stereo,
    VC_Volramp8_Stereo,
    Mix8StereoFIR_NoClick,
    Mix8SurroundFIR_NoClick,
    Mix8StereoFIR,
    Mix8SurroundFIR,
};

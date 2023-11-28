/*

 Name:  MLREG.C

 Description:
 A single routine for registering all loaders in MikMod for the current
 platform.

 Portability:
 All systems - all compilers

*/

#include "mikmod.h"
#include "uniform.h"


void Mikmod_RegisterAllLoaders(void)
{
    Mikmod_RegisterLoader(load_it);
    Mikmod_RegisterLoader(load_xm);
    Mikmod_RegisterLoader(load_s3m);
    Mikmod_RegisterLoader(load_mod);
    Mikmod_RegisterLoader(load_mtm);
    Mikmod_RegisterLoader(load_stm);
    //Mikmod_RegisterLoader(load_dsm);
    //Mikmod_RegisterLoader(load_med);
    Mikmod_RegisterLoader(load_far);   
    Mikmod_RegisterLoader(load_ult);
    Mikmod_RegisterLoader(load_669);
    Mikmod_RegisterLoader(load_amf);
    Mikmod_RegisterLoader(load_okt);
    Mikmod_RegisterLoader(load_ptm);
    Mikmod_RegisterLoader(load_m15);
}

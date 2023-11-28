 
#ifndef _WRAP16_H_
#define _WRAP16_H_

#include "vchcrap.h"
 

// TODO: benski> move these three globals into VIRTCH or VINFO (not sure which one)
//extern int           lvolsel, rvolsel;

extern void   VC_Volcalc16_Mono(VIRTCH *vc, VINFO *vnf);
extern void   VC_Volramp16_Mono(VINFO *vnf, int done);

extern void   VC_Volcalc16_Stereo(VIRTCH *vc, VINFO *vnf);
extern void   VC_Volramp16_Stereo(VINFO *vnf, int done);


extern BOOL   nc16ss_Check_Stereo(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16ss_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16ss_Check_Mono(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16ss_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags);

extern BOOL   nc16_Check_Stereo(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16_Check_Mono(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc16_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags);

// The Declicker mixers
// --------------------
// These have been separated for the purpose of allowing the assembly mixers to share in
// the fun without the compiler linking in the rest of the unwanted C mixers along with them.

void __cdecl Mix16MonoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16StereoNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16SurroundNormal_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix16MonoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16SurroundInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16StereoInterp_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix16StereoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16MonoSSI_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix16StereoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix16MonoSS_NoClick(SWORD *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

#endif

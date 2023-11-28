
#ifndef _WRAP8_H_
#define _WRAP8_H_

#include "vchcrap.h"
 
// TODO: benski> move these three globals into VIRTCH or VINFO (not sure which one)
//extern int           lvolsel, rvolsel;

//extern BOOL   VC_Lookup8_Init(VMIXER *mixer);
//extern void   VC_Lookup8_Exit(VMIXER *mixer);

extern void   VC_Volcalc8_Mono(VIRTCH *vc, VINFO *vnf);
extern void   VC_Volramp8_Mono(VINFO *vnf, int done);

extern void   VC_Volcalc8_Stereo(VIRTCH *vc, VINFO *vnf);
extern void   VC_Volramp8_Stereo(VINFO *vnf, int done);

extern BOOL   nc8ss_Check_Stereo(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8ss_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8ss_Check_Mono(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8ss_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags);

extern BOOL   nc8_Check_Stereo(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8_Check_StereoInterp(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8_Check_Mono(uint channels, uint mixmode, uint format, uint flags);
extern BOOL   nc8_Check_MonoInterp(uint channels, uint mixmode, uint format, uint flags);

void __cdecl Mix8MonoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8StereoNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8SurroundNormal_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix8MonoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8SurroundInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8StereoInterp_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix8StereoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8MonoSSI_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

void __cdecl Mix8StereoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);
void __cdecl Mix8MonoSS_NoClick(SBYTE *srce, SLONG *dest, INT64S index, INT64S increment, SLONG todo, VC_RESFILTER *resfilter, VOLINFO *volinfo);

#endif

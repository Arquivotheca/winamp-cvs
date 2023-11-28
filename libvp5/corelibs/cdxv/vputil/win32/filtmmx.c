/****************************************************************************
 *
 *   Module Title :     newLoopTest_asm.c 
 *
 *   Description  :     Codec specific functions
 *
 *   AUTHOR       :     Yaowu Xu
 *
 *****************************************************************************
 *   Revision History
 *
 *   1.02 YWX 03-Nov-00 Changed confusing variable name
 *   1.01 YWX 02-Nov-00 Added the set of functions
 *   1.00 YWX 19-Oct-00 configuration baseline
 *****************************************************************************
 */ 

#pragma warning( disable : 4799 )	// No EMMS at end of function

/****************************************************************************
 *  Header Frames
 *****************************************************************************
 */


#define STRICT              /* Strict type checking. */
#include "codec_common.h"
#include <math.h>

 /****************************************************************************
 *  Module constants.
 *****************************************************************************
 */        

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define FILTER_WEIGHT 128
#define FILTER_SHIFT  7

extern void UnpackBlock_MMX( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);

static __declspec(align(16)) short rd[]={64,64,64,64,64,64,64,64};


__declspec(align(16)) INT16  BilinearFilters_mmx[8][16] = 
{
{ 128,128,128,128,128,128,128,128,    0,  0, 0,   0,  0,  0,  0,  0 },
{ 112,112,112,112,112,112,112,112,   16, 16, 16, 16, 16, 16, 16, 16 },
{  96, 96, 96, 96, 96, 96, 96, 96,   32, 32, 32, 32, 32, 32, 32, 32 },
{  80, 80, 80, 80, 80, 80, 80, 80,   48, 48, 48, 48, 48, 48, 48, 48 },
{  64, 64, 64, 64, 64, 64, 64, 64,   64, 64, 64, 64, 64, 64, 64, 64 },
{  48, 48, 48, 48, 48, 48, 48, 48,   80, 80, 80, 80, 80, 80, 80, 80 },
{  32, 32, 32, 32, 32, 32, 32, 32,   96, 96, 96, 96, 96, 96, 96, 96 },
{  16, 16, 16, 16, 16, 16, 16, 16,  112,112,112,112,112,112,112,112 }
};

__declspec(align(16)) INT16  BicubicFilters_mmx[8][32] = 
{
{   0,  0,  0,  0,  0,  0,  0,  0,  128,128,128,128,128,128,128,128,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 },
{  -4, -4, -4, -4, -4, -4, -4, -4,  118,118,118,118,118,118,118,118,   16, 16, 16, 16, 16, 16, 16, 16,   -2, -2, -2, -2, -2, -2, -2, -2 },
{  -7, -7, -7, -7, -7, -7, -7, -7,  106,106,106,106,106,106,106,106,   34, 34, 34, 34, 34, 34, 34, 34,   -5, -5, -5, -5, -5, -5, -5, -5 },
{  -8, -8, -8, -8, -8, -8, -8, -8,   90, 90, 90, 90, 90, 90, 90, 90,   53, 53, 53, 53, 53, 53, 53, 53,   -7, -7, -7, -7, -7, -7, -7, -7 },
{  -8, -8, -8, -8, -8, -8, -8, -8,   72, 72, 72, 72, 72, 72, 72, 72,   72, 72, 72, 72, 72, 72, 72, 72,   -8, -8, -8, -8, -8, -8, -8, -8 },
{  -7, -7, -7, -7, -7, -7, -7, -7,   53, 53, 53, 53, 53, 53, 53, 53,   90, 90, 90, 90, 90, 90, 90, 90,   -8, -8, -8, -8, -8, -8, -8, -8 },
{  -5, -5, -5, -5, -5, -5, -5, -5,   34, 34, 34, 34, 34, 34, 34, 34,  106,106,106,106,106,106,106,106,   -7, -7, -7, -7, -7, -7, -7, -7 },
{  -2, -2, -2, -2, -2, -2, -2, -2,   16, 16, 16, 16, 16, 16, 16, 16,  118,118,118,118,118,118,118,118,   -4, -4, -4, -4, -4, -4, -4, -4 }
};
void FilterBlock1d_h_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq      mm1, [edi]             ; mm3 *= kernel 0 modifiers.
        movq      mm2, [edi+ 16]         ; mm3 *= kernel 0 modifiers.
        movq      mm6, [edi + 32]        ; mm3 *= kernel 0 modifiers.
        movq      mm7, [edi + 48]        ; mm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        dec         esi
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth      ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000

nextrow:
        movq		mm3, [esi]            ; mm3 = p-1..p6    
        movq        mm4, mm3              ; mm4 = p-1..p6
        punpcklbw   mm3, mm0              ; mm3 = p-1..p2
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 24               ; mm4 = p2..p6
        movq        mm5, mm4              ; mm5 = p2..p6
        punpcklbw   mm5, mm0              ; mm5 = p2..p5
        pmullw      mm5, mm7              ; mm5 *= kernel 3 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5

        movq        mm4, [esi+1]          ; mm4 = p0..p6
        movq        mm5, mm4              ; mm5 = p0..p6
        punpcklbw   mm5, mm0              ; mm5 = p0..p3
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p1..p6
        movq        mm5, mm4              ; mm5 = p1..p6
        punpcklbw   mm5, mm0              ; mm5 = p1..p4
        pmullw      mm5, mm6              ; mm5 *= kernel 2 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5


        paddsw      mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi],mm3             ; store the results in the destination


        movq		mm3, [esi+4]           ; mm3 = p-1..p6    
        movq        mm4, mm3              ; mm4 = p-1..p6
        punpcklbw   mm3, mm0              ; mm3 = p-1..p2
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 24               ; mm4 = p2..p6
        movq        mm5, mm4              ; mm5 = p2..p6
        punpcklbw   mm5, mm0              ; mm5 = p2..p5
        pmullw      mm5, mm7              ; mm5 *= kernel 3 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5

        movq        mm4, [esi+5]          ; mm4 = p0..p6
        movq        mm5, mm4              ; mm5 = p0..p6
        punpcklbw   mm5, mm0              ; mm5 = p0..p3
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p1..p6
        movq        mm5, mm4              ; mm5 = p1..p6
        punpcklbw   mm5, mm0              ; mm5 = p1..p4
        pmullw      mm5, mm6              ; mm5 *= kernel 2 modifiers
        paddsw      mm3, mm5              ; mm3 += mm5


        paddsw      mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd       [edi+4],mm3               ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}


void FilterBlock1d_v_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq      mm1, [edi]          ; mm3 *= kernel 0 modifiers.
        movq      mm2, [edi + 16]     ; mm3 *= kernel 0 modifiers.
        movq      mm6, [edi + 32]     ; mm3 *= kernel 0 modifiers.
        movq      mm7, [edi + 48]     ; mm3 *= kernel 0 modifiers.

        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        sub         esi, PixelsPerLine
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth      ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000


nextrow:
        movq		mm3, [esi]            ; mm3 = p0..p8
        punpcklbw   mm3, mm0              ; mm3 = p0..p3
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        add         esi, edx              ; move source forward 1 line to avoid 3 * pitch

        movq		mm4, [esi+2*edx]      ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm7              ; mm4 *= kernel 3 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi ]           ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi +edx]       ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm6              ; mm4 *= kernel 2 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4


        paddsw      mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and saturate

        movd        [edi],mm3             ; store the results in the destination
        
        sub         esi, edx              ;  subtract edx to get back to -1 column

        movq		mm3, [esi+4]          ; mm3 = p4..p12
        punpcklbw   mm3, mm0              ; mm3 = p4..p7
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        add         esi, edx              ; move source forward 1 line to avoid 3 * pitch

        movq		mm4, [esi+2*edx+4]    ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm7              ; mm4 *= kernel 3 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi +4]         ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi +edx+4]     ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm6              ; mm4 *= kernel 2 modifiers.
        paddsw      mm3, mm4              ; mm3 += mm4


        paddsw      mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and saturate

        movd        [edi+4],mm3           ; store the results in the destination



        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         edi,eax; 

        dec         ecx                   ; decrement count
        jnz         nextrow               ; next row

    }
}


void FilterBlock1d_h_mmxa( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq      mm1, [edi]             ; mm3 *= kernel 0 modifiers.
        movq      mm2, [edi+ 16]         ; mm3 *= kernel 0 modifiers.
        movq      mm6, [edi + 32]        ; mm3 *= kernel 0 modifiers.
        movq      mm7, [edi + 48]        ; mm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        dec         esi
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth      ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000

nextrow:
        movq		mm3, [esi]            ; mm3 = p-1..p6    
        movq        mm4, mm3              ; mm4 = p-1..p6
        punpcklbw   mm3, mm0              ; mm3 = p-1..p2
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 8                ; mm4 = p0..p6
        movq        mm5, mm4              ; mm5 = p0..p6
        punpcklbw   mm5, mm0              ; mm5 = p0..p3
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p1..p6
        movq        mm5, mm4              ; mm5 = p1..p6
        punpcklbw   mm5, mm0              ; mm5 = p1..p4
        pmullw      mm5, mm6              ; mm5 *= kernel 2 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p2..p6
        movq        mm5, mm4              ; mm5 = p2..p6
        punpcklbw   mm5, mm0              ; mm5 = p2..p5
        pmullw      mm5, mm7              ; mm5 *= kernel 3 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi],mm3             ; store the results in the destination


        movq		mm3, [esi+4]           ; mm3 = p-1..p6    
        movq        mm4, mm3              ; mm4 = p-1..p6
        punpcklbw   mm3, mm0              ; mm3 = p-1..p2
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 8                ; mm4 = p0..p6
        movq        mm5, mm4              ; mm5 = p0..p6
        punpcklbw   mm5, mm0              ; mm5 = p0..p3
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p1..p6
        movq        mm5, mm4              ; mm5 = p1..p6
        punpcklbw   mm5, mm0              ; mm5 = p1..p4
        pmullw      mm5, mm6              ; mm5 *= kernel 2 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        psrlq       mm4, 8                ; mm4 = p2..p6
        movq        mm5, mm4              ; mm5 = p2..p6
        punpcklbw   mm5, mm0              ; mm5 = p2..p5
        pmullw      mm5, mm7              ; mm5 *= kernel 3 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd       [edi+4],mm3               ; store the results in the destination

        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}


void FilterBlock1d_v_mmxa( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq      mm1, [edi]          ; mm3 *= kernel 0 modifiers.
        movq      mm2, [edi + 16]     ; mm3 *= kernel 0 modifiers.
        movq      mm6, [edi + 32]     ; mm3 *= kernel 0 modifiers.
        movq      mm7, [edi + 48]     ; mm3 *= kernel 0 modifiers.

        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        sub         esi, PixelsPerLine
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth      ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000


nextrow:
        movq		mm3, [esi]            ; mm3 = p0..p8
        punpcklbw   mm3, mm0              ; mm3 = p0..p3
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        movq		mm4, [esi +edx ]      ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi +2*edx]     ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm6              ; mm4 *= kernel 2 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        add         esi, edx              ; move source forward 1 line to avoid 3 * pitch

        movq		mm4, [esi+2*edx]      ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm7              ; mm4 *= kernel 3 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and saturate

        movd        [edi],mm3             ; store the results in the destination
        
        sub         esi, edx              ;  subtract edx to get back to -1 column

        movq		mm3, [esi+4]          ; mm3 = p4..p12
        punpcklbw   mm3, mm0              ; mm3 = p4..p7
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        movq		mm4, [esi +edx +4]      ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        movq		mm4, [esi +2*edx+4]   ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm6              ; mm4 *= kernel 2 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        add         esi, edx              ; move source forward 1 line to avoid 3 * pitch

        movq		mm4, [esi+2*edx+4]    ; mm4 = p0..p8
        punpcklbw   mm4, mm0              ; mm4 = p0..p3
        pmullw      mm4, mm7              ; mm4 *= kernel 3 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and saturate

        movd        [edi+4],mm3           ; store the results in the destination



        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         edi,eax; 

        dec         ecx                   ; decrement count
        jnz         nextrow               ; next row

    }
}


void FilterBlock1d_hb8_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq        mm1, [edi]            ; mm3 *= kernel 0 modifiers.
        movq        mm2, [edi + 16]       ; mm3 *= kernel 0 modifiers.

        mov         edi,OutputPtr
		mov			esi,SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth      ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000

nextrow:
        movq		mm3, [esi]            ; mm3 = p-1..p14    
        movq        mm4, mm3                ; mm4 = p-1..p14
        punpcklbw   mm3, mm0              ; mm3 = p-1..p6
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 8                 ; mm4 = p0..p13
        movq        mm5, mm4              ; mm5 = p0..p13
        punpcklbw   mm5, mm0              ; mm5 = p0..p7
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        paddw       mm3, rd                ; mm3 += round value
        psraw       mm3, FILTER_SHIFT      ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi],mm3               ; store the results in the destination

        movq		mm3, [esi+4]            ; mm3 = p-1..p14    
        movq        mm4, mm3                ; mm4 = p-1..p14
        punpcklbw   mm3, mm0              ; mm3 = p-1..p6
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        psrlq       mm4, 8                 ; mm4 = p0..p13
        movq        mm5, mm4              ; mm5 = p0..p13
        punpcklbw   mm5, mm0              ; mm5 = p0..p7
        pmullw      mm5, mm2              ; mm5 *= kernel 1 modifiers
        paddw       mm3, mm5              ; mm3 += mm5

        paddw       mm3, rd                ; mm3 += round value
        psraw       mm3, FILTER_SHIFT      ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi+4],mm3               ; store the results in the destination


        add         esi,SrcPixelsPerLine    ; next line
        add         edi,eax; 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row
    }
}


void FilterBlock1d_vb8_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 PixelsPerLine, UINT32 PixelStep, UINT32 OutputHeight, UINT32 OutputWidth, INT16 * Filter )
{
	(void)PixelStep;  // Unreferenced

	__asm
    {

        mov         edi, Filter
        movq      mm1, [edi]          ; mm3 *= kernel 0 modifiers.
        movq      mm2, [edi + 16]     ; mm3 *= kernel 0 modifiers.
        mov         edx, PixelsPerLine
        mov         edi, OutputPtr
		mov			esi, SrcPtr
        mov         ecx, DWORD PTR OutputHeight
        mov         eax, OutputWidth        ; destination pitch?
		pxor		mm0, mm0              ; mm0 = 00000000


nextrow:
        movq		mm3, [esi]            ; mm3 = p0..p16
        punpcklbw   mm3, mm0              ; mm3 = p0..p8
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        movq		mm4, [esi +edx ]      ; mm4 = p0..p16
        punpcklbw   mm4, mm0              ; mm4 = p0..p8
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi],mm3             ; store the results in the destination

        movq		mm3, [esi+4]          ; mm3 = p0..p16
        punpcklbw   mm3, mm0              ; mm3 = p0..p8
        pmullw      mm3, mm1              ; mm3 *= kernel 0 modifiers.

        movq		mm4, [esi +edx +4]    ; mm4 = p0..p16
        punpcklbw   mm4, mm0              ; mm4 = p0..p8
        pmullw      mm4, mm2              ; mm4 *= kernel 1 modifiers.
        paddw       mm3, mm4              ; mm3 += mm4

        paddw       mm3, rd               ; mm3 += round value
        psraw       mm3, FILTER_SHIFT     ; mm3 /= 128
        packuswb    mm3, mm0              ; pack and unpack to saturate

        movd        [edi+4],mm3           ; store the results in the destination

        // the subsequent iterations repeat 3 out of 4 of these reads.  Since the 
        // recon block should be in cache this shouldn't cost much.  Its obviously 
        // avoidable!!!. 
        add         esi,edx
        add         edi,eax 

        dec         ecx                     ; decrement count
        jnz         nextrow                 ; next row

    }
}
 
/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock2dBil
 *  
 *  INPUTS        :     Pointer to source data
 *						
 *  OUTPUTS       :     Filtered data
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Applies a bilinear filter on the intput data to produce
 *						a predictor block (UINT16)
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
_inline
void FilterBlock2dBil_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, INT16 * HFilter, INT16 * VFilter )
{

    __asm
    {
        mov         eax,        HFilter             ; 
        mov         edi,        OutputPtr           ; 
        mov         esi,        SrcPtr              ;
        lea         ecx,        [edi+64]            ;
        mov         edx,        SrcPixelsPerLine    ;
               
        movq        mm1,        [eax]               ;
        movq        mm2,        [eax+16]            ;
        
        mov         eax,        VFilter             ;       
        pxor        mm0,        mm0                 ;

        // get the first horizontal line done       ;
        movq        mm3,        [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movq        mm4,        mm3                 ; make a copy of current line
        
        punpcklbw   mm3,        mm0                 ; xx 00 01 02 03 04 05 06
        punpckhbw   mm4,        mm0                 ;

        pmullw      mm3,        mm1                 ;
        pmullw      mm4,        mm1                 ;

        movq        mm5,        [esi+1]             ;
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0                 ;

        pmullw      mm5,        mm2                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
        
        movq        mm7,        mm3                 ;
        packuswb    mm7,        mm4                 ;

        add         esi,        edx                 ; next line
NextRow:
        movq        mm3,        [esi]               ; xx 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14
        movq        mm4,        mm3                 ; make a copy of current line
        
        punpcklbw   mm3,        mm0                 ; xx 00 01 02 03 04 05 06
        punpckhbw   mm4,        mm0                 ;

        pmullw      mm3,        mm1                 ;
        pmullw      mm4,        mm1                 ;

        movq        mm5,        [esi+1]             ;
        movq        mm6,        mm5                 ;

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0                 ;

        pmullw      mm5,        mm2                 ;
        pmullw      mm6,        mm2                 ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        movq        mm5,        mm7                 ;
        movq        mm6,        mm7                 ;                

        punpcklbw   mm5,        mm0                 ;
        punpckhbw   mm6,        mm0

        pmullw      mm5,        [eax]               ;
        pmullw      mm6,        [eax]               ;
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
        
        movq        mm7,        mm3                 ;
        packuswb    mm7,        mm4                 ;    
        

        pmullw      mm3,        [eax+16]            ;
        pmullw      mm4,        [eax+16]            ;

        paddw       mm3,        mm5                 ;
        paddw       mm4,        mm6                 ;
        
        
        paddw       mm3,        rd                  ; xmm3 += round value
        psraw       mm3,        FILTER_SHIFT        ; xmm3 /= 128

        paddw       mm4,        rd                  ;
        psraw       mm4,        FILTER_SHIFT        ;
               
        packuswb    mm3,        mm4                                         

        movq        [edi],      mm3                 ; store the results in the destination

        add         esi,        edx                 ; next line
        add         edi,        8                   ; 

        cmp         edi,        ecx                 ;
        jne         NextRow                         

    }

    // First filter 1d Horizontal
	//FilterBlock1d_hb8_wmt(SrcPtr, Intermediate, SrcPixelsPerLine, 1, 9, 8, HFilter );
	// Now filter Verticaly
	//FilterBlock1d_vb8_wmt(Intermediate, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter);


}

#pragma warning( default : 4799 )	// No EMMS at end of function
 


/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlockBil_8
 *  
 *  INPUTS        :     ReconPtr1, ReconPtr12
 *							Two pointers into the block of data to be filtered
 *							These pointers bound the fractional pel position
 *						PixelsPerLine
 *							Pixels per line in the buffer pointed to by ReconPtr1 & ReconPtr12
 *						Modx, ModY
 *							The fractional pel bits used to select a filter.
 *
 *				
 *  OUTPUTS       :     ReconRefPtr
 *							A pointer to an 8x8 buffer into which UINT8 filtered data is written.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Produces a bilinear filtered fractional pel prediction block
 *						with UINT8 output
 *
 *  SPECIAL NOTES :      
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlockBil_8_mmx( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY )
{
	int diff;

	// swap pointers so ReconPtr1 smaller (above, left, above-right or above-left )
	diff=ReconPtr2-ReconPtr1;

	// The ModX and ModY arguments are the bottom three bits of the signed motion vector components (at 1/8th pel precision).
	// This works out to be what we want... despite the pointer swapping that goes on below.
	// For example... if the X component of the vector is a +ve ModX = X%8.
	//                if the X component of the vector is a -ve ModX = 8+(X%8) where X%8 is in the range -7 to -1.

	if(diff<0) 
	{											// swap pointers so ReconPtr1 smaller
		UINT8 *temp=ReconPtr1;
		ReconPtr1=ReconPtr2;
		ReconPtr2=temp;
		diff= (int)(ReconPtr2-ReconPtr1);
	}

	if( diff==1 )
	{			
		FilterBlock1d_hb8_mmx(ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 8, BilinearFilters_mmx[ModX] );
	}
	else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
	{
		FilterBlock1d_vb8_mmx(ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters_mmx[ModY]);
	}
	else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
	{										
		FilterBlock2dBil_mmx( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );
	}
	else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
	{	
		FilterBlock2dBil_mmx( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock2d
 *  
 *  INPUTS        :     Pointer to source data
 *						
 *  OUTPUTS       :     Filtered data
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Applies a 2d 4 tap filter on the intput data to produce
 *						a predictor block (UINT16)
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlock2d_mmx( UINT8 *SrcPtr, UINT8 *OutputPtr, UINT32 SrcPixelsPerLine, INT16 * HFilter, INT16 * VFilter )
{

    UINT8 Intermediate[256];

	// First filter 1d Horizontal
	FilterBlock1d_h_mmx(SrcPtr-SrcPixelsPerLine, Intermediate, SrcPixelsPerLine, 1, 11, 8, HFilter );

	// Now filter Verticaly
	FilterBlock1d_v_mmx(Intermediate+BLOCK_HEIGHT_WIDTH, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter);


}
 

/****************************************************************************
 * 
 *  ROUTINE       :     FilterBlock
 *  
 *  INPUTS        :     ReconPtr1, ReconPtr12
 *							Two pointers into the block of data to be filtered
 *							These pointers bound the fractional pel position
 *						PixelsPerLine
 *							Pixels per line in the buffer pointed to by ReconPtr1 & ReconPtr12
 *						Modx, ModY
 *							The fractional pel bits used to select a filter.
 *						UseBicubic
 *							Whether to use the bicubuc filter set or the bilinear set
 *
 *				
 *  OUTPUTS       :     ReconRefPtr
 *							A pointer to an 8x8 buffer into which the filtered data is written.
 *
 *  RETURNS       :     None.
 *
 *  FUNCTION      :     Produces a filtered fractional pel prediction block
 *						using bilinear or bicubic filters
 *
 *  SPECIAL NOTES :     
 *
 *  ERRORS        :     None.
 *
 ****************************************************************************/
void FilterBlock_mmx( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic )
{
	int diff;
    UINT8 Intermediate[256];

	// swap pointers so ReconPtr1 smaller (above, left, above-right or above-left )
	diff=ReconPtr2-ReconPtr1;

	// The ModX and ModY arguments are the bottom three bits of the signed motion vector components (at 1/8th pel precision).
	// This works out to be what we want... despite the pointer swapping that goes on below.
	// For example... if the X component of the vector is a +ve ModX = X%8.
	//                if the X component of the vector is a -ve ModX = 8+(X%8) where X%8 is in the range -7 to -1.

	if(diff<0) 
	{											// swap pointers so ReconPtr1 smaller
		UINT8 *temp=ReconPtr1;
		ReconPtr1=ReconPtr2;
		ReconPtr2=temp;
		diff= (int)(ReconPtr2-ReconPtr1);
	}

    if(!diff)
    {
        return;
    }
	if( diff==1 )
	{											        // Fractional pixel in horizontal only
		if ( UseBicubic )
			FilterBlock1d_h_mmx(ReconPtr1, Intermediate, PixelsPerLine, 1, 8, 8, BicubicFilters_mmx[ModX] );
		else
			FilterBlock1d_hb8_mmx(ReconPtr1, Intermediate, PixelsPerLine, 1, 8, 8, BilinearFilters_mmx[ModX] );
	}
	else if (diff == (int)(PixelsPerLine) )				// Fractional pixel in vertical only
	{
		if ( UseBicubic )
			FilterBlock1d_v_mmx(ReconPtr1, Intermediate, PixelsPerLine, PixelsPerLine, 8, 8, BicubicFilters_mmx[ModY]);
		else
			FilterBlock1d_vb8_mmx(ReconPtr1, Intermediate, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters_mmx[ModY]);
	}
	else if(diff == (int)(PixelsPerLine - 1))			// ReconPtr1 is Top right
	{										
		if ( UseBicubic )
			FilterBlock2d_mmx( ReconPtr1-1, Intermediate, PixelsPerLine, BicubicFilters_mmx[ModX], BicubicFilters_mmx[ModY] );
		else
			FilterBlock2dBil_mmx( ReconPtr1-1, Intermediate, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );
	}
	else if(diff == (int)(PixelsPerLine + 1) )			// ReconPtr1 is Top left
	{	
		if ( UseBicubic )
			FilterBlock2d_mmx( ReconPtr1, Intermediate, PixelsPerLine, BicubicFilters_mmx[ModX], BicubicFilters_mmx[ModY] );
		else
			FilterBlock2dBil_mmx( ReconPtr1, Intermediate, PixelsPerLine, BilinearFilters_mmx[ModX], BilinearFilters_mmx[ModY] );
	}
    UnpackBlock_MMX( Intermediate, (INT16*)ReconRefPtr, 8 );
}



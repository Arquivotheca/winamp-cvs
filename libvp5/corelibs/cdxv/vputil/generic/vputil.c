/**************************************************************************** 
 *
 *   Module Title :     vputil.c 
 *
 *   Description  :     Codec utility functions.
 *
 ***************************************************************************/
#define STRICT              /* Strict type checking */

/****************************************************************************
 *  Header Files
 ***************************************************************************/ 
#include <math.h>
#include "codec_common.h"

/****************************************************************************
*  Macros
****************************************************************************/        
#define FILTER_WEIGHT 128
#define FILTER_SHIFT  7
#define MIN(a, b) ( ( a < b ) ? a : b )

/****************************************************************************
 *  Imports
 ***************************************************************************/ 
extern void UtilMachineSpecificConfig ( void );
extern void fillidctconstants ( void );

/****************************************************************************
 *  Module Statics
 ****************************************************************************/
static INT32 BicubicFilters[8][4] =
{
    {  0, 128,   0,  0 },
    { -4, 118,  16, -2 },
    { -7, 106,  34, -5 },
    { -8,  90,  53, -7 },
    { -8,  72,  72, -8 },
    { -7,  53,  90, -8 },
    { -5,  34, 106, -7 },
    { -2,  16, 118, -4 }
};

static INT32 BilinearFilters[8][2] =
{
    { 128,   0 },
    { 112,  16 },
    {  96,  32 },
    {  80,  48 },
    {  64,  64 },
    {  48,  80 },
    {  32,  96 },
    {  16, 112 }
};

static INT32 FData[BLOCK_HEIGHT_WIDTH*11];	// Temp data bufffer used in filtering

/****************************************************************************
 *  Exports
 ****************************************************************************/
// Function pointers to platform specif routines
void (*ReconIntra)( INT16 *tmpBuffer, UINT8 *ReconPtr, UINT16 *ChangePtr, UINT32 LineStep );
void (*ReconInter)( INT16 *tmpBuffer, UINT8 *ReconPtr, UINT8 *RefPtr, INT16 *ChangePtr, UINT32 LineStep );
void (*ReconInterHalfPixel2)(  INT16 * tmpBuffer,  UINT8  * ReconPtr, UINT8 *RefPtr1, UINT8 *RefPtr2, INT16 *ChangePtr, UINT32 LineStep );
void (*fdct_short)( INT16 *InputData,  INT16 *OutputData );
void (*idct[65])( INT16 *InputData, INT16 *QuantMatrix, INT16 *OutputData );
void (*ClearSysState)( void );
void (*ReconBlock)( INT16 *SrcBlock, INT16 *ReconRefPtr, UINT8 *DestBlock, UINT32 LineStep );
void (*SubtractBlock)( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep );
void (*UnpackBlock)( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine);
void (*AverageBlock)( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine );
void (*CopyBlock)( unsigned char *src,  unsigned char *dest, unsigned int srcstride );
void (*Copy12x12)( const unsigned char *src, unsigned char *dest, unsigned int srcstride, unsigned int deststride );
void (*idctc[65])( INT16 *InputData,  INT16 *QuantMatrix,  INT16 *OutputData );
void (*FilterBlockBil_8)( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT8 *ReconRefPtr, UINT32 ReconPixelsPerLine,  INT32 ModX,  INT32 ModY );
void (*FilterBlock)( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 PixelsPerLine, INT32 ModX, INT32 ModY, BOOL UseBicubic );

/****************************************************************************
 * 
 *  ROUTINE       : ClearSysState_C
 *
 *  INPUTS        : None.
 *
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Null placeholder function.
 *
 *  SPECIAL NOTES : Stub in the C-code for a function required when using
 *                  MMX, XMM, etc. to clear system state.
 *
 ****************************************************************************/
void ClearSysState_C ( void )
{
}

/****************************************************************************
 * 
 *  ROUTINE       : AverageBlock_C
 *  
 *  INPUTS        : UINT8 *ReconPtr1          : Pointer to first reference block.
 *                  UINT8 *ReconPtr2          : Pointer to second reference block.
 *                  UINT32 ReconPixelsPerLine : Stride of reference blocks.
 *					
 *  OUTPUTS       : UINT16 *ReconRefPtr       : Pointer to output block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Takes two input blocks and creates an output block
 *                  by pixel averaging.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void AverageBlock_C ( UINT8 *ReconPtr1, UINT8 *ReconPtr2, UINT16 *ReconRefPtr, UINT32 ReconPixelsPerLine )
{
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        ReconRefPtr[0] = (INT16)(((INT32)(ReconPtr1[0]) + ((INT32)ReconPtr2[0]))>>1);
        ReconRefPtr[1] = (INT16)(((INT32)(ReconPtr1[1]) + ((INT32)ReconPtr2[1]))>>1);
        ReconRefPtr[2] = (INT16)(((INT32)(ReconPtr1[2]) + ((INT32)ReconPtr2[2]))>>1);
        ReconRefPtr[3] = (INT16)(((INT32)(ReconPtr1[3]) + ((INT32)ReconPtr2[3]))>>1);
        ReconRefPtr[4] = (INT16)(((INT32)(ReconPtr1[4]) + ((INT32)ReconPtr2[4]))>>1);
        ReconRefPtr[5] = (INT16)(((INT32)(ReconPtr1[5]) + ((INT32)ReconPtr2[5]))>>1);
        ReconRefPtr[6] = (INT16)(((INT32)(ReconPtr1[6]) + ((INT32)ReconPtr2[6]))>>1);
        ReconRefPtr[7] = (INT16)(((INT32)(ReconPtr1[7]) + ((INT32)ReconPtr2[7]))>>1);
        
        // Start next row
        ReconPtr1 += ReconPixelsPerLine;
        ReconPtr2 += ReconPixelsPerLine;

        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : UnpackBlock_C
 *  
 *  INPUTS        : UINT8 *ReconPtr           : Pointer to reference block.
 *                  UINT32 ReconPixelsPerLine : Stride of reference block.
 *					
 *  OUTPUTS       : UINT16 *ReconRefPtr       : Pointer to output block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Converts block of 8x8 unsigned 8-bit to block of 
 *                  signed 16-bit.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void UnpackBlock_C ( UINT8 *ReconPtr, INT16 *ReconRefPtr, UINT32 ReconPixelsPerLine )
{
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        ReconRefPtr[0] = (INT16)ReconPtr[0];
        ReconRefPtr[1] = (INT16)ReconPtr[1];
        ReconRefPtr[2] = (INT16)ReconPtr[2];
        ReconRefPtr[3] = (INT16)ReconPtr[3];
        ReconRefPtr[4] = (INT16)ReconPtr[4];
        ReconRefPtr[5] = (INT16)ReconPtr[5];
        ReconRefPtr[6] = (INT16)ReconPtr[6];
        ReconRefPtr[7] = (INT16)ReconPtr[7];
        
        // Start next row
        ReconPtr    += ReconPixelsPerLine;
        ReconRefPtr += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : SubtractBlock_C
 *  
 *  INPUTS        : UINT8 *SrcBlock : Pointer to 8x8 source block.
 *					UINT32 LineStep : Stride of source block.
 *
 *  OUTPUTS       : INT16 *DestPtr  : Pointer to 8x8 output block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Subtracts block pointed to by DestPtr from that pointed
 *                  to by SrcBlock. Result stored in DstPtr.
 *
 *  SPECIAL NOTES : None.
 *
 ****************************************************************************/
void SubtractBlock_C ( UINT8 *SrcBlock, INT16 *DestPtr, UINT32 LineStep )
{
    UINT32 i;

    // For each block row
    for ( i=0; i<BLOCK_HEIGHT_WIDTH; i++ )
    {
        DestPtr[0] = (INT16)((INT32)SrcBlock[0] - (INT32)DestPtr[0]);
        DestPtr[1] = (INT16)((INT32)SrcBlock[1] - (INT32)DestPtr[1]);
        DestPtr[2] = (INT16)((INT32)SrcBlock[2] - (INT32)DestPtr[2]);
        DestPtr[3] = (INT16)((INT32)SrcBlock[3] - (INT32)DestPtr[3]);
        DestPtr[4] = (INT16)((INT32)SrcBlock[4] - (INT32)DestPtr[4]);
        DestPtr[5] = (INT16)((INT32)SrcBlock[5] - (INT32)DestPtr[5]);
        DestPtr[6] = (INT16)((INT32)SrcBlock[6] - (INT32)DestPtr[6]);
        DestPtr[7] = (INT16)((INT32)SrcBlock[7] - (INT32)DestPtr[7]);
        
        // Start next row
        SrcBlock += LineStep;
        DestPtr  += BLOCK_HEIGHT_WIDTH;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : CopyBlock_C
 *
 *  INPUTS        : unsigned char *src     : Pointer to 8x8 source block.
 *                  unsigned int srcstride : Pointer to 8x8 destination block.
 *
 *  OUTPUTS       : unsigned char *dest    : Stride of blocks.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Copies a block from source to destination.
 *
 *  SPECIAL NOTES : Copies block in chunks of 32-bits at a time.
 *
 ****************************************************************************/
void CopyBlock_C ( unsigned char *src, unsigned char *dest, unsigned int srcstride )
{
	int j;
	unsigned char *s = src;
	unsigned char *d = dest;
	unsigned int stride = srcstride;

    for ( j=0; j<8; j++ )
	{
		((UINT32*)d)[0] = ((UINT32*)s)[0];
		((UINT32*)d)[1] = ((UINT32*)s)[1];
		s += stride;
		d += stride;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : Copy12x12_C
 *
 *  INPUTS        : const unsigned char *src : Pointer to source block.
 *                  unsigned int srcstride   : Stride of the source block.
 *                  unsigned int deststride  : Stride of the destination block.
 *
 *  OUTPUTS       : unsigned char *dest      : Pointer to destination block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Copies a 12x12 block from source to destination.
 *
 *  SPECIAL NOTES : None. 
 *
 ****************************************************************************/
void Copy12x12_C
(
    const unsigned char *src, 
    unsigned char *dest, 
    unsigned int srcstride,
    unsigned int deststride
)
{
	int j;
	const unsigned char *s = src;
	unsigned char *d = dest;

    for ( j=0; j<12; j++ )
	{
        d[0]  = s[0];
		d[1]  = s[1];
		d[2]  = s[2];
        d[3]  = s[3];
		d[4]  = s[4];
		d[5]  = s[5];
        d[6]  = s[6];
		d[7]  = s[7];
		d[8]  = s[8];
        d[9]  = s[9];
		d[10] = s[10];
		d[11] = s[11];
		s += srcstride;
		d += deststride;
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : InitVPUtil
 *
 *  INPUTS        : None.
 *                           
 *  OUTPUTS       : None.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Setup static initialized variables for Util.
 *
 *  SPECIAL NOTES : None
 *
 ****************************************************************************/
void InitVPUtil ( void )
{
	fillidctconstants ();
	UtilMachineSpecificConfig ();
}

/****************************************************************************
/* Fractional pixel prediction filtering...
****************************************************************************/

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock1d
 *  
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of source block.
 *                  UINT32 PixelStep        : 1 for horizontal filtering,
 *                                            SrcPixelsPerLine for vertical filtering.
 *                  UINT32 OutputHeight     : Height of the output block.
 *                  UINT32 OutputWidth      : Width of the output block.
 *                  INT32 *Filter           : Array of 4 filter taps.
 *
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to output block.
 *
 *  RETURNS       : void.
 *
 *  FUNCTION      : Applies a 1-D 4-tap filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block.
 *
 *  SPECIAL NOTES : Four filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock1d 
( 
    UINT8 *SrcPtr,
    UINT16 *OutputPtr,
    UINT32 SrcPixelsPerLine,
    UINT32 PixelStep,
    UINT32 OutputHeight,
    UINT32 OutputWidth,
    INT32 *Filter 
)
{
    UINT32 i, j;
	INT32  Temp;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply filter...
			Temp = ((INT32)SrcPtr[-(INT32)PixelStep] * Filter[0]) +
				   ((INT32)SrcPtr[0]                 * Filter[1]) +
				   ((INT32)SrcPtr[PixelStep]         * Filter[2]) +
				   ((INT32)SrcPtr[2*PixelStep]       * Filter[3]) + 
				    (FILTER_WEIGHT >> 1);       // Rounding

			// Normalize back to 0-255
			Temp = Temp >> FILTER_SHIFT;
			if ( Temp < 0 )
				Temp = 0;
			else if ( Temp > 255 )
				Temp = 255;

			OutputPtr[j] = (INT16)Temp;
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dFirstPass
 *  
 *  INPUTS        : UINT8 *SrcPtr           : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of source block.
 *                  UINT32 PixelStep        : 1 for horizontal filtering,
 *                                            SrcPixelsPerLine for vertical filtering.
 *                  UINT32 OutputHeight     : Height of the output block.
 *                  UINT32 OutputWidth      : Width of the output block.
 *                  INT32 *Filter           : Array of 4 filter taps.
 *
 *  OUTPUTS       : INT32 *OutputPtr        : Pointer to output block.
 *
 *  RETURNS       : void.
 *
 *  FUNCTION      : Applies a 1-D 4-tap filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement first-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Produces INT32 output to retain precision for next pass.
 *                  Four filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock2dFirstPass
( 
    UINT8 *SrcPtr,
    INT32 *OutputPtr,
    UINT32 SrcPixelsPerLine,
    UINT32 PixelStep,
    UINT32 OutputHeight,
    UINT32 OutputWidth,
    INT32 *Filter 
)
{
    UINT32 i, j;
	INT32  Temp;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply filter
			Temp =  ((INT32)SrcPtr[-(INT32)PixelStep] * Filter[0]) +
					((INT32)SrcPtr[0]                 * Filter[1]) +
					((INT32)SrcPtr[PixelStep]         * Filter[2]) +
					((INT32)SrcPtr[2*PixelStep]       * Filter[3]) + 
					 (FILTER_WEIGHT >> 1);      // Rounding

			// Normalize back to 0-255
			Temp = Temp >> FILTER_SHIFT;
			if ( Temp < 0 )
				Temp = 0;
			else if ( Temp > 255 )
				Temp = 255;

			OutputPtr[j] = Temp;
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dSecondPass
 *  
 *  INPUTS        : INT32 *SrcPtr           : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of source block.
 *                  UINT32 PixelStep        : 1 for horizontal filtering,
 *                                            SrcPixelsPerLine for vertical filtering.
 *                  UINT32 OutputHeight     : Height of the output block.
 *                  UINT32 OutputWidth      : Width of the output block.
 *                  INT32 *Filter           : Array of 4 filter taps.
 *
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to output block.
 *
 *  RETURNS       : void.
 *
 *  FUNCTION      : Applies a 1-D 4-tap filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement second-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Requires 32-bit input as produced by FilterBlock2dFirstPass.
 *                  Four filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock2dSecondPass 
(
    INT32 *SrcPtr,
    UINT16 *OutputPtr,
    UINT32 SrcPixelsPerLine,
    UINT32 PixelStep,
    UINT32 OutputHeight,
    UINT32 OutputWidth,
    INT32 *Filter 
)
{
    UINT32 i,j;
	INT32  Temp;

    for ( i=0; i < OutputHeight; i++ )
    {
		for ( j = 0; j < OutputWidth; j++ )
		{
			// Apply filter
			Temp = ((INT32)SrcPtr[-(INT32)PixelStep] * Filter[0]) +
				   ((INT32)SrcPtr[0]                 * Filter[1]) +
				   ((INT32)SrcPtr[PixelStep]         * Filter[2]) +
				   ((INT32)SrcPtr[2*PixelStep]       * Filter[3]) +
				    (FILTER_WEIGHT >> 1);   // Rounding

			// Normalize back to 0-255
			Temp = Temp >> FILTER_SHIFT;
			if ( Temp < 0 )
				Temp = 0;
			else if ( Temp > 255 )
				Temp = 255;

			OutputPtr[j] = (UINT16)Temp;
			SrcPtr++;
		}
			
        // Start next row
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2d
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  INT32  *HFilter         : Array of 4 horizontal filter taps.
 *                  INT32  *VFilter         : Array of 4 vertical filter taps.
 *					
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 2-D filters an 8x8 input block by applying a 4-tap 
 *                  filter horizontally followed by a 4-tap filter vertically
 *                  on the result.
 *
 *  SPECIAL NOTES : The intermediate horizontally filtered block must produce
 *                  3 more points than the input block in each column. This
 *                  is to ensure that the 4-tap filter has one extra data-point
 *                  at the top & 2 extra data-points at the bottom of each 
 *                  column so filter taps do not extend beyond data. Thus the
 *                  output of the first stage filter is an 8x11 (HxV) block.
 *
 ****************************************************************************/
void FilterBlock2d 
( 
    UINT8  *SrcPtr, 
    UINT16 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    INT32  *HFilter, 
    INT32  *VFilter 
)
{
	// First filter 1-D horizontally...
	FilterBlock2dFirstPass ( SrcPtr-SrcPixelsPerLine, FData, SrcPixelsPerLine, 1, 11, 8, HFilter );

	// then filter verticaly...
	FilterBlock2dSecondPass ( FData+BLOCK_HEIGHT_WIDTH, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter );
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock1dBil
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  UINT32 PixelStep        : Offset between filter input samples (see notes).
 *                  UINT32 OutputHeight     : Input block height.
 *                  UINT32 OutputWidth      : Input block width.
 *                  INT32  *Filter          : Array of 2 bi-linear filter taps.
 *					
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 2-tap 1-D bi-linear filter to input block in
 *                  either horizontal or vertical direction.
 *
 *  SPECIAL NOTES : PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock1dBil
( 
    UINT8  *SrcPtr, 
    UINT16 *OutputPtr, 
    UINT32  SrcPixelsPerLine, 
    UINT32  PixelStep, 
    UINT32  OutputHeight, 
    UINT32  OutputWidth, 
    INT32  *Filter 
)
{
    UINT32 i, j;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply filter 
            // NOTE: Rounding doesn't improve accuracy but is 
            //       easier to implement on certain platforms.
			OutputPtr[j] = (INT16)( ( ((INT32)SrcPtr[0]         * Filter[0]) +
							          ((INT32)SrcPtr[PixelStep] * Filter[1]) +
                                       (FILTER_WEIGHT/2) ) >> FILTER_SHIFT );		
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dBil_FirstPass
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  UINT32 PixelStep        : Offset between filter input samples (see notes).
 *                  UINT32 OutputHeight     : Input block height.
 *                  UINT32 OutputWidth      : Input block width.
 *                  INT32  *Filter          : Array of 2 bi-linear filter taps.
 *					
 *  OUTPUTS       : INT32 *OutputPtr        : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement first-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Produces INT32 output to retain precision for next pass.
 *                  Two filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock2dBil_FirstPass
( 
    UINT8 *SrcPtr,  
    INT32 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    UINT32 PixelStep, 
    UINT32 OutputHeight, 
    UINT32 OutputWidth, 
    INT32 *Filter 
)
{
    UINT32 i, j;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply bilinear filter
			OutputPtr[j] = ( ( (INT32)SrcPtr[0]          * Filter[0]) +
						       ((INT32)SrcPtr[PixelStep] * Filter[1]) +
                                (FILTER_WEIGHT/2) ) >> FILTER_SHIFT;
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dBil_SecondPass
 *  
 *  INPUTS        : INT32  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  UINT32 PixelStep        : Offset between filter input samples (see notes).
 *                  UINT32 OutputHeight     : Input block height.
 *                  UINT32 OutputWidth      : Input block width.
 *                  INT32  *Filter          : Array of 2 bi-linear filter taps.
 *					
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement second-pass
 *                  of 2-D separable filter.
 *
 *  SPECIAL NOTES : Requires 32-bit input as produced by FilterBlock2dBil_FirstPass.
 *                  Two filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock2dBil_SecondPass
(
    INT32 *SrcPtr, 
    UINT16 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    UINT32 PixelStep, 
    UINT32 OutputHeight, 
    UINT32 OutputWidth, 
    INT32 *Filter 
)
{
    UINT32 i,j;
	INT32  Temp;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply filter
			Temp =  ((INT32)SrcPtr[0]         * Filter[0]) +
					((INT32)SrcPtr[PixelStep] * Filter[1]) +
                    (FILTER_WEIGHT/2);
            OutputPtr[j] = (UINT16)(Temp >> FILTER_SHIFT);
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dBil
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  INT32  *HFilter         : Array of 2 horizontal filter taps.
 *                  INT32  *VFilter         : Array of 2 vertical filter taps.
 *					
 *  OUTPUTS       : UINT16 *OutputPtr       : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 2-D filters an 8x8 input block by applying a 2-tap 
 *                  bi-linear filter horizontally followed by a 2-tap 
 *                  bi-linear filter vertically on the result.
 *
 *  SPECIAL NOTES : The intermediate horizontally filtered block must produce
 *                  1 more point than the input block in each column. This
 *                  is to ensure that the 2-tap filter has one extra data-point
 *                  at the top of each column so filter taps do not extend 
 *                  beyond data. Thus the output of the first stage filter
 *                  is an 8x9 (HxV) block.
 *
 ****************************************************************************/
 void FilterBlock2dBil 
(
    UINT8  *SrcPtr, 
    UINT16 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    INT32  *HFilter, 
    INT32  *VFilter 
)
{
	// First filter 1-D horizontally...
	FilterBlock2dBil_FirstPass ( SrcPtr, FData, SrcPixelsPerLine, 1, 9, 8, HFilter );

	// then 1-D vertically...
	FilterBlock2dBil_SecondPass ( FData, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter );
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock_C
 *  
 *  INPUTS        : UINT8 *ReconPtr1     : Pointer to first 8x8 input block.
 *                  UINT8 *ReconPtr2     : Pointer to second 8x8 input block.
 *					UINT32 PixelsPerLine : Stride for ReconPtr1 & ReconPtr2.
 *				    INT32 ModX           : Fractional part of x-component of motion vector.
 *					INT32 ModY           : Fractional part of y-component of motion vector.
 *                  BOOL UseBicubic      : TRUE=Bicubic, FALSE=Bi-Linear filter.
 *				
 *  OUTPUTS       : UINT16 *ReconRefPtr  : Pointer to 8x8 filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Produces a filtered fractional pel prediction block
 *					using bilinear or bicubic filters.
 *
 *  SPECIAL NOTES : ReconPtr1 & ReconPtr2 point to blocks that bracket the
 *                  position of the fractional pixel motion vector. These
 *                  two blocks are combined using either a bi-linear or
 *                  bi-cubic filter to produce the output prediction block
 *                  for this motion vector.
 *                  ModX, ModY are used for filter selection--see code
 *                  comment for definition.
 *
 ****************************************************************************/
void FilterBlock_C
( 
    UINT8 *ReconPtr1,
    UINT8 *ReconPtr2,
    UINT16 *ReconRefPtr,
    UINT32 PixelsPerLine,
    INT32 ModX,
    INT32 ModY,
    BOOL UseBicubic 
)
{
	int diff;

	// ModX and ModY are the bottom three bits of the signed motion vector
    // components (in 1/8th pel units). This works out to be what we want
    // --despite the pointer swapping that goes on below.
	// For example...
    // if MV x-component is +ve then ModX = x%8.
	// if MV x-component is -ve then ModX = 8+(x%8), where X%8 is in the range -7 to -1.

	// Swap pointers to ensure that ReconPtr1 is "smaller than",
    // i.e. above, left, above-right or above-left, ReconPtr1
	diff = ReconPtr2 - ReconPtr1;

	if ( diff<0 ) 
	{
        // ReconPtr1>ReconPtr2, so swap...
		UINT8 *temp = ReconPtr1;
		ReconPtr1 = ReconPtr2;
		ReconPtr2 = temp;
		diff = (int)(ReconPtr2-ReconPtr1);
    } 

	if ( diff==1 )
	{   
        // Fractional pixel in horizontal only...											            
		if ( UseBicubic )
			FilterBlock1d ( ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 8, BicubicFilters[ModX] );
		else
			FilterBlock1dBil ( ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 8, BilinearFilters[ModX] );
	}
	else if ( diff == (int)(PixelsPerLine) )
	{
        // Fractional pixel in vertical only...
		if ( UseBicubic )
			FilterBlock1d ( ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 8, BicubicFilters[ModY] );
		else
			FilterBlock1dBil ( ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters[ModY] );
	}
	else if(diff == (int)(PixelsPerLine - 1))
	{
        // ReconPtr1 is Top right...
		if ( UseBicubic )
			FilterBlock2d ( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BicubicFilters[ModX], BicubicFilters[ModY] );
		else
			FilterBlock2dBil ( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters[ModX], BilinearFilters[ModY] );
	}
	else if(diff == (int)(PixelsPerLine + 1) )			
	{	
        // ReconPtr1 is Top left...
		if ( UseBicubic )
			FilterBlock2d ( ReconPtr1, ReconRefPtr, PixelsPerLine, BicubicFilters[ModX], BicubicFilters[ModY] );
		else
			FilterBlock2dBil ( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters[ModX], BilinearFilters[ModY] );
	}
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock1dBil_8
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  UINT32 PixelStep        : Offset between filter input samples (see notes).
 *                  UINT32 OutputHeight     : Input block height.
 *                  UINT32 OutputWidth      : Input block width.
 *                  INT32  *Filter          : Array of 2 bi-linear filter taps.
 *					
 *  OUTPUTS       : UINT8 *OutputPtr        : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 2-tap 1-D bi-linear filter to input block in
 *                  either horizontal or vertical direction.
 *
 *  SPECIAL NOTES : PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock1dBil_8
( 
    UINT8 *SrcPtr, 
    UINT8 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    UINT32 PixelStep, 
    UINT32 OutputHeight, 
    UINT32 OutputWidth, 
    INT32 *Filter )
{
    UINT32 i, j;

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ )
		{
			// Apply filter 
            // NOTE: Rounding doesn't improve accuracy but is 
            //       easier to implement on certain platforms.
			OutputPtr[j] = (UINT8)( ( ((INT32)SrcPtr[0]         * Filter[0]) + 
									  ((INT32)SrcPtr[PixelStep] * Filter[1]) + 
                                       (FILTER_WEIGHT/2) ) >> FILTER_SHIFT );
			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dBil_SecondPass_8
 *  
 *  INPUTS        : INT32  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  UINT32 PixelStep        : Offset between filter input samples (see notes).
 *                  UINT32 OutputHeight     : Input block height.
 *                  UINT32 OutputWidth      : Input block width.
 *                  INT32  *Filter          : Array of 2 bi-linear filter taps.
 *					
 *  OUTPUTS       : UINT8 *OutputPtr        : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Applies a 1-D 2-tap bi-linear filter to the source block in 
 *                  either horizontal or vertical direction to produce the
 *                  filtered output block. Used to implement second-pass
 *                  of 2-D separable bi-linear filter.
 *
 *  SPECIAL NOTES : Requires 32-bit input as produced by FilterBlock2dBil_FirstPass.
 *                  Two filter taps should sum to FILTER_WEIGHT.
 *                  PixelStep defines whether the filter is applied 
 *                  horizontally (PixelStep=1) or vertically (PixelStep=stride).
 *                  It defines the offset required to move from one input 
 *                  to the next.
 *
 ****************************************************************************/
void FilterBlock2dBil_SecondPass_8
( 
    INT32 *SrcPtr, 
    UINT8 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    UINT32 PixelStep, 
    UINT32 OutputHeight, 
    UINT32 OutputWidth, 
    INT32 *Filter 
)
{
    UINT32 i, j;
	INT32  Temp;
//	INT32  RoundValue = ((FILTER_WEIGHT*FILTER_WEIGHT) >> 1);

    for ( i=0; i<OutputHeight; i++ )
    {
		for ( j=0; j<OutputWidth; j++ ) 
		{
			// Apply bi-linear filter...
			Temp =  ((INT32)SrcPtr[0]         * Filter[0]) +
					((INT32)SrcPtr[PixelStep] * Filter[1]) + 
                    (FILTER_WEIGHT / 2);

			OutputPtr[j] = (UINT8)(Temp >> FILTER_SHIFT);

			SrcPtr++;
		}
			
        // Next row...
        SrcPtr    += SrcPixelsPerLine - OutputWidth;
        OutputPtr += OutputWidth;
    }
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlock2dBil_8
 *  
 *  INPUTS        : UINT8  *SrcPtr          : Pointer to source block.
 *                  UINT32 SrcPixelsPerLine : Stride of input block.
 *                  INT32  *HFilter         : Array of 2 horizontal filter taps.
 *                  INT32  *VFilter         : Array of 2 vertical filter taps.
 *					
 *  OUTPUTS       : UINT8 *OutputPtr        : Pointer to filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : 2-D filters an 8x8 input block by applying a 2-tap 
 *                  bi-linear filter horizontally followed by a 2-tap 
 *                  bi-linear filter vertically on the result. Output
 *                  is 8-bit unsigned.
 *
 *  SPECIAL NOTES : The intermediate horizontally filtered block must produce
 *                  1 more point than the input block in each column. This
 *                  is to ensure that the 2-tap filter has one extra data-point
 *                  at the top of each column so filter taps do not extend 
 *                  beyond data. Thus the output of the first stage filter
 *                  is an 8x9 (HxV) block.
 *
 ****************************************************************************/
void FilterBlock2dBil_8
( 
    UINT8 *SrcPtr, 
    UINT8 *OutputPtr, 
    UINT32 SrcPixelsPerLine, 
    INT32 *HFilter, 
    INT32 *VFilter 
)
{
	// First filter 1-D horizontally...
	FilterBlock2dBil_FirstPass ( SrcPtr, FData, SrcPixelsPerLine, 1, 9, 8, HFilter );

	// then filter 1-D vertically..
	FilterBlock2dBil_SecondPass_8 ( FData, OutputPtr, BLOCK_HEIGHT_WIDTH, BLOCK_HEIGHT_WIDTH, 8, 8, VFilter );
}

/****************************************************************************
 * 
 *  ROUTINE       : FilterBlockBil_8_C
 *  
 *  INPUTS        : UINT8 *ReconPtr1     : Pointer to first 8x8 input block.
 *                  UINT8 *ReconPtr2     : Pointer to second 8x8 input block.
 *					UINT32 PixelsPerLine : Stride for ReconPtr1 & ReconPtr2.
 *				    INT32 ModX           : Fractional part of x-component of motion vector.
 *					INT32 ModY           : Fractional part of y-component of motion vector.
 *				
 *  OUTPUTS       : UINT8 *ReconRefPtr   : Pointer to 8x8 filtered block.
 *
 *  RETURNS       : void
 *
 *  FUNCTION      : Produces a filtered fractional pel prediction block
 *					using bilinear filter.
 *
 *  SPECIAL NOTES : ReconPtr1 & ReconPtr2 point to blocks that bracket the
 *                  position of the fractional pixel motion vector. These
 *                  two blocks are combined using a bi-linear filter to
 *                  produce the output prediction block for this motion vector.
 *                  ModX, ModY are used for filter selection--see code
 *                  comment for definition.
 *
 ****************************************************************************/
void FilterBlockBil_8_C
( 
    UINT8 *ReconPtr1, 
    UINT8 *ReconPtr2, 
    UINT8 *ReconRefPtr, 
    UINT32 PixelsPerLine, 
    INT32  ModX, 
    INT32  ModY
)
{
	int diff;

    // ModX and ModY are the bottom three bits of the signed motion vector
    // components (in 1/8th pel units). This works out to be what we want
    // --despite the pointer swapping that goes on below.
	// For example...
    // if MV x-component is +ve then ModX = x%8.
	// if MV x-component is -ve then ModX = 8+(x%8), where X%8 is in the range -7 to -1.

    // Swap pointers to ensure that ReconPtr1 is "smaller than",
    // i.e. above, left, above-right or above-left, ReconPtr1
	diff = ReconPtr2 - ReconPtr1;

    if ( diff<0 )
	{
        // ReconPtr1>ReconPtr2, so swap...
		UINT8 *temp = ReconPtr1;
		ReconPtr1 = ReconPtr2;
		ReconPtr2 = temp;
		diff = (int)(ReconPtr2-ReconPtr1);
	}

	if ( diff==1 )
	{											        
        // Fractional pixel in horizontal only...
		FilterBlock1dBil_8 ( ReconPtr1, ReconRefPtr, PixelsPerLine, 1, 8, 8, BilinearFilters[ModX] );
	}
	else if ( diff == (int)(PixelsPerLine) )				
	{
        // Fractional pixel in vertical only...
		FilterBlock1dBil_8 ( ReconPtr1, ReconRefPtr, PixelsPerLine, PixelsPerLine, 8, 8, BilinearFilters[ModY] );
	}
	else if ( diff == (int)(PixelsPerLine - 1))
	{	
        // ReconPtr1 is Top right...
		FilterBlock2dBil_8 ( ReconPtr1-1, ReconRefPtr, PixelsPerLine, BilinearFilters[ModX], BilinearFilters[ModY] );
	}
	else if ( diff == (int)(PixelsPerLine + 1) )
	{	
        // ReconPtr1 is Top left
		FilterBlock2dBil_8 ( ReconPtr1, ReconRefPtr, PixelsPerLine, BilinearFilters[ModX], BilinearFilters[ModY] );
	}
}

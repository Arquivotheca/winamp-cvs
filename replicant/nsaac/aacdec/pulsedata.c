/*
  pulse data tool
*/
#include "block.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function reads the elements for pulse data from
  the bitstream.
*/
void CPulseData_Read(HANDLE_BIT_BUF bs,     /*!< pointer to bitstream */
                     CPulseData *PulseData) /*!< pointer to pulse data side info */
{
  int i;
  
     
  if ((PulseData->PulseDataPresent = (char) GetBits(bs,1)))
  {
      
    PulseData->NumberPulse = (char) GetBits(bs,2);
      
    PulseData->PulseStartBand = (char) GetBits(bs,6);
     /* PulseData->PulseOffset[i]
                    PulseData->PulseAmp[i]
                 */
    
    for (i=0; i<=PulseData->NumberPulse; i++)
    {
       
      PulseData->PulseOffset[i] = (char) GetBits(bs,5);
       
      PulseData->PulseAmp[i] = (char) GetBits(bs,4);
    }
  }
  
}
/*
  The function applies the pulse data to the
  specified spectral lines.
*/
void CPulseData_Apply(CPulseData *PulseData,               /*!< pointer to pulse data side info */
                      const short *pScaleFactorBandOffsets, /*!< pointer to scalefactor band offsets */
                      int *coef)                           /*!< pointer to spectrum */
{
  int i,k;
  
   
  if (PulseData->PulseDataPresent)
  {
     
    k = pScaleFactorBandOffsets[PulseData->PulseStartBand];
  
     /* PulseData->PulseOffset[i]
                    PulseData->PulseAmp[i]
                 */
    
    for (i=0; i<=PulseData->NumberPulse; i++)
    {
      
      k += PulseData->PulseOffset[i];
         
      if (coef [k] > 0) coef[k] += PulseData->PulseAmp[i];
      else              coef[k] -= PulseData->PulseAmp[i];
    }
  }
  
}

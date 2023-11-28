/*
  SBR Payload Extraction
*/
#include "math/FloatFR.h"
#include "sbrdec/sbrdecoder.h"
#include "bitbuffer.h"
#include "aacPLUScheck.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  \brief Extraction of aacPlus - specific payload from fill element
  \return void
*/
void FFRaacplus_checkForPayload(HANDLE_BIT_BUF bs,
                                SBRBITSTREAM *streamSBR,
                                int prev_element)
{
  int i;
  int count=0;
  int esc_count=0;
  
   
  
  count = ReadBits(bs,4);
   
  if (count == 15)
  {
    
    esc_count = ReadBits(bs,8);
    
    count =  esc_count + 14;
  }
  
  if (count)
  {
     unsigned char extension_type;
    
    extension_type = (unsigned char) ReadBits(bs,4);
      
    if (   (prev_element == SBR_ID_SCE || prev_element == SBR_ID_CPE)
        && ((extension_type == SBR_EXTENSION) || (extension_type == SBR_EXTENSION_CRC))
        && (streamSBR->NrElements < MAXNRELEMENTS)    )
    {
        
      streamSBR->sbrElement [streamSBR->NrElements].Data[0] = (unsigned char) ReadBits(bs,4);
       /* streamSBR->sbrElement [streamSBR->NrElements].Data[i] */
      
      for (i=1; i<count; i++)
      {
         
        streamSBR->sbrElement [streamSBR->NrElements].Data[i] = (unsigned char) ReadBits(bs,8);
      }
      
      streamSBR->sbrElement[streamSBR->NrElements].ExtensionType = extension_type;
      streamSBR->sbrElement[streamSBR->NrElements].Payload = count;
        
      streamSBR->NrElements += 1;
    }
    else
    {
      
      ReadBits(bs,4);
      
      for (i=1; i<count; i++)
      {
        
        ReadBits(bs,8);
      }
    }
  }
  
}

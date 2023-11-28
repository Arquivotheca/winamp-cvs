/*
  Bitstream routines
*/
#include "aac_ram.h"
#include "math/FloatFR.h"
#include "bitbuffer.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
  The function loads bits from bitstream buffer.
  
  return:  valid bits
*/
unsigned long GetBits(HANDLE_BIT_BUF hBitBuf,  /*!< pointer to current data in bitstream */
                      int nBits)               /*!< number of bits */
{
  unsigned short tmp;
  
  
  tmp = (unsigned short) ReadBits(hBitBuf,nBits);
  
  return (tmp);
}
/*
  The function rewinds the bitstream pointeers
*/
void PushBack(HANDLE_BIT_BUF hBitBuf, /*!< pointer to current data in bitstream */
              int nBits)              /*!< number of bits */
{
  
   
  WindBitBufferBidirectional(hBitBuf,-nBits);
  
}
/*
  The function applies byte alignement
*/
void ByteAlign(HANDLE_BIT_BUF hBitBuf, /*!< pointer to current data in bitstream */
               long *pByteAlignBits)   /*!< pointer to last state of cntBits */
{
  int alignment;
  
    
  alignment = (*pByteAlignBits - hBitBuf->cntBits) % 8;
  
  if (alignment)
  {
     
    GetBits(hBitBuf,8 - alignment);
  }
  
  *pByteAlignBits = hBitBuf->cntBits;
  
}

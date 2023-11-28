/*
  CRC check coutines
*/
#include "sbr_crc.h"
#include "sbr_bitb.h"
#include "sbr_const.h"
#include "math/FloatFR.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
const unsigned short MAXCRCSTEP = 16;
/*
  \brief     crc calculation
*/
static unsigned long
calcCRC (HANDLE_CRC hCrcBuf, unsigned long bValue, int nBits)
{
  int i;
  unsigned long bMask = (1UL << (nBits - 1));
  
    
  
  for (i = 0; i < nBits; i++, bMask >>= 1) {
    unsigned short flag = (hCrcBuf->crcState & hCrcBuf->crcMask) ? 1 : 0;
    unsigned short flag1 = (bMask & bValue) ? 1 : 0;
         
    
    flag ^= flag1;
     
    hCrcBuf->crcState <<= 1;
    
    if (flag)
    {
        
      hCrcBuf->crcState ^= hCrcBuf->crcPoly;
    }
  }
  
  return (hCrcBuf->crcState);
}
/*
  \brief     crc
*/
static int
getCrc (HANDLE_BIT_BUFFER hBitBuf, unsigned long NrBits)
{
  int i;
  int CrcStep = NrBits / MAXCRCSTEP;
  int CrcNrBitsRest = (NrBits - CrcStep * MAXCRCSTEP);
  unsigned long bValue;
  CRC_BUFFER CrcBuf;
  
  DIV(1);   
  
  CrcBuf.crcState = SBR_CRC_START;
  CrcBuf.crcPoly  = SBR_CRC_POLY;
  CrcBuf.crcMask  = SBR_CRC_MASK;
  
  for (i = 0; i < CrcStep; i++) {
    
    bValue = getbits (hBitBuf, MAXCRCSTEP);
     
    calcCRC (&CrcBuf, bValue, MAXCRCSTEP);
  }
  
  bValue = getbits (hBitBuf, CrcNrBitsRest);
   
  calcCRC (&CrcBuf, bValue, CrcNrBitsRest);
   /* counting post operation */
  
  return (CrcBuf.crcState & SBR_CRC_RANGE);
}
/*
  \brief   crc interface
  \return  1: CRC OK, 0: CRC check failure
*/
int
SbrCrcCheck (HANDLE_BIT_BUFFER hBitBuf,
             long NrBits)
{
  int crcResult = 1;
  BIT_BUFFER BitBufferCRC;
  unsigned long NrCrcBits;
  unsigned long crcCheckResult;
  long NrBitsAvailable;
  unsigned long crcCheckSum;
  
   
  
  crcCheckSum = getbits (hBitBuf, SI_SBR_CRC_BITS);
   
  CopyBitbufferState (hBitBuf, &BitBufferCRC);
   
  NrBitsAvailable = GetNrBitsAvailable(&BitBufferCRC);
  
  if (NrBitsAvailable <= 0){
    
    return 0;
  }
  
  NrCrcBits = min (NrBits, NrBitsAvailable);
   
  crcCheckResult = getCrc (&BitBufferCRC, NrCrcBits);
   
  if (crcCheckResult != crcCheckSum) {
    
    crcResult = 0;
  }
  
  return crcResult;
}

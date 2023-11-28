/*
  Bitbuffer management
*/
#include <assert.h>
#include "sbr_bitb.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/*
 \brief     Reads n bits from Bitbuffer
 \return    bits
*/
unsigned long
getbits (HANDLE_BIT_BUFFER hBitBuf,
         int n)
{
  unsigned long ret_value = 0;
  assert(n <= 32);
  
   
  
  while (n>8) {
    
    n -= 8;
      
    ret_value += getbits(hBitBuf,8) << n;
  }
    
  if (hBitBuf->buffered_bits <= 8) {
       
    hBitBuf->buffer_word = (hBitBuf->buffer_word << 8) | *hBitBuf->char_ptr++;
     
    hBitBuf->buffered_bits += 8;
  }
   
  hBitBuf->buffered_bits -= n;
    
  ret_value +=
    (hBitBuf->buffer_word >> hBitBuf->buffered_bits) & ((1 << n) - 1);
    
  hBitBuf->nrBitsRead += n;
  
  return (ret_value);
}
/*
  \brief       Initialize variables for reading the bitstream buffer
*/
void
initBitBuffer (HANDLE_BIT_BUFFER hBitBuf,
	       unsigned char *start_ptr,
               unsigned long bufferLen)
{
  
   
  hBitBuf->char_ptr = start_ptr;
  hBitBuf->buffer_word = 0;
  hBitBuf->buffered_bits = 0;
  hBitBuf->nrBitsRead = 0;
  hBitBuf->bufferLen = bufferLen;
  
}
/*
  \brief   Copy the bitbuffer state to a second bitbuffer instance
*/
void
CopyBitbufferState (HANDLE_BIT_BUFFER hBitBuf,
                    HANDLE_BIT_BUFFER hBitBufDest)
{
  
   
  hBitBufDest->char_ptr = hBitBuf->char_ptr;
  hBitBufDest->buffer_word = hBitBuf->buffer_word;
  hBitBufDest->buffered_bits = hBitBuf->buffered_bits;
  hBitBufDest->nrBitsRead = hBitBuf->nrBitsRead;
  hBitBufDest->bufferLen = hBitBuf->bufferLen;
  
}
/*
  \brief       returns number bit read since initialisation
  \return      number bit read since initialisation
*/
unsigned long
GetNrBitsRead (HANDLE_BIT_BUFFER hBitBuf)
{
    
  
  return (hBitBuf->nrBitsRead);
}
/*
  \brief       returns number bits available in bit buffer
  \return      number bits available in bit buffer
*/
long
GetNrBitsAvailable (HANDLE_BIT_BUFFER hBitBuf)
{
     
  
  return (hBitBuf->bufferLen - hBitBuf->nrBitsRead);
}

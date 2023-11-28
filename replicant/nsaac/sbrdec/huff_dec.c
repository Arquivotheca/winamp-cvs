/*
  Huffman Decoder
*/
#include "huff_dec.h"
#include "math/FloatFR.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
/***************************************************************************/
/*
  \brief     Decodes one huffman code word
  \return    decoded value
****************************************************************************/
int
DecodeHuffmanCW (Huffman h,
                 HANDLE_BIT_BUFFER hBitBuf)
{
  char index = 0;
  int value, bit;
  
   
  
  while (index >= 0) {
    
    bit = getbits (hBitBuf, 1);
     
    index = h[index][bit];
  }
  
  value = index+64;
  
  return value;
}

#include "dse.h"
#include "bitstream.h"
#include "math/counters.h" /* the 3GPP instrumenting tool */
void DSE_Read(HANDLE_BIT_BUF bs, long *byteBorder)
{
  char data_byte_align_flag;
  short cnt, i;
  
  GetBits(bs, 4);
  data_byte_align_flag = GetBits(bs, 1);
  cnt = GetBits(bs, 8);
   
  if (cnt == 255) 
	{     
    cnt += GetBits(bs, 8);
  }
  
  if (data_byte_align_flag) 
	{
    ByteAlign(bs, byteBorder);
  }
  
  for (i = 0; i < cnt; i++) 
	{
    GetBits(bs, 8);
  }
}

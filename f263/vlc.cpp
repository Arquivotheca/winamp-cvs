#include "Decoder.h"
#include "vlc_table.h"

static int PRED_type_EP[]     =  {0,0,1,1,1,2,2,2,3,3};
static int QUANT_present_EP[] =  {0,1,0,0,1,0,0,1,0,1};
static int CBP_present_EP[]   =  {1,1,0,1,1,0,1,1,1,1};
static VLCtab MBTYPEtabEP[] = {
{-1,0}, {9,8}, {8,7}, {8,7}, {7,6}, {7,6}, {7,6}, {7,6}, 
{4,5}, {4,5}, {4,5}, {4,5}, {4,5}, {4,5}, {4,5}, {4,5}, 
{5,5}, {5,5}, {5,5}, {5,5}, {5,5}, {5,5}, {5,5}, {5,5}, 
{6,5}, {6,5}, {6,5}, {6,5}, {6,5}, {6,5}, {6,5}, {6,5}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3},
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}
};
static int PRED_type_B[]    =   {0,0,1,1,1,2,2,2,3,3,3,4,4};
static int QUANT_present_B[] =  {0,1,0,0,1,0,0,1,0,0,1,0,1};
static int CBPC_pattern_EI[]  =  {0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3};
static int CBP_present_B[]   =  {1,1,0,1,1,0,1,1,0,1,1,1,1};
static VLCtab MBTYPEtabB[] = {
{-1,0}, {12,7}, {11,6}, {11,6}, {10,5}, {10,5}, {10,5}, {10,5}, 
{1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, {1,4}, 
{8,5}, {8,5}, {8,5}, {8,5}, {9,5}, {9,5}, {9,5}, {9,5}, 
{4,5}, {4,5}, {4,5}, {4,5}, {7,5}, {7,5}, {7,5}, {7,5},
{5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, 
{5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, {5,3}, 
{6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, 
{6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, {6,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3} 
};

static int QUANT_present_EI[] =  {0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};
static VLCtab MBTYPEtabEI[] = {
{-1,0}, {8,8}, {5,7}, {5,7}, {6,7}, {6,7}, {7,7}, {7,7},
{-1,0}, {9,8}, {10,8}, {11,8}, {12,8}, {13,8}, {14,8}, {15,8},
{4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, 
{4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, {4,4}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, {1,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, {2,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, 
{3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}, {3,3}
};

static int PRED_type_EI[]     =  {1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3};

int Decoder::getTMNMV()
{
  int code;

  if (buffer.getbits1())
  {
    return 0;
  }

  if ((code = buffer.showbits(12))>=512)
  {
    code = (code>>8) - 2;
    buffer.flushbits(TMNMVtab0[code].len);

    return TMNMVtab0[code].val;
  }

  if (code>=128)
  {
    code = (code>>2) -32;
    buffer.flushbits(TMNMVtab1[code].len);


    return TMNMVtab1[code].val;
  }

  if ((code-=5)<0)
  {
    fault=1;
    return 0;
  }

  buffer.flushbits(TMNMVtab2[code].len);

  return TMNMVtab2[code].val;
}


int Decoder::getMCBPC()
{
	  int code;

  code = buffer.showbits (13);

  if (code >> 4 == 1)
  {
    /* macroblock stuffing */
    buffer.flushbits (9);
    return 255;
  }
  if (code == 0)
  {
    fault = 1;
    return 0;
  }
  if (code >= 4096)
  {
    buffer.flushbits (1);
    return 0;
  }
  if (code >= 16)
  {
    buffer.flushbits (MCBPCtab0[code >> 4].len);
    return MCBPCtab0[code >> 4].val;
  } 
	else
  {
    buffer.flushbits (MCBPCtab1[code - 8].len);
    return MCBPCtab1[code - 8].val;
  }
}

int Decoder::getMCBPCintra()
{
  int code;

  code = buffer.showbits(9);

  if (code == 1) {
    /* macroblock stuffing */
    buffer.flushbits(9);
    return 255;
  }

  if (code < 8) {
    fault = 1;
    return 0;
  }

  code >>= 3;
    
  if (code>=32)
  {
    buffer.flushbits(1);
    return 3;
  }

  buffer.flushbits(MCBPCtabintra[code].len);

  return MCBPCtabintra[code].val;
}

int Decoder::getCBPY()
{
  int code;

  code = buffer.showbits(6);
  if (code < 2) {
    fault = 1;
    return -1;
  }
    
  if (code>=48)
  {
    buffer.flushbits(2);
    return 0;
  }

  buffer.flushbits(CBPYtab[code].len);

  return CBPYtab[code].val;
}

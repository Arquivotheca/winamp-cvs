#include <stdio.h>

#include <math.h>
#include "mp2decode.h"

//-------------------------------------------------------------------------*
// Layer 2 allocation tables
//-------------------------------------------------------------------------*


typedef struct 
{
	short bits;
	short d;
} allocation_tab;


static allocation_tab g_alloc_0[] = {
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767} 
};

static allocation_tab g_alloc_1[] = {
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767} 
};

static allocation_tab g_alloc_2[] = {
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63} 
};

static allocation_tab g_alloc_3[] = {
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63} 
};

static allocation_tab g_alloc_4[] = {
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9}  
};

static char g_base_values[3][9] = {
	{ 1 , 0, 2 , } ,
	{ 17, 18, 0 , 19, 20 , } ,
	{ 21, 1, 22, 23, 0, 24, 25, 2, 26 } 
};
static float g_scales[27] = {
	0.0f,-2.0f/3.0f,2.0f/3.0f,2.0f/7.0f,2.0f/15.0f,2.0f/31.0f,2.0f/63.0f,2.0f/127.0f,2.0f/255.0f,
		2.0f/511.0f,2.0f/1023.0f,2.0f/2047.0f,2.0f/4095.0f,2.0f/8191.0f,2.0f/16383.0f,2.0f/32767.0f, 
		2.0f/65535.0f,-4.0f/5.0f,-2.0f/5.0f,2.0f/5.0f,4.0f/5.0f,-8.0f/9.0f,-4.0f/9.0f,-2.0f/9.0f,
		2.0f/9.0f,4.0f/9.0f,8.0f/9.0f
};

static char g_translate[3][2][16] = 
{ { { 0,2,2,2,2,2,2,0,0,0,1,1,1,1,1,0 } ,
{ 0,2,2,0,0,0,1,1,1,1,1,1,1,1,1,0 } } ,
{ { 0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0 } ,
{ 0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0 } } ,
{ { 0,3,3,3,3,3,3,0,0,0,1,1,1,1,1,0 } ,
{ 0,3,3,0,0,0,1,1,1,1,1,1,1,1,1,0 } } };

static allocation_tab *g_tables[5] = { g_alloc_0, g_alloc_1, g_alloc_2, g_alloc_3 , g_alloc_4 };
static char g_sblims[5] = { 27 , 30 , 8, 12 , 30 };


//-------------------------------------------------------------------------*
//
//                   C M p 2 D e c o d e
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CMp2Decode::CMp2Decode
(
 CMpegBitStream &_Bs, 
 int             _Quality, 
 int             _Downmix,
 DecoderHooks *_hooks
 ) :

m_Polyphase(m_Info, _Quality, _Downmix),

m_Bs(_Bs),

m_Quality(_Quality),
m_Downmix(_Downmix),
hooks(_hooks)

{
	// full reset
	Init(true);

	int i,j,k,l,len;
	char tablen[3] = { 3 , 5 , 9 };
	char *itable;
	char *tables[3] = { m_tab_3, m_tab_5, m_tab_9};

	for(i=0;i<3;i++)
	{
		itable = tables[i];
		len = tablen[i];
		for(j=0;j<len;j++)
			for(k=0;k<len;k++)
				for(l=0;l<len;l++)
				{
					*itable++ = g_base_values[i][l];
					*itable++ = g_base_values[i][k];
					*itable++ = g_base_values[i][j];
				}
	}
	for(k=0;k<27;k++)
	{
		float m=g_scales[k];
		float *table = m_scales[k];
		for (j=3,i=0;i<63;i++,j--) *table++ =  (m * (float)pow(2.0f,(float) j / 3.0f))*32768.0f;
		*table++ = 0.0;
	}
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CMp2Decode::~CMp2Decode()
{
}

//-------------------------------------------------------------------------*
//   Init
//-------------------------------------------------------------------------*

void CMp2Decode::Init(bool fFullReset)
{

	if ( fFullReset )
	{

		// reset polyphase
		m_Polyphase.Init();

		// reset all spectrum members
		ZeroPolySpectrum();
	}
}

//-------------------------------------------------------------------------*
//   Decode
//-------------------------------------------------------------------------*

SSC CMp2Decode::Decode(void *pPcm, int cbPcm,  int *pcbUsed)
{
	int  nChannels = m_Downmix ? 1 : m_Bs.GetHdr()->GetChannels();
	SSC  dwResult  = SSC_OK;
	int  nOutBytes;

	int layer=m_Bs.GetHdr()->GetLayer();
	//
	// return if wrong layer
	//
	if ( layer != 2  && layer != 1)
	{
		// error wrong layer
		return SSC_E_MPGA_WRONGLAYER;
	}

	//
	// calculate number of ouput bytes
	//
	nOutBytes = (m_Bs.GetHdr()->GetSamplesPerFrame() * nChannels * sizeof(float)) >> m_Quality;

	//
	// check if PCM buffer is large enough
	//
	if ( cbPcm < nOutBytes )
	{
		// error buffer too small
		return SSC_E_MPGA_BUFFERTOOSMALL;
	}

	//
	// skip mpeg header
	//
	m_Bs.Ff(m_Bs.GetHdr()->GetHeaderLen());

	//
	// set info structure
	//
	SetInfo();


	if (layer==2) dwResult=Decode2(pPcm);
	else dwResult=Decode1(pPcm);

	//
	// seek to end of frame
	//
	m_Bs.Seek(m_Bs.GetHdr()->GetFrameLen() - m_Bs.GetBitCnt());

	//
	// set number of bytes used in PCM buffer
	//
	if ( pcbUsed && SSC_SUCCESS(dwResult) )
		*pcbUsed = nOutBytes;

	return dwResult;
}


SSC CMp2Decode::Decode2(void *pPcm)
{

	int nChannels = m_Bs.GetHdr()->GetChannels();
	int i,gr,js_bound;
	allocation_tab *alloc_tab, *alloc1;
	unsigned char allocation[64];
	unsigned char scalefacts[64];
	unsigned char *scfsi,*bita;
	int c_p=0;
	int scalefactor[192],*scale;
	int sblimit,sblimit2; 

	{
		int table;
		if (m_Bs.GetHdr()->GetMpegVersion() != 0) table = 4;
		else table = g_translate[m_Bs.GetHdr()->GetSampleRateNdx()][2-nChannels][m_Bs.GetHdr()->GetBitrateNdx()];
		sblimit  =  g_sblims[table];
		sblimit2 =  g_sblims[table]*nChannels;
		alloc_tab = g_tables[table];
	}

	if (m_Bs.GetHdr()->GetMode() == 1) js_bound = 4 + m_Bs.GetHdr()->GetModeExt()*4;
	else js_bound=sblimit;

	bita = allocation;
	alloc1=alloc_tab;
	if (nChannels==2)
	{
		i=js_bound; 
		while (i--)
		{
			*bita++ = (char) m_Bs.GetBits(alloc1->bits);
			*bita++ = (char) m_Bs.GetBits(alloc1->bits);
			alloc1 += 1<<alloc1->bits;
		}
		i=sblimit-js_bound;
		while (i--)
		{
			bita[1] = bita[0] = (char) m_Bs.GetBits(alloc1->bits);
			bita+=2;
			alloc1 += 1<<alloc1->bits;
		}
	}
	else
	{
		i=js_bound;
		while (i--)
		{
			*bita++ = (char) m_Bs.GetBits(alloc1->bits);
			alloc1 += 1<<alloc1->bits;
		}
	}

	bita = allocation;
	scfsi=scalefacts;
	i=sblimit2;
	while (i--) if (*bita++) *scfsi++ = (char) m_Bs.GetBits(2);

	i=sblimit2;
	bita = allocation;
	scfsi=scalefacts;
	scale=scalefactor;
	while (i--) if (*bita++)
	{
		switch (*scfsi++) 
		{
		case 0: 
			scale[0] = m_Bs.GetBits(6);
			scale[1] = m_Bs.GetBits(6);
			scale[2] = m_Bs.GetBits(6);
			break;
		case 1 : 
			scale[1] = scale[0] = m_Bs.GetBits(6);
			scale[2] = m_Bs.GetBits(6);
			break;
		case 2: 
			scale[1] = scale[2] = scale[0] = m_Bs.GetBits(6);
			break;
		case 3:
			scale[0] = m_Bs.GetBits(6);
			scale[2] = scale[1] = m_Bs.GetBits(6);
			break;
		}
		scale+=3;
	}

	for (gr=0;gr<12;gr++) 
	{
		allocation_tab *alloc1 = alloc_tab;
		bita=allocation;
		scale=scalefactor;

		for (i = 0; i < js_bound; i ++)
		{
			int channel;
			for (channel = 0; channel < nChannels; channel++)
			{
				int ba=*bita++;
				if (ba) 
				{
					allocation_tab *alloc2 = alloc1+ba;
					if (alloc2->d < 0) 
					{
						float cm=m_scales[alloc2->bits][scale[gr/4]];
						m_PolySpectrum[channel][c_p+0][i] = ((float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d)) * cm;
						m_PolySpectrum[channel][c_p+1][i] = ((float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d)) * cm;
						m_PolySpectrum[channel][c_p+2][i] = ((float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d)) * cm;
					}        
					else 
					{
						unsigned int idx=(unsigned int) m_Bs.GetBits(alloc2->bits),	m=scale[gr/4];
						char *tab=0;
						switch (alloc2->d)
						{
						case 3: tab=m_tab_3; break;
						case 5: tab=m_tab_5; break;
						case 9: tab=m_tab_9; break;
						}
						if (tab != 0)
						{
							tab += idx+idx+idx;
							m_PolySpectrum[channel][c_p+0][i] = m_scales[tab[0]][m];
							m_PolySpectrum[channel][c_p+1][i] = m_scales[tab[1]][m];
							m_PolySpectrum[channel][c_p+2][i] = m_scales[tab[2]][m];  
						}
						else
						{
							m_PolySpectrum[channel][c_p][i]=m_PolySpectrum[channel][c_p+1][i]=m_PolySpectrum[channel][c_p+2][i]=0.0;
						}
					}
					scale+=3;
				}
				else 
				{
					m_PolySpectrum[channel][c_p][i]=m_PolySpectrum[channel][c_p+1][i]=m_PolySpectrum[channel][c_p+2][i]=0.0;
				}
			}
			alloc1+=(1<<alloc1->bits);
		}

		for (; i < sblimit; i ++)
		{
			int ba;
			bita++;
			if ( (ba=*bita++) )
			{
				allocation_tab *alloc2 = alloc1+ba;
				if (alloc2->d < 0)
				{
					float cm_l=m_scales[alloc2->bits][scale[gr/4]];
					float cm_r=m_scales[alloc2->bits][scale[gr/4+3]];
					float t;
					t=(float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d); 
					m_PolySpectrum[0][c_p+0][i] = t * cm_l; 
					m_PolySpectrum[1][c_p+0][i] = t * cm_r; 
					t=(float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d); 
					m_PolySpectrum[0][c_p+1][i] = t * cm_l; 
					m_PolySpectrum[1][c_p+1][i] = t * cm_r;
					t=(float) ((int)m_Bs.GetBits(alloc2->bits) + alloc2->d); 
					m_PolySpectrum[0][c_p+2][i] = t * cm_l;
					m_PolySpectrum[1][c_p+2][i] = t * cm_r;
				}
				else
				{
					unsigned int idx,ml,mr;
					char *tab=0;
					ml = scale[gr/4]; mr = scale[gr/4+3];
					idx = (unsigned int) m_Bs.GetBits(alloc2->bits);
					switch (alloc2->d)
					{
					case 3: tab=m_tab_3; break;
					case 5: tab=m_tab_5; break;
					case 9: tab=m_tab_9; break;
					}
					if (tab != 0)
					{
						tab += idx+idx+idx;
						m_PolySpectrum[0][c_p+0][i] = m_scales[tab[0]][ml]; 
						m_PolySpectrum[1][c_p+0][i] = m_scales[tab[0]][mr];
						m_PolySpectrum[0][c_p+1][i] = m_scales[tab[1]][ml]; 
						m_PolySpectrum[1][c_p+1][i] = m_scales[tab[1]][mr];
						m_PolySpectrum[0][c_p+2][i] = m_scales[tab[2]][ml]; 
						m_PolySpectrum[1][c_p+2][i] = m_scales[tab[2]][mr];
					}
					else 
					{
						m_PolySpectrum[1][c_p+0][i]=m_PolySpectrum[0][c_p][i]=
							m_PolySpectrum[1][c_p+1][i]=m_PolySpectrum[0][c_p+1][i]=
							m_PolySpectrum[1][c_p+2][i]=m_PolySpectrum[0][c_p+2][i]=0.0;
					}
				}
				scale+=6;
			}
			else 
			{
				m_PolySpectrum[1][c_p][i]=m_PolySpectrum[0][c_p][i]=
					m_PolySpectrum[1][c_p+1][i]=m_PolySpectrum[0][c_p+1][i]=
					m_PolySpectrum[1][c_p+2][i]=m_PolySpectrum[0][c_p+2][i]=0.0;
			}
			alloc1+=(1<<alloc1->bits);
		}

		{
			int j;
			for (j=0;j<nChannels;j++) for (i=sblimit;i<32;i++)
			{
				int x;
				for (x = 0; x < 3; x ++)
					m_PolySpectrum[j][c_p+x][i]=0.0;
			}
		}

		if (gr==5 || gr==11)
		{
			if (m_Downmix && nChannels == 2)
			{
				int x=32*18;
				float *p=&m_PolySpectrum[0][0][0];
				while (x--)
				{
					p[0] = (p[32*18]+p[0])*0.5f;
					p++;
				}
			}

			if (hooks)
			{
				hooks->layer2_eq(&m_PolySpectrum[0][0][0],m_Downmix ? 1 : nChannels,m_Bs.GetHdr()->GetSampleRate(),18);
			}

			pPcm = (unsigned char *)m_Polyphase.Apply(m_PolySpectrum, (float *)pPcm, 18);
			c_p=0;
		} 
		else c_p+=3;
	}	


	return SSC_OK;
}



SSC CMp2Decode::Decode1(void *pPcm)
{
	int nChannels = m_Bs.GetHdr()->GetChannels();


	if (nChannels==2) 
	{
		unsigned int scale_index[64], *sca;
		unsigned int allocation[64], *al;
		int js_top = m_Bs.GetHdr()->GetMode()==1?(m_Bs.GetHdr()->GetModeExt()<<2)+4:32;
		int i;

		al = allocation; 
		i=js_top+js_top; 
		while (i--) *al++ = m_Bs.GetBits(4);
		i=32-js_top; 
		while (i--) *al++ = m_Bs.GetBits(4);

		al = allocation; 
		sca=scale_index; 
		i=js_top+js_top; 
		while (i--) if ((*al++)) *sca++ = m_Bs.GetBits(6);
		i=32-js_top;
		while (i--) 
		{
			if ((*al++)) 
			{
				*sca++ = m_Bs.GetBits(6);
				*sca++ = m_Bs.GetBits(6);
			}
		}

		float *f_l = &m_PolySpectrum[0][0][0];
		float *f_r = &m_PolySpectrum[1][0][0];
		for (i = 0; i < 12; i ++)
		{
			int l=js_top;
			sca = scale_index; al = allocation;

			while (l--)
			{
				int c=*al++;
				if (c) *f_l++ = (float) ((int)m_Bs.GetBits(c+1) + 1 - (1<<c)) * m_scales[c+1][*sca++];
				else *(f_l++) = 0;

				c=*al++;
				if (c) *f_r++ = (float) ((int) m_Bs.GetBits(c+1) + 1 - (1<<c)) * m_scales[c+1][*sca++];
				else *(f_r++) = 0;
			}
			l=32-js_top;
			while (l--)
			{
				int c=*al++;
				if (c) 
				{
					float samp = (float) ((int)m_Bs.GetBits(c+1) + 1 - (1<<c));
					*f_l++ = samp * m_scales[c+1][*sca++];
					*f_r++ = samp * m_scales[c+1][*sca++];
				}
				else *f_l++ = *f_r++ = 0.0;
			}
		}
		if (m_Downmix)
		{
			int x;
			for (x = 0; x < 12*32; x ++)
				m_PolySpectrum[0][0][x] = (m_PolySpectrum[0][0][x]+m_PolySpectrum[1][0][x])*0.5f;
		}
	} 
	else 
	{
		int i;
		unsigned int *al, *sca, scale_index[32], allocation[32];
		al = allocation; 
		i=32; 
		while (i--) *al++ = m_Bs.GetBits(4);

		al = allocation; 
		sca=scale_index; 
		i=32; 
		while (i--) if ((*al++)) *sca++ = m_Bs.GetBits(6);

		float *f = &m_PolySpectrum[0][0][0];
		for (i = 0; i < 12; i ++)
		{
			unsigned int *s = scale_index;
			unsigned int *a = allocation;
			int l=32;
			while (l--)
			{
				int c=*a++;
				if (c) *f++ = (float) ((int)m_Bs.GetBits(c+1) + 1 - (1<<c)) * m_scales[c+1][*s++];
				else *(f++) = 0;
			}
		}
	}

	if (hooks)
	{
		hooks->layer2_eq(&m_PolySpectrum[0][0][0],m_Downmix ? 1 : nChannels,m_Bs.GetHdr()->GetSampleRate(),12);
	}

	m_Polyphase.Apply(m_PolySpectrum, (float *)pPcm, 12);

	return SSC_OK;
}



//-------------------------------------------------------------------------*
//   ZeroPolySpectrum
//-------------------------------------------------------------------------*

void CMp2Decode::ZeroPolySpectrum()
{
	int ch, ss, sb;

	// reset spectrum to zero
	for ( ch=0; ch<2; ch++ )
		for ( ss=0; ss<SSLIMIT; ss++ )
			for ( sb=0; sb<SBLIMIT; sb++ )
				m_PolySpectrum[ch][ss][sb] = 0.0;
}

//-------------------------------------------------------------------------*
//   SetInfo
//-------------------------------------------------------------------------*

void CMp2Decode::SetInfo()
{
	static const int fhgVTab[] = {1, 0, 2};

	const CMpegHeader *hdr = m_Bs.GetHdr();

	m_Info.stereo             = hdr->GetChannels();
	m_Info.sample_rate_ndx    = hdr->GetSampleRateNdx();
	m_Info.frame_bits         = hdr->GetFrameLen();
	m_Info.mode               = hdr->GetMode();
	m_Info.mode_ext           = hdr->GetModeExt();
	m_Info.header_size        = hdr->GetHeaderLen();
	m_Info.IsMpeg1            = hdr->GetMpegVersion()==0 ? true:false;
	m_Info.fhgVersion         = fhgVTab[hdr->GetMpegVersion()];
	m_Info.protection         = hdr->GetCrcCheck();
}

/*-------------------------------------------------------------------------*/

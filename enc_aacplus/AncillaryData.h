#ifndef NULLSOFT_ENC_AACPLUS_ANCILLARYDATAH
#define NULLSOFT_ENC_AACPLUS_ANCILLARYDATAH

#define PRE_PADDING_MAGIC_WORD 'lluN'
#define POST_PADDING_MAGIC_WORD 'tfos'

#pragma pack(push, 1)
struct AncillaryData
{
	int magicWord; // set to 'lluN' for pre-delay, 'tfos' for post-delay 
	unsigned short padding;
};
#pragma pack(pop)


#endif
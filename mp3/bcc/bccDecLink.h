/***************************************************************************\
*                   Fraunhofer IIS MP3s
*              (c) 1996 - 2006 Fraunhofer IIS
*                   All Rights Reserved.
*
*
*
*   This software and/or program is protected by copyright law and
*   international treaties. Any reproduction or distribution of this
*   software and/or program, or any portion of it, may result in severe
*   civil and criminal penalties, and will be prosecuted to the maximum
*   extent possible under law.
*
***************************************************************************/

#ifndef _MP3SDECLINK_H_
#define _MP3SDECLINK_H_

#ifdef __cplusplus
extern "C"
{
#endif




  typedef enum _SA_DEC_MODE
{
	SA_DEC_OFF      = 0x0001,
	SA_DEC_BYPASS   = 0x0002,
	SA_DEC_CONCEIL  = 0x0004,
	SA_DEC_2_TO_51  = 0x0008,
	SA_DEC_1_TO_51  = 0x0010,

#ifdef MP3S_FOUR_CHANNEL
  SA_DEC_2_TO_40  = 0x0020,
#endif

} SA_DEC_MODE;


typedef enum _SA_DEC_ERROR
{
	SA_DEC_NO_ERROR,
	SA_DEC_MEMORY_ERROR,
	SA_DEC_DECODER_ERROR,
	SA_DEC_NO_SA_INFORMATION
} SA_DEC_ERROR;


/* structure containing SA Decoder Info */

typedef struct SA_DEC_INFO {
	int SampleRate;
	int nChannelsOut;
	SA_DEC_MODE usedMode;
	SA_DEC_MODE configuredMode;
} SA_DEC_INFO;


 typedef struct SADEC* SADEC_HANDLE;



/* this function creates a valid SA Decoder Handle */

SA_DEC_ERROR IIS_SADec_Init( SADEC_HANDLE* phSADec, int maxOutChannels );

/* creates SADecInfo */

SA_DEC_ERROR IIS_SADec_InitInfo(SADEC_HANDLE hSADec, int samplingRate, int nChannelsIn);


SA_DEC_INFO IIS_SADec_GetInfo( SADEC_HANDLE );

/* decodes anc data, fills hSADec bitstream */

SA_DEC_ERROR IIS_SADec_DecodeAncData( SADEC_HANDLE hSADec,unsigned char* pDataIn,
                                      unsigned int nDataInLength,
                                      unsigned char* pAncBytesNotUsed,
                                      int* numAncBytesNotUsed);



SA_DEC_ERROR IIS_SADec_DecodeFrame(SADEC_HANDLE hSADec,
					  float* pInBuffer,   int InSamples,
					  char*  pAncBytes,   int nAncBytes,
					  float* pOutBuffer,  int OutBufferSize,
					  int*   pOutSamples, int operatingMode,
            int totalAttenuation, int lfeAttenuation,
            unsigned char* pAncBytesNotUsed,
            int* numAncBytesNotUsed );


SA_DEC_ERROR IIS_SADec_Free(SADEC_HANDLE* phSADec);

SA_DEC_ERROR IIS_SADec_Reset(SADEC_HANDLE hSADec);


SA_DEC_ERROR IIS_SADec_SetBackChannelDelay(SADEC_HANDLE hSADec, int LsDelay, int RsDelay);

int IIS_SADec_GetSXFlagInfo(SADEC_HANDLE hSADec);


#ifdef __cplusplus
}
#endif

#endif

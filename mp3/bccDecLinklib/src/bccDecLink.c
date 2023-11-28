/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1996 - 2008 Fraunhofer IIS
 *                       All Rights Reserved.
 *
 *
 *    This software and/or program is protected by copyright law and
 *    international treaties. Any reproduction or distribution of this
 *    software and/or program, or any portion of it, may result in severe
 *    civil and criminal penalties, and will be prosecuted to the maximum
 *    extent possible under law.
 *
\***************************************************************************/

#include <assert.h>
#include "mod_buf.h"
#include "bccDecLink.h"
#include "getstream.h"
#include "bcc_dec_api.h"
#include "bcc_nlc_dec.h"
#include "crc_12.h"

#define MAX_SA_BYTES 200
#define MAX_FRAMELEN  2000
#define MAX_NUM_BCC_DATA 8



typedef enum
{
  INDIVIDUAL=0,
  MP3S=1
#ifndef NO_MP3S_ADM
  ,MP3S_ADM=2
#endif
} BCC_CONFIG_ID;

typedef enum
{
  RESERVED=0,
  ICLD=1,
  INFO=2,
  ICC=3,
  TNS=4
} BCC_DATA_ID;


static SA_DEC_ERROR SADec_GetConfig(SADEC_HANDLE hSADec,unsigned int samplingRate,int *sameConfig);

static int  SADec_Process(SADEC_HANDLE hSADec,
                          float* pDataIn, float* pDataOut,
                          int totalAttenuation, int lfeAttenuation);

static void ChannelMap(float* pDataIn, float* pDataOut,
             int nChannelsIn, int nChannelsOut, int SamplesPerChannel );

static void ParseInfoChunk(SADEC_HANDLE hSADec);

#ifndef NO_MP3S_ADM
static const int defaultgainIdx = 13;
static float calcUpmixGain( int  upmixGainIdx);
#endif

typedef struct BCC_CHUNK_DATA
{
  BCC_DATA_ID bccDataID;
  int bccDataLenBitsPresent;
  int bccDataOptional;
  int bccDataLenBits;

  int bccNeedsReset;

} BCC_CHUNK_DATA;


/* structure containing SA Decoder Data */

struct SADEC {
  BCC_dparams      bccParams;
  BCC_dstate       bccState;
  Streamin_state       stream;
  int                  bitstreamformat;
  SA_DEC_MODE          configuredMode;
  SA_DEC_MODE          usedMode;
  int                  SampleRate;
  int                  FrameLen;
  int                  nChannelsIn;     /* Number of input channels */
  int                  nChannelsOut;    /* Number of bcc channels */
  int                  nChannelsOutMax; /* Max number of output channels given by the user */
  int                  bFade;
  /*
    bcc config
  */
  int codecToBccConfigAlignment;
  int frameToBlockSync;
  BCC_CONFIG_ID bccConfigID;
  int numBccData;
  int numBccBlocks;
  int numBccDataMand;
  int optBccDataPresent;

  BCC_CHUNK_DATA chunkData[MAX_NUM_BCC_DATA];

#ifndef NO_MP3S_ADM
  float gain;
#endif
  HANDLE_MODULO_BUFFER hModuloBuffer;

};


SA_DEC_ERROR IIS_SADec_Init( SADEC_HANDLE* phSADec, int maxOutChannels)
{
  int bccData = 0;

  SADEC_HANDLE hSADec = NULL;

  hSADec = (SADEC_HANDLE)calloc(1,sizeof(struct SADEC));

  if(!hSADec)
  {
    *phSADec = NULL;
    return SA_DEC_MEMORY_ERROR;
    }

  *phSADec = hSADec;

  hSADec->configuredMode = SA_DEC_OFF;
  hSADec->nChannelsOutMax = maxOutChannels;
  hSADec->hModuloBuffer  = NULL;
#ifndef NO_MP3S_ADM
  hSADec->gain  = 0.63f;
#endif


  /* Init BCC data info chunk */

  for ( bccData = 0; bccData < MAX_NUM_BCC_DATA; ++bccData )
    hSADec->chunkData[bccData].bccDataID = RESERVED;

  initinstream(&hSADec->stream, NULL);


  return SA_DEC_NO_ERROR;
}


SA_DEC_ERROR IIS_SADec_InitInfo(SADEC_HANDLE hSADec, int samplingRate, int nChannelsIn)
{
  SA_DEC_ERROR error = SA_DEC_NO_ERROR;
  int useSameBccConfig,sameConfig;

  hSADec->nChannelsIn     = nChannelsIn;
  hSADec->SampleRate      = samplingRate;


  /*
    check if payload is available
  */

  if(!hSADec->stream.dataend-hSADec->stream.dataptr){
    return(SA_DEC_NO_SA_INFORMATION);
  }


  useSameBccConfig = getbits(&hSADec->stream,1);
  if(useSameBccConfig)
    return(SA_DEC_NO_ERROR);




  /* Get configuration from SA-Payload */
  error = SADec_GetConfig(hSADec, samplingRate, &sameConfig );

  if((error == SA_DEC_NO_ERROR) && sameConfig)
    return(SA_DEC_NO_ERROR);




  hSADec->configuredMode  = SA_DEC_BYPASS;
  hSADec->usedMode        = SA_DEC_BYPASS;
  hSADec->bFade           = 0;

  if(error != SA_DEC_NO_ERROR)
      return(error);


  /* init sa Decoder */
  {
  int    npbchan     =  0;
  int    refchan     =  0;
  int    lfchan      =  3;
  int    seplevels   =  0;
  int    cuelevels   =  17;
  int    wintype     =  1;
  int    corlevels   =  4;
  float  partwidth   =  2;
  float  levrange    = 24;
  int    srate       = hSADec->SampleRate ;
  float  delrange    = 400.f * srate * 1e-6f;
  int    nsepsources =  0;
  int    cuebits     =  3;
  int    corbits     =  2;
  int    nosmooth    =  0;
  char   tsmooth     =  1;
  char   hybrid      =  2;
  int    nhpart      =  0;
  float  ildscale    = 1.0f;
  float  ild2itd     = ILD2ITD_OFF;
  int    winshift     = hSADec->FrameLen;
  int    framesize    = hSADec->FrameLen;
  int    fftsize      = 1024;
  int    winspan      = 1024;
  /* int    aot_framelen = 2*hSADec->FrameLen; */


  if ( hSADec->nChannelsIn == 2 && hSADec->nChannelsOut == 6)
  {
    /* 2 to 5.1 mode */
    hSADec->bitstreamformat = ILD;
    npbchan = 6;
  }

  if ( hSADec->nChannelsIn == 1 && hSADec->nChannelsOut == 6)
  {
    /* 1 to 5.1 mode */
    /* currently not supported */
    hSADec->bitstreamformat = ILD;
    npbchan = 5;
  }


  if( bcc_dinit(
            hSADec->bitstreamformat,
            ildscale,
            ild2itd,
            cuelevels,
            cuebits,
            corlevels,
            corbits,
            seplevels,   /* ??? */
            levrange,
            delrange,
            npbchan,
            nsepsources,
            refchan,
            lfchan,
            srate,
            hSADec->numBccBlocks,
            framesize,
            fftsize,
            winspan,
            winshift,
            wintype,
            partwidth,
            nosmooth,
            tsmooth,
            hybrid,
            nhpart,
            &hSADec->bccParams,
            &hSADec->bccState) )
      return SA_DEC_NO_ERROR;


  if ( bcc_dinitdecoder(&hSADec->stream, 1, &hSADec->bccParams) )
  {
    bcc_ddone( &(hSADec)->bccParams, &(hSADec)->bccState);
    return SA_DEC_NO_ERROR;
  }

  }

  if ( !hSADec->hModuloBuffer )
  {
    hSADec->hModuloBuffer = CreateFloatModuloBuffer(( hSADec->FrameLen + MAX_FRAMELEN ) * hSADec->nChannelsIn);
    ZeroFloatModuloBuffer ( hSADec->hModuloBuffer, hSADec->FrameLen * hSADec->nChannelsIn  );
  }


  /* everything worked fine, so set the configuredMode */

  if ( hSADec->nChannelsIn == 2 && hSADec->nChannelsOut == 6)
    hSADec->configuredMode = SA_DEC_2_TO_51;

  if ( hSADec->nChannelsIn == 1 && hSADec->nChannelsOut == 6)
    hSADec->configuredMode = SA_DEC_1_TO_51;


  /* Parse info chunk if present */
  ParseInfoChunk( hSADec );


  hSADec->usedMode = SA_DEC_BYPASS;

  return SA_DEC_NO_ERROR;


}

SA_DEC_INFO IIS_SADec_GetInfo( SADEC_HANDLE hSADec)
{
  SA_DEC_INFO info;
  info.SampleRate     = (hSADec != NULL ) ? hSADec->SampleRate      : 0;
  info.nChannelsOut   = (hSADec != NULL ) ? hSADec->nChannelsOutMax : 0;
  info.usedMode       = (hSADec != NULL ) ? hSADec->usedMode        : 0;
  info.configuredMode = (hSADec != NULL ) ? hSADec->configuredMode  : 0;
  return info;
}


SA_DEC_ERROR IIS_SADec_DecodeFrame(SADEC_HANDLE hSADec,
            float* pInBuffer,   int InSamples,
            char*  pAncBytes,   int nAncBytes,
            float* pOutBuffer,  int OutBufferSize,
            int*   pOutSamples, int operatingMode,
            int totalAttenuation, int lfeAttenuation,
            unsigned char* pAncBytesLeft,
            int* numAncBytesLeft)

{


  SA_DEC_ERROR error = SA_DEC_NO_ERROR;


  if (hSADec == NULL)
    return SA_DEC_DECODER_ERROR;

  *pOutSamples = InSamples/hSADec->nChannelsIn * hSADec->nChannelsOutMax;

  if ( OutBufferSize < *pOutSamples )
    {
      *pOutSamples = 0;
      return SA_DEC_MEMORY_ERROR;
    }


  /* If bcc decoder will create more output channels than the user wants we cannot process bcc */
  if (hSADec->nChannelsOut > hSADec->nChannelsOutMax )
    operatingMode = SA_DEC_BYPASS;


  /* decode anc Data */
  error = IIS_SADec_DecodeAncData(hSADec, pAncBytes, nAncBytes, pAncBytesLeft, numAncBytesLeft);


  if ( error != SA_DEC_NO_ERROR ) /* error decoding anc data */
  {
    if (!(hSADec->configuredMode & ( SA_DEC_BYPASS | SA_DEC_OFF )) ) /* sa was configured so conceil */
    {
      AddFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
      ReadFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
      ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
             InSamples/hSADec->nChannelsIn );

      hSADec->usedMode = SA_DEC_CONCEIL;
    }
    else /* conceilment not possible */
    {
      ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
           InSamples/hSADec->nChannelsIn );

      hSADec->usedMode = SA_DEC_BYPASS;
    }

    hSADec->bFade = 1;

    return SA_DEC_NO_ERROR;
  }

  /*
    read and process bcc config
  */



  error=IIS_SADec_InitInfo( hSADec,  hSADec->SampleRate, hSADec->nChannelsIn);


  if ( hSADec->configuredMode & ( SA_DEC_BYPASS | SA_DEC_OFF )) /* sa still not configured*/
  {
    /* conceilment not possible */
    ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
           InSamples/hSADec->nChannelsIn );

    hSADec->usedMode = hSADec->configuredMode;
    hSADec->bFade    = 1;
  }
  else /* sa configured */
  {
    AddFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );

    if ( operatingMode != SA_DEC_BYPASS) /* user wants sa */
    {

      if(hSADec->frameToBlockSync)
	  {
        if(SADec_Process(hSADec, pInBuffer, pOutBuffer,
			 totalAttenuation, lfeAttenuation))
		{
			/* decoding error, so conceil */
			ReadFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
			ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
					   InSamples/hSADec->nChannelsIn );

			hSADec->usedMode = SA_DEC_CONCEIL;
			hSADec->bFade    = 1;

			return SA_DEC_NO_ERROR;

			/* return(SA_DEC_DECODER_ERROR); */
		}
	  }
      else /* unsupported, conceil */
	  {
		ReadFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
		ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
				   InSamples/hSADec->nChannelsIn );

		hSADec->usedMode = SA_DEC_CONCEIL;
		hSADec->bFade    = 1;

		return SA_DEC_NO_ERROR;

                /* return(SA_DEC_DECODER_ERROR); */ /* unsupported */
	  }


      ReadFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
      if ( (hSADec->usedMode == SA_DEC_BYPASS ) || ( hSADec->bFade == 1) ) /* sa dec processing succeeded */
      {
        ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
               InSamples/hSADec->nChannelsIn );

        hSADec->usedMode = SA_DEC_BYPASS;
        hSADec->bFade    = 0;

      }

    }
    else /* bypass, because user wants it */
    {
      ReadFloatModuloBufferValues( hSADec->hModuloBuffer, pInBuffer, InSamples );
      ChannelMap(pInBuffer, pOutBuffer, hSADec->nChannelsIn, hSADec->nChannelsOutMax,
             InSamples/hSADec->nChannelsIn );

      hSADec->usedMode = SA_DEC_BYPASS;
      hSADec->bFade    = 1;
    }
  }



  return SA_DEC_NO_ERROR;
}


SA_DEC_ERROR IIS_SADec_DecodeAncData(SADEC_HANDLE hSADec,
                                     unsigned char* pDataIn,
                                     unsigned int nDataInLength,
                                     unsigned char* pAncBytesLeft,
                                     int* numAncBytesLeft)
{
  unsigned int i=0;
  unsigned short SYNCWORD = 0xCF30;


  if ( (nDataInLength == 0) || (nDataInLength <= 4) )
    {
#ifndef NDEBUG
      printf("Warning: Ancillary data length to small %d\n", nDataInLength);
#endif

      return SA_DEC_NO_SA_INFORMATION;
    }


  for (i=0; i < (nDataInLength-4); i++)
    {
      if ( (pDataIn[i] != ((SYNCWORD>>8)&0xff)) ||
           ((pDataIn[i+1]&0xf0) != (SYNCWORD&0x00f0)) )
        {
          continue;
        }
      else
        {
          unsigned short crcTrans = 0, crcCalc = 0;

          /*
           * get bccChunkLength in bytes
           */

          int nLength = 0;
          nLength = (pDataIn[i+1]<<4)&0xf0;
          nLength |= ((pDataIn[i+2]>>4)&0x00ff);

          /*
           * crc check
           */
          if ( (i+nLength) <= nDataInLength ) /* avoid reading over array */
            {
              crcCalc = Crc_12(&pDataIn[i+4],nLength-4);
            }
          else
            {
              continue;
            }

          crcTrans = (pDataIn[i+2]&0x0f)<<8;
          crcTrans |= pDataIn[i+3];

          /*
           * transmitted crc and calculated crc are equal ?
           */

          if ( crcTrans != crcCalc )
            {
              /* crc error */
#ifndef NDEBUG
            printf("Warning: Ancillary data crc check failed %04x != %04x\n",crcTrans,crcCalc );
#endif

            continue;
            }

          /*
           *  copy decoded bccData to output buffer
           */
          initinstream(&hSADec->stream, NULL);
          copyinbuf(&hSADec->stream, &pDataIn[i+4],(nLength-4));


          /*
           *  All other data in pDataIn is copied together in pAncBytesLeft buffer
           */
          if( (pAncBytesLeft!=NULL) && (numAncBytesLeft!=NULL) ) {
            /* All ancData before BCC */
            unsigned char* tmp  = pAncBytesLeft;
            unsigned char* tmp2 = pDataIn;
            memcpy(tmp, pDataIn, i );

            /* All ancData after BCC */
            tmp  += i;
            tmp2 += i + nLength;
            memcpy(tmp,tmp2, nDataInLength - ( nLength + i) );

            *numAncBytesLeft  = nDataInLength - nLength;
          }


          return SA_DEC_NO_ERROR;
        }
    }

  if( (pAncBytesLeft!=NULL) && (numAncBytesLeft!=NULL) ) {
    memcpy(pAncBytesLeft, pDataIn, nDataInLength);
    *numAncBytesLeft  = nDataInLength;
  }

  return SA_DEC_NO_SA_INFORMATION;
}

static SA_DEC_ERROR SADec_GetConfig( SADEC_HANDLE hSADec,unsigned int samplingRate, int *sameConfig)
{
  /* get version from SA-Data */


  int bccData = 0;
  int tmp = 0;
  int i = 0;

  BCC_CHUNK_DATA sActChunkData[MAX_NUM_BCC_DATA] ;


  *sameConfig=1;


  tmp = hSADec->codecToBccConfigAlignment;
  hSADec->codecToBccConfigAlignment = getbits(&hSADec->stream,1);

  /* Reset BCC if codecToBccConfigAlignment has changed */

  if(tmp != hSADec->codecToBccConfigAlignment)
    *sameConfig=0;

  if(hSADec->codecToBccConfigAlignment) {
    hSADec->bccConfigID=MP3S;
    hSADec->frameToBlockSync = 1;
  }
  else{
    tmp = hSADec->bccConfigID;
    hSADec->bccConfigID = getbits(&hSADec->stream,3);


    /* Reset BCC if ConfigID has changed */

    if(tmp != hSADec->bccConfigID)
      *sameConfig=0;

    hSADec->frameToBlockSync = getbits(&hSADec->stream,1);

    if( !hSADec->frameToBlockSync ) {
#ifndef NDEBUG
      printf("Warning: Unsynchronous SAC data transmission not supported, switching back to stereo\n");
#endif
      return SA_DEC_NO_SA_INFORMATION;
    }
  }


  switch(hSADec->bccConfigID )
  {
#ifndef NO_MP3S_ADM
    case MP3S_ADM: {
      int gainExp = defaultgainIdx;
      float gain;
      if( getbits(&hSADec->stream,1) ) {
        gainExp = getbits(&hSADec->stream,7);
      }
      gain    = calcUpmixGain(gainExp);
      if (gain != hSADec->gain) {
        hSADec->gain  = gain;
      }
    }
      /* fallthrough! */
#endif
    case MP3S:
      switch ( samplingRate )
      {
      case 44100:
      case 48000:
        hSADec->nChannelsOut  = 6;
        hSADec->SampleRate    = samplingRate;

#ifdef PCM_SURROUND_EMBEDDING
        hSADec->FrameLen      = 512;
        hSADec->numBccBlocks  = 1;
#else
        hSADec->FrameLen      = 576;
        hSADec->numBccBlocks  = 2;

#endif

        hSADec->numBccDataMand=1;
        sActChunkData[0].bccDataID = ICLD;

#ifndef NO_MP3S_ADM
	/* if() clause necessary because of
	   fallthrough in  MP3S_ADM */
	if(hSADec->bccConfigID==MP3S) {
	  hSADec->gain = 0.63f;
	}
#endif

        break;
      default:
#ifndef NDEBUG
        printf("Warning: Unsupported samplerate %d \n",samplingRate);
#endif

        return SA_DEC_NO_SA_INFORMATION;
      }
    break;
    default:
#ifndef NDEBUG
      printf("Warning: Reserved bccConfigID %d, skipping\n",hSADec->bccConfigID);
#endif
      return SA_DEC_NO_SA_INFORMATION;
  }

  /* Read mandatory chunks */

  for (bccData = 0; bccData < hSADec->numBccDataMand; bccData++)
    {
      sActChunkData[bccData].bccDataOptional = 0;        /* not optional, but mandatory */
      sActChunkData[bccData].bccDataLenBitsPresent = 0;  /* no len info */
      sActChunkData[bccData].bccDataLenBits = 0;

      /* Set bccNeedsReset to 1, if chunk requires a decoder reset !!!! */

      sActChunkData[bccData].bccNeedsReset = 0;
    }


  /* Check if we have optional chunks */

  hSADec->optBccDataPresent=getbits(&hSADec->stream,1);

  if(hSADec->optBccDataPresent)
    {
      /* Get number of all chunks */
      /* +1: First chunk is implicit signaled by optBccDataPresent */

      hSADec->numBccData = hSADec->numBccDataMand + getbits(&hSADec->stream,3) + 1;


      /* Loop over optional data */
      for (bccData = hSADec->numBccDataMand; bccData < hSADec->numBccData; bccData++)
        {
          sActChunkData[bccData].bccDataOptional = 1;

          /* Get chunk ID */

          sActChunkData[bccData].bccDataID = getbits(&hSADec->stream,6);


          /* Set bccNeedsReset to 1, if chunk requires a decoder reset !!!!   */
          /* actually we do not need a sophisticated case-management here, as */
          /* we do not have optional chunks that require a reset, yet...      */

          sActChunkData[bccData].bccNeedsReset = 0;


          /* Last chunk does not have a len info */

          if ( bccData != hSADec->numBccData - 1 )
            {
              /* Get len bit present */
              sActChunkData[bccData].bccDataLenBitsPresent = getbits(&hSADec->stream,1);

              /* If len bit present -> read len */
              if ( sActChunkData[bccData].bccDataLenBitsPresent )
                sActChunkData[bccData].bccDataLenBits = getbits(&hSADec->stream,4)+1;

            }
          else
            sActChunkData[bccData].bccDataLenBitsPresent = 0;

        }
    }
  else
    hSADec->numBccData = hSADec->numBccDataMand;


  /* Check if any of the new chunks requires a decoder reset */
  /* but only, if sameConfig was not already set to zero by  */
  /* a previous header change                                */

  if ( 1 == *sameConfig )
  {

  for ( bccData = 0; bccData < hSADec->numBccData; ++bccData )
    {
      if ( sActChunkData[bccData].bccNeedsReset )
        {
          *sameConfig = 0;
          for ( i = 0; i < MAX_NUM_BCC_DATA; ++i )
            {
              /* chunk was present last frame -> reset was done last frame -> no reset -> sameConfig */
              if ( hSADec->chunkData[i].bccDataID == sActChunkData[bccData].bccDataID )
                {
                  *sameConfig = 1;
                  break;
                }

            }

          if ( *sameConfig == 0 ) /* We definitely need a reset */
            break;

        }

    }
  }

  /* Initialize rest of struct with RESERVED  */

  for ( bccData = hSADec->numBccData; bccData < MAX_NUM_BCC_DATA; ++bccData )
    sActChunkData[bccData].bccDataID = RESERVED;



  /* Copy struct to hSADec Handle */

  memcpy( &hSADec->chunkData, &sActChunkData, sizeof( BCC_CHUNK_DATA ) * MAX_NUM_BCC_DATA );



  return SA_DEC_NO_ERROR;

}



static int SADec_Process( SADEC_HANDLE hSADec,
                          float* pDataIn,
                          float* pDataOut,
                          int totalAttenuation,
                          int lfeAttenuation )
{

  int i, bccData;

  int prevDataNoLenBitsPresent = 0;
  int icldDataDecoded = 0;
  float upmixGain = hSADec->gain;


  hSADec->usedMode = SA_DEC_BYPASS;
  hSADec->bccParams.infoChunk.sxFlag = -1;

  if( (0!=totalAttenuation) && (MP3S==hSADec->bccConfigID) ) {
    upmixGain = 1.f;
  }

  if ((hSADec->configuredMode == SA_DEC_BYPASS) || !(hSADec->stream.dataend-hSADec->stream.dataptr))
    return 1;

  /*
    at the moment, numBccData must be one
  */
  /* assert(hSADec->numBccData == 1); */

  for( bccData=0; bccData<hSADec->numBccData; bccData++ ) {

    int bccDataLen = 0, skipBits = 0;

    /* read optional data */

    if( hSADec->chunkData[bccData].bccDataOptional ) {
      if( hSADec->chunkData[bccData].bccDataLenBitsPresent ) {
        bccDataLen = getbits(&hSADec->stream,hSADec->chunkData[bccData].bccDataLenBits);
      }
    }

    switch(hSADec->chunkData[bccData].bccDataID) {
    case ICLD:
      {

        /*
          documentation not up to date !!!

        int codingSheme,resolutionEnhancement;

        codingSheme = getbits(&hSADec->stream,1);
        resolutionEnhancement = getbits(&hSADec->stream,1);
        */


        if( icldDataDecoded ) {
#ifndef NDEBUG
          printf("Warning: Superfluous ICLD data chunk found, skipping\n");
#endif
          if( hSADec->chunkData[bccData].bccDataLenBitsPresent && !prevDataNoLenBitsPresent ) {
            for( skipBits=0; skipBits<bccDataLen; skipBits++ ) {
              getbits(&hSADec->stream,1);
            }
          }
          else {
            prevDataNoLenBitsPresent = 1;
          }
          break;
        }


        if(bcc_cues_nldec( &(hSADec->stream),
                        &(hSADec->bccParams),
                        &(hSADec->bccState)) != BCC_OK)
          return 1;


        for ( i = 0; i < hSADec->numBccBlocks; ++i ){
          int c;
          /*
            transfer IO channel active flags for current granule
          */
          for(c=0;c<hSADec->bccParams.npbchan;c++)
            hSADec->bccState.chActive[0][c] = (float)hSADec->bccState.ioChanActive[i][c];


          if( bcc_sideinfo_dec(&(hSADec->stream),
                             hSADec->bitstreamformat,
                             i,
                             NULL,
                             NULL,
                             &(hSADec->bccParams),
                             &(hSADec->bccState)) != BCC_OK)
                   return 1;

          if( bcc_dprocessframe(&(hSADec->bccParams),
                              &(hSADec->bccState),
                              (void*) &pDataIn[hSADec->nChannelsIn  * hSADec->FrameLen * i],
                              (void*) &pDataOut[hSADec->nChannelsOut* hSADec->FrameLen * i],
#ifndef NO_MP3S_ADM
				upmixGain,
#endif
                              1,  /* interleaved */
                              1,  /* float */
                              1) != BCC_OK) /* float */
                  return 1;
        } /* numBccBlocks*/

        icldDataDecoded = 1;
      }
      break;

	case INFO:
		{
			int num_info_bits = -1;

			if( hSADec->chunkData[bccData].bccDataLenBitsPresent ) {
				num_info_bits = bccDataLen;

				if( num_info_bits == 0 ) break;
			}
			else {
				num_info_bits = 8;
			}

			if( !prevDataNoLenBitsPresent ) {
				if( bcc_info_dec(&(hSADec->stream),
					&(hSADec->bccParams),
					num_info_bits) != BCC_OK ) return 1;

			}
			else {
#ifndef NDEBUG
				fprintf( stderr, "\nUnable to decode InfoData chunk due to missing length information of previously occuring optional chunk!\n" );
#endif
			}
		}
		break;





    default:
      {
        if( hSADec->chunkData[bccData].bccDataOptional ) {
#ifndef NDEBUG
          /* printf("Warning: Reserved bccDataID %d, skipping\n",hSADec->bccDataID[bccData]); */
#endif
          if( hSADec->chunkData[bccData].bccDataLenBitsPresent && !prevDataNoLenBitsPresent ) {
            for( skipBits=0; skipBits<bccDataLen; skipBits++ ) {
              getbits(&hSADec->stream,1);
            }
          }
          else {
            prevDataNoLenBitsPresent = 1;
          }
        }
        else {

#ifndef NDEBUG
          printf("Warning: Unsupported mandatory data chunk, bccDataID %d\n",hSADec->chunkData[bccData].bccDataID);
#endif
          return 1;/* unsupported */
        }
      }

      break;
    }

  }


  if( !icldDataDecoded ) {
#ifndef NDEBUG
    printf("Warning: No ICLD data chunk found\n");
#endif
    return 1;
  }


  if( !prevDataNoLenBitsPresent ) {
    if( !bytealign(&(hSADec->stream)) ) {
#ifndef NDEBUG
      printf("Warning: Byte alignment bits unequal zero\n");
#endif
      return 1;
    }
  }


#if 0
  if( totalAttenuation ) {
    /* scale for identical loudness of stereo and multichannel output */
    for(i = 0;i < hSADec->nChannelsOut * hSADec->bccParams.framesize * hSADec->numBccBlocks;i++)
          pDataOut[i]*= upmixGain; /*0.635f;*/
  }
#endif

  if ( lfeAttenuation )
  {
    float lfe_attenuation = (float) pow(10.0,(1.0/2.0));
    for(i = 3;i < hSADec->nChannelsOut * hSADec->bccParams.framesize * hSADec->numBccBlocks;
        i = i +  hSADec->nChannelsOut)
      pDataOut[i]/= lfe_attenuation;
  }

  if ( hSADec->nChannelsIn == 2 && hSADec->nChannelsOut == 6 )
    hSADec->usedMode = SA_DEC_2_TO_51;

  if ( hSADec->nChannelsIn == 1 && hSADec->nChannelsOut == 6 )
    hSADec->usedMode = SA_DEC_1_TO_51;

  return 0;
}


SA_DEC_ERROR IIS_SADec_Free(SADEC_HANDLE* phSADec)
{
  if(*phSADec)
  {
    if (!((*phSADec)->configuredMode & ( SA_DEC_BYPASS | SA_DEC_OFF )) )
      bcc_ddone( &((*phSADec)->bccParams), &((*phSADec)->bccState));

    if ( (*phSADec)->hModuloBuffer )
      DeleteFloatModuloBuffer( (*phSADec)->hModuloBuffer);

    free(*phSADec);
    *phSADec = 0;
     return SA_DEC_NO_ERROR;
   }
  else
    return SA_DEC_DECODER_ERROR;
}


SA_DEC_ERROR IIS_SADec_Reset(SADEC_HANDLE hSADec)
{
	if(hSADec)
	{
		hSADec->bFade       = 1;

    /* Reset Modulo Buffer */
    if ( hSADec->hModuloBuffer )
      {
        ResetFloatModuloBuffer( hSADec->hModuloBuffer );
        ZeroFloatModuloBuffer ( hSADec->hModuloBuffer, hSADec->FrameLen * hSADec->nChannelsIn  );
      }
 
    bcc_dreset(&(hSADec->bccParams), &(hSADec->bccState));

    return SA_DEC_NO_ERROR;
	}
	else
		return SA_DEC_MEMORY_ERROR;
}



static void ChannelMap(float* pDataIn, float* pDataOut, int nChannelsIn, int nChannelsOut, int SamplesPerChannel )
{
  int sample = 0;
  int chIn1  = 0;
  int chIn2  = 0;

  if ( nChannelsIn == nChannelsOut )
    memcpy( pDataOut, pDataIn, nChannelsOut * SamplesPerChannel * sizeof(float));
  else
    for( sample = 0; sample < SamplesPerChannel; ++sample)
    {
      for ( chIn1 = 0; chIn1 < nChannelsIn; ++chIn1 )
        pDataOut[nChannelsOut*sample + chIn1] = pDataIn[nChannelsIn*sample + chIn1];

      for ( chIn2 = chIn1; chIn2 < nChannelsOut; ++chIn2 )
        pDataOut[nChannelsOut*sample + chIn2]=0;
    }
}

#ifndef NO_MP3S_ADM
static float calcUpmixGain( int  upmixGainIdx)
{
  float upmixGain = 1.0f;

  if( 8 == upmixGainIdx) {
    upmixGain = 0.63f;
  }
  else if (upmixGainIdx <= 92) {
    upmixGain = pow( 2.0f, -(12.f-(float)upmixGainIdx)/6.f );
  }
  else {
    upmixGain = pow( 2.0f, -(104.f-2.f*(float)upmixGainIdx)/6.f );
  }

  return upmixGain;
}
#endif


SA_DEC_ERROR IIS_SADec_SetBackChannelDelay(SADEC_HANDLE hSADec, int LsDelay, int RsDelay)
{
  if ( hSADec )
    bcc_set_backchannel_delay( &hSADec->bccParams, LsDelay, RsDelay);

  return SA_DEC_NO_ERROR;
}

int IIS_SADec_GetSXFlagInfo(SADEC_HANDLE hSADec)
{
	return hSADec->bccParams.infoChunk.sxFlag;
}


static void ParseInfoChunk(SADEC_HANDLE hSADec)
{
  int bccData = 0;
  int prevDataNoLenBitsPresent = 0;

  for( bccData=0; bccData<hSADec->numBccData; bccData++ )
  {

    int bccDataLen = 0, skipBits = 0;

    /* read optional data */

    if( hSADec->chunkData[bccData].bccDataOptional )
    {
      if( hSADec->chunkData[bccData].bccDataLenBitsPresent )
      {
        bccDataLen = getbits(&hSADec->stream,hSADec->chunkData[bccData].bccDataLenBits);
      }
    }

    switch(hSADec->chunkData[bccData].bccDataID)
    {
    case INFO:
      {
        int num_info_bits = -1;

        if( hSADec->chunkData[bccData].bccDataLenBitsPresent )
        {
          num_info_bits = bccDataLen;
          if( num_info_bits == 0 ) break;
        }
        else
        {
          num_info_bits = 8;
        }

        if( !prevDataNoLenBitsPresent )
        {
          if( bcc_info_dec(&(hSADec->stream), &(hSADec->bccParams), num_info_bits) != BCC_OK )
            return;

        }
        else
        {
#ifndef NDEBUG
          fprintf( stderr, "\nUnable to decode InfoData chunk due to missing length information of previously occuring optional chunk!\n" );
#endif
        }
      }
      break;
    default:
      if( hSADec->chunkData[bccData].bccDataOptional )
      {
        if( hSADec->chunkData[bccData].bccDataLenBitsPresent && !prevDataNoLenBitsPresent )
        {
          for( skipBits=0; skipBits<bccDataLen; skipBits++ )
            getbits(&hSADec->stream,1);
        }
        else
          prevDataNoLenBitsPresent = 1;

      }
      break;
    }
  }
}

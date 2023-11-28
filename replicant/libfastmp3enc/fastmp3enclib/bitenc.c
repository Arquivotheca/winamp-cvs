/***********************************************************************************************
*                                   MPEG Layer3-Audio Encoder                                  *
*                                                                                              *
*                                 © 1999-2005 by Fraunhofer IIS                                *
*                                       All Rights Reserved                                    *
*                                                                                              *
*   This software and/or program is protected by copyright law and international treaties.     *
*   Any reproduction or distribution of this software and/or program, or any portion of it,    *
*   may result in severe civil and criminal penalties, and will be prosecuted to the           *
*   maximum extent possible under law.                                                         *
*                                                                                              *
*   $Id: bitenc.c,v 1.1 2007/05/29 16:02:27 audiodsp Exp $                             *
*   author:   M. Werner                                                                        *
*   contents/description: Bitstream encoder                                                    *
*                                                                                              *
************************************************************************************************/

#include "mconfig.h"
#include "mp3alloc.h"
#include <assert.h>

#include "bitenc.h"
#include "mp3bitbuf.h"
#include "mp3bitmux.h"
#include "mp3crc.h"
#include "psy_const.h"
#include "bit_cnt.h"
#include "dyn_bits.h"
#include "qc_data.h"
#include "interface.h"
#include "mathlib.h" /* provides min() and max() */
#include "utillib.h"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

struct BITSTREAM_ENCODER
{
  ALIGN_16_BYTE struct BITSTREAMENCODER_INIT init;

  int nChannels;
  int streamType ;
  int sampleRateIndex ;
  int nGranules ;
  int sideInfoBits ;
  int hdSiBits ;
  int headerBits ;
  int crcBits ;
  int channelMode ;

  int backPointer ;

  HANDLE_BIT_BUF hSiBitstream;
  HANDLE_BIT_BUF hDyBitstream;

  ALIGN_16_BYTE struct MUX_LIST muxList ;
  int ancDataBits;
  unsigned char* ancFhgBuf;
};

/* bitstream constants */

static const int MPEG_HEADER_BITS = 32;
static const int MPEG_CRC_BITS = 16;
static const int MPEG1_SI_SINGLE_BITS = 136;
static const int MPEG1_SI_DOUBLE_BITS = 256;
static const int MPEG2_SI_SINGLE_BITS =  72;
static const int MPEG2_SI_DOUBLE_BITS = 136;

static const int SI_BITSTREAM_SIZE =  MAX_MUX_FRAME * 304 ;/*304=(MPEG1_SI_DOUBLE_BITS+MPEG_HEADER_BITS+MPEG_CRC_BITS);*/
static const int DY_BITSTREAM_SIZE = 4096*2*2 ;

static const int SCF_BANDS = 4 ;

static const int emphasis        = 0;
static const int l3PrivateBits   = 0;

/** Globalgain offset incorporates the scaling for 16 bit/sample resolution. 
    Just remove the 60 when switching to float [-1:1] */


static const int globalGainOffset = 210 - PCM_CORRECTION;
static const unsigned short crcPoly         = 0x8005;
static const unsigned short crcPolyHighbit  = 0x8000;

int BSNew(struct BITSTREAM_ENCODER **phBS)
{
  struct BITSTREAM_ENCODER *hBS = (struct BITSTREAM_ENCODER *)mp3Alloc(sizeof(struct BITSTREAM_ENCODER));

  /*
    allocate si,dy and out bit buffer
   */

  if (hBS)
  {
    hBS->hSiBitstream = mp3CreateBitBuffer(SI_BITSTREAM_SIZE);
    hBS->hDyBitstream = mp3CreateBitBuffer(DY_BITSTREAM_SIZE);

    if (hBS->hSiBitstream == 0 ||
        hBS->hDyBitstream == 0)
    {
      BSDelete(hBS); hBS = 0;
    }
  }

  *phBS = hBS ;
  return (hBS == 0);
}

void BSDelete(struct BITSTREAM_ENCODER *hBS)
{
  if (hBS)
  {
    if (hBS->hSiBitstream) mp3DeleteBitBuffer(hBS->hSiBitstream);
    if (hBS->hDyBitstream) mp3DeleteBitBuffer(hBS->hDyBitstream);

    if (hBS->ancFhgBuf) {
      mp3Free(hBS->ancFhgBuf);
      hBS->ancFhgBuf = NULL;
    }

    mp3Free(hBS) ;
  }
}

int BSInit(struct BITSTREAM_ENCODER *hBS, const struct BITSTREAMENCODER_INIT *init)
{
  hBS->init = *init;

  if (init->sampleRate > 24000) {
    hBS->streamType = MPEG1;
    switch (init->sampleRate) {

#ifndef MP3_SURROUND

    case 32000:
      hBS->sampleRateIndex = 2;
      break;

#endif

    case 44100:
      hBS->sampleRateIndex = 0;
      break;
    case 48000:
      hBS->sampleRateIndex = 1;
      break;
    default:
      goto create_error;
    }
  } else if(init->sampleRate > 12000){
    hBS->streamType = MPEG2;
    switch (init->sampleRate) {

#ifndef MP3_SURROUND

    case 16000:
      hBS->sampleRateIndex = 2;
      break;
    case 22050:
      hBS->sampleRateIndex = 0;
      break;
    case 24000:
      hBS->sampleRateIndex = 1;
      break;

#endif

    default:
      goto create_error;
    }
  }else{
    hBS->streamType = MPEG2_5;
    switch (init->sampleRate) {

#ifndef MP3_SURROUND

    case 8000:
      hBS->sampleRateIndex = 2;
      break;
    case 11025:
      hBS->sampleRateIndex = 0;
      break;
    case 12000:
      hBS->sampleRateIndex = 1;
      break;

#endif

    default:
      goto create_error;
    }
  }
  
  hBS->headerBits = MPEG_HEADER_BITS;
  hBS->crcBits = init->protection ? MPEG_CRC_BITS : 0;
  hBS->channelMode = init->channelMode;
  switch (hBS->streamType) {
  case MPEG1:
    hBS->nGranules = 2;
    switch (init->channelMode) {
    case STEREO:
    case JOINT_STEREO:
    case DUAL_CHANNEL:
      hBS->sideInfoBits = MPEG1_SI_DOUBLE_BITS;
      hBS->nChannels = 2;
      break;
    case SINGLE_CHANNEL:
      hBS->sideInfoBits = MPEG1_SI_SINGLE_BITS;
      hBS->nChannels = 1;
      break;
    default:
      goto create_error;
    }
    break;
  case MPEG2:
  case MPEG2_5:
    hBS->nGranules = 1;
    switch (init->channelMode) {
    case STEREO:

    case JOINT_STEREO:
    case DUAL_CHANNEL:
      hBS->sideInfoBits = MPEG2_SI_DOUBLE_BITS;
      hBS->nChannels = 2;
      break;
    case SINGLE_CHANNEL:
      hBS->sideInfoBits = MPEG2_SI_SINGLE_BITS;
      hBS->nChannels = 1;
      break;
    default:
      goto create_error;
    }
    break;
  }

  hBS->hdSiBits       =
    hBS->headerBits   +
    hBS->crcBits      +
    hBS->sideInfoBits;

  mp3InitBitMux(&(hBS->muxList), hBS->hSiBitstream, hBS->hDyBitstream);

  hBS->backPointer = 0 ;

  hBS->ancDataBits = 0 ;

  if (init->ancMode == 1) {
    /* FhG ancillary mode */
    hBS->ancFhgBuf = (unsigned char*)mp3Calloc(256, sizeof(unsigned char));
  } 
  else {
    hBS->ancFhgBuf = NULL;
  }

  return 0;

create_error :
  return 1;
}

/*****************************************************************************

    functionname:encodeHeader  
    description:   
    returns:
    input:         
    output:        
    globals:       

*****************************************************************************/
static void encodeHeader (int            streamType,
                          int            crcProtection,
                          int            bitRateIndex,
                          int            sampleRateIndex,
                          int            channelMode,
                          int            paddingOn,
                          int            modeExtension,
                          int            privateBit,
                          int            copyRightBit,
                          int            originalCopyBit,
                          HANDLE_BIT_BUF hBitstream)
{
  /*
      save read pointer if crc protection is active
  */

  switch(streamType)
  {
    case MPEG1:
    case MPEG2:
      mp3WriteBits(hBitstream,0xfff,12);
    break;
    case MPEG2_5:
      mp3WriteBits(hBitstream,0xffe,12);
    break;
  }

  mp3WriteBits(hBitstream,streamType == MPEG1 ? 1:0,1);
  mp3WriteBits(hBitstream,1,2);     
  mp3WriteBits(hBitstream,crcProtection ? 0:1,1);
  mp3WriteBits(hBitstream,bitRateIndex,4);
  mp3WriteBits(hBitstream,sampleRateIndex,2);
  mp3WriteBits(hBitstream,paddingOn,1);
  mp3WriteBits(hBitstream,privateBit,1);
  mp3WriteBits(hBitstream,channelMode,2);
  mp3WriteBits(hBitstream,modeExtension,2);
  mp3WriteBits(hBitstream,copyRightBit,1);
  mp3WriteBits(hBitstream,originalCopyBit,1);
  mp3WriteBits(hBitstream,emphasis,2);

  /* leave crc bits blank. They will be calculated and filled in later. */
  if(crcProtection)
    mp3WriteBits(hBitstream,0,16);
}

/*****************************************************************************

    functionname:encodeSiHeader  
    description:   
    returns:
    input:         
    output:        
    globals:       

*****************************************************************************/
static void encodeSiHeader (int            streamType,
                            int            channelMode,
                            int            backPointer,
                            HANDLE_BIT_BUF hBitstream)
{
  switch(streamType){
  case MPEG1:
    assert(backPointer < 512);
    mp3WriteBits(hBitstream,backPointer,9);
    mp3WriteBits(hBitstream,l3PrivateBits,channelMode == SINGLE_CHANNEL ? 5:3);
    break;

  case MPEG2:
  case MPEG2_5:
    assert(backPointer < 256);
    mp3WriteBits(hBitstream,backPointer,8);
    mp3WriteBits(hBitstream,l3PrivateBits,channelMode == SINGLE_CHANNEL ? 1:2);
    break;
  }
}

/*****************************************************************************

    functionname:encodeSiData  
    description: encodes side info data for one granule  
    returns:
    input:         
    output:        
    globals:       

*****************************************************************************/
static void encodeSiData (int             streamType,
                          int             channel,
                          const struct QC_OUT  *qcOut,
                          HANDLE_BIT_BUF  hBitstream)
{
  assert(qcOut->part2_3Length[channel] < 4096);

  mp3WriteBits(hBitstream,qcOut->part2_3Length[channel],12);
  mp3WriteBits(hBitstream,qcOut->regionInfo[channel].bigValuePairs,9);
  mp3WriteBits(hBitstream,qcOut->globalGain[channel] + globalGainOffset,8);

  if(streamType == MPEG1)
    mp3WriteBits(hBitstream,qcOut->scfCompress[channel],4);
  else
    mp3WriteBits(hBitstream,qcOut->scfCompress[channel],9);

  if (qcOut->blockType[channel] == LONG_WINDOW)
  {
    mp3WriteBits(hBitstream,0,1); /* windowSwitchingFlag == false */

    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].tableSelect[0],5);
    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].tableSelect[1],5);
    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].tableSelect[2],5);

    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].regionCountMinus1[0],4);
    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].regionCountMinus1[1],3);
  }
  else
  {
    mp3WriteBits(hBitstream,1,1); /* windowSwitchingFlag == true */

    switch(qcOut->blockType[channel]){
      case START_WINDOW:
      case STOP_WINDOW:
      case SHORT_WINDOW:
        mp3WriteBits(hBitstream,qcOut->blockType[channel],2);
        mp3WriteBits(hBitstream,0,1);
      break;
    }

    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].tableSelect[0],5);
    mp3WriteBits(hBitstream,qcOut->regionInfo[channel].tableSelect[1],5);
 
    mp3WriteBits(hBitstream,qcOut->subblockGain[channel][0],3);
    mp3WriteBits(hBitstream,qcOut->subblockGain[channel][1],3);
    mp3WriteBits(hBitstream,qcOut->subblockGain[channel][2],3);
  }

  if(streamType == MPEG1)
    mp3WriteBits(hBitstream,qcOut->preEmphasisFlag[channel],1);

  mp3WriteBits(hBitstream,qcOut->scfScale[channel],1);
  mp3WriteBits(hBitstream,qcOut->regionInfo[channel].count1Table,1);

#ifdef PLOTMTV
  {
    /* 
     * debug stuff 
     */

    int temp = qcOut->globalGain[channel] + globalGainOffset ;

    sendDebout("bitenc",1,1,"part2_3Length",MTV_INT, &qcOut->part2_3Length[channel]);
    sendDebout("bitenc",1,1,"bigValuePairs",MTV_INT, &qcOut->regionInfo[channel].bigValuePairs);
    sendDebout("bitenc",1,1,"globalGain",MTV_INT,&temp);

    sendDebout("bitenc",1,1,"scfCompress",MTV_INT, &qcOut->scfCompress[channel]);

    sendDebout("bitenc",1,1,"blockType",MTV_INT, &qcOut->blockType[channel]);

    if (qcOut->blockType[channel] == LONG_WINDOW)
    {
      sendDebout("bitenc",1,1,"tableSelect0",MTV_INT, &qcOut->regionInfo[channel].tableSelect[0]);
      sendDebout("bitenc",1,1,"tableSelect1",MTV_INT, &qcOut->regionInfo[channel].tableSelect[1]);
      sendDebout("bitenc",1,1,"tableSelect2",MTV_INT, &qcOut->regionInfo[channel].tableSelect[2]);

      sendDebout("bitenc",1,1,"regionCountMinus1_0",MTV_INT, &qcOut->regionInfo[channel].regionCountMinus1[0]);
      sendDebout("bitenc",1,1,"regionCountMinus1_1",MTV_INT, &qcOut->regionInfo[channel].regionCountMinus1[1]);
    }
    else
    {
      sendDebout("bitenc",1,1,"tableSelect0",MTV_INT, &qcOut->regionInfo[channel].tableSelect[0]);
      sendDebout("bitenc",1,1,"tableSelect1",MTV_INT, &qcOut->regionInfo[channel].tableSelect[1]);

      sendDebout("bitenc",1,1,"subblockGain0",MTV_INT, &qcOut->subblockGain[channel][0]);
      sendDebout("bitenc",1,1,"subblockGain1",MTV_INT, &qcOut->subblockGain[channel][1]);
      sendDebout("bitenc",1,1,"subblockGain2",MTV_INT, &qcOut->subblockGain[channel][2]);
    }

    if(streamType == MPEG1)
      sendDebout("bitenc",1,1,"preEmphasisFlag",MTV_INT, &qcOut->preEmphasisFlag[channel]);

    sendDebout("bitenc",1,1,"scfScale",MTV_INT, &qcOut->scfScale[channel]);
    sendDebout("bitenc",1,1,"count1Table",MTV_INT, &qcOut->regionInfo[channel].count1Table);
  }
#endif
}

/*****************************************************************************

    functionname:encodeDyData  
    description: encodes dynpart data for one granule  
    returns:
    input:         
    output:        
    globals:       

*****************************************************************************/
static void encodeDyData (const struct QC_OUT *qcOut,
                          int                 channel,
                          HANDLE_BIT_BUF      hBitstream)
{
  int i,j,scfOffset,specOffset,scfBits=0;

  int p = mp3GetBitsAvail(hBitstream);

  /*
      encode scalefactors
  */
  scfOffset=0;
  for(i=0;i<SCF_BANDS;i++){
    scfBits = qcOut->scfBitsPerPartition[channel][i];

    if (scfBits) {
      for(j=0;j<qcOut->scfCntPerPartition[channel][i];j++)
        mp3WriteBits(hBitstream,qcOut->scf[channel][scfOffset+j],scfBits);
    }
    scfOffset+=qcOut->scfCntPerPartition[channel][i];
  }

  sendDebout("bitenc",21,1,"scf",MTV_INT, &qcOut->scf[channel]);

  /*
      huffman encode spectrum
  */

  specOffset = 0;

  /*
      big values
  */

  for(i=0;i<qcOut->regionInfo[channel].nrOfSubregions;i++){
    for(j=0;j<qcOut->regionInfo[channel].pairsSubregion[i];j++){
      mp3EncodeHuffPair(qcOut->quantSpec[channel]+specOffset,
                        qcOut->regionInfo[channel].tableSelect[i],
                        hBitstream);
      specOffset+=2;
    }
  }
  /*
    count 1
  */
  for(i=0;i<qcOut->regionInfo[channel].count1;i++){
    mp3EncodeHuffQuadruple(qcOut->quantSpec[channel]+specOffset,
                           qcOut->regionInfo[channel].count1Table,
                           hBitstream);
    specOffset+=4;
  }

  p = mp3GetBitsAvail(hBitstream) - p;

  /* if part2_3length is larger than what we have written so far,
     fill with 1 bits. */

  for (; p+16 <= qcOut->part2_3Length[channel]; p+=16)
  {
    mp3WriteBits(hBitstream, 0xffff, 16);
  }

  p = qcOut->part2_3Length[channel]-p ;
  mp3WriteBits(hBitstream, (1UL<<p)-1, p);
}

/*****************************************************************************

    functionname:calcFrameLen
    description:    
    returns:     
    input:         
    output:        
    globals:       

*****************************************************************************/
static int calcFrameLen (struct BITSTREAM_ENCODER *hBS, int bitrateIndex)
{
  static const int layer3Bitrates[3][16] =
  {
    { 8000L, 32000L, 40000L, 48000L, 56000L, 64000L, 80000L, 96000L, 112000L, 128000L, 160000L, 192000L, 224000L, 256000L, 320000L, -1L},
    { 4000L, 8000L, 16000L, 24000L, 32000L, 40000L, 48000L, 56000L, 64000L, 80000L, 96000L, 112000L, 128000L, 144000L, 160000L, -1L},
    { 4000L, 8000L, 16000L, 24000L, 32000L, 40000L, 48000L, 56000L, 64000L, 0, 0, 0, 0, 0, 0, -1L},
  };

  int samplesPerFrame = (hBS->streamType == MPEG1) ? 2*FRAME_LEN_LONG : FRAME_LEN_LONG ;
  int bitRate = layer3Bitrates[hBS->streamType][bitrateIndex] ;

  int result;

  result = (samplesPerFrame>>3) * bitRate;
  result /= hBS->init.sampleRate;

  return (result*8);
}

/*****************************************************************************

    functionname: BSWrite  
    description:  main function of write process 
    returns:      
    input:         
    output:        

*****************************************************************************/

int BSWrite(struct BITSTREAM_ENCODER *hBS,
            struct QC_OUT *qcOut [2],
            int bitrateIndex, int padding, int modeEx,
            unsigned char *bitStreamOutData, int *outBytes,
            int anc_mode,
            unsigned char * ancDataBytes,
            int * numAncDataBytes,
            int bitsPerChannelUsed[2],
            int* oflOffset,
            unsigned int* writeOflOnce, 
            int ancBitsToWrite
            )
{
  int i, j;
  int totalDynBits = 0; 
  int dbgval;
  int stuffingBits;
  int availableFrameBits = calcFrameLen (hBS, bitrateIndex) + (padding ? 8 : 0) ;
  int maxBackPointer = (hBS->streamType == MPEG1) ? 511:255;
  int fillBitsAll = 0 ;
  int oflBits = 0; 
  
  if (maxBackPointer * 8 + availableFrameBits > 7680)
  {
    maxBackPointer = (7680 - availableFrameBits )/8;
    if (maxBackPointer < 0)
      maxBackPointer = 0;
  }

  /* calculate total dynamic bits */
  totalDynBits = 0;
  for (i = 0; i < hBS->nGranules; i++) {
    for (j = 0; j < hBS->nChannels; j++) {
      totalDynBits += qcOut [i]->part2_3Length[j] ;
      bitsPerChannelUsed[j] += qcOut [i]->part2_3Length[j] ;
    }
  }

  if(ancBitsToWrite > 0) {

    /* ancillary error */
    assert((availableFrameBits - 
            hBS->hdSiBits - 
            totalDynBits -
            ancBitsToWrite +
            hBS->backPointer*8) >= 0);

    /* consider ofl bits */
    if ( writeOflOnce )
      if ( *writeOflOnce > 0 ) {
        oflBits = OFL_V1_LEN * 8;
      }

    if (numAncDataBytes != NULL) {
      if (*numAncDataBytes != 0) {
        switch (anc_mode) {
        case 0: /* ANC_RAW_MODE */
          *numAncDataBytes = *numAncDataBytes -  ( (ancBitsToWrite-oflBits) / 8 ) ;
          break;

        case 1: /* ANC_FHG_MODE */
          {
            unsigned long checksum = 0;
            int bytesToWrite = (ancBitsToWrite-oflBits) / 8;
            /*int diff = 0;*/
 
            if ( bytesToWrite > 0) {
              
              /* sync */
              hBS->ancFhgBuf[0]  = 0xA5;

              /* length */
              assert(bytesToWrite <= 255);
              hBS->ancFhgBuf[1]  = (unsigned char)bytesToWrite;

              /* data */
              for( i=0; i < (bytesToWrite - (ANC_FHG_WRAPPING/8)); i++) {
                hBS->ancFhgBuf[i+2] = ancDataBytes[i];
              }
       
              for( i=0; i < (bytesToWrite - 1); i++) {
                checksum += hBS->ancFhgBuf[i];
              }

              /* checksum */
              hBS->ancFhgBuf[bytesToWrite-1] = (unsigned char)(checksum & 0x000000ff);

              *numAncDataBytes = (*numAncDataBytes - bytesToWrite) + (ANC_FHG_WRAPPING/8);
            }
          }
          break;

        default:
          /* no valid ancillary mode */ 
          break;
        }
      }
    }
  } 

  stuffingBits = (hBS->backPointer - maxBackPointer)*8 +
    (availableFrameBits - hBS->hdSiBits - totalDynBits - ancBitsToWrite);

  if (stuffingBits > 0)
  {
    /* put fillbits into part2_3Length. Be careful that no granule exceeds
       the limit of 7680 bits per granule. */

    for (i = 0; i < hBS->nGranules; i++)
    {
      int nBitsThisGranule = hBS->hdSiBits ; /* count header/sideinfo bits in */

      for (j = 0; j < hBS->nChannels; j++)
      {
        /* don't write more than 4092 bits into dynpart */
        int fillBitsPerPart = min(stuffingBits, (1<<12)-4 - qcOut [i]->part2_3Length[j]) ;
        /* don't exceed 7680 bits limit */
        if (fillBitsPerPart > 7680 - (nBitsThisGranule + qcOut [i]->part2_3Length[j]))
          fillBitsPerPart = 7680 - (nBitsThisGranule + qcOut [i]->part2_3Length[j]) ;

        /* if nBitsThisGranule + qcOut [i]->part2_3Length[j] > 7680, the quantizer
           is in error. We want to catch the situation in debug mode; but if this
           should happen in the release, we silently "do the right thing" */
        assert(fillBitsPerPart >= 0) ;
        if (fillBitsPerPart < 0)
        {
          fillBitsPerPart = 0;
        }

        /* put fillbits into dynpart, i.e. increase part2_3_length. In theory, we
           could put any number of bits there, but this creates some difficult
           situations for the decoder. To make the bitstreams work even with
           non-compliant decoders, only put multiples of four bits into dynpart. */

        fillBitsPerPart -= (fillBitsPerPart % 4);

        qcOut [i]->part2_3Length[j] += fillBitsPerPart ;
        nBitsThisGranule += qcOut [i]->part2_3Length[j] ;
        totalDynBits += fillBitsPerPart ;
        stuffingBits -= fillBitsPerPart ;
        fillBitsAll += fillBitsPerPart ;
      }
    }
  }
  else
  {
    stuffingBits = (8-(totalDynBits % 8))%8;
  }
  

  /* for dual channel only: try to give fillbits to both channels NOT independently */
  {
    int sumOfAll = 0 ;
    for ( i = 0; i < 2 ; i++){
      sumOfAll += bitsPerChannelUsed[i] ;
    }
    
    sumOfAll += fillBitsAll ;
    sumOfAll += stuffingBits ;
    for ( i = 0; i < 2 ; i++){
      int add = (int)( sumOfAll/2.0 - bitsPerChannelUsed[i]);
      if ( ( (float)sumOfAll/2.0 - (float)bitsPerChannelUsed[i] - add ) >= 0.5 )
        add += 1 ;      
      if ( add > fillBitsAll+stuffingBits ) add = fillBitsAll+stuffingBits;
      if ( add < 0 ) add = 0;
      bitsPerChannelUsed[i] += add + ancBitsToWrite/2 ; 
    }
  }

  dbgval = mp3GetBitsAvail(hBS->hSiBitstream);

  /*
     encode header in si bitstream
   */

  encodeHeader(hBS->streamType,
               hBS->init.protection,
               bitrateIndex,
               hBS->sampleRateIndex,
               hBS->channelMode,
               padding,
               modeEx,
               hBS->init.privateBit,
               hBS->init.copyRightBit,
               hBS->init.originalCopyBit,
               hBS->hSiBitstream);

  /*
     encode sideinfo header in si bitstream 
   */
  encodeSiHeader(hBS->streamType,
                 hBS->channelMode,
                 hBS->backPointer,
                 hBS->hSiBitstream);

  /*
     encode sideInfo save scalefactor select information
   */

  if (hBS->streamType == MPEG1) {
    for (j = 0; j < hBS->nChannels; j++)
      mp3WriteBits(hBS->hSiBitstream,0,4); /* scfsi not used anymore */
  }

  /*
     encode side info gr/ch Data in si bitstream
   */

  for (i = 0; i < hBS->nGranules; i++) {
    for (j = 0; j < hBS->nChannels; j++) {
      encodeSiData(hBS->streamType, j, qcOut[i], hBS->hSiBitstream);
    }
  }

  /*
    encode dynpart gr/ch in dy bitstream
   */

  dbgval = mp3GetBitsAvail(hBS->hDyBitstream);

  for (i = 0; i < hBS->nGranules; i++) {
    for (j = 0; j < hBS->nChannels; j++) {
      encodeDyData(qcOut [i], j, hBS->hDyBitstream);
    }
  }

  hBS->backPointer += (availableFrameBits - hBS->hdSiBits)/8 
    - (totalDynBits+stuffingBits+ancBitsToWrite)/8  ;

  assert(hBS->backPointer >= 0);

  /*
    we might not have been able to put all fillbits into the dynpart because
    there is a restriction on part2_3_length. The rest of the bits will have to
    go into the ancillary data.
  */

  while (stuffingBits >= 16)
  {
    mp3WriteBits(hBS->hDyBitstream, 0, 16);
    stuffingBits -= 16;
  }
  if (stuffingBits)
    mp3WriteBits(hBS->hDyBitstream, 0, stuffingBits);

  /*
    Write OFL
  */
  if (writeOflOnce) {
    if ( *writeOflOnce > 0 ) {
      *oflOffset = addOflDummy( hBS, writeOflOnce );
    }
  }

  /*
    Write static anc data bits
  */  
  for (i = 0; i < hBS->ancDataBits; i++) {
    mp3WriteBits(hBS->hDyBitstream, 0, 1);
  }

  /*
    Write dynamic anc data bits
  */  
  if (anc_mode == 0) { 
    /* ANC_RAW_MODE */
    for (i=0; i<=((ancBitsToWrite-oflBits)/8)-1; i++) {
      mp3WriteBits(hBS->hDyBitstream, (BIT_BUF_WORD)ancDataBytes[i], 8);
    }
  }
  else if(anc_mode == 1) { 
    /* ANC_FHG_MODE */
    if ( hBS->ancFhgBuf != NULL ) {
      for (i=0; i<=((ancBitsToWrite-oflBits)/8)-1; i++) {
        mp3WriteBits(hBS->hDyBitstream, (BIT_BUF_WORD)hBS->ancFhgBuf[i], 8);
      }
    }
  }

  dbgval = mp3GetBitsAvail(hBS->hDyBitstream) - dbgval;


  /*
     compose header/si stream and dynpart stream;
     multiplex header/si stream into output
   */
  *outBytes = mp3ComposeMuxFrames(&(hBS->muxList),
                                  hBS->init.protection,
                                  hBS->hdSiBits,
                                  availableFrameBits - hBS->hdSiBits,
                                  bitStreamOutData);

  return totalDynBits + hBS->hdSiBits + stuffingBits + ancBitsToWrite; 
}

/*****************************************************************************

    functionname: addOflDummy
    description:  write dummy original file length data
    returns:      begin of ofl data in byte from file begin 

*****************************************************************************/
int addOflDummy(struct BITSTREAM_ENCODER *hBS,
                unsigned int* writeOflOnce) 
{
  int i=0;
  unsigned int oflPos=0;
  unsigned long crcReg = 0x000000ff;
  unsigned char tmpOfl[OFL_V1_LEN] = {0x00,0x00,0x00,0x00,0x00,
                                      0x00,0x00,0x00,0x00,0x00};
  /* sync */
  tmpOfl[0] = 0xB << 4;

  /* version 1 */
  tmpOfl[0] |= 0x04;

  tmpOfl[1] = 0x00;
  tmpOfl[2] = 0x00;
  
  tmpOfl[3] = 0xff;
  tmpOfl[4] = 0xff;
  tmpOfl[5] = 0xff;
  tmpOfl[6] = 0xff;

  /* add. delay */
  tmpOfl[7] = 0x00;
  tmpOfl[8] = 0x00;

  /* calc crc */
  for ( i=0; i < (OFL_V1_LEN - 1); i++ ) {
    mp3CrcAdvance( 0x0045, 0x0080, &crcReg, tmpOfl[i] ); 
  }

  /* crc */
  tmpOfl[9] = (unsigned char)(crcReg & 0x000000ff); 

  for ( i=0; i < OFL_V1_LEN; i++ ) {
    mp3WriteBits( hBS->hDyBitstream, (BIT_BUF_WORD)tmpOfl[i], 8 );
  }

  /* only once */
  *writeOflOnce = 0;

  oflPos = mp3GetBitcnt(hBS->hSiBitstream);
  oflPos += mp3GetBitcnt(hBS->hDyBitstream);
  oflPos -= (OFL_V1_LEN * 8);
  assert( !(oflPos % 8) );

  return ( oflPos / 8 ); 
}

void BSFlush (struct BITSTREAM_ENCODER *hBS,
              unsigned char *bitStreamOutData,
              int *outBytes)
{
  /*
   * flush the bitreservoir.
   */

  *outBytes = mp3FlushMuxFrames(&(hBS->muxList),
                                hBS->init.protection,
                                bitStreamOutData);
}

int GetHdSiBits(const struct BITSTREAM_ENCODER *hBS)
{
  return hBS->hdSiBits ;
}

int *mp3GetFrameStats(struct BITSTREAM_ENCODER *hBS)
{
  return hBS->muxList.frameStats;
}

/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: mp3decifc.cpp
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-28
 *   contents/description: Mp3 Decoder interface (C-style)
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:45 $
 * $Id: mp3decifc.cpp,v 1.1 2009/04/28 20:17:45 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "mp3decifc.h"
#include "mp3ssc.h"
#include "mpgadecoder.h"
#include "mp3decinfo.h"
#ifdef KSA_DRM
#include "mp3drmifc.h"
#endif
#include <string.h>


//-------------------------------------------------------------------------*
//
//                   C o m m o n
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   mp3decGetErrorText
//-------------------------------------------------------------------------*

const char * MP3DECAPI mp3decGetErrorText(SSC ssc)
{
  return CMp3Ssc(ssc);
}

//-------------------------------------------------------------------------*
//
//                   C + +   I N T E R F A C E
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//
//                   I M p g a D e c o d e r
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   Release
//-------------------------------------------------------------------------*

void IMpgaDecoder::Release()
{
  delete this;
}

//-------------------------------------------------------------------------*
//
//                   I M p 3 D e c I n f o
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   Release
//-------------------------------------------------------------------------*

void IMp3DecInfo::Release()
{
  delete this;
}

//-------------------------------------------------------------------------*
//   mp3decCreateObject
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decCreateObject(IMpgaDecoder **ppObject)
{
  return mp3decCreateObjectEx(0, 0, 0, ppObject);
}

//-------------------------------------------------------------------------*
//   mp3decCreateObjectEx
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decCreateObjectEx
    (
    int Quality,
    int Resolution,
    int Downmix,
    IMpgaDecoder **ppObject
    )
{
  if ( NULL == ppObject )
    return SSC_E_WRONGPARAMETER;

  CMpgaDecoder *pDec = new CMpgaDecoder(Quality, Resolution, Downmix);

  if ( NULL == pDec )
    return SSC_E_OUTOFMEMORY;

  *ppObject = static_cast<IMpgaDecoder*>(pDec);
  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decCreateObjectExtBuf
//-------------------------------------------------------------------------*

LINKSPEC SSC MP3DECAPI mp3decCreateObjectExtBuf
    (
    unsigned char *pBuffer,
    int            cbBuffer,
    int            Quality,
    int            Resolution,
    int            Downmix,
    IMpgaDecoder **ppObject
    )
{
  int tmp;

  // check cbBuffer (must be power of 2)
  for ( tmp=0; tmp<16; tmp++ )
    if ( (1 << tmp) >= cbBuffer )
      break;

  tmp = (1<<tmp);
  if ( tmp != cbBuffer )
    return SSC_E_WRONGPARAMETER; // error

  if ( NULL == ppObject )
    return SSC_E_WRONGPARAMETER;

  *ppObject = NULL;

  if ( cbBuffer < 2048 )
    return SSC_E_MPGA_BUFFERTOOSMALL;

  CMpgaDecoder *pDec = new CMpgaDecoder(pBuffer, cbBuffer, Quality, Resolution, Downmix);

  if ( NULL == pDec )
    return SSC_E_OUTOFMEMORY;

  *ppObject = static_cast<IMpgaDecoder*>(pDec);
  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decCreateInfoObject
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decCreateInfoObject(IMp3DecInfo **ppObject)
{
  if ( NULL == ppObject )
    return SSC_E_WRONGPARAMETER;

  CMp3DecInfo *pDecInfo = new CMp3DecInfo;

  if ( NULL == pDecInfo )
    return SSC_E_OUTOFMEMORY;

  *ppObject = static_cast<IMp3DecInfo*>(pDecInfo);
  return SSC_OK;
}

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C   I N T E R F A C E
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//
//                   P r i v a t e   F u n c t i o n s
//
//-------------------------------------------------------------------------*

static const char FourCC1[] = { '\x73', '\x69', '\x72', '\x00' };
static const char FourCC2[] = { '\x4d', '\x70', '\x33', '\x49' };

//
// the 'real' MP3DEC_HANDLE
//
typedef struct
  {
  char          FourCC[4]; // signature to check a handle
  CMpgaDecoder *pDec;      // pointer to decoder object
  } MP3DEC_HANDLE_INTERN;

//
// the 'real' MP3DECINFO_HANDLE
//
typedef struct
  {
  char         FourCC[4]; // signature to check a handle
  CMp3DecInfo *pInfo;     // pointer to decoder info object
  } MP3DECINFO_HANDLE_INTERN;

/*-------------------------------------------------------------------------*\
 *
 *                   M p 3   D e c o d e r
 *
\*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//   IsValidHandle
//-------------------------------------------------------------------------*

static bool IsValidHandle(MP3DEC_HANDLE handle)
{
  // check if NULL
  if ( handle == NULL )
    return false;

  // cast to MP3DEC_HANDLE_INTERN
  MP3DEC_HANDLE_INTERN *hIntern = (MP3DEC_HANDLE_INTERN*)handle;

  // check FourCC
  if ( memcmp(hIntern->FourCC, FourCC1, sizeof(FourCC1)) != 0 )
    return false;

  // check if pDec != NULL
  if ( hIntern->pDec == NULL )
    return false;

  // ... all tests passsed, let's hope this handle is valid
  return true;
}

//-------------------------------------------------------------------------*
//   GetDec
//-------------------------------------------------------------------------*

static CMpgaDecoder *GetDec(MP3DEC_HANDLE handle)
{
  if ( !IsValidHandle(handle) )
    return NULL;

  return ((MP3DEC_HANDLE_INTERN*)handle)->pDec;
}

//-------------------------------------------------------------------------*
//   CreateHandle
//-------------------------------------------------------------------------*

static MP3DEC_HANDLE CreateHandle
    (
    int             Quality,
    int             Resolution,
    int             Downmix
    )
{
  // create the handle itself
  MP3DEC_HANDLE_INTERN *hIntern = new MP3DEC_HANDLE_INTERN;

  if ( hIntern == NULL )
    goto cleanup;

  // set FourCC
  memcpy(hIntern->FourCC, FourCC1, sizeof(FourCC1));

  // create decoder object
  hIntern->pDec = new CMpgaDecoder(Quality, Resolution, Downmix);

  if ( hIntern->pDec == NULL )
    goto cleanup;

  return (MP3DEC_HANDLE)hIntern;

cleanup:

  if ( hIntern )
    {
    // delete decoder object
    if ( hIntern->pDec )
      delete hIntern->pDec;

    // delete the handle itself
    delete hIntern;
    }

  return NULL;
}

//-------------------------------------------------------------------------*
//   DeleteHandle
//-------------------------------------------------------------------------*

static bool DeleteHandle(MP3DEC_HANDLE handle)
{
  if ( !IsValidHandle(handle) )
    return false;

  // delete decoder object
  delete GetDec(handle);

  // delete the handle itself
  delete (MP3DEC_HANDLE_INTERN*)handle;

  return true;
}

/*-------------------------------------------------------------------------*\
 *
 *                   M p 3   D e c o d e r   I n f o
 *
\*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//   InfoIsValidHandle
//-------------------------------------------------------------------------*

static bool InfoIsValidHandle(MP3DECINFO_HANDLE handle)
{
  // check if NULL
  if ( handle == NULL )
    return false;

  // cast to MP3DEC_HANDLE_INTERN
  MP3DECINFO_HANDLE_INTERN *hIntern = (MP3DECINFO_HANDLE_INTERN*)handle;

  // check FourCC
  if ( memcmp(hIntern->FourCC, FourCC2, sizeof(FourCC2)) != 0 )
    return false;

  // check if pDec != NULL
  if ( hIntern->pInfo == NULL )
    return false;

  // ... all tests passsed, let's hope this handle is valid
  return true;
}

//-------------------------------------------------------------------------*
//   InfoGetInfo
//-------------------------------------------------------------------------*

static CMp3DecInfo *InfoGetInfo(MP3DECINFO_HANDLE handle)
{
  if ( !InfoIsValidHandle(handle) )
    return NULL;

  return ((MP3DECINFO_HANDLE_INTERN*)handle)->pInfo;
}

//-------------------------------------------------------------------------*
//   InfoCreateHandle
//-------------------------------------------------------------------------*

static MP3DECINFO_HANDLE InfoCreateHandle()
{
  // create the handle itself
  MP3DECINFO_HANDLE_INTERN *hIntern = new MP3DECINFO_HANDLE_INTERN;

  if ( hIntern == NULL )
    goto cleanup;

  // set FourCC
  memcpy(hIntern->FourCC, FourCC2, sizeof(FourCC2));

  // create decoder info object
  hIntern->pInfo = new CMp3DecInfo;

  if ( hIntern->pInfo == NULL )
    goto cleanup;

  return (MP3DECINFO_HANDLE)hIntern;

cleanup:

  if ( hIntern )
    {
    // delete decoder info object
    if ( hIntern->pInfo )
      delete hIntern->pInfo;

    // delete the handle itself
    delete hIntern;
    }

  return NULL;
}

//-------------------------------------------------------------------------*
//   InfoDeleteHandle
//-------------------------------------------------------------------------*

static bool InfoDeleteHandle(MP3DECINFO_HANDLE handle)
{
  if ( !InfoIsValidHandle(handle) )
    return false;

  // delete decoder object
  delete InfoGetInfo(handle);

  // delete the handle itself
  delete (MP3DECINFO_HANDLE_INTERN*)handle;

  return true;
}

//-------------------------------------------------------------------------*
//
//                   E x p o r t e d   F u n c t i o n s
//
//-------------------------------------------------------------------------*

/*-------------------------------------------------------------------------*\
 *
 *                   M p 3   D e c o d e r
 *
\*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//   mp3decOpen
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decOpen
    (
    MP3DEC_HANDLE *handle,
    int            Quality,
    int            Resolution,
    int            Downmix
    )
{
  if ( handle == NULL )
    {
    // valid pointer needed!
    return SSC_E_WRONGPARAMETER;
    }

  // create the handle
  *handle = CreateHandle(Quality, Resolution, Downmix);

  // return status
  return ((*handle != NULL) ? SSC_OK : SSC_E_OUTOFMEMORY);
}

//-------------------------------------------------------------------------*
//   mp3decClose
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decClose(MP3DEC_HANDLE handle)
{
  bool tmp_bool = true;

  // delete handle
  tmp_bool = DeleteHandle(handle);

  // return status
  return ((tmp_bool==true) ? SSC_OK : SSC_E_INVALIDHANDLE);
}

//-------------------------------------------------------------------------*
//   mp3decReset
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decReset(MP3DEC_HANDLE handle)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // reset decoder
  pDec->Reset();

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decDecode
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decDecode
    (
     MP3DEC_HANDLE  handle,
     unsigned char *pPcm,
     int            cbPcm,
     int           *pcbUsed,
     unsigned char *ancData,
     int *numAncBytes,
     int oflOn,
     unsigned int *startDelay,
     unsigned int *totalLength
    )
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // decode frame
  return pDec->DecodeFrame(pPcm,
                           cbPcm,
                           pcbUsed,
                           ancData,
                           numAncBytes,
                           oflOn,
                           startDelay,
                           totalLength);
}


//-------------------------------------------------------------------------*
//   mp3decDecodeToFloat
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decDecodeToFloat
    (
     MP3DEC_HANDLE  handle,
     float         *pPcm,
     int            cbPcm,
     int           *pcbUsed,
     unsigned char *ancData,
     int *numAncBytes,
     int oflOn,
     unsigned int *startDelay,
     unsigned int *totalLength
    )
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // decode frame
  return pDec->DecodeFrame(pPcm,
                           cbPcm,
                           pcbUsed,
                           ancData,
                           numAncBytes,
                           oflOn,
                           startDelay,
                           totalLength);
}


//-------------------------------------------------------------------------*
//   mp3decGetStreamInfo
//-------------------------------------------------------------------------*

const MP3STREAMINFO * MP3DECAPI mp3decGetStreamInfo(MP3DEC_HANDLE handle)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return NULL;

  // get streaminfo
  return (const MP3STREAMINFO *)(void*)(pDec->GetStreamInfo());
}

//-------------------------------------------------------------------------*
//   mp3decFill
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decFill
    (
    MP3DEC_HANDLE        handle,
    const unsigned char *pBuffer,
    int                  cbBuffer,
    int                 *pcbCopied
    )
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // check pcpCopied
  if ( pcbCopied == NULL )
    return SSC_E_WRONGPARAMETER;

  // fill decoder
  *pcbCopied = pDec->Fill(pBuffer, cbBuffer);

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decGetInputFree
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decGetInputFree(MP3DEC_HANDLE handle, int *pValue)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // check pValue
  if ( pValue == NULL )
    return SSC_E_WRONGPARAMETER;

  // get input free
  *pValue = pDec->GetInputFree();

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decGetInputLeft
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decGetInputLeft(MP3DEC_HANDLE handle, int *pValue)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // check pValue
  if ( pValue == NULL )
    return SSC_E_WRONGPARAMETER;

  // get input left
  *pValue = pDec->GetInputLeft();

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decSetInputEof
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decSetInputEof(MP3DEC_HANDLE handle)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // set input eof
  pDec->SetInputEof();

  return SSC_OK;
}

//-------------------------------------------------------------------------*
//   mp3decIsEof
//-------------------------------------------------------------------------*

int MP3DECAPI mp3decIsEof(MP3DEC_HANDLE handle)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return 0;

  return pDec->IsEof() ? 1: 0;
}

/*-------------------------------------------------------------------------*\
 *
 *                   M p 3   D e c o d e r   I n f o
 *
\*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//   mp3decInfoOpen
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decInfoOpen(MP3DECINFO_HANDLE *handle)
{
  if ( handle == NULL )
    {
    // valid pointer needed!
    return SSC_E_WRONGPARAMETER;
    }

  // create the handle
  *handle = InfoCreateHandle();

  // return status
  return ((*handle != NULL) ? SSC_OK : SSC_E_OUTOFMEMORY);
}

//-------------------------------------------------------------------------*
//   mp3decInfoClose
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decInfoClose(MP3DECINFO_HANDLE handle)
{
  bool tmp_bool = true;

  // delete handle
  tmp_bool = InfoDeleteHandle(handle);

  // return status
  return ((tmp_bool==true) ? SSC_OK : SSC_E_INVALIDHANDLE);
}

//-------------------------------------------------------------------------*
//   mp3decInfoInit
//-------------------------------------------------------------------------*

int MP3DECAPI mp3decInfoInit
    (
    MP3DECINFO_HANDLE  handle,
    unsigned char     *pBuffer,
    unsigned int       cbBuffer,
    unsigned int       cbMp3Data
    )
{
  // check handle & get decoder info object
  CMp3DecInfo *pInfo = InfoGetInfo(handle);

  if ( pInfo == NULL )
    return 0;

  return pInfo->Init(pBuffer, cbBuffer, cbMp3Data) ? 1 : 0;
}

//-------------------------------------------------------------------------*
//   mp3decInfoGetInfo
//-------------------------------------------------------------------------*

const MP3FILEINFO * MP3DECAPI mp3decInfoGetInfo(MP3DECINFO_HANDLE handle)
{
  // check handle & get decoder info object
  CMp3DecInfo *pInfo = InfoGetInfo(handle);

  if ( pInfo == NULL )
    return NULL;

  // get streaminfo
  return (const MP3FILEINFO *)(void*)(pInfo->GetInfo());
}

//-------------------------------------------------------------------------*
//   mp3decInfoSeekPointByPercent
//-------------------------------------------------------------------------*

unsigned int MP3DECAPI mp3decInfoSeekPointByPercent
    (
    MP3DECINFO_HANDLE handle,
    float             fPercent
    )
{
  // check handle & get decoder info object
  CMp3DecInfo *pInfo = InfoGetInfo(handle);

  if ( pInfo == NULL )
    return 0;

  return pInfo->SeekPointByPercent(fPercent);
}

//-------------------------------------------------------------------------*
//   mp3decInfoSeekPointByTime
//-------------------------------------------------------------------------*

unsigned int MP3DECAPI mp3decInfoSeekPointByTime
    (
    MP3DECINFO_HANDLE handle,
    unsigned int      dwTime
    )
{
  // check handle & get decoder info object
  CMp3DecInfo *pInfo = InfoGetInfo(handle);

  if ( pInfo == NULL )
    return 0;

  return pInfo->SeekPointByTime(dwTime);
}

//-------------------------------------------------------------------------*
//   mp3decGetScfBuffer
//-------------------------------------------------------------------------*

#ifdef KSA_DRM
SSC MP3DECAPI mp3decGetScfBuffer(MP3DEC_HANDLE  handle,
					  const unsigned char** ppBuffer,
					  unsigned int* pBufSize)
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  // decode frame
  pDec->GetScfBuffer(ppBuffer, pBufSize);

  return SSC_OK;
}
#endif


//-------------------------------------------------------------------------*
//   mp3decGetLastAnc
//-------------------------------------------------------------------------*

SSC MP3DECAPI mp3decGetLastAnc
    (
     MP3DEC_HANDLE  handle,
     unsigned char *ancData,
     int *numAncBytes
    )
{
  // check handle & get decoder object
  CMpgaDecoder *pDec = GetDec(handle);

  if ( pDec == NULL )
    return SSC_E_INVALIDHANDLE;

  return pDec->GetLastAncData(ancData, numAncBytes);
}


/*-------------------------------------------------------------------------*/

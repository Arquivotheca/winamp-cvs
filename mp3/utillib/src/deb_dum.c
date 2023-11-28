/******************************************************************************

                (C) copyright Fraunhofer - IIS (1997-2006)
                         All Rights Reserved

 Initial author:       B.Teichmann <tmn@iis.fhg.de>
 contents/description: dummy implementations for debug output routines

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: deb_dum.c,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

#include "utillib.h"

#define MEMBER_GRAPH void
#define SUBTAG_LIST void
#define OUTPUT_LIST void

/**** public functions:  ***********/
HANDLE_ERROR_INFO initDebout(const char* rcfilename, char *identString, int enabStartLater, int startFrame,int startGran,int channelMask,int showLayerMask, char *outFile)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  const char*   pc_dummy;
  int i_dummy;
  pc_dummy = rcfilename;
  pc_dummy = identString;
  i_dummy  = enabStartLater;
  i_dummy  = startFrame;
  i_dummy  = startGran;
  i_dummy  = channelMask;
  i_dummy  = showLayerMask;
  pc_dummy = outFile;

  return noError;
}

HANDLE_ERROR_INFO setDeboutVars(int frameCnt,int granCnt,int channel,int layer)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  int i_dummy;
  i_dummy = frameCnt;
  i_dummy = granCnt;
  i_dummy = channel;
  i_dummy = layer;

  return noError;
}

HANDLE_ERROR_INFO  sendDebout(const char tagName[], int npts, const int incVec,
                              const char subTagName[], const MTV_DATA_TYPE dtype, const void *dataPtr)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  const char*   pc_dummy;
  int           i_dummy;
  MTV_DATA_TYPE mdt_dummy;
  const void*   pv_dummy;
  pc_dummy  = tagName;
  i_dummy   = npts;
  i_dummy   = incVec;
  pc_dummy  = subTagName;
  mdt_dummy = dtype;
  pv_dummy = dataPtr;

  return noError;
}

HANDLE_ERROR_INFO  sendDeboutHist(const char tagName[],
                                  const char subTagName[],
                                  const MTV_DATA_TYPE dtype ,
                                  const void *dataPtr)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  const char*   pc_dummy;
  MTV_DATA_TYPE mdt_dummy;
  const void*   pv_dummy;
  pc_dummy = tagName;
  pc_dummy = subTagName;
  mdt_dummy = dtype;
  pv_dummy = dataPtr;

  return noError;
}

HANDLE_ERROR_INFO  addDeboutHist(const char tagName[],
                                 const char subTagName[],
                                 const MTV_DATA_TYPE dtype ,
                                 const void *dataPtr)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  const char*   pc_dummy;
  MTV_DATA_TYPE mdt_dummy;
  const void*   pv_dummy;
  pc_dummy = tagName;
  pc_dummy = subTagName;
  mdt_dummy = dtype;
  pv_dummy = dataPtr;

  return noError;
}

HANDLE_ERROR_INFO showDebout( void  )
{
  return  noError;
}

int getDeboutVar(MTV_DATA_TYPE dtype,
                 const char tagName[],
                 void * pVar,
                 const double pMin,
                 const double pMax,
                 const double pDefault)
{
  /* Remove compiler warnings */
  /* ugly but effective */
  MTV_DATA_TYPE mdt_dummy;
  const char*   pc_dummy;
  double        d_dummy;
  mdt_dummy = dtype;
  pc_dummy  = tagName;
  pVar      = pVar;
  d_dummy   = pMin;
  d_dummy   = pMax;
  d_dummy   = pDefault;

  return 0;
}

void deboutClose(void) {
}

/******************************************************************************

                (C) copyright Fraunhofer - IIS (1997-2006)
                         All Rights Reserved

 project:              utility library
 Initial author:       B.Teichmann <tmn@iis.fhg.de>
 contents/description: - generic debug output and plotmtv interface module header
                       - part of the interface to utillib
                       - do not include this file, include utillib.h instead

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: deb_out.h,v 1.1 2009/04/28 20:17:47 audiodsp Exp $

******************************************************************************/

#ifndef _deb_out_h
#define _deb_out_h

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "errorhnd.h"

#ifdef __cplusplus
extern "C" {
#endif

  enum _MTV_ENUM {MTV_DOUBLE,MTV_FLOAT,MTV_ABSFLOAT,MTV_LONG,MTV_INT,MTV_ABSINT,MTV_SHORT,MTV_FLOAT_SQA,MTV_FLOAT_COMP_SQA,MTV_DOUBLE_SQA};
  typedef enum _MTV_ENUM  MTV_DATA_TYPE;

  enum _MTV_SHOW {WAIT,NOWAIT};
  typedef enum _MTV_SHOW  MTV_SHOW;

  HANDLE_ERROR_INFO initDebout(const char* rcfilename, char *identString, int enabStartLater, int startFrame,int startGran,int channel,int showLayerMask, char* outFileName);
  HANDLE_ERROR_INFO sendDeboutFFT(const char tagName[], int npts,  const int incVec, const char subTagName[], const void *dataPtr);
  HANDLE_ERROR_INFO sendDeboutABS(const char tagName[], int npts,  const int incVec, const char subTagName[], const void *dataPtr);
  HANDLE_ERROR_INFO sendDeboutPWR(const char tagName[], int npts,  const int incVec, const char subTagName[], const int noBins, const void *dataPtr);
  /* send the data to the debout module, prints the data, if printing is enabled, but does not show any graphics, this can be done by showDeboutU() */

  void deboutClose(void); /* free memory */

  HANDLE_ERROR_INFO   getXYsquareGraph(double **xPoints,double **yPoints,int *numPoints,const int incVec,const MTV_DATA_TYPE dtype ,const void *dataPtr);
  HANDLE_ERROR_INFO   getXYGraph(double **xPoints,double **yPoints, int * numPoints,const int incVec,const MTV_DATA_TYPE dtype ,const void *dataPtr);

#ifdef DEBUG_PLOT
  HANDLE_ERROR_INFO  sendDebout(const char tagName[], int npts, const int incVec,
                                const char subTagName[], const MTV_DATA_TYPE dtype, const void *dataPtr);

  HANDLE_ERROR_INFO  sendDeboutHist(const char tagName[],
                                    const char subTagName[],
                                    const MTV_DATA_TYPE dtype ,
                                    const void *dataPtr);

  HANDLE_ERROR_INFO  addDeboutHist(const char tagName[],
                                   const char subTagName[],
                                   const MTV_DATA_TYPE dtype ,
                                   const void *dataPtr);
#else

#define sendDebout(a,b,c,d,e,f)  ((HANDLE_ERROR_INFO)noError)
#define sendDeboutHist(a,b,c,d)  ((HANDLE_ERROR_INFO)noError)
#define addDeboutHist(a,b,c,d)   ((HANDLE_ERROR_INFO)noError)

#endif

  HANDLE_ERROR_INFO  showDebout( void ); /*call this whenever you want to see something */
  HANDLE_ERROR_INFO setDeboutVars( int frameCnt, int granCnt, int channel,int layer);

  int getDeboutVar(MTV_DATA_TYPE dtype,
                   const char tagName[],
                   void * pVar,
                   const double pMin,
                   const double pMax,
                   const double pDefault);

  void plotMultVec( long npts, int incr ,MTV_DATA_TYPE dtype, void *vector1, ... );
  /* just for use in the debugger: type call plotMultVec(...) in the GNUdebugger command window,
     the last vector MUST be the NULL vector !

     TO USE THIS FEATURE YOU NEED A default debout.rc FILE IN THE COURRENT WORK DIRECTORY or at the location defined in the env variable DEBOUT_RC,
     WHICH ENABLES THE tagNames CALLED:   "direct1" and "direct2"

     in other words: debout.rc should include  at least these lines :
     :direct1
     %xlabel=""
     %ylabel=""
     %xlog=On
     %ylog=Off
     %boundary=True
     end
     :direct2
     %xlabel=""
     %ylabel=""
     %xlog=On
     %ylog=Off
     %boundary=True
     end
     # end of mplot.rc

     you can replace the words "True"(or "On")  with "False"(or "Off") and vice versa
  */

unsigned int GetBuildDateDebout(void);
unsigned int GetVersionDebout(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
/*
Deb_out prints or plots 1 dim. arrays (=vector):

A resource file controlls, whether a certain vector should realy be
printed , ploted or should be ignored .

To use deb_out , the initDebout has to be called once, to tell deb_out
the name (including path) of the resouce file, to set a unique string
which is used for the name of a plotwindow, and some other stuff (see
initDebout(...) obove).

Then each time a interesting vector occured in your programm , you should send the vector to deb_out.
(see sendDebout(..) above)

The resource file has the following systax:

1. Everything after '#' is ingored

2. A valid tagName definition starts with a ':',ends with a ':' and is
followed by either a 'x' or a valid prinf()-like format string.
Example:

:enegyData:x

or

:enegyData:"%4.1f optional-text"

3. All remaining lines until the 'end' are just copied to the plotmtv
data file, if 'x' -option is used, otherwise (if printf -format string
is used) all these lines including the 'end' is ignored
Example:

:enegyData:x
%grid=on
%xmin=1e+3
%ylog=on
end


4. If the first line of a tagName definition (the line that starts with
the ':') is preceded by a '#' all the rest is ignored, that means : to
exclude a vector from plotting or printing, just insert a '#' before
the first tagName line.
Example:

#:enegyData:x
%grid=on
%xmin=1e+3
%ylog=on
end

5a. If you want the data to be printed , you are ready.

5b.If you want the data to be ploted to an X-Window frame, then you must
call showDebout(). Each time you call that function , all data which was
send by sendDebout() since the last call to showDebout() are ploted in
one X-Window frame in this case the user program waits until you quit
the X-window frame with the 'quit' button.

6. newGen aacmain (file dummy.c) :

6a. there is already a initDebout() call, which uses some variables set
by some commandline options (see -h for more info)

6b. the showDebout() is called at the end of each granule (after the
bitstream is writted) feel free to introduce other showDebout() calls
whereever is seems appropriate)

6c. DO NOT USE PRINTF() FOR PRINTING ANY DATA!!!!!!!!!! instead use
sendDebout() even for single values (vector length = 1, incr = 0)

6d. you can use more than on resource file, but the resurce file in general/debout.rc shall contain ALL tagNames
(althought they can be commented out)

6e. and please: use meaningfull names for the tagNames !!

 */

/****************************************************************************

                      (C) copyright Coding Technologies (2003)
                               All Rights Reserved

 This software module was developed by Coding Technologies (CT). This is
 company confidential information and the property of CT, and can not be
 reproduced or disclosed in any form without written authorization of CT.

 Those intending to use this software module for other purposes are advised
 that this infringe existing or pending patents. CT has no liability for
 use of this software module or derivatives thereof in any implementation.
 Copyright is not released for any means.  CT retains full right to use the
 code for its own purpose, assign or sell the code to a third party and to
 inhibit any user or third party from using the code. This copyright notice
 must be included in all copies or derivative works.

 $Id: exception.h,v 1.1 2009/04/28 20:13:58 audiodsp Exp $

****************************************************************************/

/*!
  \brief   common exception object
  \author  Stefan Gewinner
*/

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

/*! aacPlus decoder error codes */
enum {
  AAC_OK           = 0x0000,
  AAC_FAILURE_BASE = 0x1000
  ,AAC_ENDOFSTREAM                    /*!< end of file reached */
  ,AAC_NOVALIDFRAME                   /*!< no complete valid frame in buffer */
  ,AAC_UNIMPLEMENTED                  /*!< the feature is not implemented */
  ,AAC_NOTADIFHEADER                  /*!< handled internally */
  ,AAC_ADTSCRCERROR                   /*!< the ADTS-CRC was wrong */
  ,AAC_INVALIDHANDLE                  /*!< invalid decoder instance passed */
  ,AAC_INVALIDCODEBOOK                /*!< invalid huffman codebook */
  ,AAC_INVALIDPREDICTORRESET          /*!< invalid predictor reset signalled */
  ,AAC_INVALIDPULSEOFFSET             /*!< invalid pulse data() signalled */
  ,AAC_INVALIDTNSDATA                 /*!< invalid tns data () signalled */
  ,AAC_INVALIDSECTIONDATA             /*!< invalid section data () signalled */
  ,AAC_INVALIDSPECTRALDATA            /*!< invalid spectral data () found */
  ,AAC_INVALIDSAMPLINGFREQUENCY       /*!< the indicated sampling frequency was invalid */
  ,AAC_INVALIDFRAME                   /*!< invalid frame found in bitstream */
  ,AAC_UNSUPPORTEDWINDOWSHAPE         /*!< handled internally */
  ,AAC_DOESNOTEXIST                   /*!< bitstream does not exist */
  ,AAC_INVALIDCONFIGTYPE              /*!< the type of out of band configuration was invalid */
  ,AAC_UNSUPPORTED_AOT                /*!< audio object type found is not supported */
  ,AAC_CONFIG_TOO_LONG                /*!< out-of-band config is too long for internal buffer */
  ,AAC_INIT_LIMITER_FAILED            /*!< limiter could not be instanciated */
};

#ifdef __cplusplus

/** Base Class For Exceptions.

    This class defines the interface that all exceptions possibly thrown by
    \Ref{CAacDecoder} or its input objects are derived from. 
*/

class CAacException
{

public :

  /// Exception Constructor.

  CAacException (int _value, char *_text) : value(_value), text(_text) {} ;

  /// Exception Destructor.

  ~CAacException () {} ;

  /** Exception Code Method.

      This method returns an exception status code, that can be used programatically.
  */

  int What () { return value ; }

  /** Exception Text Method.

      This method returns a pointer to a plain text explanation of what happened.
      It might be used e.g. for user notification messages or the like.
  */

  char *Explain () { return text ; }

protected :

  int value ;
  char *text ;

} ;

#define DECLARE_EXCEPTION(a,b,c) class a : public CAacException { public : a () : CAacException (b, c) {} ; }

#endif

#endif

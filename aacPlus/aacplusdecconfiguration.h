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

 $Id: aacplusdecconfiguration.h,v 1.1 2009/04/28 20:13:57 audiodsp Exp $

****************************************************************************/
/*!
  \file
  \brief  aacPlus Decoder Configuration $Revision: 1.1 $
  \author Andreas Schneider
*/

#ifndef __AACPLUSDECCONFIGURATION_H__
#define __AACPLUSDECCONFIGURATION_H__

/** aacPlus decoder configuration.

    This struct allows us to switch certain features of the aacPlus decoder
    on or off.
*/

typedef struct {

  unsigned int enableConcealment;
  unsigned int enableOutputLimiter;
  unsigned int enableSBRUpsampling;
}
AACPLUSDECCONFIGURATION, *PAACPLUSDECCONFIGURATION ;

#ifdef __cplusplus

class CAacPlusDecConfiguration
{
 protected:

  AACPLUSDECCONFIGURATION m_config;

 public:

  CAacPlusDecConfiguration() {
    m_config.enableConcealment = 0;
    m_config.enableOutputLimiter = 0;
    m_config.enableSBRUpsampling = 0;
  }

  CAacPlusDecConfiguration(AACPLUSDECCONFIGURATION config) {
    m_config = config;
  }

  PAACPLUSDECCONFIGURATION GetAacPlusDecConfigurationPtr( void) {
    return &m_config;
  }

  void SetAacConcealment( bool WantConcealment) {
    m_config.enableConcealment = (WantConcealment) ? 1 : 0;
  }

  void SetOutputLimiter( bool WantLimiter) {
    m_config.enableOutputLimiter = (WantLimiter) ? 1 : 0;
  }

  void SetSBRUpsampling( bool WantSBRUpsampling) {
    m_config.enableSBRUpsampling = (WantSBRUpsampling) ? 1 : 0;
  }
  

  bool GetAacConcealment( void) {
    return (m_config.enableConcealment) ? true : false;
  }

  bool GetOutputLimiter( void) {
    return (m_config.enableOutputLimiter) ? true : false;
  }

  bool GetSBRUpsampling( void) {
    return (m_config.enableSBRUpsampling) ? true : false;
  }
};
#endif

#endif

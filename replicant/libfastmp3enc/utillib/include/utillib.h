/******************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

 $Id: utillib.h,v 1.1 2007/05/29 16:02:41 audiodsp Exp $
 project:              utility library
 Initial author:       B. Teichmann
 contents/description: - interface to utillib
                       - only this file shall be included

 This software and/or program is protected by copyright law and
 international treaties. Any reproduction or distribution of this
 software and/or program, or any portion of it, may result in severe
 civil and criminal penalties, and will be prosecuted to the maximum
 extent possible under law.

 $Id: utillib.h,v 1.1 2007/05/29 16:02:41 audiodsp Exp $

******************************************************************************/

#ifndef __UTIL_H
#define __UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "IISutillib/deb_out.h"
#include "IISutillib/errorhnd.h"
#include "IISutillib/uci.h"
#include "IISutillib/ngsalloc.h"

typedef struct {
  char date[20];
  char versionNo[20];
} UtilLibInfo;

void UtilGetLibInfo (UtilLibInfo* libInfo);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __UTIL_H */

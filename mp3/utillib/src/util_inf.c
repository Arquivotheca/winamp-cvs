/******************************** MPEG Audio Encoder **************************

                       (C) copyright Fraunhofer-IIS (1997-2006)
                               All Rights Reserved

   Initial author:       O. Kunz
   contents/description: Configuration and commandline parsing tool

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: util_inf.c,v 1.1 2009/04/28 20:17:48 audiodsp Exp $

******************************************************************************/

/* library info stuff
 */
#include <string.h>
#include "utillib.h"


/* version number has to be adapted
 * according to general rules
 */

#define VER_NO "1.1.5"

/*
 * include this
 * in your external include file:

----------------------

  typedef struct {
    char date[20];
    char versionNo[20];
  } UtilLibInfo;

  void UtilGetLibInfo (UtilLibInfo* libInfo);

----------------------

 */

void UtilGetLibInfo (UtilLibInfo* libInfo)
{
  strcpy(libInfo->date,__DATE__);
  strcpy(libInfo->versionNo, VER_NO);
}

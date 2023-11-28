/*****************************************************************************

                       (C) copyright Fraunhofer-IIS (1997-2005)
                               All Rights Reserved

   This software and/or program is protected by copyright law and
   international treaties. Any reproduction or distribution of this
   software and/or program, or any portion of it, may result in severe
   civil and criminal penalties, and will be prosecuted to the maximum
   extent possible under law.

   $Id: math_inf.c,v 1.1 2009/04/28 20:17:42 audiodsp Exp $

******************************************************************************/

/* library info stuff
 */

#include "mathlib.h"
#include <string.h>


/* version number has to be adapted
 * according to general rules
 */

#define VER_NO "1.1.4"

/*
 * include this
 * in your external include file:

----------------------

  typedef struct {
    char date[20];
    char versionNo[20];
  } MathLibInfo;

  void MathGetLibInfo (MathLibInfo* libInfo);

----------------------

 */

void MathGetLibInfo (MathLibInfo* libInfo) {
  strcpy(libInfo->date,__DATE__);
  strcpy(libInfo->versionNo, VER_NO);
}

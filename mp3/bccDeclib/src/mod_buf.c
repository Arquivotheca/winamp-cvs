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
#include "bastypes.h"
#include "utillib.h"
#include "mod_buf.h"
#include "mathlib.h"


/*****************************************************************************

    functionname: CreateFloatModuloBuffer  
    description:  creates a circular buffer
    returns:      Status info
    input:        number of FLOAT values inside circular buffer
    output:       Handle

*****************************************************************************/
HANDLE_MODULO_BUFFER CreateFloatModuloBuffer(UINT size)
{
    HANDLE_MODULO_BUFFER pModuloBuffer;

    assert(size > 0);

    pModuloBuffer = (HANDLE_MODULO_BUFFER)ngsCalloc(1,sizeof(MODULO_BUFFER));

    if(pModuloBuffer == NULL)
        return(INVALID_HANDLE);

    pModuloBuffer->size = size;
    pModuloBuffer->paBuffer = (FLOAT*)ngsCalloc(sizeof(FLOAT),size);

    if(pModuloBuffer->paBuffer == NULL) {
      DeleteFloatModuloBuffer(pModuloBuffer);
      return INVALID_HANDLE;
    }

    pModuloBuffer->current_position = 0;
    pModuloBuffer->readPosition     = 0;
    pModuloBuffer->valuesLeft        = 0;

    return(pModuloBuffer);
}



/*****************************************************************************

    functionname: ResetFloatModuloBuffer  
    description:  resets a circular buffer
    returns:      Status info
    input:        number of FLOAT values inside circular buffer
    output:       Handle

*****************************************************************************/
BOOL ResetFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer)
{

  if (hModuloBuffer) 
    {
      setFLOAT(0.0f, hModuloBuffer->paBuffer, hModuloBuffer->size);
      hModuloBuffer->current_position = 0;
      hModuloBuffer->readPosition     = 0;
      hModuloBuffer->valuesLeft       = 0;
    }


  return(TRUE);
}



/*****************************************************************************

    functionname: DeleteFloatModuloBuffer  
    description:  frees memeory 
    returns:      
    input:        Handle
    output:       

*****************************************************************************/
void DeleteFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer)
{
  if (hModuloBuffer) {
    if (hModuloBuffer->paBuffer) ngsFree(hModuloBuffer->paBuffer);
    ngsFree(hModuloBuffer);
  }

  hModuloBuffer = INVALID_HANDLE;
}



/*****************************************************************************

    functionname: AddFloatModuloBufferValues  
    description:  adds values to circular buffer
    returns:      TRUE (size match is verified by assertion)
    input:        Handle, ptr to values to enter, number of values to enter
    output:       

*****************************************************************************/
BOOL AddFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, const FLOAT *values, UINT n){
    UINT lim1, lim2;

    assert(n <= hModuloBuffer->size);

    hModuloBuffer->valuesLeft+=n;

    lim1 = min(hModuloBuffer->current_position+n, hModuloBuffer->size) - hModuloBuffer->current_position;
    copyFLOAT(values,
	      hModuloBuffer->paBuffer+hModuloBuffer->current_position,
	      lim1);

    hModuloBuffer->current_position = (hModuloBuffer->current_position+lim1)%hModuloBuffer->size;

    lim2 = n - lim1;

    if (lim2 > 0) {
      copyFLOAT(values+lim1,
		hModuloBuffer->paBuffer+hModuloBuffer->current_position,
		lim2);
      hModuloBuffer->current_position=(hModuloBuffer->current_position+lim2)%hModuloBuffer->size;
    }
    return(TRUE);
}



/*****************************************************************************

    functionname: ZeroFloatModuloBuffer
    description:  set n elements of circular buffer to zero
    returns:      TRUE
    input:        Handle, number of elements to zero out
    output:       

*****************************************************************************/
BOOL ZeroFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer, UINT n){
    UINT lim1, lim2;
    assert(n <= hModuloBuffer->size);

    hModuloBuffer->valuesLeft+=n;

    lim1 = min(hModuloBuffer->current_position+n, hModuloBuffer->size) - hModuloBuffer->current_position;
    setFLOAT(0.0f, &hModuloBuffer->paBuffer[hModuloBuffer->current_position],
               lim1);

    hModuloBuffer->current_position=(hModuloBuffer->current_position+lim1)%hModuloBuffer->size;

    lim2 = n - lim1;
    if (lim2 > 0) {
      setFLOAT(0.0f, &hModuloBuffer->paBuffer[hModuloBuffer->current_position], 
		 lim2);

      hModuloBuffer->current_position = (hModuloBuffer->current_position+lim2)%hModuloBuffer->size;
    }
    return(TRUE);
}



/*****************************************************************************

    functionname: GetFloatModuloBufferValues  
    description:  checks out circular buffer values
    returns:      TRUE
    input:        Handle, number of values to retrieve, logical position in 
                  circular buffer
    output:       values retrieved

*****************************************************************************/
BOOL GetFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, FLOAT *values, UINT n, UINT age){
    UINT lim1, lim2;
    UINT iReqPosition;

    assert(n <= hModuloBuffer->size);
    assert(age <= hModuloBuffer->size);
    assert(n <= age);

    iReqPosition = (hModuloBuffer->size + hModuloBuffer->current_position - age) % hModuloBuffer->size;

    lim1 = min(iReqPosition + n, hModuloBuffer->size) - iReqPosition;
    copyFLOAT(hModuloBuffer->paBuffer+iReqPosition, values, lim1);

    iReqPosition = (iReqPosition + lim1) % hModuloBuffer->size;

    lim2 = n - lim1;

    if (lim2 > 0) 
      copyFLOAT(hModuloBuffer->paBuffer+iReqPosition, values+lim1, lim2);
    
    return(TRUE);
}


/*****************************************************************************

    functionname: ReadFloatModuloBufferValues  
    description:  checks out circular buffer values from read pointer
    returns:      TRUE
    input:        Handle, number of values to retrieve
    output:       values retrieved

*****************************************************************************/
BOOL ReadFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, FLOAT *values, UINT n){
  UINT lim1, lim2;
  UINT iReqPosition;

  assert(n <= hModuloBuffer->size);

  hModuloBuffer->valuesLeft-=n;
  
  iReqPosition = hModuloBuffer->readPosition;

  lim1 = min(iReqPosition + n, hModuloBuffer->size) - iReqPosition;
  copyFLOAT(hModuloBuffer->paBuffer+iReqPosition, values, lim1);
  
  lim2 = n - lim1;
  
  if (lim2 > 0) {
    copyFLOAT(hModuloBuffer->paBuffer,
              values+lim1,
              lim2);

    hModuloBuffer->readPosition = lim2;

  } else {   
    hModuloBuffer->readPosition = iReqPosition+lim1;
  }

  return(TRUE);
}

UINT GetValuesLeft(HANDLE_MODULO_BUFFER hModuloBuffer){

  return hModuloBuffer->valuesLeft; 
}


/*****************************************************************************

    functionname: MoveFloatModuloBufferReadPtr
    description:  moves the read pointer ahead/back for n values
    returns:      TRUE on success, else FALSE
    input:        Handle, number of values to move the read pointer forward/backward
    output:       

*****************************************************************************/
BOOL MoveFloatModuloBufferReadPtr(HANDLE_MODULO_BUFFER hModuloBuffer, INT n)
{
  /* \\\ valid old values -> backward moving of ReadPtr only within that range 
     /// valid new values -> forward moving of ReadPtr only within that range  
  _________________________________
 |\\\\\|////////////////|\\\\\\\\\\|
 |\\\\\|////////////////|\\\\\\\\\\|
       ^                ^
       |                |
       read             write


  */


  /* Backward moving */
  if ( 0 > n ) 
    {
      int newPos = 0;

      /* Valid samples */
#ifndef NDEBUG
      assert( -n < (hModuloBuffer->size - hModuloBuffer->valuesLeft) );
#else
      if ( -n >= (hModuloBuffer->size - hModuloBuffer->valuesLeft) ) {
        return (FALSE);
      }
#endif
      newPos = hModuloBuffer->readPosition + n; /* n is already negative !! */ 
      
      if ( 0 > newPos )
        newPos = hModuloBuffer->size + newPos;  /* newPos is negative */

      hModuloBuffer->readPosition = newPos;
      hModuloBuffer->valuesLeft -= n;  /* n is already negative !! */   

    }

  if ( 0 < n )
    {
      int newPos = 0;
      
      /* Valid samples */
#ifndef NDEBUG
      assert( n <= hModuloBuffer->valuesLeft  );
#else
      if ( n > hModuloBuffer->valuesLeft ) {
        return (FALSE);
      }
#endif

      newPos = hModuloBuffer->readPosition + n; 

      if ( newPos > hModuloBuffer->size )
        newPos = newPos % hModuloBuffer->size;
 
      
      hModuloBuffer->readPosition = newPos;
      hModuloBuffer->valuesLeft -= n;
    }

  return(TRUE);
}

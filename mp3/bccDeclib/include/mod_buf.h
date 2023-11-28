/***************************************************************************\
 *                         Fraunhofer IIS 
 *                 (c) 1997 - 2008 Fraunhofer IIS
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

#ifndef __MOD_BUF_H
#define __MOD_BUF_H

#include "bastypes.h"

typedef struct{
    UINT  size;
    UINT  current_position;
    UINT  readPosition;
    UINT  valuesLeft;
    FLOAT *paBuffer;
}MODULO_BUFFER;


typedef MODULO_BUFFER *HANDLE_MODULO_BUFFER;

HANDLE_MODULO_BUFFER CreateFloatModuloBuffer(UINT size);
BOOL                 ResetFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer);
void                 DeleteFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer);
BOOL                 AddFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, const FLOAT *values, UINT n);
BOOL                 ZeroFloatModuloBuffer(HANDLE_MODULO_BUFFER hModuloBuffer, UINT n);
BOOL                 GetFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, FLOAT *values, UINT n, UINT age);
BOOL                 ReadFloatModuloBufferValues(HANDLE_MODULO_BUFFER hModuloBuffer, FLOAT *values, UINT n);
UINT                 GetValuesLeft(HANDLE_MODULO_BUFFER hModuloBuffer);
BOOL                 MoveFloatModuloBufferReadPtr(HANDLE_MODULO_BUFFER hModuloBuffer, INT n);

#endif

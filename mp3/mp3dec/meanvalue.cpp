/***************************************************************************\
 *
*                    MPEG Layer3-Audio Decoder
*                  © 1997-2006 by Fraunhofer IIS
 *                        All Rights Reserved
 *
 *   filename: meanvalue.cpp
 *   project : ---
 *   author  : Martin Sieler
 *   date    : 1998-02-14
 *   contents/description: calc mean value
 *
 *
\***************************************************************************/

/*
 * $Date: 2009/04/28 20:17:44 $
 * $Id: meanvalue.cpp,v 1.1 2009/04/28 20:17:44 audiodsp Exp $
 */

/* ------------------------ includes --------------------------------------*/

#include "meanvalue.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   C M e a n V a l u e
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   Reset
//-------------------------------------------------------------------------*

void CMeanValue::Reset()
{
  m_Count      = 0;
  m_Sum        = 0;
  m_FirstValue = 0;
  m_Min        = 0;
  m_Max        = 0;
  m_bFixed     = true;
}

//-------------------------------------------------------------------------*
//   operator +=
//-------------------------------------------------------------------------*

CMeanValue& CMeanValue::operator+= (int nValue)
{
  // check if all values are the same
  if ( m_Count == 0 )
    {
    // first time: set m_FirstValue, m_Min and m_Max
    m_FirstValue = nValue;
    m_Min        = nValue;
    m_Max        = nValue;
    }
  else if ( m_FirstValue != nValue )
    {
    // if any value is different from m_FirstValue: fixed = false
    m_bFixed = false;
    }

  if ( nValue < m_Min )
    m_Min = nValue;

  if ( nValue > m_Max )
    m_Max = nValue;

  m_Sum += nValue;
  m_Count++;

  return *this;
}

/*-------------------------------------------------------------------------*/

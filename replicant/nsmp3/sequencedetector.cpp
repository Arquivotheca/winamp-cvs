/***************************************************************************\
*
*                    MPEG Layer3-Audio Decoder
*                  � 1997-2006 by Fraunhofer IIS
*                        All Rights Reserved
*
*   filename: sequencedetector.cpp
*   project : ---
*   author  : Martin Sieler
*   date    : 1998-02-14
*   contents/description: sequence detector
*
*
\***************************************************************************/

/*
* $Date: 2010/11/17 20:46:05 $
* $Id: sequencedetector.cpp,v 1.1 2010/11/17 20:46:05 audiodsp Exp $
*/

/* ------------------------ includes --------------------------------------*/

#include "sequencedetector.h"
#include <stdlib.h>
/*-------------------------- defines --------------------------------------*/

#ifndef NULL
#define NULL 0
#endif

/*-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
//
//                   CSequenceDetector
//
//-------------------------------------------------------------------------*

//-------------------------------------------------------------------------*
//   constructor
//-------------------------------------------------------------------------*

CSequenceDetector::CSequenceDetector(int nLimit)
{
	m_pArray    = NULL;
	m_pDisabled = NULL;
	m_Limit     = nLimit;

	if ( m_Limit > 0 )
	{
		m_pDisabled = (bool *)malloc(sizeof(bool) * m_Limit);
		m_pArray    = (int *)malloc(sizeof(int) * m_Limit);
	}

	Reset();
}

//-------------------------------------------------------------------------*
//   destructor
//-------------------------------------------------------------------------*

CSequenceDetector::~CSequenceDetector()
{
	if ( m_pArray != NULL )
		free(m_pArray);

	if ( m_pDisabled != NULL )
		free(m_pDisabled);
}

//-------------------------------------------------------------------------*
//   Reset
//-------------------------------------------------------------------------*

void CSequenceDetector::Reset()
{
	for ( int i=0; i<m_Limit; i++ )
	{
		m_pDisabled[i] = false;
		m_pArray[i]    = -1;
	}

	m_Count = 0;
}

//-------------------------------------------------------------------------*
//   SetValue
//-------------------------------------------------------------------------*

CSequenceDetector& CSequenceDetector::operator+= (int nValue)
{
	int Ndx;

	if ( nValue < 0 )
		return *this;

	// set array
	if ( m_Count < m_Limit )
		m_pArray[m_Count] = nValue;

	// check
	for ( int i=0; i<m_Limit; i++ )
	{
		if ( m_pDisabled[i] )
			continue;

		Ndx = m_Count % (i+1);

		if ( m_pArray[Ndx] != nValue )
			m_pDisabled[i] = true;
	}

	m_Count++;

	return *this;
}

//-------------------------------------------------------------------------*
//   GetLength
//-------------------------------------------------------------------------*

int CSequenceDetector::GetLength() const
{
	for ( int i=0; i<m_Limit; i++ )
	{
		if ( !m_pDisabled[i] )
			return (i+1);
	}

	return 0;
}

//-------------------------------------------------------------------------*
//   GetValue
//-------------------------------------------------------------------------*

int CSequenceDetector::GetValue(int nIndex) const
{
	int Len = GetLength();

	if ( Len > 0 && nIndex >= 0 && nIndex < Len )
		return m_pArray[nIndex];
	else
		return -1;
}

//-------------------------------------------------------------------------*
//   GetSum
//-------------------------------------------------------------------------*

int CSequenceDetector::GetSum() const
{
	int Sum = 0;
	int Len = GetLength();

	for ( int i=0; i<Len; i++ )
		Sum += m_pArray[i];

	return Sum;
}

/*-------------------------------------------------------------------------*/

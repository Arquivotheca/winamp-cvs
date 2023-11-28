//---------------------------------------------------------------------------\ 
//
//               (C) copyright Fraunhofer - IIS (2000)
//                        All Rights Reserved
//
//   filename: CVbriHeader.cpp
//             MPEG Layer-3 Audio Decoder
//   author  : Martin Weishart martin.weishart@iis.fhg.de
//   date    : 2000-02-11
//   contents/description: provides functions to read a VBRI header
//                         of a MPEG Layer 3 bitstream encoded 
//                         with variable bitrate using Fraunhofer 
//                         variable bitrate format 
//
//--------------------------------------------------------------------------/

//#include <windows.h>

#include "CVbriHeader.h"
#include "MPEGHeader.h"
#include "foundation/error.h"
#include <stdlib.h>

//---------------------------------------------------------------------------\ 
//
//   Constructor: set position in buffer to parse and create a 
//                VbriHeaderTable
//
//---------------------------------------------------------------------------/

CVbriHeader::CVbriHeader()
{
  position = 0;
  VbriTable=0;
  VbriStreamFrames=0;
	encoderDelay=0;
	samples_per_frame=0;
}

//---------------------------------------------------------------------------\ 
//
//   Destructor: delete a VbriHeaderTable and a VbriHeader
//
//---------------------------------------------------------------------------/

CVbriHeader::~CVbriHeader()
{
  free(VbriTable);
}

uint64_t CVbriHeader::GetSamples() const
{
	uint64_t samples;
	samples = (uint64_t)(VbriStreamFrames-1) * (uint64_t)samples_per_frame;
	samples -= encoderDelay;
	return samples;
}

uint32_t CVbriHeader::GetFrames() const
{
	return VbriStreamFrames; 
}

//---------------------------------------------------------------------------\  
//
//   Method:   checkheader
//             Reads the header to a struct that has to be stored and is 
//             used in other functions to determine file offsets
//   Input:    buffer containing the first frame
//   Output:   fills struct VbriHeader
//   Return:   0 on success; 1 on error
//
//---------------------------------------------------------------------------/

int CVbriHeader::readVbriHeader(const MPEGHeader &frame, const uint8_t *Hbuffer, size_t buffer_length)
{
  unsigned int i, TableLength ;

	position=0;
	SampleRate = frame.GetSampleRate();
	samples_per_frame = frame.GetSamplesPerFrame();

	if (buffer_length < 58)
		return NErr_Insufficient;

  // data indicating silence
  position += 32;
  
  // if a VBRI Header exists read it

  if ( *(Hbuffer+position  ) == 'V' &&
       *(Hbuffer+position+1) == 'B' &&
       *(Hbuffer+position+2) == 'R' &&
       *(Hbuffer+position+3) == 'I'){
    
    position += 4;
		
		//position += 2;
    unsigned int vbriVersion = readFromBuffer(Hbuffer, 2);    // version

    encoderDelay = readFromBuffer(Hbuffer, 2);     // delay

		position += 2;
    //readFromBuffer(Hbuffer, 2);     // quality

    VbriStreamBytes  = readFromBuffer(Hbuffer, 4);
    VbriStreamFrames = readFromBuffer(Hbuffer, 4);
    VbriTableSize    = readFromBuffer(Hbuffer, 2);
    unsigned int VbriTableScale   = readFromBuffer(Hbuffer, 2);
    unsigned int VbriEntryBytes   = readFromBuffer(Hbuffer, 2);
    VbriEntryFrames  = readFromBuffer(Hbuffer, 2);
    
    TableLength = VbriTableSize*VbriEntryBytes;

    if (VbriTableSize > 32768) return 0;
    
		if (position + VbriTableSize*VbriEntryBytes >= buffer_length)
			return NErr_False;

    VbriTable = (int *)malloc(sizeof(int) * (VbriTableSize + 1));

		if (!VbriTable)
			return NErr_OutOfMemory;

		for ( i = 0 ; i <= VbriTableSize ; i++)
		{
			VbriTable[i] = readFromBuffer(Hbuffer, VbriEntryBytes) * VbriTableScale ;
		}
  }
  else
  {
    return NErr_False;
  }
  return NErr_Success;
}



//---------------------------------------------------------------------------\ 
//
//   Method:   seekPointByTime
//             Returns a point in the file to decode in bytes that is nearest 
//             to a given time in seconds
//   Input:    time in seconds
//   Output:   None
//   Returns:  point belonging to the given time value in bytes
//
//---------------------------------------------------------------------------/

uint64_t CVbriHeader::seekPointByTime(double EntryTimeInMilliSeconds) const
{

  unsigned int i=0, fraction = 0;
	uint64_t SeekPoint = 0;

  double TotalDuration ;
  double DurationPerVbriFrames ;
  double AccumulatedTime = 0.0 ;
 
  TotalDuration		= ((double)VbriStreamFrames * (double)samples_per_frame) 
						  / (double)SampleRate * 1000.0 ;
  DurationPerVbriFrames = (double)TotalDuration / (double)(VbriTableSize+1) ;
 
  if ( EntryTimeInMilliSeconds > TotalDuration ) EntryTimeInMilliSeconds = TotalDuration; 
 
  while (AccumulatedTime <= EntryTimeInMilliSeconds)
	{
    SeekPoint	      += VbriTable[i] ;
    AccumulatedTime += DurationPerVbriFrames;
    i++;
  }
  
  // Searched too far; correct result
  fraction = ( (int)(((( AccumulatedTime - EntryTimeInMilliSeconds ) / DurationPerVbriFrames ) 
			 + (1.0/(2.0*(double)VbriEntryFrames))) * (double)VbriEntryFrames));

  
  SeekPoint -= (uint64_t)((double)VbriTable[i-1] * (double)(fraction) 
				 / (double)VbriEntryFrames) ;

  return SeekPoint ;

}

double CVbriHeader::GetLengthSeconds() const
{ 
	if (!VbriStreamFrames || !SampleRate) return 0;

	double nf=VbriStreamFrames-1;
	double sr=SampleRate;
	return nf*(double)samples_per_frame/sr;
}

#if 0
//---------------------------------------------------------------------------\ 
//
//   Method:   seekTimeByPoint
//             Returns a time in the file to decode in seconds that is 
//             nearest to a given point in bytes
//   Input:    time in seconds
//   Output:   None
//   Returns:  point belonging to the given time value in bytes
//
//---------------------------------------------------------------------------/

float CVbriHeader::seekTimeByPoint(unsigned int EntryPointInBytes){

  unsigned int SamplesPerFrame, i=0, AccumulatedBytes = 0, fraction = 0;

  float SeekTime = 0.0f;
  float TotalDuration ;
  float DurationPerVbriFrames ;

  (SampleRate >= 32000) ? (SamplesPerFrame = 1152) : (SamplesPerFrame = 576) ;

  TotalDuration		= ((float)VbriStreamFrames * (float)SamplesPerFrame) 
						  / (float)SampleRate;
  DurationPerVbriFrames = (float)TotalDuration / (float)(VbriTableSize+1) ;
 
  while (AccumulatedBytes <= EntryPointInBytes){
    
    AccumulatedBytes += VbriTable[i] ;
    SeekTime	       += DurationPerVbriFrames;
    i++;
    
  }
  
  // Searched too far; correct result
  fraction = (int)(((( AccumulatedBytes - EntryPointInBytes ) /  (float)VbriTable[i-1]) 
                    + (1/(2*(float)VbriEntryFrames))) * (float)VbriEntryFrames);
  
  SeekTime -= (DurationPerVbriFrames * (float) ((float)(fraction) / (float)VbriEntryFrames)) ;
 
  return SeekTime ;

}

#endif



//---------------------------------------------------------------------------\ 
//
//   Method:   seekPointByPercent
//             Returns a point in the file to decode in bytes that is 
//             nearest to a given percentage of the time of the stream
//   Input:    percent of time
//   Output:   None
//   Returns:  point belonging to the given time percentage value in bytes
//
//---------------------------------------------------------------------------/

uint64_t CVbriHeader::GetSeekPoint(double percent) const
{
	// interpolate in TOC to get file seek point in bytes
	uint32_t a;
	uint64_t seekpoint=0;
	
	if (percent < 0.0)
		percent = 0.0;
	if (percent > 1.0)
		percent=1.0;

	percent*=(double)VbriTableSize;

	a = (int)(percent);
	if (a > VbriTableSize-1) a = VbriTableSize-1;

	
	for (uint32_t i=0;i<a;i++)
	{
		seekpoint+=VbriTable[i];
	}

	if (a != VbriTableSize-1)
		seekpoint += (uint64_t) ((percent - a) * (double)VbriTable[a+1]);

	return seekpoint;
}


//---------------------------------------------------------------------------\ 
//
//   Method:   readFromBuffer
//             reads from a buffer a segment to an int value
//   Input:    Buffer containig the first frame
//   Output:   none
//   Return:   number containing int value of buffer segmenet
//             length
//
//---------------------------------------------------------------------------/

int CVbriHeader::readFromBuffer(const uint8_t *HBuffer, int length)
{

  int i, b, number = 0;
  
  if (HBuffer)
	{
   
    for( i = 0;  i < length ; i++ )
		{ 
      b = length-1-i  ;                                                            
      number = number | (unsigned int)( HBuffer[position+i] & 0xff ) << ( 8*b );

    }
    position += length ;
    return number;

  }
  else{
    return 0;
  }
}



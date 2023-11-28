/* getbits.c, bit level routines                                            */

/*
 * All modifications (mpeg2decode -> mpeg2play) are
 * Copyright (C) 1996, Stefan Eckart. All Rights Reserved.
 */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include <io.h>

static unsigned int mask[8]=
{
		0x1,
		0x3,
		0x7,
		0xF,
		0x1F,
		0x3F,
		0x7F,
		0xFF
};

static unsigned int msk[33] =
{
  0x00000000,0x00000001,0x00000003,0x00000007,
  0x0000000f,0x0000001f,0x0000003f,0x0000007f,
  0x000000ff,0x000001ff,0x000003ff,0x000007ff,
  0x00000fff,0x00001fff,0x00003fff,0x00007fff,
  0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
  0x000fffff,0x001fffff,0x003fffff,0x007fffff,
  0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
  0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
  0xffffffff
};

unsigned int Num_Bits(struct bit_stream *bitstream)
{
	return bitstream->numBits;
}

unsigned char showbits1(BitStream *bitstream) 
{
	unsigned char byte = bitstream->data[0];
	unsigned int count = (bitstream->numBits-1) % 8;
	byte &= mask[count];
	byte >>= count;
	return byte;
}

unsigned int Show_Bits(BitStream *bitstream, int n) 
{
	unsigned int val;
	unsigned int c;
	switch((bitstream->numBits+7) >> 3)
	{
	case 0:
		return 0;
	case 1:
		val=(bitstream->data[0]<<24);
		break;
	case 2:
		val=(bitstream->data[0]<<24) | (bitstream->data[1]<<16);
		break;
	case 3:
		val=(bitstream->data[0]<<24) | (bitstream->data[1]<<16) | (bitstream->data[2]<<8);
		break;
	default:
		val=(bitstream->data[0]<<24) | (bitstream->data[1]<<16) | (bitstream->data[2]<<8) | bitstream->data[3];
		break;
	}
	c = ((bitstream->numBits-1) & 7) + 25;
	return (val>>(c-n)) & msk[n];
}


void Flush_Buffer(BitStream *bitstream, int n)
{
	unsigned int newpos;
	unsigned int oldpos = (bitstream->numBits+7)>>3;
	bitstream->numBits-=n;
	newpos = (bitstream->numBits+7)>>3;
	bitstream->data  += (oldpos - newpos);
}


unsigned int Get_Bits1(BitStream *bitstream)
{
	unsigned char byte = bitstream->data[0];
	unsigned int count = (bitstream->numBits-1) % 8;
	byte &= mask[count];
	byte >>= count;

	bitstream->numBits--;
	if ((bitstream->numBits % 8) == 0)
		bitstream->data++;
	return byte;
}

unsigned int Get_Bits(BitStream *bitstream, int n)
{
	unsigned int val = Show_Bits(bitstream, n);
	Flush_Buffer(bitstream, n);
	return val;
}

/* MPEG-1 system layer demultiplexer */

int Get_Byte(BitStream *bitstream)
{
	return Get_Bits(bitstream, 8);
}

/* extract a 16-bit word from the bitstream buffer */
int Get_Word(BitStream *bitstream)
{
	return Get_Bits(bitstream, 16);
}

void Flush_Buffer32(BitStream *bitstream)
{
	Flush_Buffer(bitstream, 32);
}


unsigned int Get_Bits32(BitStream *bitstream)
{
  unsigned int l;

  l = Show_Bits(bitstream, 32);
  Flush_Buffer32(bitstream);

  return l;
}


int Get_Long(BitStream *bitstream)
{
  int i;

  i = Get_Word(bitstream);
  return (i<<16) | Get_Word(bitstream);
}



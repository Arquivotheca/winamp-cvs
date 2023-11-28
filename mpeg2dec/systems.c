/* systems.c, systems-specific routines                                 */

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

#include "config.h"
#include "global.h"

/* initialize buffer, call once before first getbits or showbits */

/* parse system layer, ignore everything we don't need */
void Next_Packet(BitStream *bitstream)
{
  unsigned int code;

  for(;;)
  {
    code = Get_Long(bitstream);

    /* remove system layer byte stuffing */
    while ((code & 0xffffff00) != 0x100)
      code = (code<<8) | Get_Byte(bitstream);

    switch(code)
    {
    case PACK_START_CODE: /* pack header */
      /* skip pack header (system_clock_reference and mux_rate) */
			Flush_Buffer(bitstream, 8*16);
      break;
    case VIDEO_ELEMENTARY_STREAM:   
      code = Get_Word(bitstream);             /* packet_length */
// TODO?      decoder->ld->Rdmax = decoder->ld->Rdptr + code;

      code = Get_Byte(bitstream);

      if((code>>6)==0x02)
      {
				Flush_Buffer(bitstream, 8);
        code=Get_Byte(bitstream);  /* parse PES_header_data_length */
				Flush_Buffer(bitstream, 8*code); /* advance pointer by PES_header_data_length */
        printf("MPEG-2 PES packet\n");
        return;
      }
      else if(code==0xff)
      {
        /* parse MPEG-1 packet header */
        while((code=Get_Byte(bitstream))== 0xFF);
      }
       
      /* stuffing bytes */
      if(code>=0x40)
      {
        if(code>=0x80)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(1);
        }
        /* skip STD_buffer_scale */
				Flush_Buffer(bitstream, 8);
        code = Get_Byte(bitstream);
      }

      if(code>=0x30)
      {
        if(code>=0x40)
        {
          fprintf(stderr,"Error in packet header\n");
          exit(1);
        }
        /* skip presentation and decoding time stamps */
				Flush_Buffer(bitstream, 9 * 8);
      }
      else if(code>=0x20)
      {
        /* skip presentation time stamps */
				Flush_Buffer(bitstream, 4 * 8);
      }
      else if(code!=0x0f)
      {
        fprintf(stderr,"Error in packet header\n");
        exit(1);
      }
      return;
    case ISO_END_CODE: /* end */
      /* simulate a buffer full of sequence end codes */
#if 0 // tODO???
      l = 0;
      while (l<2048)
      {
        decoder->ld->Rdbfr[l++] = SEQUENCE_END_CODE>>24;
        decoder->ld->Rdbfr[l++] = SEQUENCE_END_CODE>>16;
        decoder->ld->Rdbfr[l++] = SEQUENCE_END_CODE>>8;
        decoder->ld->Rdbfr[l++] = SEQUENCE_END_CODE&0xff;
      }
      decoder->ld->Rdptr = decoder->ld->Rdbfr;
      decoder->ld->Rdmax = decoder->ld->Rdbfr + 2048;
#endif
      return;
    default:
      if(code>=SYSTEM_START_CODE)
      {
        /* skip system headers and non-video packets*/
        code = Get_Word(bitstream);
				Flush_Buffer(bitstream, code * 8);
        //decoder->ld->Rdptr += code;
      }
      else
      {
        fprintf(stderr,"Unexpected startcode %08x in system layer\n",code);
        exit(1);
      }
      break;
    }
  }
}




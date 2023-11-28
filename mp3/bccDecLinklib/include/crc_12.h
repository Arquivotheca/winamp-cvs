/***************************************************************************\
*                           Fraunhofer IIS 
*                  (c) 1996 - 2008 Fraunhofer IIS
*                         All Rights Reserved.
*
*
*
*   This software and/or program is protected by copyright law and
*   international treaties. Any reproduction or distribution of this
*   software and/or program, or any portion of it, may result in severe
*   civil and criminal penalties, and will be prosecuted to the maximum
*   extent possible under law.
*
\***************************************************************************/

#ifndef _CRC12_H_
#define _CRC12_H_

#define CRCPOLY 0x80f /* x^12 + x^11 + x^3 + x^2 + x + 1 */

#if 0
/* this two functions are for generating and
   printing the lookup table */
void BuildCrc_12Table(void);
void PrintCrc_12Table(void);
#endif

unsigned short Crc_12(unsigned char *data, int len);

#endif /* _CRC12_H_ */

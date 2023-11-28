#include "bitbuffer.h"
#include "aacdec/bitstream.h"

void PCE_Read(HANDLE_BIT_BUF bs,
	       long *byteBorder)
{
	unsigned long i;
		unsigned long front_ce_count, side_ce_count, back_ce_count, lfe_ce_count;
		unsigned long associated_data_elements_count, cc_count;
		unsigned long comment_field_bytes;
		
		ReadBits(bs, 4); /* element instance tag */
		ReadBits(bs, 2); /* object type */
		ReadBits(bs, 4); /* sf index */
		front_ce_count = ReadBits(bs, 4); /* number of front CEs */
		side_ce_count = ReadBits(bs, 4); /* number of side CEs */
		back_ce_count = ReadBits(bs, 4); /* number of back CEs */
		lfe_ce_count = ReadBits(bs, 2); /* number of LFE CEs */
		associated_data_elements_count = ReadBits(bs, 3); /* number of associated data elements */
		cc_count = ReadBits(bs, 4); /* valid CC elements */
		
		if (ReadBits(bs, 1))  /* mono mixdown */
			ReadBits(bs, 4); /* mono mixdown element number */
		if (ReadBits(bs, 1)) /* stereo mixdown */
			ReadBits(bs, 4); /* stereo mixdown element number */
		if (ReadBits(bs, 1)) /* matrix index flag */
		{
			ReadBits(bs, 2); /* matrix mixdown index */
			ReadBits(bs, 1); /* enable pseudo-surround */
		}
		
    for (i = 0; i < front_ce_count; i++)
    {
			ReadBits(bs, 1); /* is CPE */
			ReadBits(bs, 4); /* tag select */
    }
    for (i = 0; i < side_ce_count; i++)
    {
			ReadBits(bs, 1); /* is CPE */
			ReadBits(bs, 4); /* tag select */
    }
    for (i = 0; i < back_ce_count; i++)
    {
			ReadBits(bs, 1); /* is CPE */
			ReadBits(bs, 4); /* tag select */
    }
    for (i = 0; i < lfe_ce_count; i++)
    {
			ReadBits(bs, 4);
    }
    for (i = 0; i < associated_data_elements_count; i++)
			ReadBits(bs, 4); /* tag select */
    for (i = 0; i < cc_count; i++)
    {
			ReadBits(bs, 1); /* is ind sw */
			ReadBits(bs, 4); /* tag select */
    }
		ByteAlign(bs, byteBorder);
    //faad_byte_align(ld);
		comment_field_bytes = ReadBits(bs, 8);
		
    
    for (i = 0; i < comment_field_bytes; i++)
    {
			ReadBits(bs, 8);
    }
}
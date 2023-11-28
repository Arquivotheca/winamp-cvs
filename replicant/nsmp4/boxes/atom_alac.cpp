#include "mp4common.h"

MP4AlacAtom::MP4AlacAtom(MP4Atom *parent, const char *type) : MP4Atom(parent, type)
{
	AddVersionAndFlags(); /* 0, 1 */

	AddProperty(new MP4BytesProperty("decoderConfig")); /* 2 */
}

void MP4AlacAtom::Read() 
{
	// calculate size of the configuration data from the atom size
	((MP4BytesProperty*)m_pProperties[2])->SetValueSize(m_size-4);
	MP4Atom::Read();
}
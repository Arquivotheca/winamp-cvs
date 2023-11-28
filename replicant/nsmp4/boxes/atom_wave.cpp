#include "mp4common.h"

/* this is for the weird wave atom in quicktime files */
MP4WaveAtom::MP4WaveAtom(MP4Atom *parent, const char *type) : MP4Atom(parent, type) 
{
  ExpectChildAtom("frma", Optional, Optional);
	ExpectChildAtom("mp4a", Optional, Optional);
	ExpectChildAtom("esds", Optional, Optional);
}

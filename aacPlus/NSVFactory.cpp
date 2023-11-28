#include "NSVFactory.h"
#include "nsvdec.h"
#include "api.h"
#include "../nsv/nsvlib.h"

IAudioDecoder *NSVFactory::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
	switch (format)
	{
	case NSV_MAKETYPE('A', 'A', 'C', ' ') :							
	case NSV_MAKETYPE('A', 'A', 'C', 'P'):
	case NSV_MAKETYPE('A', 'P', 'L', ' '):
		{
			EAACP_Decoder *dec;
			WASABI_API_MEMMGR->New(&dec);
			if (!dec->OK())
			{
				WASABI_API_MEMMGR->Delete(dec);
				dec = 0;
			}
			return dec;
		}

	default:
		return 0;
	}
}


#define CBCLASS NSVFactory
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEAUDIODECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS
#ifndef NULLSOFT_AACPLUSDECODER_NSVFACTORY_H
#define NULLSOFT_AACPLUSDECODER_NSVFACTORY_H

#include "../nsv/svc_nsvFactory.h"

class NSVFactory : public svc_nsvFactory
{
public:
	IAudioDecoder *CreateAudioDecoder(FOURCC format, IAudioOutput **output);

protected:
	RECVS_DISPATCH;
};

#endif
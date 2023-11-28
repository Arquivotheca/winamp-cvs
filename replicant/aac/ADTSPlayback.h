#include "audio/ifc_audioout.h"
#include "nswasabi/PlaybackBase.h"
#include "nsaac/nsaac.h"

// implements local file playback
class ADTSPlayback : public PlaybackBase
{
public:
	ADTSPlayback(nx_string_t filename, ifc_player *player);
	~ADTSPlayback();

private:
	ifc_audioout *out;
	FILE *adts_file;
	nsaac_decoder_t decoder;
	volatile int paused;
	volatile int stopped;

	unsigned int bps;
	unsigned int sample_rate;
private:
	int Init();

	/* Thread function */
	static nx_thread_return_t NXTHREADCALL ADTSPlayerThreadFunction(nx_thread_parameter_t param);
	nx_thread_return_t NXTHREADCALL DecodeLoop();
};
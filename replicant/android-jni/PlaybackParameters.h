#pragma once
#include "replaygain/ifc_replaygain_settings.h"
#include "player/ifc_playback_parameters.h"
#include "android-audio-pro/ifc_audiotrackpro_settings.h"

class PlaybackParameters : public ifc_playback_parameters, public ifc_replaygain_settings, public ifc_audiotrackpro_settings
{
public:
	PlaybackParameters();

	/* replaygain setters */
	void ReplayGain_SetFlags(int new_flags); // set everything at once (for deserialization)
	void ReplayGain_SetState(bool status); // true to turn on, false to turn off
	void ReplayGain_SetMode(int new_mode); // set just the mode (REPLAYGAIN_OFF, REPLAYGAIN_MODE_TRACK or REPLAYGAIN_MODE_ALBUM)
	void ReplayGain_SetClippingPrevention(bool prevent_clipping); // set just the clipping prevention (true or false)
	void ReplayGain_SetAuto(bool auto_replaygain); // set just auto-mode on or off (true or false)
	void ReplayGain_SetDefaultGain(double new_gain);
	void ReplayGain_SetPreamp(double new_preamp);

	/* replaygain getters */
	int ReplayGain_GetFlags(); // gets all the flags at once (for serialization)
	bool ReplayGain_GetState(); // true if on, false if off
	int ReplayGain_GetMode(); // get just the mode (REPLAYGAIN_OFF, REPLAYGAIN_MODE_TRACK or REPLAYGAIN_MODE_ALBUM)
	bool ReplayGain_GetClippingPrevention();  // get just the clipping prevention (true or false)
	bool ReplayGain_GetAuto(); // get just auto-mode on or off (true or false)
	double ReplayGain_GetDefaultGain();
	double ReplayGain_GetPreamp();

	/* crossfade setters */
	void Crossfade_SetStatus(bool new_status);
	void Crossfade_SetTime(double new_time);
	
	/* crossfade getters */
	bool Crossfade_GetStatus();
	double Crossfade_GetTime();
private:
	volatile int replaygain_flags;
	volatile float replaygain_default_gain;
	volatile float replaygain_preamp;

	volatile bool crossfade_status;
	volatile float crossfade_time;

	void GetDefaultGain(double *gain);
private:
	/* Dispatchable */
	int WASABICALL Dispatchable_QueryInterface(GUID interface_guid, void **object);

	/* ifc_replaygain_settings */
	int WASABICALL ReplayGainSettings_GetGain(ifc_metadata *metadata, double *gain, int *warning);
	int WASABICALL ReplayGainSettings_AddToHistory(double seconds, double gain);
	
	/* ifc_audiotrackpro_settings */
	int WASABICALL AudioTrackProSettings_GetCrossfadeStatus();
	int WASABICALL AudioTrackProSettings_GetCrossfadeTime(double *seconds);
};

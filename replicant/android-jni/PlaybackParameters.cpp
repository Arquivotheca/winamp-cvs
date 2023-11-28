#include "PlaybackParameters.h"
#include "foundation/error.h"
#include "metadata/ifc_metadata.h"
#include "metadata/MetadataKeys.h"
#include <math.h>

PlaybackParameters::PlaybackParameters()
{
	replaygain_flags = REPLAYGAIN_OFF|REPLAYGAIN_MODE_ALBUM; /* off, but album mode is default if it gets turned on */
	replaygain_default_gain = 0.5;
	replaygain_preamp = 1.0;

	crossfade_status = 0;
	crossfade_time = 2.0;
}

int PlaybackParameters::Dispatchable_QueryInterface(GUID interface_guid, void **object)
{
	if (interface_guid == ifc_audiotrackpro_settings::GetInterfaceGUID())
	{
		*object = (ifc_audiotrackpro_settings *)this;
		ifc_audiotrackpro_settings::Retain();
		return NErr_Success;
	}
	else if (interface_guid == ifc_replaygain_settings::GetInterfaceGUID())
	{
		*object = (ifc_replaygain_settings *)this;
		ifc_replaygain_settings::Retain();
		return NErr_Success;
	}

	return NErr_Unknown;
}

void PlaybackParameters::ReplayGain_SetFlags(int new_flags)
{
	replaygain_flags = new_flags;
}

void PlaybackParameters::ReplayGain_SetState(bool status)
{
	int flags = replaygain_flags; /* save a local copy so it doesn't change out from under us, and we don't do partial updates */
	flags &= ~REPLAYGAIN_ON; // clear flags for both modes
	if (status)
	flags |= REPLAYGAIN_ON;
	replaygain_flags = flags; /* store value */
}

void PlaybackParameters::ReplayGain_SetMode(int new_mode)
{
	int flags = replaygain_flags; /* save a local copy so it doesn't change out from under us, and we don't do partial updates */
	flags &= ~REPLAYGAIN_MODE_MASK; // clear flags for both modes
	flags |= new_mode;
	replaygain_flags = flags; /* store value */
}

void PlaybackParameters::ReplayGain_SetClippingPrevention(bool prevent_clipping)
{
	int flags = replaygain_flags; /* save a local copy so it doesn't change out from under us, and we don't do partial updates */
	if (prevent_clipping)
		flags |= REPLAYGAIN_PREVENT_CLIPPING; // set flag
	else
		flags &= ~REPLAYGAIN_PREVENT_CLIPPING; // clear flag
	replaygain_flags = flags; /* store value */
}

void PlaybackParameters::ReplayGain_SetAuto(bool auto_replaygain)
{
	int flags = replaygain_flags; /* save a local copy so it doesn't change out from under us, and we don't do partial updates */
	if (auto_replaygain)
		flags |= REPLAYGAIN_AUTO; // set flag
	else
		flags &= ~REPLAYGAIN_AUTO; // clear flag
	replaygain_flags = flags; /* store value */
}

void PlaybackParameters::ReplayGain_SetDefaultGain(double new_gain)
{
	replaygain_default_gain = new_gain;
}

void PlaybackParameters::ReplayGain_SetPreamp(double new_preamp)
{
	replaygain_preamp = new_preamp;
}

int PlaybackParameters::ReplayGain_GetFlags()
{
	return replaygain_flags;
}

bool PlaybackParameters::ReplayGain_GetState()
{
	return !!(replaygain_flags & REPLAYGAIN_ON);
}

int PlaybackParameters::ReplayGain_GetMode()
{
	return replaygain_flags & REPLAYGAIN_MODE_MASK;
}

bool PlaybackParameters::ReplayGain_GetClippingPrevention()
{
	if (replaygain_flags & REPLAYGAIN_PREVENT_CLIPPING)
		return true;
	else
		return false;
}

bool PlaybackParameters::ReplayGain_GetAuto()
{
	if (replaygain_flags & REPLAYGAIN_AUTO)
		return true;
	else
		return false;
}

double PlaybackParameters::ReplayGain_GetDefaultGain()
{
	return replaygain_default_gain;
}

double PlaybackParameters::ReplayGain_GetPreamp()
{
	return replaygain_preamp;
}

void PlaybackParameters::Crossfade_SetStatus(bool new_status)
{
	crossfade_status = new_status;
}

void PlaybackParameters::Crossfade_SetTime(double new_time)
{
	crossfade_time = new_time;
}

bool PlaybackParameters::Crossfade_GetStatus()
{
	return crossfade_status;
}

double PlaybackParameters::Crossfade_GetTime()
{
	return crossfade_time;
}

void PlaybackParameters::GetDefaultGain(double *gain)
{
	/* TODO: auto mode */
	*gain = replaygain_default_gain * replaygain_preamp;
}

int PlaybackParameters::ReplayGainSettings_GetGain(ifc_metadata *metadata, double *gain, int *warning)
{
	int flags = replaygain_flags; /* save a local copy so it doesn't change out from under us */
	if (!ReplayGain_GetState())
		return NErr_Disabled;

	if (!metadata)
	{
		GetDefaultGain(gain);
		if (warning) *warning = NErr_Unknown;
		return NErr_Success;
	}

	double adjustment, peak;

	if (flags & REPLAYGAIN_MODE_ALBUM)
	{
		int ret = metadata->GetReal(MetadataKeys::ALBUM_GAIN, 0, &adjustment);
		if (ret == NErr_Success) /* if there's no album data, we'll just fall through and run in track flags */
		{
			double rg = pow(10.0f, adjustment / 20.0f) * replaygain_preamp;

			if ((flags & REPLAYGAIN_PREVENT_CLIPPING) && metadata->GetReal(MetadataKeys::ALBUM_PEAK, 0, &peak) == NErr_Success)
			{
				if (rg > 1.0/peak)
					rg = 1.0/peak;

				*gain = rg;
				if (warning) *warning = NErr_Success;
				return NErr_Success;
			}
			else
			{
				*gain = rg;
				if (warning) *warning = NErr_Success;
				return NErr_Success;
			}	
		}
	}

	/* track flags is assumed from not being album flags, simplifies the fall-through logic*/
	int ret = metadata->GetReal(MetadataKeys::TRACK_GAIN, 0, &adjustment);
	if (ret == NErr_Empty) /* no track data */
	{
		GetDefaultGain(gain);
		if (warning) *warning=NErr_False;
		return NErr_Success;
	}
	else if (ret != NErr_Success) /* key not understood or other error*/
	{
		GetDefaultGain(gain);
		if (warning) *warning=NErr_Unknown;
		return NErr_Success;
	}

	double rg = pow(10.0f, adjustment / 20.0f) * replaygain_preamp;

	if ((flags & REPLAYGAIN_PREVENT_CLIPPING) && metadata->GetReal(MetadataKeys::TRACK_PEAK, 0, &peak) == NErr_Success)
	{
		if (rg > 1.0/peak)
			rg = 1.0/peak;

		*gain = rg;
		if (warning) *warning = NErr_Success;
		return NErr_Success;
	}
	else
	{
		*gain = rg;
		if (warning) *warning = NErr_Success;
		return NErr_Success;
	}	
}

int PlaybackParameters::ReplayGainSettings_AddToHistory(double seconds, double gain)
{
	return NErr_NotImplemented;
}

int PlaybackParameters::AudioTrackProSettings_GetCrossfadeStatus()
{
	if (crossfade_status)
		return NErr_True;
	else
		return NErr_False;
}

int PlaybackParameters::AudioTrackProSettings_GetCrossfadeTime(double *seconds)
{
	*seconds = crossfade_time;
	if (crossfade_status)
		return NErr_True;
	else
		return NErr_False;
}

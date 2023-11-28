#pragma once
#include "foundation/dispatch.h"
#include "metadata/ifc_metadata.h"
#include "audio/ifc_equalizer.h"
#include "nu/PtrDeque.h"
#include "nx/nxuri.h"
/*
You will be potentially be called on a high priority thread.
So if you need to anything 'expensive' you should queue something up
on the command thread or the UI thread.

Note that this base class inherits from PtrDequeNode.  This is for Player's usage! 
Do not try to add it to your own PtrDeque !
*/

class NOVTABLE cb_playerevents : public Wasabi2::Dispatchable, public nu::PtrDequeNode
{
protected:
	cb_playerevents() : Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_playerevents() {}
public:
	void OnLengthChanged(double new_length) {  PlayerEvents_OnLengthChanged(new_length); }
	void OnPositionChanged(double new_position) {  PlayerEvents_OnPositionChanged(new_position); }
	void OnMetadataChanged(ifc_metadata *metadata) {  PlayerEvents_OnMetadataChanged(metadata); }
	void OnEqualizerChanged(ifc_equalizer *equalizer) {  PlayerEvents_OnEqualizerChanged(equalizer); }
	void OnLoaded(nx_uri_t filename) {  PlayerEvents_OnLoaded(filename); }
	void OnInitialized() {  PlayerEvents_OnInitialized(); }
	void OnError(int error_code) { PlayerEvents_OnError(error_code); }
	void OnEndOfFile() { PlayerEvents_OnEndOfFile(); }
	void OnSeekComplete(int error_code, double new_position) { PlayerEvents_OnSeekComplete(error_code, new_position); }
	void OnSeekable(int is_seekable) { PlayerEvents_OnSeekable(is_seekable); }
	void OnBuffering(int percent) { PlayerEvents_OnBuffering(percent); }
	void OnStopped() { PlayerEvents_OnStopped(); }
	void OnReady() { PlayerEvents_OnReady(); }
	void OnClosed() { PlayerEvents_OnClosed(); }
	void OnBitrateChanged(double new_bitrate) { PlayerEvents_OnBitrateChanged(new_bitrate); }
	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual void WASABICALL PlayerEvents_OnLengthChanged(double new_length)=0;
	virtual void WASABICALL PlayerEvents_OnPositionChanged(double new_position)=0;
	virtual void WASABICALL PlayerEvents_OnMetadataChanged(ifc_metadata *metadata)=0;
	virtual void WASABICALL PlayerEvents_OnEqualizerChanged(ifc_equalizer *equalizer)=0;
	virtual void WASABICALL PlayerEvents_OnLoaded(nx_uri_t filename)=0;
	/* this one gets called on the Player thread, this callback can be useful if you need to know the thread ID, e.g. */
	virtual void WASABICALL PlayerEvents_OnInitialized()=0;
	virtual void WASABICALL PlayerEvents_OnError(int error_code)=0;
	virtual void WASABICALL PlayerEvents_OnEndOfFile()=0;
	virtual void WASABICALL PlayerEvents_OnSeekComplete(int error_code, double new_position)=0;
	virtual void WASABICALL PlayerEvents_OnSeekable(int is_seekable)=0;
	virtual void WASABICALL PlayerEvents_OnBuffering(int percent)=0;
	virtual void WASABICALL PlayerEvents_OnStopped()=0;
	virtual void WASABICALL PlayerEvents_OnReady()=0;
	virtual void WASABICALL PlayerEvents_OnClosed()=0;
	virtual void WASABICALL PlayerEvents_OnBitrateChanged(double new_bitrate)=0;
};

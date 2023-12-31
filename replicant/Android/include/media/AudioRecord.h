/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIORECORD_H_
#define AUDIORECORD_H_

#include <stdint.h>
#include <sys/types.h>

#include <media/IAudioFlinger.h>
#include <media/IAudioRecord.h>
#include <media/AudioTrack.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <utils/IInterface.h>
#include <utils/IMemory.h>
#include <utils/threads.h>


namespace android {

// ----------------------------------------------------------------------------

class AudioRecord
{
public:

    // input sources values must always be defined in the range
    // [AudioRecord::DEFAULT_INPUT, AudioRecord::NUM_INPUT_SOURCES[
    enum input_source {
        DEFAULT_INPUT   =-1,
        MIC_INPUT       = 0,
        VOICE_UPLINK_INPUT = 1,
        VOICE_DOWNLINK_INPUT = 2,
        VOICE_CALL_INPUT = 3,
        NUM_INPUT_SOURCES
    };

    static const int DEFAULT_SAMPLE_RATE = 8000;

    /* Events used by AudioRecord callback function (callback_t).
     * 
     * to keep in sync with frameworks/base/media/java/android/media/AudioRecord.java
     */
    enum event_type {
        EVENT_MORE_DATA = 0,        // Request to reqd more data from PCM buffer.
        EVENT_OVERRUN = 1,          // PCM buffer overrun occured.
        EVENT_MARKER = 2,           // Record head is at the specified marker position
                                    // (See setMarkerPosition()).
        EVENT_NEW_POS = 3,          // Record head is at a new position 
                                    // (See setPositionUpdatePeriod()).
    };

    /* Create Buffer on the stack and pass it to obtainBuffer()
     * and releaseBuffer().
     */

    class Buffer
    {
    public:
        enum {
            MUTE    = 0x00000001
        };
        uint32_t    flags;
        int         channelCount;
        int         format;
        size_t      frameCount;
        size_t      size;
        union {
            void*       raw;
            short*      i16;
            int8_t*     i8;
        };
    };

    /* These are static methods to control the system-wide AudioFlinger
     * only privileged processes can have access to them
     */

//    static status_t setMasterMute(bool mute);

    /* As a convenience, if a callback is supplied, a handler thread
     * is automatically created with the appropriate priority. This thread
     * invokes the callback when a new buffer becomes ready or an overrun condition occurs.
     * Parameters:
     *
     * event:   type of event notified (see enum AudioRecord::event_type).
     * user:    Pointer to context for use by the callback receiver.
     * info:    Pointer to optional parameter according to event type:
     *          - EVENT_MORE_DATA: pointer to AudioRecord::Buffer struct. The callback must not read
     *          more bytes than indicated by 'size' field and update 'size' if less bytes are
     *          read.
     *          - EVENT_OVERRUN: unused.
     *          - EVENT_MARKER: pointer to an uin32_t containing the marker position in frames.
     *          - EVENT_NEW_POS: pointer to an uin32_t containing the new position in frames.
     */

    typedef void (*callback_t)(int event, void* user, void *info);

    /* Constructs an uninitialized AudioRecord. No connection with
     * AudioFlinger takes place.
     */
                        AudioRecord();

    /* Creates an AudioRecord track and registers it with AudioFlinger.
     * Once created, the track needs to be started before it can be used.
     * Unspecified values are set to the audio hardware's current
     * values.
     *
     * Parameters:
     *
     * inputSource:        Select the audio input to record to (e.g. AudioRecord::MIC_INPUT).
     * sampleRate:         Track sampling rate in Hz.
     * format:             PCM sample format (e.g AudioSystem::PCM_16_BIT for signed
     *                     16 bits per sample).
     * channelCount:       Number of PCM channels (e.g 2 for stereo).
     * frameCount:         Total size of track PCM buffer in frames. This defines the
     *                     latency of the track.
     * flags:              A bitmask of acoustic values from enum record_flags.  It enables
     *                     AGC, NS, and IIR.
     * cbf:                Callback function. If not null, this function is called periodically
     *                     to provide new PCM data.
     * notificationFrames: The callback function is called each time notificationFrames PCM
     *                     frames are ready in record track output buffer.
     * user                Context for use by the callback receiver.
     */

     enum record_flags {
         RECORD_AGC_ENABLE = AudioSystem::AGC_ENABLE,
         RECORD_NS_ENABLE  = AudioSystem::NS_ENABLE,
         RECORD_IIR_ENABLE = AudioSystem::TX_IIR_ENABLE
     };

                        AudioRecord(int inputSource,
                                    uint32_t sampleRate = 0,
                                    int format          = 0,
                                    int channelCount    = 0,
                                    int frameCount      = 0,
                                    uint32_t flags      = 0,
                                    callback_t cbf = 0,
                                    void* user = 0,
                                    int notificationFrames = 0);


    /* Terminates the AudioRecord and unregisters it from AudioFlinger.
     * Also destroys all resources assotiated with the AudioRecord.
     */
                        ~AudioRecord();


    /* Initialize an uninitialized AudioRecord.
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful intialization
     *  - INVALID_OPERATION: AudioRecord is already intitialized or record device is already in use
     *  - BAD_VALUE: invalid parameter (channelCount, format, sampleRate...)
     *  - NO_INIT: audio server or audio hardware not initialized
     *  - PERMISSION_DENIED: recording is not allowed for the requesting process
     * */
            status_t    set(int inputSource     = 0,
                            uint32_t sampleRate = 0,
                            int format          = 0,
                            int channelCount    = 0,
                            int frameCount      = 0,
                            uint32_t flags      = 0,
                            callback_t cbf = 0,
                            void* user = 0,
                            int notificationFrames = 0,
                            bool threadCanCallJava = false);


    /* Result of constructing the AudioRecord. This must be checked
     * before using any AudioRecord API (except for set()), using
     * an uninitialized AudioRecord produces undefined results.
     * See set() method above for possible return codes.
     */
            status_t    initCheck() const;

    /* Returns this track's latency in milliseconds.
     * This includes the latency due to AudioRecord buffer size
     * and audio hardware driver.
     */
            uint32_t     latency() const;

   /* getters, see constructor */

            int         format() const;
            int         channelCount() const;
            uint32_t    frameCount() const;
            int         frameSize() const;
            int         inputSource() const;


    /* After it's created the track is not active. Call start() to
     * make it active. If set, the callback will start being called.
     */
            status_t    start();

    /* Stop a track. If set, the callback will cease being called and
     * obtainBuffer returns STOPPED. Note that obtainBuffer() still works
     * and will fill up buffers until the pool is exhausted.
     */
            status_t    stop();
            bool        stopped() const;

    /* get sample rate for this record track
     */
            uint32_t    getSampleRate();

    /* Sets marker position. When record reaches the number of frames specified,
     * a callback with event type EVENT_MARKER is called. Calling setMarkerPosition
     * with marker == 0 cancels marker notification callback. 
     * If the AudioRecord has been opened with no callback function associated, 
     * the operation will fail.
     *
     * Parameters:
     *
     * marker:   marker position expressed in frames.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioRecord has no callback installed.
     */
            status_t    setMarkerPosition(uint32_t marker);
            status_t    getMarkerPosition(uint32_t *marker);


    /* Sets position update period. Every time the number of frames specified has been recorded, 
     * a callback with event type EVENT_NEW_POS is called. 
     * Calling setPositionUpdatePeriod with updatePeriod == 0 cancels new position notification 
     * callback. 
     * If the AudioRecord has been opened with no callback function associated,
     * the operation will fail.
     *
     * Parameters:
     *
     * updatePeriod:  position update notification period expressed in frames.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - INVALID_OPERATION: the AudioRecord has no callback installed.
     */
            status_t    setPositionUpdatePeriod(uint32_t updatePeriod);
            status_t    getPositionUpdatePeriod(uint32_t *updatePeriod);


    /* Gets record head position. The position is the  total number of frames 
     * recorded since record start. 
     *
     * Parameters:
     *
     *  position:  Address where to return record head position within AudioRecord buffer.
     *
     * Returned status (from utils/Errors.h) can be:
     *  - NO_ERROR: successful operation
     *  - BAD_VALUE:  position is NULL
     */
            status_t    getPosition(uint32_t *position);

            
            
    /* obtains a buffer of "frameCount" frames. The buffer must be
     * filled entirely. If the track is stopped, obtainBuffer() returns
     * STOPPED instead of NO_ERROR as long as there are buffers availlable,
     * at which point NO_MORE_BUFFERS is returned.
     * Buffers will be returned until the pool (buffercount())
     * is exhausted, at which point obtainBuffer() will either block
     * or return WOULD_BLOCK depending on the value of the "blocking"
     * parameter.
     */

        enum {
            NO_MORE_BUFFERS = 0x80000001,
            STOPPED = 1
        };

            status_t    obtainBuffer(Buffer* audioBuffer, int32_t waitCount);
            void        releaseBuffer(Buffer* audioBuffer);


    /* As a convenience we provide a read() interface to the audio buffer.
     * This is implemented on top of lockBuffer/unlockBuffer.
     */
            ssize_t     read(void* buffer, size_t size);

private:
    /* copying audio tracks is not allowed */
                        AudioRecord(const AudioRecord& other);
            AudioRecord& operator = (const AudioRecord& other);

    /* a small internal class to handle the callback */
    class ClientRecordThread : public Thread
    {
    public:
        ClientRecordThread(AudioRecord& receiver, bool bCanCallJava = false);
    private:
        friend class AudioRecord;
        virtual bool        threadLoop();
        virtual status_t    readyToRun() { return NO_ERROR; }
        virtual void        onFirstRef() {}
        AudioRecord& mReceiver;
        Mutex       mLock;
    };

            bool processAudioBuffer(const sp<ClientRecordThread>& thread);

    sp<IAudioRecord>        mAudioRecord;
    sp<IMemory>             mCblkMemory;
    sp<ClientRecordThread>  mClientRecordThread;
    Mutex                   mRecordThreadLock;

    uint32_t                mFrameCount;

    audio_track_cblk_t*     mCblk;
    uint8_t                 mFormat;
    uint8_t                 mChannelCount;
    uint8_t                 mInputSource;
    uint8_t                 mReserved;
    status_t                mStatus;
    uint32_t                mLatency;

    volatile int32_t        mActive;

    callback_t              mCbf;
    void*                   mUserData;
    uint32_t                mNotificationFrames;
    uint32_t                mRemainingFrames;
    uint32_t                mMarkerPosition;
    bool                    mMarkerReached;
    uint32_t                mNewPosition;
    uint32_t                mUpdatePeriod;
};

}; // namespace android

#endif /*AUDIORECORD_H_*/

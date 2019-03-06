/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

// aace/communication/AlexaComms.java
// This is an automatically generated file.

package com.amazon.aace.communication;

import com.amazon.aace.alexa.AudioChannel;
import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Speaker;
import com.amazon.aace.core.PlatformInterface;


/**
 * The @c AlexaComms class should be extended by the platform implementation to participate in Communications via Alexa.
 */
abstract public class AlexaComms extends PlatformInterface
{
    /// An enumeration representing the state of a call.
    public enum CallState {
        /// The call is connecting.
        CONNECTING,
        /// An incoming call is causing a ringtone to be played.
        INBOUND_RINGING,
        /// The call has successfully connected.
        CALL_CONNECTED,
        /// The call has ended.
        CALL_DISCONNECTED,
        /// No current call state to be relayed to the user.
        NONE
    };

    /**
     * Constructs @c AlexaComms object
     *
     * @param ringtoneMediaPlayer @c MediaPlayer to use for playing ringtone.
     * @param ringtoneSpeaker @c Speaker to control ringtone volume.
     * @param callAudioMediaPlayer @ MediaPlayer to use for playing call audio coming from other end.
     * @param callAudioSpeaker @ Speaker to control call audio volume.
     */
    public AlexaComms(
            MediaPlayer ringtoneMediaPlayer,
            Speaker ringtoneSpeaker,
            MediaPlayer callAudioMediaPlayer,
            Speaker callAudioSpeaker) {
        m_ringtoneMediaPlayer = ringtoneMediaPlayer;
        m_ringtoneSpeaker = ringtoneSpeaker;
        m_callAudioMediaPlayer = callAudioMediaPlayer;
        m_callAudioSpeaker = callAudioSpeaker;

        // AlexaCommsLib relies upon JNI OnLoad callback to initialize itself
        // hence the explicit load of the lib here
        System.loadLibrary("AlexaCommsLib");
    }

    /// @name aace::communication::AlexaComms Functions
    /// @{
    /**
     * Allows the observer to react to a change in call state.
     *
     * @param state The new CallState.
     */
    protected abstract void callStateChanged(CallState state);
    /// @}

    /**
     * Accepts an incoming call.
     */
    public void acceptCall() {
        acceptCall( getNativeObject() );
    }

    /**
     * Stops the call.
     */
    public void stopCall() {
        stopCall( getNativeObject() );
    }

    /**
     * Writes audio samples to the Engine for streaming it to the other end of a connected Alexa
     * Comms call.
     * Audio samples should typically be streamed in 10ms, 320-byte chunks and should be encoded as
     *
     * @li 16bit LPCM
     *
     * @li 16kHz sample rate
     *
     * @li Single channel
     *
     * @li Little endian byte order
     *
     * @param  data The audio sample buffer to write
     *
     * @param  size The number of samples in the buffer
     *
     * @return The number of samples successfully written to the Engine or a negative error code
     * if data could not be written
     */
    public long writeMicrophoneAudioData(byte[] data, long size) {
        return writeMicrophoneAudioData( getNativeObject(), data, 0, size );
    }

    /**
     * Returns @c MediaPlayer to use for playing ringtone.
     */
    public MediaPlayer getRingtoneMediaPlayer() {
        return m_ringtoneMediaPlayer;
    }

    /**
     * Returns @c Speaker to control ringtone volume.
     */
    public Speaker getRingtoneSpeaker() {
        return m_ringtoneSpeaker;
    }

    /**
     * Returns @c MediaPlayer to use for playing call audio coming from other end of the call.
     */
    public MediaPlayer getCallAudioMediaPlayer() {
        return m_callAudioMediaPlayer;
    }

    /**
     * Returns @c Speaker to control the call audio.
     */
    public Speaker getCallAudioSpeaker() {
        return m_callAudioSpeaker;
    }

    private native void acceptCall( long nativeObject );
    private native void stopCall( long nativeObject );
    private native long writeMicrophoneAudioData(long nativeObject, byte[] data, long offset, long size);

    private MediaPlayer m_ringtoneMediaPlayer;
    private Speaker m_ringtoneSpeaker;
    private MediaPlayer m_callAudioMediaPlayer;
    private Speaker m_callAudioSpeaker;
}

// END OF FILE

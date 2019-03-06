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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * SpeechRecognizer should be extended to initiate voice interactions with Alexa and provide audio input to AVS.
 * SpeechRecognizer provides interfaces for
 *
 * @li Initiating a dialog interaction with Alexa
 * via press-and-hold, press-and-release, and voice-initiated user actions
 *
 * @li Streaming audio input to the Engine
 *
 * @li Enabling and disabling the wake word engine
 *
 * @li Handling wake word and end of speech detection
 *
 * @note For observing Alexa dialog state transitions, see @c AlexaClient.dialogStateChanged().
 */
abstract public class SpeechRecognizer extends PlatformInterface
{
    /**
     * Describes type of event that initiated the speech request.
     */
    public enum Initiator
    {
        /**
         * Hold-to-talk speech initiator type.
         */
        HOLD_TO_TALK("HOLD_TO_TALK"),
        /**
         * Tap-to-talk speech initiator type.
         */
        TAP_TO_TALK("TAP_TO_TALK"),
        /**
         * Wakeword speech initiator type.
         */
        WAKEWORD("WAKEWORD");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Initiator( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /*
     * Defines an unspecified value for the speech recognizers audio index.
     */
    static public final long UNSPECIFIED_INDEX = -1;

    private boolean m_initialWakewordDetectionEnabled = false;

    public SpeechRecognizer( boolean wakewordDetectionEnabled ) {
        m_initialWakewordDetectionEnabled = wakewordDetectionEnabled;
    }

    public boolean getInitialWakewordDetectionEnabled() {
        return m_initialWakewordDetectionEnabled;
    }

    /**
     * Notifies the platform implementation when a wake word is detected
     *
     * @param  wakeword The wake word that was detected
     *
     * @return @c true if the Engine should initiate a recognize event, @c false if the Engine should ignore the invocation
     */
    public boolean wakewordDetected( String wakeword ) {
        return true;
    }

    /**
     * Notifies the platform implementation when end of
     * speech is detected for the current recognize event
     */
    public void endOfSpeechDetected() {
    }

    /**
     * Notifies the platform implementation to start writing audio samples to the Engine via @c write(). The platform should continue writing audio samples until the Engine calls
     * @c stopAudioInput().
     *
     * @return @c true if the platform handled the call successfully, else @c false
     */
    public boolean startAudioInput() {
        return false;
    }

    /**
     * Notifies the platform implementation to stop writing audio samples to the Engine
     *
     * @return @c true if the platform handled the call successfully, else @c false
     */
    public boolean stopAudioInput() {
        return false;
    }

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-hold action on the
     * platform. The Engine will call @c startAudioInput() to notify the platform implementation when
     * to start writing audio samples.
     * The platform implementation should call @c stopCapture() to terminate speech recognition on release
     * of the press-and-hold action.
     *
     * @return @c true if the Engine successfully initiated a recognize event, else @c false
     */
    public boolean holdToTalk() {
        return holdToTalk( getNativeObject() );
    }

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-release action on the
     * platform. The Engine will call @c startAudioInput() to notify the platform implementation when
     * to start writing audio samples.
     * The Engine will terminate the recognize event initiated by the press-and-release action
     * when end of speech is detected.
     *
     * @return @c true if the Engine successfully started a recognize event, else @c false
     */
    public boolean tapToTalk() {
        return tapToTalk( getNativeObject() );
    }

    /**
     * Notifies the Engine of a speech recognition event. The Engine will call @c startAudioInput() to notify
     * the platform implementation when to start writing audio samples.
     * If the initator type is @c HOLD_TO_TALK, then the platform implementation should call @c stopCapture()
     * to terminate speech recognition on release of the press-and-hold action. Otherwise, the Engine will
     * terminate the recognize event when end of speech is detected.
     *
     * @param initiator The @c Initiator type for the speech recognition event
     * @param keywordBegin The sample index where the keyword begins. This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to @c UNSPECIFIED_INDEX.
     * @param keywordEnd The sample index where the keyword ends. This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to @c UNSPECIFIED_INDEX.
     * @param keyword The keyword being recognized, e.g. "alexa". This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to null.
     * @return @c true if the Engine successfully started a recognize event, else @c false
     */
    public boolean startCapture( Initiator initiator, long keywordBegin, long keywordEnd, String keyword ) {
        return startCapture( getNativeObject(), initiator, keywordBegin, keywordEnd, keyword );
    }

    /**
     * Notifies the Engine to terminate the current recognize event. The Engine will call @c stopAudioInput() to notify the platform implementation when to stop writing audio samples.
     *
     * @return @c true if the Engine successfully terminated the current recognize event, else @c false
     */
    public boolean stopCapture() {
        return stopCapture( getNativeObject() );
    }

    /**
     * Writes audio samples to the Engine for processing by the wake word engine or streaming to AVS.
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
    public long write( byte[] data, long size ) {
        return write( getNativeObject(), data, 0, size );
    }

    /**
     * Notifies the Engine to enable the wake word engine. Wake word must be supported in the Engine to be enabled
     * by this call.
     *
     * @return @c true if the Engine successfully enabled wake word detection, else @c false
     */
    public boolean enableWakewordDetection() {
        return enableWakewordDetection( getNativeObject() );
    }

    /**
     * Notifies the Engine to disable the wake word engine
     *
     * @return @c true if the Engine successfully disabled wakeword detection, else @c false
     */
    public boolean disableWakewordDetection() {
        return disableWakewordDetection( getNativeObject() );
    }

    /**
     * Checks if wake word detection is enabled in the Engine
     *
     * @return @c true if wake word detection is enabled, else @c false
     */
    public boolean isWakewordDetectionEnabled() {
        return isWakewordDetectionEnabled( getNativeObject() );
    }

    private native boolean holdToTalk( long nativeObject );
    private native boolean tapToTalk( long nativeObject );
    private native boolean startCapture( long nativeObject, Initiator initiator, long keywordBegin, long keywordEnd, String keyword );
    private native boolean stopCapture( long nativeObject );
    private native long write( long nativeObject, byte[] data, long offset, long size );
    private native boolean enableWakewordDetection( long nativeObject );
    private native boolean disableWakewordDetection( long nativeObject );
    private native boolean isWakewordDetectionEnabled( long nativeObject );
}

// END OF FILE

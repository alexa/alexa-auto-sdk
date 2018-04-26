/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/alexa/SpeechRecognizer.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c SpeechRecognizer class should be extended by the platform implementation to provide audio input data to the Engine/AVS.
 */
abstract public class SpeechRecognizer extends PlatformInterface
{
    private boolean m_initialWakewordDetectionEnabled = false;

    public SpeechRecognizer( boolean wakewordDetectionEnabled ) {
        m_initialWakewordDetectionEnabled = wakewordDetectionEnabled;
    }

    public boolean getInitialWakewordDetectionEnabled() {
        return m_initialWakewordDetectionEnabled;
    }

    /**
     * Called when the platform implementation should handle that a wakeword has been spoken.
     *
     * @param [in] wakeword The wakeword that was detected.
     * @return @c true if the @c SpeechRecognizer should continue recognizing audio invoked by the wakeword, or @c false if the wakeword invocation should be ignored.
     */
    public boolean wakewordDetected( String wakeword ) {
        return false;
    }

    /**
     * Called when the platform implementation should handle that the end of speech has been reached.
     */
    public void endOfSpeechDetected() {
    }

    /**
     * Called when audio input should start being streamed by the platform implementation. The @c write() method should be called whenever new audio samples are available.
     * It is required that audio input continue to be streamed until the @c stopAudioInput() method is called.
     */
    public boolean startAudioInput() {
        return false;
    }

    /**
     * Called when audio input should stop being streamed by the platform implementation.
     * e.g. after @c endOfSpeechDetected() or @c stopCapture() is called.
     */
    public boolean stopAudioInput() {
        return false;
    }

    /**
     * Initiate a Recognize Event from a press-and-hold action. The platform implementation must start streaming the audio samples provided by the @c write() method to the Engine, and should reflect the hold-to-talk behavior.
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the recognizer was successfully started.
     */
    public boolean holdToTalk() {
        return holdToTalk( getNativeObject() );
    }

    /**
     * Initiate a Recognize Event from a tap-to-talk action. The platform implementation must start streaming the audio samples provided by the @c write() method to the Engine, and should reflect the tap-to-talk behavior.
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the recognizer was successfully started.
     */
    public boolean tapToTalk() {
        return tapToTalk( getNativeObject() );
    }

    /**
     * Notify the Engine to stop streaming audio and terminate the current Recognize Event.
     *
     * @return @c true if the recognizer was successfully stopped.
     */
    public boolean stopCapture() {
        return stopCapture( getNativeObject() );
    }

    /**
     * Write audio samples to be processed by the wakeword engine and/or streamed to AVS. Audio samples should be formatted as mono 16kHz LPCM encoded with 16 bits per sample, little-endian. Typically, samples are provided every 10 milliseconds in 160 sample chunks.
     *
     * @param [in] data The data buffer to write.
     * @param [in] size The number of short samples in the buffer.
     * @return The number of short written, or zero if the stream has closed, or a negative error code if the stream is still open, but no data could be written.
     */
    public long write( byte[] data, long size ) {
        return write( getNativeObject(), data, 0, size );
    }

    /**
     * Tell the Engine to enable wakeword detection.
     *
     * @return @c true if wakeword detection was enabled, else @c false.
     */
    public boolean enableWakewordDetection() {
        return enableWakewordDetection( getNativeObject() );
    }

    /**
     * Tell the Engine to disable wakeword detection.
     *
     * @return @c true if wakeword detection was disabled, else @c false.
     */
    public boolean disableWakewordDetection() {
        return disableWakewordDetection( getNativeObject() );
    }

    /**
     * Get the current @c wakewordDetectionEnabled state.
     * @return @c true if wakeword detection is enabled, else @c false.
     */
    public boolean isWakewordDetectionEnabled() {
        return isWakewordDetectionEnabled( getNativeObject() );
    }

    private native boolean holdToTalk( long nativeObject );
    private native boolean tapToTalk( long nativeObject );
    private native boolean stopCapture( long nativeObject );
    private native long write( long nativeObject, byte[] data, long offset, long size );
    private native boolean enableWakewordDetection( long nativeObject );
    private native boolean disableWakewordDetection( long nativeObject );
    private native boolean isWakewordDetectionEnabled( long nativeObject );
}

// END OF FILE

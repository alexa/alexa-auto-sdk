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

#ifndef AACE_ALEXA_SPEECH_RECOGNIZER_H
#define AACE_ALEXA_SPEECH_RECOGNIZER_H

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c SpeechRecognizer class should be extended by the platform implementation to provide audio input data to
 * the Engine/AVS.
 */
class SpeechRecognizer : public aace::core::PlatformInterface {
protected:
    SpeechRecognizer( bool wakewordDetectionEnabled = true );

public:
    virtual ~SpeechRecognizer() = default;

    /**
     * Initiate a Recognize Event from a press-and-hold action. The platform implementation must start streaming
     * the audio samples provided by the @c write() method to the Engine, and should reflect the hold-to-talk behavior.
     *
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the recognizer was successfully started.
     */
    bool holdToTalk();

    /**
     * Initiate a Recognize Event from a tap-to-talk action. The platform implementation must start streaming
     * the audio samples provided by the @c write() method to the Engine, and should reflect the tap-to-talk behavior.
     *
     * Audio capture is terminated by calling @c stopCapture().
     *
     * @return @c true if the recognizer was successfully started.
     */
    bool tapToTalk();

    /**
     * Notify the Engine to stop streaming audio and terminate the current Recognize Event.
     *
     * @return @c true if the recognizer was successfully stopped.
     */
    bool stopCapture();

    /**
     * Write audio samples to be processed by the wakeword engine and/or streamed to AVS.
     * Audio samples should be formatted as mono 16kHz LPCM encoded with 16 bits per sample, little-endian.
     * Typically, samples are provided every 10 milliseconds in 160 sample chunks.
     *
     * @param [in] data The data buffer to write.
     * @param [in] size The number of int16_t samples in the buffer.
     * @return The number of int16_t written, or zero if the stream has closed, or a negative error code
     * if the stream is still open, but no data could be written.
     */
    ssize_t write( const int16_t* data, const size_t size );

    /**
     * Tell the Engine to enable wakeword detection.
     *
     * @return @c true if wakeword detection was enabled, else @c false.
     */
    bool enableWakewordDetection();

    /**
     * Tell the Engine to disable wakeword detection.
     *
     * @return @c true if wakeword detection was disabled, else @c false.
     */
    bool disableWakewordDetection();

    /**
     * Get the current @c wakewordDetectionEnabled state.
     * @return @c true if wakeword detection is enabled, else @c false.
     */
    bool isWakewordDetectionEnabled();

    /**
     * Called when the platform implementation should handle that a wakeword has been spoken.
     *
     * @param [in] wakeword The wakeword that was detected.
     * @return @c true if the @c SpeechRecognizer should continue recognizing
     * audio invoked by the wakeword, or @c false if the wakeword invocation should be ignored.
     */
    virtual bool wakewordDetected( const std::string& wakeword );

    /**
     * Called when the platform implementation should handle that the end of speech has been reached.
     */
    virtual void endOfSpeechDetected();

    /**
     * Called when audio input should start being streamed by the platform implementation. The @c write() method
     * should be called whenever new audio samples are available.
     *
     * It is required that audio input continue to be streamed until the @c stopAudioInput() method is called.
     */
    virtual bool startAudioInput() = 0;

    /**
     * Called when audio input should stop being streamed by the platform implementation.
     *
     * e.g. after @c endOfSpeechDetected() or @c stopCapture() is called.
     */
    virtual bool stopAudioInput() = 0;

    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::SpeechRecognizerEngineInterface> speechRecognizerEngineInterface );

private:
    bool m_wakewordDetectionEnabled;
    std::shared_ptr<aace::alexa::SpeechRecognizerEngineInterface> m_speechRecognizerEngineInterface;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_SPEECH_RECOGNIZER_H

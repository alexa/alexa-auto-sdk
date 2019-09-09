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

#ifndef AACE_ALEXA_SPEECH_RECOGNIZER_H
#define AACE_ALEXA_SPEECH_RECOGNIZER_H

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * SpeechRecognizer should be extended to initiate voice interactions with Alexa and provide audio input to AVS.
 * 
 * SpeechRecognizer provides interfaces for 
 * @li Initiating a dialog interaction with Alexa
 * via press-and-hold, press-and-release, and voice-initiated user actions
 * @li Streaming audio input to the Engine
 * @li Enabling and disabling the wake word engine
 * @li Handling wake word and end of speech detection
 *
 * @note For observing Alexa dialog state transitions, see @c AlexaClient::dialogStateChanged().
 */
class SpeechRecognizer : public aace::core::PlatformInterface {
protected:
    SpeechRecognizer( bool wakewordDetectionEnabled = true );

public:
    virtual ~SpeechRecognizer();

    /**
     * Describes type of event that initiated the speech request
     * @sa @c aace::alexa::SpeechRecognizerEngineInterface::Initiator
     */
    using Initiator = SpeechRecognizerEngineInterface::Initiator;

    /*
     * Defines an unspecified value for the speech recognizers audio index.
     */
    static constexpr uint64_t UNSPECIFIED_INDEX = SpeechRecognizerEngineInterface::UNSPECIFIED_INDEX;

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-hold action on the 
     * platform. The Engine will call @c startAudioInput() to notify the platform implementation when
     * to start writing audio samples.
     *
     * The platform implementation should call @c stopCapture() to terminate speech recognition on release
     * of the press-and-hold action.
     *
     * @return @c true if the Engine successfully initiated a recognize event, else @c false
     */
    bool holdToTalk();

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-release action on the 
     * platform. The Engine will call @c startAudioInput() to notify the platform implementation when
     * to start writing audio samples.
     *
     * The Engine will terminate the recognize event initiated by the press-and-release action
     * when end of speech is detected.
     *
     * @return @c true if the Engine successfully started a recognize event, else @c false
     */
    bool tapToTalk();

    /**
     * Notifies the Engine of a speech recognition event. The Engine will call @c startAudioInput() to notify 
     * the platform implementation when to start writing audio samples.
     *
     * If the initator type is @c HOLD_TO_TALK, then the platform implementation should call @c stopCapture() 
     * to terminate speech recognition on release of the press-and-hold action. Otherwise, the Engine will 
     * terminate the recognize event when end of speech is detected.
     *
     * @param [in] initiator The @c Initiator type for the speech recognition event
     * @param [in] keywordBegin The sample index where the keyword begins. This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to @c SpeechRecognizer::UNSPECIFIED_INDEX.
     * @param [in] keywordEnd The sample index where the keyword ends. This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to @c SpeechRecognizer::UNSPECIFIED_INDEX.
     * @param [in] keyword The keyword being recognized, e.g. "alexa". This is required when the
     * initator type is @c WAKEWORD, otherwise should be set to an empty string.
     * @return @c true if the Engine successfully started a recognize event, else @c false
     */
    bool startCapture( Initiator initiator, uint64_t keywordBegin = UNSPECIFIED_INDEX, uint64_t keywordEnd = UNSPECIFIED_INDEX, const std::string& keyword = "" );

    /**
     * Notifies the Engine to terminate the current recognize event. The Engine will call @c stopAudioInput()
     * to notify the platform implementation when to stop writing audio samples.
     *
     * @return @c true if the Engine successfully terminated the current recognize event, else @c false
     */
    bool stopCapture();

    /**
     * Notifies the Engine to enable the wake word engine. Wake word must be supported in the Engine to be enabled
     * by this call.
     *
     * @return @c true if the Engine successfully enabled wake word detection, else @c false
     */
    bool enableWakewordDetection();

    /**
     * Notifies the Engine to disable the wake word engine
     *
     * @return @c true if the Engine successfully disabled wakeword detection, else @c false
     */
    bool disableWakewordDetection();

    /**
     * Checks if wake word detection is enabled in the Engine
     *
     * @return @c true if wake word detection is enabled, else @c false
     */
    bool isWakewordDetectionEnabled();

    /**
     * Notifies the platform implementation when a wake word is detected
     *
     * @param [in] wakeword The wake word that was detected
     * @return @c true if the Engine should initiate a recognize event, @c false
     * if the Engine should ignore the invocation
     */
    virtual bool wakewordDetected( const std::string& wakeword );

    /**
     * Notifies the platform implementation when end of 
     * speech is detected for the current recognize event
     */
    virtual void endOfSpeechDetected();

    /**
     * @internal
     * Sets the Engine interface delegate
     *
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::SpeechRecognizerEngineInterface> speechRecognizerEngineInterface );

private:
    std::weak_ptr<aace::alexa::SpeechRecognizerEngineInterface> m_speechRecognizerEngineInterface;

    bool m_wakewordDetectionEnabled;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_SPEECH_RECOGNIZER_H

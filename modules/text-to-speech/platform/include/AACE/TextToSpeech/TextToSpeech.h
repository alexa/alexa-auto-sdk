/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEXTTOSPEECH_TEXTTOSPEECH_H
#define AACE_TEXTTOSPEECH_TEXTTOSPEECH_H

#include <string>

#include "AACE/Audio/AudioStream.h"
#include "AACE/Core/PlatformInterface.h"
#include "TextToSpeechEngineInterface.h"

/** @file */

namespace aace {
namespace textToSpeech {

/**
 * The @c TextToSpeech platform interface should be extended to allow the platform to synthesize 
 * Alexa and non-Alexa speech using a text or a SSML document.
 *
 * The platform implementation is responsible for making request for speech synthesis
 * and requesting capabilities of any Text To Speech provider. The platform should also handle
 * callbacks for each @c prepareSpeech() and @c getCapabilities() requests.
 *
 * @note The TextToSpeech platform is not responsible for the playback of the synthesized speech.
 */
class TextToSpeech : public aace::core::PlatformInterface {
protected:
    TextToSpeech() = default;

public:
    virtual ~TextToSpeech();

    /**
     * Notifies the Engine to prepare a speech asset with @c speechId and @c text.
     * This is an asynchronous call. The platform is notified about the result of 
     * this operation through the @c prepareSpeechCompleted() or @c prepareSpeechFailed() 
     * callbacks.
     *
     * @param [in] speechId The unique identifier of the speech asset.
     * @param [in] text The text in plain or SSML format.
     * @param [in] provider The Text To Speech provider to be used to generate the speech asset. 
     * This parameter should be "text-to-speech-provider" since there is only one provider in
     * Auto SDK.
     * @param [in] options Additional options for the speech synthesis request. 
     * Default value is empty string which generates speech in Alexa's voice. 
     * This parameter need not be specified if the speech synthesis is expected to be 
     * in Alexa's voice
     * 
     * Sample options payload :
     * @code{.json})
     * {
     *      "requestPayload" : {
     *          // Payload defined by the Text To Speech provider 
     *      }
     * }
     * @endcode
     *
     * Refer to the Text to Speech provider module documentation for complete details of the 
     * requestPayload schema.
     * @return true if the request was successful, else false if an error occurred.
     *
     */
    bool prepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& provider,
        const std::string& options = "");

    /** 
     * Retrieves the capabilities of the @c provider.
     *
     * This is an asynchronous call. The result of the operation is provided by 
     * the capabilitiesReceived() callback. 
     * @param [in] provider The unique identifier of the provider.
     * This parameter should be "text-to-speech-provider" since there is only one provider in
     * Auto SDK.
     * @param [in] requestId  The unique identifier for this request.
     * @return true if the request was successful, else false if an error occurred.
     *
     */
    bool getCapabilities(const std::string& requestId, const std::string& provider);

    /**
     * Notifies the platform implementation that the @c prepareSpeech() operation of 
     * speech asset with @c speechId was successful.
     *
     * @param [in] speechId The unique identifier of the speech asset.
     * @param [in] preparedAudio The audio asset.
     * @param [in] metadata The metadata of the speech asset. 
     * 
     * Refer to the Text to Speech provider module documentation for complete details of the 
     * metadata schema.
     *
     */
    virtual void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) = 0;

    /**
     * Notifies the platform implementation that the @c prepareSpeech() operation for 
     * the speech asset with @c speechId failed.
     *
     * @param [in] id The unique identifier of the speech asset.
     * @param [in] reason The reason for the failure.
     *
     * Refer to the Text to Speech provider module documentation for complete details of the 
     * error responses.
     *
     */
    virtual void prepareSpeechFailed(const std::string& speechId, const std::string& reason) = 0;

    /**
     * Notifies the platform implementation of a Text to Speech provider's capabilities.
     * The @c requestId corresponds to the ID from the original @c getCapabilities() request.
     * 
     * @param [in] capabilities The capabilities corresponding to a Text to Speech provider.
     * @param [in] requestId  The unique identifier for the original @c getCapabilities() request.
     *
     * Refer to the Text to Speech provider module documentation for complete details of the 
     * capabilities payload.
     *
     */
    virtual void capabilitiesReceived(const std::string& requestId, const std::string& capabilities) = 0;

    /**
     * @internal
     * Sets the Engine interface delagate
     *
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<TextToSpeechEngineInterface> ttsEngineInterface);

private:
    std::weak_ptr<aace::textToSpeech::TextToSpeechEngineInterface> m_ttsEngineInterface;
};

}  // namespace textToSpeech
}  // namespace aace

#endif  // AACE_TEXTTOSPEECH_TEXTTOSPEECH_H

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

#ifndef AACE_ENGINE_TEXTTOSPEECH_PREPARE_SPEECH_RESULT_H
#define AACE_ENGINE_TEXTTOSPEECH_PREPARE_SPEECH_RESULT_H

#include <functional>
#include <memory>
#include <string>

#include <AACE/Audio/AudioStream.h>

namespace aace {
namespace engine {
namespace textToSpeech {

/**
 * The PrepareSpeechResult class is used by any Text to Speech Provider to encapsulate the
 * result of a speech synthesis request
 */
class PrepareSpeechResult {
public:
    PrepareSpeechResult() = default;
    PrepareSpeechResult(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& speechMetadata,
        const std::string& m_failureReason);
    PrepareSpeechResult& operator=(const PrepareSpeechResult& other) = default;

    /**
     * Function to set the synthesized speech.
     * @param [in] preparedAudio The Audio stream of the prepared audio
     */
    void setPreparedAudio(std::shared_ptr<aace::audio::AudioStream> preparedAudio);

    // Get the prepared audio
    std::shared_ptr<aace::audio::AudioStream> getPreparedAudio();

    /**
     * Function to set the unique identifier for the speech request.
     * @param [in] speechId The unique identifier used to identify a speech synthesis
     *        request
     */
    void setSpeechId(const std::string& speechId);

    // Get the speech request identifier
    std::string getSpeechId() const;

    /**
     * Function to set the failure reason of a failed speech synthesis request.
     * @param [in] failureReason The error encountered by the Text To Speech provider for
     *        the speech synthesis request. Every Text To Speech provider defines it's own failure
     *        errors
     * 
     */
    void setFailureReason(const std::string& failureReason);

    // Get the error encountered for the speech synthesis request
    std::string getFailureReason() const;

    /**
     * Function to set the metadata of a synthesized speech resource.
     * @param [in] metadata The metdata associated with the synthesized
     * speech
     * 
     */
    void setSpeechMetadata(const std::string& metadata);

    // Get the metadata for a synthesized speech
    std::string getSpeechMetadata() const;

private:
    std::string m_speechId;
    std::shared_ptr<aace::audio::AudioStream> m_preparedAudio;
    std::string m_speechMetadata;
    std::string m_failureReason;
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECH_PREPARE_SPEECH_RESULT_H

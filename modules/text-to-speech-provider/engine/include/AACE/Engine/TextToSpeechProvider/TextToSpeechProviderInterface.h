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

#ifndef AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_INTERFACE_H
#define AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_INTERFACE_H

#include <string>
#include <memory>
#include "AACE/Audio/AudioStream.h"

namespace aace {
namespace engine {
namespace textToSpeechProvider {

class TextToSpeechProviderInterface {
public:
    virtual ~TextToSpeechProviderInterface() = default;
    /**
     * Notification of a successful prepareSpeech() operation.
     *
     * @param [in] speechId The unique identifier of the speech asset.
     * @param [in] preparedAudio The audio asset.
     * @param [in] metadata The metadata of the speech asset. 
     * The payload is defined by the Text To Speech provider.
     *
     */
    virtual void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) = 0;

    /**
     * Notification of a failed prepareSpeech() operation for 
     * the speech asset with @c speechId.
     *
     * @param [in] id The unique identifier of the speech asset.
     * @param [in] reason The reason for the failure. The reason is specified by  
     * Text to Speech provider.
     *
     */
    virtual void prepareSpeechFailed(const std::string& speechId, const std::string& reason) = 0;
};

}  // namespace textToSpeechProvider
}  // namespace engine
}  // namespace aace

#endif  //AACE_ENGINE_TEXTTOSPEECHPROVIDER_TEXTTOSPEECHPROVIDER_INTERFACE_H

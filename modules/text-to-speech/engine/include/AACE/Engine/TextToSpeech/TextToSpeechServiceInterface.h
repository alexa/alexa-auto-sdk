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

#ifndef AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SERVICE_INTERFACE_H
#define AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SERVICE_INTERFACE_H

#include <unordered_map>

#include "TextToSpeechSynthesizerInterface.h"

namespace aace {
namespace engine {
namespace textToSpeech {

/**
 * Interface used by every Text To Speech provider Engine service to register a
 * Text To Speech provider with the Text To Speech Engine Service. This interface is implemented
 * by the TextToSpeech Engine service and can be used to get the registered Text To Speech providers
 */
class TextToSpeechServiceInterface {
public:
    virtual ~TextToSpeechServiceInterface() = default;

    /**
     * Function to get the registered Text To Speech provider
     * 
     * @param [in] name of the Text to Speech provider
     * @return The Text To Speech provider
     */
    virtual std::shared_ptr<TextToSpeechSynthesizerInterface> getTextToSpeechProvider(const std::string& name) = 0;

    /**
     * Function used by any Text To Speech provider Engine service to register
     * a Text To Speech provider
     * 
     * @param [in] name of the Text to Speech provider
     * @param [in] textToSpeechProvider The Text to Speech provider
     */
    virtual void registerTextToSpeechProvider(
        const std::string& textToSpeechProviderName,
        std::shared_ptr<TextToSpeechSynthesizerInterface> textToSpeechProvider) = 0;
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SERVICE_INTERFACE_H

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

#ifndef AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SYNTHESIZER_INTERFACE_H
#define AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SYNTHESIZER_INTERFACE_H

#include <future>

#include "PrepareSpeechResult.h"

namespace aace {
namespace engine {
namespace textToSpeech {

/**
 * Interface implemented by different Text To Speech Providers to provide the Text To Speech
 * functionality.
 */
class TextToSpeechSynthesizerInterface {
public:
    virtual ~TextToSpeechSynthesizerInterface() = default;

    /**
     * Notifies the provider to prepare a speech asset with @c speechId and @c text.
     *
     * @param [in] speechId The unique identifier of the speech asset.
     * @param [in] text The text in plain or SSML format.
     * @param [in] options Expected properties/attributes of the generated speech asset. 
     * The payload is defined by the Text To Speech provider
     * @return future object of PrepareSpeechResult. The Text To Speech provider will set the result
     *        values in PrepareSpeechResult object
     */
    virtual std::future<PrepareSpeechResult> prepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& requestPayload) = 0;

    /** 
     * Retrieves the capabilities of the Text To Speech provider. These properties can be different
     * as per the provider. Every Text To Speech provider defines it's own capabilities payload.
     *
     * @return future object of the Capabilities string. The Text To Speech provider will set the 
     *        value of this future using a promise.
     */
    virtual std::future<std::string> getCapabilities(const std::string& requestId) = 0;
};

}  // namespace textToSpeech
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_TEXTTOSPEECH_TEXTTOSPEECH_SYNTHESIZER_INTERFACE_H

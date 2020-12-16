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

#ifndef AACE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_INTERFACE_H
#define AACE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_INTERFACE_H

/** @file */

namespace aace {
namespace textToSpeech {

class TextToSpeechEngineInterface {
public:
    virtual ~TextToSpeechEngineInterface() = default;
    virtual bool onPrepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& provider,
        const std::string& options) = 0;
    virtual bool onGetCapabilities(const std::string& requestId, const std::string& provider) = 0;
};

}  // namespace textToSpeech
}  // namespace aace

#endif  // AACE_TEXTTOSPEECH_TEXTTOSPEECH_ENGINE_INTERFACE_H

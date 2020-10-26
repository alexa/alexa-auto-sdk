/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_ALEXA_ALEXA_CONFIGURATION_BINDER_H
#define AACE_JNI_ALEXA_ALEXA_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Alexa/AlexaConfiguration.h>
#include <AACE/JNI/Alexa/EqualizerControllerBinder.h>

namespace aace {
namespace jni {
namespace alexa {

//
// JTemplateRuntimeTimeoutType
//

class JTemplateRuntimeTimeoutTypeConfig
        : public EnumConfiguration<aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType> {
public:
    using T = aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/config/AlexaConfiguration$TemplateRuntimeTimeoutType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DISPLAY_CARD_TTS_FINISHED_TIMEOUT, "DISPLAY_CARD_TTS_FINISHED_TIMEOUT"},
                {T::DISPLAY_CARD_AUDIO_PLAYBACK_FINISHED_TIMEOUT, "DISPLAY_CARD_AUDIO_PLAYBACK_FINISHED_TIMEOUT"},
                {T::DISPLAY_CARD_AUDIO_PLAYBACK_STOPPED_PAUSED_TIMEOUT,
                 "DISPLAY_CARD_AUDIO_PLAYBACK_STOPPED_PAUSED_TIMEOUT"}};
    }
};

using JTemplateRuntimeTimeoutType =
    JEnum<aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType, JTemplateRuntimeTimeoutTypeConfig>;

//
// JTemplateRuntimeTimeout
//

class JTemplateRuntimeTimeout : public JObject {
public:
    JTemplateRuntimeTimeout(jobject obj) :
            JObject(obj, "com/amazon/aace/alexa/config/AlexaConfiguration$TemplateRuntimeTimeout"){};

    aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeout getTemplateRuntimeTimeout();

    static std::vector<aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeout> convert(
        jobjectArray timeoutArrObj);
};

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_ALEXA_CONFIGURATION_BINDER_H
/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_ALEXA_ALEXA_SPEAKER_BINDER_H
#define AACE_JNI_ALEXA_ALEXA_SPEAKER_BINDER_H

#include <AACE/Alexa/AlexaSpeaker.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

//
// AlexaSpeakerHandler
//

class AlexaSpeakerHandler : public aace::alexa::AlexaSpeaker {
public:
    AlexaSpeakerHandler(jobject obj);

    // aace::alexa::AlexaSpeaker
    void speakerSettingsChanged(SpeakerType type, bool local, int8_t volume, bool mute) override;

private:
    JObject m_obj;
};

//
// AlexaSpeakerBinder
//

class AlexaSpeakerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AlexaSpeakerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_alexaSpeakerHandler;
    }

    std::shared_ptr<AlexaSpeakerHandler> getAlexaSpeaker() {
        return m_alexaSpeakerHandler;
    }

private:
    std::shared_ptr<AlexaSpeakerHandler> m_alexaSpeakerHandler;
};

//
// JSpeakerType
//

class JSpeakerTypeConfig : public EnumConfiguration<AlexaSpeakerHandler::SpeakerType> {
public:
    using T = AlexaSpeakerHandler::SpeakerType;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/AlexaSpeaker$SpeakerType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::ALEXA_VOLUME, "ALEXA_VOLUME"}, {T::ALERTS_VOLUME, "ALERTS_VOLUME"}};
    }
};

using JSpeakerType = JEnum<AlexaSpeakerHandler::SpeakerType, JSpeakerTypeConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_ALEXA_SPEAKER_BINDER_H

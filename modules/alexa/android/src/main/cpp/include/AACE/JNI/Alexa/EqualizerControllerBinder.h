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

#ifndef AACE_JNI_ALEXA_EQUALIZER_CONTROLLER_BINDER_H
#define AACE_JNI_ALEXA_EQUALIZER_CONTROLLER_BINDER_H

#include <vector>

#include <AACE/Alexa/EqualizerController.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class EqualizerControllerHandler : public aace::alexa::EqualizerController {
public:
    EqualizerControllerHandler(jobject obj);

    // aace::alexa::EqualizerController
    void setBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) override;
    std::vector<EqualizerBandLevel> getBandLevels() override;

private:
    JObject m_obj;
};

class EqualizerControllerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    EqualizerControllerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_equalizerControllerHandler;
    }

    std::shared_ptr<EqualizerControllerHandler> getEqualizerController() {
        return m_equalizerControllerHandler;
    }

private:
    std::shared_ptr<EqualizerControllerHandler> m_equalizerControllerHandler;
};

//
// JEqualizerBand
//

class JEqualizerBandConfig : public EnumConfiguration<EqualizerControllerHandler::EqualizerBand> {
public:
    using T = EqualizerControllerHandler::EqualizerBand;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/EqualizerController$EqualizerBand";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::BASS, "BASS"}, {T::MIDRANGE, "MIDRANGE"}, {T::TREBLE, "TREBLE"}};
    }
};

using JEqualizerBand = JEnum<EqualizerControllerHandler::EqualizerBand, JEqualizerBandConfig>;

//
// JEqualizeBandLevel
//

class JEqualizeBandLevel : public JObject {
public:
    JEqualizeBandLevel(jobject obj) : JObject(obj, "com/amazon/aace/alexa/EqualizerController$EqualizerBandLevel") {
    }

    EqualizerControllerHandler::EqualizerBandLevel getBandLevel();

    static std::vector<EqualizerControllerHandler::EqualizerBandLevel> convert(jobjectArray bandLevelsArrObj);
};

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_EQUALIZER_CONTROLLER_BINDER_H
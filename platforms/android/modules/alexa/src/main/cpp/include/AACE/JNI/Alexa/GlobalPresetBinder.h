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

#ifndef AACE_JNI_ALEXA_GLOBAL_PRESET_BINDER_H
#define AACE_JNI_ALEXA_GLOBAL_PRESET_BINDER_H

#include <AACE/Alexa/GlobalPreset.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class GlobalPresetHandler : public aace::alexa::GlobalPreset {
public:
    GlobalPresetHandler(jobject obj);

    // aace::alexa::GlobalPreset
    void setGlobalPreset(int preset) override;

private:
    JObject m_obj;
};

class GlobalPresetBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    GlobalPresetBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_globalPresetHandler;
    }

private:
    std::shared_ptr<GlobalPresetHandler> m_globalPresetHandler;
};

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  //AACE_JNI_ALEXA_GLOBAL_PRESET_BINDER_H

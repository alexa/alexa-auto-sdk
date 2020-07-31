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

#ifndef AACE_JNI_APL_APL_BINDER_H
#define AACE_JNI_APL_APL_BINDER_H

#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

#include "AACE/APL/APL.h"

namespace aace {
namespace jni {
namespace apl {

class APLHandler : public aace::apl::APL {
public:
    APLHandler(jobject obj);

    std::string getVisualContext() override;

    // aace::apl::APL

    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) override;
    void clearDocument() override;
    void executeCommands(const std::string& jsonPayload, const std::string& token) override;
    void interruptCommandSequence() override;

private:
    JObject m_obj;
};

class APLBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    APLBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_aplHandler;
    }

    std::shared_ptr<APLHandler> getAPL() {
        return m_aplHandler;
    }

private:
    std::shared_ptr<APLHandler> m_aplHandler;
};

//
// JActivityEvent
//

class JActivityEventConfig : public EnumConfiguration<APLHandler::ActivityEvent> {
public:
    using T = APLHandler::ActivityEvent;

    const char* getClassName() override {
        return "com/amazon/aace/apl/APL$ActivityEvent";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {
            {T::ACTIVATED, "ACTIVATED"},
            {T::DEACTIVATED, "DEACTIVATED"},
            {T::ONE_TIME, "ONE_TIME"},
            {T::INTERRUPT, "INTERRUPT"},
            {T::UNKNOWN, "UNKNOWN"},
        };
    }
};

using JActivityEvent = JEnum<APLHandler::ActivityEvent, JActivityEventConfig>;

}  // namespace apl
}  // namespace jni
}  // namespace aace

#endif  //AACE_JNI_APL_APL_BINDER_H

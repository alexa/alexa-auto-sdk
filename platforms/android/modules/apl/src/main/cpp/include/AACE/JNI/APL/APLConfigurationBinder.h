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

#ifndef AACE_JNI_APL_APL_CONFIGURATION_BINDER_H
#define AACE_JNI_APL_APL_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>

#include "AACE/APL/APLConfiguration.h"

namespace aace {
namespace jni {
namespace apl {

//
// JAlexaPresentationTimeoutType
//

class JAlexaPresentationTimeoutTypeConfig
        : public EnumConfiguration<aace::apl::config::APLConfiguration::AlexaPresentationTimeoutType> {
public:
    using T = aace::apl::config::APLConfiguration::AlexaPresentationTimeoutType;

    const char* getClassName() override {
        return "com/amazon/aace/apl/APLConfiguration$AlexaPresentationTimeoutType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DISPLAY_DOCUMENT_INTERACTION_IDLE_TIMEOUT, "DISPLAY_DOCUMENT_INTERACTION_IDLE_TIMEOUT"}};
    }
};

using JAlexaPresentationTimeoutType =
    JEnum<aace::apl::config::APLConfiguration::AlexaPresentationTimeoutType, JAlexaPresentationTimeoutTypeConfig>;

//
// JAlexaPresentationTimeout
//

class JAlexaPresentationTimeout : public JObject {
public:
    JAlexaPresentationTimeout(jobject obj) :
            JObject(obj, "com/amazon/aace/apl/APLConfiguration$AlexaPresentationTimeout"){};

    aace::apl::config::APLConfiguration::AlexaPresentationTimeout getAlexaPresentationTimeout();

    static std::vector<aace::apl::config::APLConfiguration::AlexaPresentationTimeout> convert(
        jobjectArray timeoutArrObj);
};

}  // namespace apl
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_APL_APL_CONFIGURATION_BINDER_H
/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_AMAZONLITE_AMAZONLITE_CONFIGURATION_BINDER_H
#define AACE_JNI_AMAZONLITE_AMAZONLITE_CONFIGURATION_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/AmazonLite/AmazonLiteConfiguration.h>

namespace aace {
namespace jni {
namespace amazonLite {

    //
    // JModelConfig
    //

    class JModelConfig : public JObject {
    public:
        JModelConfig( jobject obj ): JObject( obj, "com/amazon/aace/amazonlite/config/AmazonLiteConfiguration$ModelConfig" ) {}

        aace::amazonLite::config::AmazonLiteConfiguration::ModelConfig getModelConfig();

        static std::vector<aace::amazonLite::config::AmazonLiteConfiguration::ModelConfig> convert( jobjectArray modelConfigArrObj );
    };
}
}
}

#endif // AACE_JNI_AMAZONLITE_AMAZONLITE_CONFIGURATION_BINDER_H


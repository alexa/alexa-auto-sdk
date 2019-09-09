/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_ALEXA_TEMPLATE_RUNTIME_BINDER_H
#define AACE_JNI_ALEXA_TEMPLATE_RUNTIME_BINDER_H

#include <AACE/Alexa/TemplateRuntime.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

    class TemplateRuntimeHandler : public aace::alexa::TemplateRuntime {
    public:
        TemplateRuntimeHandler( jobject obj );

        // aace::alexa::TemplateRuntime
        void renderTemplate( const std::string& payload ) override;
        void renderPlayerInfo( const std::string& payload ) override;
        void clearTemplate() override;
        void clearPlayerInfo() override;

    private:
        JObject m_obj;
    };

    class TemplateRuntimeBinder : public aace::jni::core::PlatformInterfaceBinder {
    public:
        TemplateRuntimeBinder( jobject obj );

        std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
            return m_templateRuntimeHandler;
        }

    private:
        std::shared_ptr<TemplateRuntimeHandler> m_templateRuntimeHandler;
    };

} // aace::alexa
} // aace::jni
} // aace

#endif //AACE_JNI_ALEXA_TEMPLATE_RUNTIME_BINDER_H

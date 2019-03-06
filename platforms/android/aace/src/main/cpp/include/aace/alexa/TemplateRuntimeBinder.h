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

// aace/alexa/TemplateRuntimeBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_TEMPLATE_RUNTIME_BINDER_H
#define AACE_ALEXA_TEMPLATE_RUNTIME_BINDER_H

#include "AACE/Alexa/TemplateRuntime.h"
#include "aace/core/PlatformInterfaceBinder.h"

class TemplateRuntimeBinder : public PlatformInterfaceBinder, public aace::alexa::TemplateRuntime {
public:
    TemplateRuntimeBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    void renderTemplate( const std::string & payload ) override;
    void renderPlayerInfo( const std::string & payload ) override;
    void clearTemplate() override;
    void clearPlayerInfo() override;

private:
    jmethodID m_javaMethod_renderTemplate_payload = nullptr;
    jmethodID m_javaMethod_renderPlayerInfo_payload = nullptr;
    jmethodID m_javaMethod_clearTemplate = nullptr;
    jmethodID m_javaMethod_clearPlayerInfo = nullptr;

};

#endif //AACE_ALEXA_TEMPLATE_RUNTIME_BINDER_H

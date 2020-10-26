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

#include "AACE/Alexa/TemplateRuntime.h"

namespace aace {
namespace alexa {

TemplateRuntime::~TemplateRuntime() = default;  // key function

void TemplateRuntime::displayCardCleared() {
    if (auto m_templateRuntimeEngineInterface_lock = m_templateRuntimeEngineInterface.lock()) {
        m_templateRuntimeEngineInterface_lock->onDisplayCardCleared();
    }
}

void TemplateRuntime::setEngineInterface(
    std::shared_ptr<aace::alexa::TemplateRuntimeEngineInterface> templateRuntimeEngineInterface) {
    m_templateRuntimeEngineInterface = templateRuntimeEngineInterface;
}

}  // namespace alexa
}  // namespace aace

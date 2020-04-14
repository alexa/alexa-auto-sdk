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

#include "AACE/APL/APL.h"

namespace aace {
namespace apl {

APL::~APL() = default;  // key function

void APL::setEngineInterface(std::shared_ptr<APLEngineInterface> aplEngineInterface) {
    m_aplEngineInterface = aplEngineInterface;
}

void APL::clearCard() {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onClearCard();
    }
}

void APL::clearAllExecuteCommands() {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onClearAllExecuteCommands();
    }
}

void APL::sendUserEvent(const std::string& payload) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onSendUserEvent(payload);
    }
}

void APL::setAPLMaxVersion(const std::string& aplMaxVersion) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onSetAPLMaxVersion(aplMaxVersion);
    }
}

void APL::setDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onSetDocumentIdleTimeout(documentIdleTimeout);
    }
}

void APL::renderDocumentResult(const std::string& token, bool result, const std::string& error) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onRenderDocumentResult(token, result, error);
    }
}

void APL::executeCommandsResult(const std::string& token, bool result, const std::string& error) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onExecuteCommandsResult(token, result, error);
    }
}

void APL::processActivityEvent(const std::string& source, ActivityEvent event) {
    if (m_aplEngineInterface != nullptr) {
        m_aplEngineInterface->onProcessActivityEvent(source, event);
    }
}

}  // namespace apl
}  // namespace aace
/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Authorization/Authorization.h>

namespace aace {
namespace authorization {

Authorization::~Authorization() = default;  // key function

void Authorization::startAuthorization(const std::string& service, const std::string& data) {
    if (auto m_authorizationEngineInterface_lock = m_authorizationEngineInterface.lock()) {
        m_authorizationEngineInterface_lock->onStartAuthorization(service, data);
    }
}

void Authorization::cancelAuthorization(const std::string& service) {
    if (auto m_authorizationEngineInterface_lock = m_authorizationEngineInterface.lock()) {
        m_authorizationEngineInterface_lock->onCancelAuthorization(service);
    }
}

void Authorization::sendEvent(const std::string& service, const std::string& payload) {
    if (auto m_authorizationEngineInterface_lock = m_authorizationEngineInterface.lock()) {
        m_authorizationEngineInterface_lock->onSendEvent(service, payload);
    }
}

void Authorization::logout(const std::string& service) {
    if (auto m_authorizationEngineInterface_lock = m_authorizationEngineInterface.lock()) {
        m_authorizationEngineInterface_lock->onLogout(service);
    }
}

void Authorization::setEngineInterface(std::shared_ptr<AuthorizationEngineInterface> authorizationEngineInterface) {
    m_authorizationEngineInterface = authorizationEngineInterface;
}

}  // namespace authorization
}  // namespace aace

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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Authorization/AuthorizationProvider.h>

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.authorization.AuthorizationProvider");

namespace aace {
namespace engine {
namespace authorization {

void AuthorizationProvider::setListener(std::shared_ptr<AuthorizationProviderListenerInterface> listener) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(m_listener.lock(), "listenerAlreadySet");
        m_listener = listener;
    } catch (std::exception& ex) {
        AACE_ERROR(LX("aace.engine.authorization.AuthorizationProvider").d("reason", ex.what()));
    }
}

std::shared_ptr<AuthorizationProviderListenerInterface> AuthorizationProvider::getAuthorizationProviderListener() {
    AACE_DEBUG(LX(TAG));
    if (auto listener_lock = m_listener.lock()) {
        return listener_lock;
    }
    return nullptr;
}

}  // namespace authorization
}  // namespace engine
}  // namespace aace

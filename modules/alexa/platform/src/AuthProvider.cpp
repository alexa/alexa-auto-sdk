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

#include "AACE/Alexa/AuthProvider.h"

namespace aace {
namespace alexa {

AuthProvider::~AuthProvider() = default;  // key function

void AuthProvider::authStateChanged(AuthState authState, AuthError authError) {
    if (auto m_authProviderEngineInterface_lock = m_authProviderEngineInterface.lock()) {
        m_authProviderEngineInterface_lock->onAuthStateChanged(authState, authError);
    }
}

void AuthProvider::setEngineInterface(
    std::shared_ptr<aace::alexa::AuthProviderEngineInterface> authProviderEngineInterface) {
    m_authProviderEngineInterface = authProviderEngineInterface;
}

}  // namespace alexa
}  // namespace aace

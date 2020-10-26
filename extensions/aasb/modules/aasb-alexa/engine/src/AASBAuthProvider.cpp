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

#include <AASB/Engine/Alexa/AASBAuthProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/AuthProvider/AuthStateChangedMessage.h>
#include <AASB/Message/Alexa/AuthProvider/GetAuthStateMessage.h>
#include <AASB/Message/Alexa/AuthProvider/GetAuthStateMessageReply.h>
#include <AASB/Message/Alexa/AuthProvider/GetAuthTokenMessage.h>
#include <AASB/Message/Alexa/AuthProvider/GetAuthTokenMessageReply.h>

#include <nlohmann/json.hpp>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAuthProvider");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAuthProvider> AASBAuthProvider::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto authProvider = std::shared_ptr<AASBAuthProvider>(new AASBAuthProvider());
        ThrowIfNot(authProvider->initialize(messageBroker), "initializeFailed");

        return authProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAuthProvider::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    m_messageBroker = messageBroker;

    // create a wp reference
    std::weak_ptr<AASBAuthProvider> wp = shared_from_this();

    messageBroker->subscribe(
        aasb::message::alexa::authProvider::AuthStateChangedMessage::topic(),
        aasb::message::alexa::authProvider::AuthStateChangedMessage::action(),
        [wp](const Message& message) {
            try {
                auto sp = wp.lock();
                ThrowIfNull(sp, "invalidWeakPtrReference");

                aasb::message::alexa::authProvider::AuthStateChangedMessage::Payload payload =
                    nlohmann::json::parse(message.payload());

                sp->authStateChanged(
                    static_cast<AuthState>(payload.authState), static_cast<AuthError>(payload.authError));
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "AuthStateChangedMessage").d("reason", ex.what()));
            }
        });

    return true;
}

//
// aace::alexa::AuthProvider
//

std::string AASBAuthProvider::getAuthToken() {
    try {
        AACE_VERBOSE(LX(TAG));

        if (m_cachedAuthToken.empty()) {
            auto m_messageBroker_lock = m_messageBroker.lock();
            ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

            aasb::message::alexa::authProvider::GetAuthTokenMessage message;

            auto result = m_messageBroker_lock->publish(message.toString()).get();

            ThrowIfNot(result.valid(), "waitForAuthTokenTimeout");

            aasb::message::alexa::authProvider::GetAuthTokenMessageReply::Payload payload =
                nlohmann::json::parse(result.payload());

            m_cachedAuthToken = payload.authToken;
        }

        return m_cachedAuthToken;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::string();
    }
}

aace::alexa::AuthProvider::AuthState AASBAuthProvider::getAuthState() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::authProvider::GetAuthStateMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForAuthStateTimeout");

        aasb::message::alexa::authProvider::GetAuthStateMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        m_authState = static_cast<AuthState>(payload.state);

        return m_authState;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return m_authState;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

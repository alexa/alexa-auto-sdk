/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/CBL/AASBCBL.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Cbl/CBL/CBLState.h>
#include <AASB/Message/Cbl/CBL/CBLStateChangedReason.h>
#include <AASB/Message/Cbl/CBL/CancelMessage.h>
#include <AASB/Message/Cbl/CBL/CBLStateChangedMessage.h>
#include <AASB/Message/Cbl/CBL/ClearRefreshTokenMessage.h>
#include <AASB/Message/Cbl/CBL/GetRefreshTokenMessage.h>
#include <AASB/Message/Cbl/CBL/GetRefreshTokenMessageReply.h>
#include <AASB/Message/Cbl/CBL/SetRefreshTokenMessage.h>
#include <AASB/Message/Cbl/CBL/ResetMessage.h>
#include <AASB/Message/Cbl/CBL/SetUserProfileMessage.h>
#include <AASB/Message/Cbl/CBL/StartMessage.h>

#include <future>

namespace aasb {
namespace engine {
namespace cbl {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.cbl.AASBCBL");

// aliases
using Message = aace::engine::aasb::Message;

bool AASBCBL::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBCBL> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::cbl::cbl::StartMessage::topic(),
            aasb::message::cbl::cbl::StartMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->start();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "StartMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::cbl::cbl::CancelMessage::topic(),
            aasb::message::cbl::cbl::CancelMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->cancel();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "CancelMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::cbl::cbl::ResetMessage::topic(),
            aasb::message::cbl::cbl::ResetMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->reset();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ResetMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AASBCBL> AASBCBL::create(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBCBL>(new AASBCBL());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBCBLFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

void AASBCBL::cblStateChanged(
    CBLState state,
    CBLStateChangedReason reason,
    const std::string& url,
    const std::string& code) {
    try {
        AACE_VERBOSE(LX(TAG).d("state", state).d("reason", reason).d("url", url).d("code", code));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::cbl::cbl::CBLStateChangedMessage message;
        message.payload.state = static_cast<aasb::message::cbl::CBLState>(state);
        message.payload.reason = static_cast<aasb::message::cbl::CBLStateChangedReason>(reason);

        if (state == CBLState::CODE_PAIR_RECEIVED) {
            message.payload.url = url;
            message.payload.code = code;
        }

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBCBL::clearRefreshToken() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        m_cachedRefreshToken.clear();

        aasb::message::cbl::cbl::ClearRefreshTokenMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBCBL::setRefreshToken(const std::string& refreshToken) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        m_cachedRefreshToken = refreshToken;

        aasb::message::cbl::cbl::SetRefreshTokenMessage message;
        message.payload.refreshToken = refreshToken;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string AASBCBL::getRefreshToken() {
    try {
        AACE_VERBOSE(LX(TAG));

        if (m_cachedRefreshToken.empty()) {
            auto m_messageBroker_lock = m_messageBroker.lock();
            ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

            aasb::message::cbl::cbl::GetRefreshTokenMessage message;

            auto result = m_messageBroker_lock->publish(message.toString()).get();

            ThrowIfNot(result.valid(), "waitForRefreshTokenTimeout");

            aasb::message::cbl::cbl::GetRefreshTokenMessageReply::Payload reply =
                nlohmann::json::parse(result.payload());

            m_cachedRefreshToken = reply.refreshToken;
        }

        return m_cachedRefreshToken;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::string();
    }
}

void AASBCBL::setUserProfile(const std::string& name, const std::string& email) {
}

}  // namespace cbl
}  // namespace engine
}  // namespace aasb

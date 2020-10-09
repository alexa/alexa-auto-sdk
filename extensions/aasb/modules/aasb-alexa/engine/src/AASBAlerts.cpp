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

#include <AASB/Engine/Alexa/AASBAlerts.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/Alerts/AlertCreatedMessage.h>
#include <AASB/Message/Alexa/Alerts/AlertDeletedMessage.h>
#include <AASB/Message/Alexa/Alerts/AlertState.h>
#include <AASB/Message/Alexa/Alerts/AlertStateChangedMessage.h>
#include <AASB/Message/Alexa/Alerts/LocalStopMessage.h>
#include <AASB/Message/Alexa/Alerts/RemoveAllAlertsMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBAlerts");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBAlerts> AASBAlerts::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the speech synthesizer platform handler
        auto alerts = std::shared_ptr<AASBAlerts>(new AASBAlerts());

        // initialize the platform handler
        ThrowIfNot(alerts->initialize(messageBroker), "initializeFailed");

        return alerts;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBAlerts::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBAlerts> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::alerts::LocalStopMessage::topic(),
            aasb::message::alexa::alerts::LocalStopMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->localStop();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalStopMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::alerts::RemoveAllAlertsMessage::topic(),
            aasb::message::alexa::alerts::RemoveAllAlertsMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->removeAllAlerts();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "RemoveAllAlertsMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::alexa::Alerts
//

void AASBAlerts::alertStateChanged(const std::string& alertToken, AlertState state, const std::string& reason) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alerts::AlertStateChangedMessage message;

        message.payload.alertToken = alertToken;
        message.payload.state = static_cast<aasb::message::alexa::AlertState>(state);
        message.payload.reason = reason;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAlerts::alertCreated(const std::string& alertToken, const std::string& detailedInfo) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alerts::AlertCreatedMessage message;

        message.payload.alertToken = alertToken;
        message.payload.detailedInfo = detailedInfo;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBAlerts::alertDeleted(const std::string& alertToken) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::alerts::AlertDeletedMessage message;

        message.payload.alertToken = alertToken;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

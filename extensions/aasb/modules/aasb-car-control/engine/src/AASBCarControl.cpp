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

#include <AASB/Engine/CarControl/AASBCarControl.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/CarControl/CarControl/AdjustRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustModeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustControllerValueMessageReply.h>

#include <AASB/Message/CarControl/CarControl/SetRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetModeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetPowerControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetToggleControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetControllerValueMessageReply.h>

namespace aasb {
namespace engine {
namespace carControl {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.carControl.AASBCarControl");

// aliases
using Message = aace::engine::aasb::Message;

AASBCarControl::AASBCarControl(uint32_t asyncReplyTimeout) {
    AACE_VERBOSE(LX(TAG).d("asyncReplyTimeout", asyncReplyTimeout));
    m_replyMessageTimeout = asyncReplyTimeout;
}

std::shared_ptr<AASBCarControl> AASBCarControl::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
    uint32_t asyncReplyTimeout) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the car control platform handler
        auto carControl = std::shared_ptr<AASBCarControl>(new AASBCarControl(asyncReplyTimeout));

        // initialize the platform handler
        ThrowIfNot(carControl->initialize(messageBroker), "initializeAASBCarControlFailed");

        return carControl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBCarControl::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBCarControl> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::carControl::carControl::SetControllerValueMessageReply::topic(),
            aasb::message::carControl::carControl::SetControllerValueMessageReply::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    auto promise = sp->getReplyMessagePromise(message.replyTo());
                    ThrowIfNull(promise, "invalidPromise");

                    aasb::message::carControl::carControl::SetControllerValueMessageReply::Payload payload =
                        nlohmann::json::parse(message.payload());
                    promise->set_value(payload.success);
                    AACE_VERBOSE(LX(TAG, "SetControllerValueMessageReply").m("setControllerValueReplyPromiseSet"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "SetControllerValueMessageReply").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::carControl::carControl::AdjustControllerValueMessageReply::topic(),
            aasb::message::carControl::carControl::AdjustControllerValueMessageReply::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    auto promise = sp->getReplyMessagePromise(message.replyTo());
                    ThrowIfNull(promise, "invalidPromise");

                    aasb::message::carControl::carControl::AdjustControllerValueMessageReply::Payload payload =
                        nlohmann::json::parse(message.payload());
                    promise->set_value(payload.success);
                    AACE_VERBOSE(
                        LX(TAG, "AdjustControllerValueMessageReply").m("adjustControllerValueReplyPromiseSet"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "AdjustControllerValueMessageReply").d("reason", ex.what()));
                }
            });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::carControl::CarControl
//

/**
 * PowerController
 */
bool AASBCarControl::turnPowerControllerOn(const std::string& endpointId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetPowerControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.turnOn = true;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::turnPowerControllerOff(const std::string& endpointId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetPowerControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.turnOn = false;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::isPowerControllerOn(const std::string& endpointId, bool& isOn) {
    return true;
}

/**
 * ToggleController
 */
bool AASBCarControl::turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetToggleControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;
        message.payload.turnOn = true;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetToggleControllerValueMessage message;
        message.payload.turnOn = false;
        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn) {
    return true;
}

/**
 * RangeController
 */
bool AASBCarControl::setRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double value) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetRangeControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;
        message.payload.value = value;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::adjustRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double delta) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::AdjustRangeControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;
        message.payload.delta = delta;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::getRangeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    double& value) {
    return true;
}

/**
 * ModeController
 */
bool AASBCarControl::setModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    const std::string& value) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::SetModeControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;
        message.payload.value = value;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::adjustModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    int delta) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::carControl::carControl::AdjustModeControllerValueMessage message;

        message.payload.endpointId = endpointId;
        message.payload.controllerId = controllerId;
        message.payload.delta = delta;

        m_messageBroker_lock->publish(message.toString()).send();

        std::string messageId = message.header.id;
        return waitForAsyncReply(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBCarControl::getModeControllerValue(
    const std::string& endpointId,
    const std::string& controllerId,
    std::string& value) {
    return true;
}

bool AASBCarControl::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the car control reply message to fulfill
    std::shared_ptr<CarControlPromise> promise = std::make_shared<CarControlPromise>();

    // create a future to receive the promised car control reply message when it is received
    std::shared_future<bool> future(promise->get_future());

    bool success = false;
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(m_replyMessageTimeout)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void AASBCarControl::addReplyMessagePromise(const std::string& messageId, std::shared_ptr<CarControlPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "addReplyMessagePromise").d("reason", ex.what()));
    }
}

void AASBCarControl::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "removeReplyMessagePromise").d("reason", ex.what()));
    }
}

std::shared_ptr<AASBCarControl::CarControlPromise> AASBCarControl::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("keys", m_promiseMap.size()).d("messageId", messageId));
        return nullptr;
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aasb

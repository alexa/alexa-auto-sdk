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

#include <AASB/Engine/Navigation/AASBNavigation.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Navigation/Navigation/AlternateRouteType.h>
#include <AASB/Message/Navigation/Navigation/AnnounceManeuverMessage.h>
#include <AASB/Message/Navigation/Navigation/AnnounceRoadRegulationMessage.h>
#include <AASB/Message/Navigation/Navigation/CancelNavigationMessage.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplay.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplayMessage.h>
#include <AASB/Message/Navigation/Navigation/ErrorCode.h>
#include <AASB/Message/Navigation/Navigation/ErrorType.h>
#include <AASB/Message/Navigation/Navigation/EventName.h>
#include <AASB/Message/Navigation/Navigation/GetNavigationStateMessage.h>
#include <AASB/Message/Navigation/Navigation/GetNavigationStateMessageReply.h>
#include <AASB/Message/Navigation/Navigation/NavigateToPreviousWaypointMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationErrorMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationEventMessage.h>
#include <AASB/Message/Navigation/Navigation/RoadRegulation.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesSucceededMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowPreviousWaypointsMessage.h>
#include <AASB/Message/Navigation/Navigation/StartNavigationMessage.h>

namespace aasb {
namespace engine {
namespace navigation {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.navigation.AASBNavigation");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBNavigation> AASBNavigation::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBNavigation>(new AASBNavigation());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBNavigationFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBNavigation::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBNavigation> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::navigation::navigation::NavigationEventMessage::topic(),
            aasb::message::navigation::navigation::NavigationEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::NavigationEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->navigationEvent(static_cast<EventName>(payload.event));

                    AACE_INFO(LX(TAG, "NavigationEventMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "NavigationEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::navigation::navigation::NavigationErrorMessage::topic(),
            aasb::message::navigation::navigation::NavigationErrorMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::NavigationErrorMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->navigationError(
                        static_cast<ErrorType>(payload.type),
                        static_cast<ErrorCode>(payload.code),
                        payload.description);

                    AACE_INFO(LX(TAG, "NavigationErrorMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "NavigationErrorMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::topic(),
            aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->showAlternativeRoutesSucceeded(payload.payload);

                    AACE_INFO(LX(TAG, "ShowAlternativeRoutesSucceededMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ShowAlternativeRoutesSucceededMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::navigation::Navigation
//

void AASBNavigation::showPreviousWaypoints() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ShowPreviousWaypointsMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::navigateToPreviousWaypoint() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::NavigateToPreviousWaypointMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::showAlternativeRoutes(AlternateRouteType alternateRouteType) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ShowAlternativeRoutesMessage message;

        message.payload.alternateRouteType =
            static_cast<aasb::message::navigation::navigation::AlternateRouteType>(alternateRouteType);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::controlDisplay(ControlDisplay controlDisplay) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ControlDisplayMessage message;

        message.payload.controlDisplay =
            static_cast<aasb::message::navigation::navigation::ControlDisplay>(controlDisplay);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AASBNavigation::cancelNavigation() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::CancelNavigationMessage message;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AASBNavigation::getNavigationState() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::GetNavigationStateMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).timeout(std::chrono::milliseconds(500)).get();

        if (result.valid()) {
            std::string payload = result.payload();
            aasb::message::navigation::navigation::GetNavigationStateMessageReply::Payload replyPayload =
                nlohmann::json::parse(payload);
            m_cachedNavState = replyPayload.navigationState;
        }

        AACE_INFO(LX(TAG, "GetNavigationStateMessage").m("ReplyReceived"));

        return m_cachedNavState;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AASBNavigation::startNavigation(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::StartNavigationMessage message;

        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::announceManeuver(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::AnnounceManeuverMessage message;

        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::announceRoadRegulation(RoadRegulation roadRegulation) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::AnnounceRoadRegulationMessage message;

        message.payload.roadRegulation =
            static_cast<aasb::message::navigation::navigation::RoadRegulation>(roadRegulation);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace navigation
}  // namespace engine
}  // namespace aasb

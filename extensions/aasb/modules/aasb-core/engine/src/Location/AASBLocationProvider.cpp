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

#include <AASB/Engine/Location/AASBLocationProvider.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Location/LocationProvider/GetCountryMessage.h>
#include <AASB/Message/Location/LocationProvider/GetCountryMessageReply.h>
#include <AASB/Message/Location/LocationProvider/GetLocationMessage.h>
#include <AASB/Message/Location/LocationProvider/GetLocationMessageReply.h>

namespace aasb {
namespace engine {
namespace location {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.location.AASBLocationProvider");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBLocationProvider> AASBLocationProvider::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // create the location provider platform handler
        auto locationProvider = std::shared_ptr<AASBLocationProvider>(new AASBLocationProvider());

        // initialize the platform handler
        ThrowIfNot(locationProvider->initialize(messageBroker), "initializeFailed");

        return locationProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBLocationProvider::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // save the message broker reference
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");
        m_messageBroker = messageBroker;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::location::LocationProvider
//

aace::location::Location AASBLocationProvider::getLocation() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::location::locationProvider::GetLocationMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetLocationTimeout");

        aasb::message::location::locationProvider::GetLocationMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        // parse the location from payload
        m_location = aace::location::Location(payload.location.latitude, payload.location.longitude);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }

    return m_location;
}

std::string AASBLocationProvider::getCountry() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::location::locationProvider::GetCountryMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForGetCountryTimeout");

        aasb::message::location::locationProvider::GetCountryMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());

        return payload.country;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

}  // namespace location
}  // namespace engine
}  // namespace aasb

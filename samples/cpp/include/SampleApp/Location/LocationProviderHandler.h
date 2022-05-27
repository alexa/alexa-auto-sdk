/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_LOCATION_LOCATIONPROVIDERHANDLER_H
#define SAMPLEAPP_LOCATION_LOCATIONPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>
#include <AACE/Location/LocationProvider.h>

#include <AASB/Message/Location/LocationProvider/GetLocationMessage.h>
#include <AASB/Message/Location/LocationProvider/LocationServiceAccessChangedMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace location {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocationProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class LocationProviderHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    LocationProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LocationProviderHandler> {
        return std::shared_ptr<LocationProviderHandler>(new LocationProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles the GetCountryMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetCountryMessage(const std::string& message);

    /**
     * Handles the GetLocationMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetLocationMessage(const std::string& message);

    /**
     * Notifies the Engine of a change in location service access. Use this function when the device's
     * access to location service provider changes. E.g., system location access is not granted to the
     * application.
     *
     * @param [in] access Access to the location service
     */
    void locationServiceAccessChanged(aasb::message::location::locationProvider::LocationServiceAccess access);

    /**
     * Provides the implementation for getting country.
     */
    std::string getCountry();

    /**
     * Provides the implementation for getting location.
     */
    aasb::message::location::locationProvider::Location getLocation();

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace location
}  // namespace sampleApp

#endif  // SAMPLEAPP_LOCATION_LOCATIONPROVIDERHANDLER_H

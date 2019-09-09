/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "LocationProviderDispatcher.h"

#include <json-c/json.h>

#include <aasb/Consts.h>

#include "AlexaConsts.h"

namespace agl {
namespace dispatcher {
namespace locationprovider {

// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

// Namespaces.
using namespace agl::common::interfaces;
using namespace aasb::bridge;

// Logging Tag for this file.
static std::string TAG = "alexa::plugins::LocationProviderDispatcher";

std::shared_ptr<LocationProviderDispatcher> LocationProviderDispatcher::create(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IAASBController> aasbController,
    std::shared_ptr<IConfigurationProvider> configProvider) {

    return std::shared_ptr<LocationProviderDispatcher>(new LocationProviderDispatcher(logger, aasbController, configProvider));
}

LocationProviderDispatcher::LocationProviderDispatcher(
    std::shared_ptr<ILogger> logger,
    std::shared_ptr<IAASBController> aasbController,
    std::shared_ptr<IConfigurationProvider> configProvider) :
        m_logger(logger),
        m_aasbController(aasbController),
        m_configProvider(configProvider) {
}

void LocationProviderDispatcher::onReceivedDirective(
    const std::string& action,
    const std::string& payload) {

    m_logger->log(Level::DEBUG, TAG, "Processing locationprovider directive: " + action);

    if (action == aasb::bridge::ACTION_LOCATION_REQUEST_CURRENT_LOCATION) {
        json_object *responseJ = json_object_new_object();
        json_object *isSucceededJ = json_object_new_boolean(true);

        auto location = m_configProvider->getCurrentLocation();
        json_object *latitudeJ = json_object_new_double(location.first);
        json_object *longitudeJ = json_object_new_double(location.second);

        json_object_object_add(responseJ, aasb::bridge::JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED.c_str(), isSucceededJ);
        json_object_object_add(responseJ, aasb::bridge::JSON_ATTR_LOCATION_LATITUDE.c_str(), latitudeJ);
        json_object_object_add(responseJ, aasb::bridge::JSON_ATTR_LOCATION_LONGITUDE.c_str(), longitudeJ);

        m_aasbController->onReceivedEvent(
            TOPIC_LOCATIONPROVIDER,
            ACTION_LOCATION_RESPONSE_CURRENT_LOCATION,
            std::string(json_object_to_json_string(responseJ)));
    }
}


}  // namespace locationprovider
}  // namespace dispatcher
}  // namespace agl
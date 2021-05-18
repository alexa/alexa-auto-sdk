/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/AlexaEngineLocationStateProvider.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaEngineLocationStateProvider");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AlexaEngineLocationStateProvider";

// state provider constants
static const alexaClientSDK::avsCommon::avs::NamespaceAndName LOCATION_STATE{"Geolocation", "GeolocationState"};

std::shared_ptr<AlexaEngineLocationStateProvider> AlexaEngineLocationStateProvider::create(
    std::shared_ptr<aace::engine::location::LocationServiceInterface> locationProvider,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    try {
        ThrowIfNull(locationProvider, "locationProviderIsNull");
        ThrowIfNull(contextManager, "contextManagerIsNull");

        auto alexaEngineLocationStateProvider = std::shared_ptr<AlexaEngineLocationStateProvider>(
            new AlexaEngineLocationStateProvider(locationProvider, contextManager));
        alexaEngineLocationStateProvider->initialize();
        return alexaEngineLocationStateProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "failToCreateAlexaEngineLocationStateProvider").d("reason", ex.what()));
        return nullptr;
    }
}

void AlexaEngineLocationStateProvider::initialize() {
    // add AlexaEngineLocationStateProvider to observer for LocationServiceInterface
    m_locationProvider->addObserver(shared_from_this());
    // add the location state to the context manager
    m_contextManager->setStateProvider(LOCATION_STATE, shared_from_this());
}

AlexaEngineLocationStateProvider::AlexaEngineLocationStateProvider(
    std::shared_ptr<aace::engine::location::LocationServiceInterface> locationProvider,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) :
        m_locationServiceAccess(aace::location::LocationProvider::LocationServiceAccess::ENABLED),
        m_locationProvider(locationProvider),
        m_contextManager(contextManager) {
}

void AlexaEngineLocationStateProvider::shutdown() {
    m_executor.shutdown();

    if (m_locationProvider != nullptr) {
        m_locationProvider->removeObserver(shared_from_this());
        m_locationProvider.reset();
    }

    if (m_contextManager != nullptr) {
        m_contextManager->removeStateProvider(LOCATION_STATE);
        m_contextManager.reset();
    }
}

void AlexaEngineLocationStateProvider::provideState(
    const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
    const unsigned int stateRequestToken) {
    m_executor.submit(
        [this, stateProviderName, stateRequestToken] { executeProvideState(stateProviderName, stateRequestToken); });
}

void AlexaEngineLocationStateProvider::onLocationServiceAccessChanged(LocationServiceAccess access) {
    std::lock_guard<std::mutex> lock(m_locationServiceAccessMutex);
    m_locationServiceAccess = access;
}

void AlexaEngineLocationStateProvider::executeProvideState(
    const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
    const unsigned int stateRequestToken) {
    try {
        ThrowIfNull(m_contextManager, "contextManagerIsNull");

        std::unique_lock<std::mutex> lock(m_locationServiceAccessMutex);
        auto access = m_locationServiceAccess;
        lock.unlock();

        aace::location::Location location;
        if (access == aace::location::LocationProviderEngineInterface::LocationServiceAccess::ENABLED) {
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "executeProvideState", "GetLocation", 1);
            location = m_locationProvider->getLocation();
        }

        if (location.isValid()) {
            // build the context payload
            rapidjson::Document document(rapidjson::kObjectType);

            // add timestamp
            std::string time = location.getTimeAsString();

            document.AddMember(
                "timestamp",
                rapidjson::Value().SetString(time.c_str(), time.length(), document.GetAllocator()),
                document.GetAllocator());

            // add location coordinate
            rapidjson::Value coordinate(rapidjson::kObjectType);

            coordinate.AddMember("latitudeInDegrees", location.getLatitude(), document.GetAllocator());
            coordinate.AddMember("longitudeInDegrees", location.getLongitude(), document.GetAllocator());
            coordinate.AddMember(
                "accuracyInMeters",
                location.getAccuracy() != aace::location::Location::UNDEFINED ? location.getAccuracy() : 0,
                document.GetAllocator());

            document.AddMember("coordinate", coordinate, document.GetAllocator());

            // add location altitude
            if (location.getAltitude() != aace::location::Location::UNDEFINED) {
                rapidjson::Value altitude(rapidjson::kObjectType);

                altitude.AddMember("altitudeInMeters", location.getAltitude(), document.GetAllocator());
                altitude.AddMember(
                    "accuracyInMeters",
                    location.getAccuracy() != aace::location::Location::UNDEFINED ? location.getAccuracy() : 0,
                    document.GetAllocator());

                document.AddMember("altitude", altitude, document.GetAllocator());
            }

            // set the context location state
            ThrowIf(
                m_contextManager->setState(
                    LOCATION_STATE,
                    aace::engine::utils::json::toString(document),
                    alexaClientSDK::avsCommon::avs::StateRefreshPolicy::ALWAYS,
                    stateRequestToken) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS,
                "contextManagerSetStateFailed");
        } else {
            ThrowIf(
                m_contextManager->setState(
                    LOCATION_STATE,
                    "",
                    alexaClientSDK::avsCommon::avs::StateRefreshPolicy::SOMETIMES,
                    stateRequestToken) != alexaClientSDK::avsCommon::sdkInterfaces::SetStateResult::SUCCESS,
                "contextManagerSetStateFailed");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace

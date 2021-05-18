/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_HANDLER_H
#define SAMPLEAPP_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_HANDLER_H

#include <string>
#include <AACE/LocalNavigation/LocalSearchProvider.h>

#include "SampleApp/Extension.h"

namespace sampleApp {
namespace localNavigation {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalSearchProviderExtension
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class LocalSearchProviderExtension : public extension::Extension {
    // Enable extension for dynamic loading
    ENABLE_EXTENSION;

public:
    LocalSearchProviderExtension(const std::string& name) : Extension(name){};
    virtual ~LocalSearchProviderExtension() = default;

    bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<aace::propertyManager::PropertyManager> propertyManager) override;
    bool validate(const std::vector<nlohmann::json>& configs) override;
    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterfaceHandler() override;

private:
    std::shared_ptr<aace::localNavigation::LocalSearchProvider> m_localSearchProviderHandler;
};

/**
 * This sample implementation of LocalSearchProvider provides sample error
 * responses to search and lookup requests from the Engine. In order for offline
 * POI search and navigation features to work, your implementation must delegate
 * search and lookup requests to a navigation provider with offline search
 * functionality.
 */
class LocalSearchProviderHandler : public aace::localNavigation::LocalSearchProvider {
private:
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    LocalSearchProviderHandler(std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LocalSearchProviderHandler> {
        return std::shared_ptr<LocalSearchProviderHandler>(new LocalSearchProviderHandler(args...));
    }

    /// @name @c LocalSearchProvider methods
    /// @{
    bool poiSearchRequest(const std::string& request) override;
    bool poiLookupRequest(const std::string& request) override;
    /// @}

private:
    /**
     * Utility method to get the request ID from the specified search or lookup request.
     *
     * @param [in] request A search or lookup request in LocalSearchProvider JSON format.
     * @param [out] requestId The reference to update with the request ID from the specified request.
     * @return @c true if @a requestId was updated with the request ID, false otherwise
     */
    bool getRequestId(const std::string request, std::string& requestId);

    /**
     * Utility method to create a sample error response for a request with the specified request ID.
     *
     * @param requestId The request ID for which to generate a sample error response.
     * @return An error response in the format required by LocalSearchProvider.
     */
    std::string createErrorResponse(std::string requestId);

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
};

}  // namespace localNavigation
}  // namespace sampleApp

#endif  // SAMPLEAPP_LOCAL_NAVIGATION_LOCAL_SEARCH_PROVIDER_HANDLER_H

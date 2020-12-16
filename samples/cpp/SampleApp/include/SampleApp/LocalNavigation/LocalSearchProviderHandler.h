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
#include "SampleApp/Logger/LoggerHandler.h"

namespace sampleApp {
namespace localNavigation {

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
    LocalSearchProviderHandler(std::shared_ptr<logger::LoggerHandler> loggerHandler);

public:
    static std::shared_ptr<LocalSearchProviderHandler> create(std::shared_ptr<logger::LoggerHandler> loggerHandler);

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

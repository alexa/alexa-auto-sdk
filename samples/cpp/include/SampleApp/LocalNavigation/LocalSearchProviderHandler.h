/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Extension.h"

#include <AACE/Core/MessageBroker.h>

namespace sampleApp {
namespace localNavigation {

/**
 * This sample implementation of LocalSearchProvider provides sample error
 * responses to search and lookup requests from the Engine. In order for offline
 * local search and navigation features to work, your implementation must delegate
 * search and lookup requests to a navigation provider with offline search
 * functionality.
 */
class LocalSearchProviderHandler : public extension::Extension {
    // Enable extension for dynamic loading
    ENABLE_EXTENSION;

private:
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

public:
    LocalSearchProviderHandler(const std::string& name) : Extension(name){};
    virtual ~LocalSearchProviderHandler() = default;

    /// @name @c Extension functions
    /// @{
    bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker) override;
    bool validate(const std::vector<nlohmann::json>& configs) override;
    /// @}

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles a SearchRequestMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSearchRequestMessage(const std::string& message);

    /**
     * Handles a LookupRequestMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleLookupRequestMessage(const std::string& message);

    /**
     * Provides implementation of handling a search request.
     * 
     * @param [in] request A search request
     */
    void searchRequest(const std::string& request);

    /**
     * Provides implementation of handling a lookup request.
     * 
     * @param [in] request A lookup request
     */
    void lookupRequest(const std::string& request);

    /**
     * Publishes SearchResponseMessage to respond to a SearchRequestMessage query.
     * 
     * @param [in] response A search response
     */
    void searchResponse(const std::string& response);

    /**
     * Publishes LookupResponseMessage to respond to a LookupRequestMessage query.
     * 
     * @param [in] response A lookup response
     */
    void lookupResponse(const std::string& response);

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

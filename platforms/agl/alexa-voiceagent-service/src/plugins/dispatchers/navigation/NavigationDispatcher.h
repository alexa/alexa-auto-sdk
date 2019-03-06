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

#ifndef AGL_ALEXA_SVC_NAVIGATION_CAPABILITY_H_
#define AGL_ALEXA_SVC_NAVIGATION_CAPABILITY_H_

#include <memory>
#include <string>

#include <aasb/RequestHandler.h>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/capability/ICapabilityMessageDispatcher.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace dispatcher {
namespace navigation {

/**
 * @c NavigationDispatcher provides the dispatching functionality to route navigation messages
 * from vshl-capabilities AGL binding to AASB and similarly to route outgoing messages (such as
 * set-destination, cancel-navigation) from AASB to vshl-capabilities AGL binding.
 */
class NavigationDispatcher : public agl::capability::interfaces::ICapabilityMessageDispatcher {
public:
    /**
     * Creates a new instance of @c NavigationDispatcher.
     *
     * @param logger An instance of logger.
     * @param requestHandler AASB Request handler to dispatch events to AASB.
     * @param api AFB API instance to communicate with other AGL services (like vshl-capabilities)
     */
    static std::shared_ptr<NavigationDispatcher> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::RequestHandler> requestHandler,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    /// @name ICapabilityMessageDispatcher Functions
    /// @{
    void onReceivedDirective(const std::string& action, const std::string& jsonPayload) override;
    /// @}

private:
    /**
     * Constructor for @c NavigationDispatcher.
     */
    NavigationDispatcher(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::RequestHandler> requestHandler,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    // Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

    // AASB Request handler to dispatch events to AASB.
    std::shared_ptr<aasb::bridge::RequestHandler> m_requestHandler;

    // AFB API object for events pub/sub, and for calling other AGL services.
    std::shared_ptr<agl::common::interfaces::IAFBApi> m_api;
};

}  // namespace navigation
}  // namespace dispatcher
}  // namespace agl

#endif  // AGL_ALEXA_SVC_NAVIGATION_CAPABILITY_H_

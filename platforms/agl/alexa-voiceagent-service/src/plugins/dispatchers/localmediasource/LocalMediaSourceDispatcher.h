/*Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AGL_ALEXA_SVC_LOCALMEDIASOURCE_CAPABILITY_H_
#define AGL_ALEXA_SVC_LOCALMEDIASOURCE_CAPABILITY_H_

#include <memory>
#include <string>

#include <aasb/interfaces/IAASBController.h>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/capability/ICapabilityMessageDispatcher.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace dispatcher {
namespace localmediasource {

/**
 * @c LocalMediaSourceDispatcher provides the dispatching functionality to route localmediasource messages
 * from vshl-capabilities AGL binding to AASB and similarly to route outgoing messages
 * from AASB to vshl-capabilities AGL binding.
 */
class LocalMediaSourceDispatcher : public agl::capability::interfaces::ICapabilityMessageDispatcher {
public:
    /**
     * Creates a new instance of @c LocalMediaSourceDispatcher.
     *
     * @param logger An instance of logger.
     * @param IAASBController AASB controller to dispatch events to AASB.
     * @param api AFB API instance to communicate with other AGL services (like vshl-capabilities)
     */
    static std::shared_ptr<LocalMediaSourceDispatcher> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::IAASBController> aasbController,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    /**
     * Subscribe to receive localmediasource events from vshl-capabilities.
     *
     * @return true, if succeeded, false otherwise.
     */
    bool subscribeToLocalMediaSourceEvents();

    /**
     * Process the get state response event from vshl-capabilities.
     *
     * @param payload Data for the event.
     */
    void onGetStateResponse(const std::string& payload);

    /**
     * Process the player event from vshl-capabilities.
     *
     * @param payload Data for the event.
     */
    void onPlayerEvent(const std::string& payload);

    /**
     * Process the player error event from vshl-capabilities.
     *
     * @param payload Data for the event.
     */
    void onPlayerError(const std::string& payload);


    /// @name ICapabilityMessageDispatcher Functions
    /// @{
    void onReceivedDirective(const std::string& action, const std::string& jsonPayload) override;
    /// @}

private:
    /**
     * Constructor for @c LocalMediaSourceDispatcher.
     */
    LocalMediaSourceDispatcher(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::IAASBController> aasbController,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    // Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

    // AASB controller to dispatch events to AASB.
    std::shared_ptr<aasb::bridge::IAASBController> m_aasbController;

    // AFB API object for events pub/sub, and for calling other AGL services.
    std::shared_ptr<agl::common::interfaces::IAFBApi> m_api;
};

}  // namespace localmediasource
}  // namespace dispatcher
}  // namespace agl

#endif  // AGL_ALEXA_SVC_LOCALMEDIASOURCE_CAPABILITY_H_

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

#ifndef AGL_ALEXA_SVC_LOCATIONPROVIDER_CAPABILITY_H_
#define AGL_ALEXA_SVC_LOCATIONPROVIDER_CAPABILITY_H_

#include <memory>
#include <string>

#include <aasb/interfaces/IAASBController.h>
#include <aasb/interfaces/IConfigurationProvider.h>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/capability/ICapabilityMessageDispatcher.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace dispatcher {
namespace locationprovider {

/**
 * @c LocationProviderDispatcher provides the dispatching functionality to route location provider messages
 */
class LocationProviderDispatcher : public agl::capability::interfaces::ICapabilityMessageDispatcher {
public:
    /**
     * Creates a new instance of @c LocationProviderDispatcher.
     *
     * @param logger An instance of logger.
     * @param IAASBController AASB controller to dispatch events to AASB.
     * @param IConfigurationProvider Configuration provider
     */
    static std::shared_ptr<LocationProviderDispatcher> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::IAASBController> aasbController,
        std::shared_ptr<aasb::bridge::IConfigurationProvider> configProvider);

    /// @name ICapabilityMessageDispatcher Functions
    /// @{
    void onReceivedDirective(const std::string& action, const std::string& jsonPayload) override;
    /// @}

private:
    /**
     * Constructor for @c LocationProviderDispatcher.
     */
    LocationProviderDispatcher(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::IAASBController> aasbController,
        std::shared_ptr<aasb::bridge::IConfigurationProvider> configProvider);

    // Logger.
    std::shared_ptr<agl::common::interfaces::ILogger> m_logger;

    // AASB controller to dispatch events to AASB.
    std::shared_ptr<aasb::bridge::IAASBController> m_aasbController;

    // Configuration provider
    std::shared_ptr<aasb::bridge::IConfigurationProvider> m_configProvider;
};

}  // namespace locationprovider
}  // namespace dispatcher
}  // namespace agl

#endif  // AGL_ALEXA_SVC_LOCATIONPROVIDER_CAPABILITY_H_

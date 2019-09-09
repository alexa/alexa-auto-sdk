/*Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AGL_ALEXA_SVC_LOCALVOICECONTROL_CARCONTROL_CAPABILITY_H_
#define AGL_ALEXA_SVC_LOCALVOICECONTROL_CARCONTROL_CAPABILITY_H_

#include <memory>
#include <string>

#include <aasb/interfaces/IAASBController.h>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/capability/ICapabilityMessageDispatcher.h"
#include "interfaces/utilities/logging/ILogger.h"

namespace agl {
namespace dispatcher {
namespace localVoiceControl {
namespace carControl {

/**
 * @c CarControlDispatcher provides the dispatching functionality to route car control messages
 * from vshl-capabilities AGL binding to AASB and similarly to route outgoing messages
 * from AASB to vshl-capabilities AGL binding.
 */
class CarControlDispatcher : public agl::capability::interfaces::ICapabilityMessageDispatcher {
public:
    /**
     * Creates a new instance of @c CarControlDispatcher.
     *
     * @param logger An instance of logger.
     * @param IAASBController AASB controller to dispatch events to AASB.
     * @param api AFB API instance to communicate with other AGL services (like vshl-capabilities)
     */
    static std::shared_ptr<CarControlDispatcher> create(
        std::shared_ptr<agl::common::interfaces::ILogger> logger,
        std::shared_ptr<aasb::bridge::IAASBController> aasbController,
        std::shared_ptr<agl::common::interfaces::IAFBApi> api);

    /**
     * Subscribe to receive carcontrol events from vshl-capabilities.
     *
     * @return true, if succeeded, false otherwise.
     */
    bool subscribeToCarControlEvents();

    // Responses from VSHL Capabilities.
    void onIsClimateOnResponse(const std::string& payload);
    void onIsClimateSyncOnResponse(const std::string& payload);
    void onIsAirRecirculationOnResponse(const std::string& payload);
    void onIsAirConditionerOnResponse(const std::string& payload);
    void onGetAirConditionerModeResponse(const std::string& payload);
    void onIsHeaterOnResponse(const std::string& payload);
    void onGetHeaterTemperatureResponse(const std::string& payload);
    void onIsFanOnResponse(const std::string& payload);
    void onGetFanSpeedResponse(const std::string& payload);
    void onIsVentOnResponse(const std::string& payload);
    void onGetVentPositionResponse(const std::string& payload);
    void onIsWindowDefrosterOnResponse(const std::string& payload);
    void onIsLightOnResponse(const std::string& payload);
    void onGetLightColorResponse(const std::string& payload);

    /// @name ICapabilityMessageDispatcher Functions
    /// @{
    void onReceivedDirective(const std::string& action, const std::string& jsonPayload) override;
    /// @}

private:
    /**
     * Constructor for @c CarControlDispatcher.
     */
    CarControlDispatcher(
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

}  // namespace carControl
}  // namespace localVoiceControl
}  // namespace dispatcher
}  // namespace agl

#endif  // AGL_ALEXA_SVC_LOCALVOICECONTROL_CARCONTROL_CAPABILITY_H_

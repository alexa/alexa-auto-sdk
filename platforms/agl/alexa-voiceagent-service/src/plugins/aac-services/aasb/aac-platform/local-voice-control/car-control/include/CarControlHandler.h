/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_LOCALVOICECONTROL_CARCONTROL_HANDLER_H
#define AASB_LOCALVOICECONTROL_CARCONTROL_HANDLER_H

#include <memory>

#include <AACE/CarControl/CarControl.h>
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"
#include "SyncOverAsync.h"

namespace aasb {
namespace carControl {

class CarControlHandler : public aace::carControl::CarControl {
public:
    static std::shared_ptr<CarControlHandler> create(
            std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
            std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    /**
     * PowerController
     */
    bool turnPowerControllerOn(const std::string& controlId) override;
    bool turnPowerControllerOff(const std::string& controlId) override;
    bool isPowerControllerOn(const std::string& controlId, bool& isOn) override;

    /**
     * ToggleController
     */
    bool turnToggleControllerOn(const std::string& controlId, const std::string& controllerId) override;
    bool turnToggleControllerOff(const std::string& controlId, const std::string& controllerId) override;
    bool isToggleControllerOn(const std::string& controlId, const std::string& controllerId, bool& isOn) override;

    /**
     * RangeController
     */
    bool setRangeControllerValue(const std::string& controlId, const std::string& controllerId, double value) override;
    bool adjustRangeControllerValue(const std::string& controlId, const std::string& controllerId, double delta) override;
    bool getRangeControllerValue(const std::string& controlId, const std::string& controllerId, double& value) override;

    /**
     * ModeController
     */
    bool setModeControllerValue(const std::string& controlId, const std::string& controllerId, const std::string& value) override;
    bool adjustModeControllerValue(const std::string& controlId, const std::string& controllerId, int delta) override;
    bool getModeControllerValue(const std::string& controlId, const std::string& controllerId, std::string& value) override;

    /**
     * Process incoming events from AASB client meant for topic @c TOPIC_CARCONTROL
     *
     * @param action Type of event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string &action, const std::string &payload);

private:
    // Constructor
    CarControlHandler(
            std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
            std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // Helper methods
    bool turnOnOffEndPoint(const std::string& controlId, const std::string& action, const std::string& controllerId = "");
    bool isEndPointTurnedOn(const std::string& controlId,
                            bool& isOn,
                            const std::string& action,
                            bridge::SyncOverAsync& syncOverAsyncHelper,
                            const std::string& controllerId = "");

    // Sync over async calls
    bridge::SyncOverAsync m_callIsPowerControllerOn;
    bridge::SyncOverAsync m_callIsToggleControllerOn;
    bridge::SyncOverAsync m_callGetModeControllerValue;
    bridge::SyncOverAsync m_callGetRangeControllerValue;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

} // namespace carControl
} // namespace aasb

#endif // AASB_LOCALVOICECONTROL_CARCONTROL_HANDLER_H
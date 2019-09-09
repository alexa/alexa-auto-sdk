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

    // Climate Control
    bool turnClimateControlOn(const std::string& zoneId) override;
    bool turnClimateControlOff(const std::string& zoneId) override;
    bool isClimateControlOn(const std::string& zoneId, bool& isOn) override;

    bool turnClimateSyncOn(const std::string& zoneId) override;
    bool turnClimateSyncOff(const std::string& zoneId) override;
    bool isClimateSyncOn(const std::string& zoneId, bool& isOn) override;

    bool turnAirRecirculationOn(const std::string& zoneId);
    bool turnAirRecirculationOff(const std::string& zoneId);
    bool isAirRecirculationOn(const std::string& zoneId, bool& isOn);

    // AirConditioner
    bool turnAirConditionerOn(const std::string& zoneId) override;
    bool turnAirConditionerOff(const std::string& zoneId) override;
    bool isAirConditionerOn(const std::string& zoneId, bool& isOn) override;

    bool setAirConditionerMode(const std::string& zoneId, AirConditionerMode mode);
    bool getAirConditionerMode(const std::string& zoneId, AirConditionerMode& mode);

    // Heater
    bool turnHeaterOn(const std::string& zoneId) override;
    bool turnHeaterOff(const std::string& zoneId) override;
    bool isHeaterOn(const std::string& zoneId, bool& isOn) override;

    bool setHeaterTemperature(const std::string& zoneId, double value) override;
    bool adjustHeaterTemperature(const std::string& zoneId, double delta) override;
    bool getHeaterTemperature(const std::string& zoneId, double& value) override;

    // Fan
    bool turnFanOn(const std::string& zoneId) override;
    bool turnFanOff(const std::string& zoneId) override;
    bool isFanOn(const std::string& zoneId, bool& isOn) override;

    bool setFanSpeed(const std::string& zoneId, double value) override;
    bool adjustFanSpeed(const std::string& zoneId, double delta) override;
    bool getFanSpeed(const std::string& zoneId, double& value) override;

    // Vent
    bool turnVentOn(const std::string& zoneId);
    bool turnVentOff(const std::string& zoneId);
    bool isVentOn(const std::string& zoneId, bool& isOn);

    bool setVentPosition(const std::string& zoneId, VentPosition value);
    bool getVentPosition(const std::string& zoneId, VentPosition& vent);

    // Defroster
    bool turnWindowDefrosterOn(const std::string& zoneId) override;
    bool turnWindowDefrosterOff(const std::string& zoneId) override;
    bool isWindowDefrosterOn(const std::string& zoneId, bool& isOn) override;

    // Lights
    bool turnLightOn(const std::string& zoneId, LightType type) override;
    bool turnLightOff(const std::string& zoneId, LightType type) override;
    bool isLightOn(const std::string& zoneId, LightType type, bool& isOn) override;

    bool setLightColor(const std::string& zoneId, LightType type, LightColor color) override;
    bool getLightColor(const std::string& zoneId, LightType type, LightColor& color) override;

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

    std::string ventPositionToString(const aace::carControl::CarControl::VentPosition& vent);
    aace::carControl::CarControl::VentPosition ventPositionFromString(const std::string& positionString);
    std::string lightTypeToString(const aace::carControl::CarControl::LightType& type);
    std::string airConditionerModeToString(const aace::carControl::CarControl::AirConditionerMode& mode);
    aace::carControl::CarControl::AirConditionerMode airConditionerModeFromString(const std::string& modeString);
    std::string lightColorToString(const aace::carControl::CarControl::LightColor& color);
    aace::carControl::CarControl::LightColor lightColorFromString(const std::string& colorString);

    // Helper methods
    bool turnOnOffEndPoint(const std::string& zoneId, const std::string& action, const std::string& type);
    bool isEndPointTurnedOn(const std::string& zoneId,
                            bool& isOn,
                            const std::string& action,
                            bridge::SyncOverAsync& syncOverAsyncHelper,
                            const std::string& type);
    bool setValue(const std::string& zoneId, double value, const std::string& action);
    bool adjustValue(const std::string& zoneId, double delta, const std::string& action);
    bool getValue(const std::string& zoneId,
                  double& value,
                  const std::string& action,
                  bridge::SyncOverAsync& syncOverAsyncHelper);

    // Sync over async calls
    bridge::SyncOverAsync m_callIsClimateControlOn;
    bridge::SyncOverAsync m_callIsClimateControlSyncOn;
    bridge::SyncOverAsync m_callIsAirRecirculationOn;
    bridge::SyncOverAsync m_callIsAirConditionerOn;
    bridge::SyncOverAsync m_callGetAirConditionerMode;
    bridge::SyncOverAsync m_callIsHeaterOn;
    bridge::SyncOverAsync m_callGetHeaterTemperature;
    bridge::SyncOverAsync m_callIsFanOn;
    bridge::SyncOverAsync m_callGetFanSpeed;
    bridge::SyncOverAsync m_callIsVentOn;
    bridge::SyncOverAsync m_callGetVentPosition;
    bridge::SyncOverAsync m_callIsWindowDefrosterOn;
    bridge::SyncOverAsync m_callIsLightOn;
    bridge::SyncOverAsync m_callGetLightColor;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

} // namespace carControl
} // namespace aasb

#endif // AASB_LOCALVOICECONTROL_CARCONTROL_HANDLER_H
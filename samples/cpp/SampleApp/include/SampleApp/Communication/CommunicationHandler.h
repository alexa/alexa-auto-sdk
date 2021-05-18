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

#ifndef SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H
#define SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H

#include <AACE/Communication/AlexaComms.h>

#include "SampleApp/Extension.h"

namespace sampleApp {
namespace communication {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CommunicationExtension
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class CommunicationExtension : public extension::Extension {
    // Enable extension for dynamic loading
    ENABLE_EXTENSION;

public:
    CommunicationExtension(const std::string& name) : Extension(name){};
    virtual ~CommunicationExtension() = default;

    bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<aace::propertyManager::PropertyManager> propertyManager) override;
    bool validate(const std::vector<nlohmann::json>& configs) override;
    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterfaceHandler() override;

private:
    std::shared_ptr<aace::communication::AlexaComms> m_communicationHandler;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CommunicationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class CommunicationHandler : public aace::communication::AlexaComms /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    CommunicationHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<CommunicationHandler> {
        return std::shared_ptr<CommunicationHandler>(new CommunicationHandler(args...));
    }

    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::communication::AlexaComms interface
    auto callDisplayInfo(const std::string& displayInfo) -> void override;
    auto callStateChanged(CallState state) -> void override;

protected:
    /**
     * Helper method to display the state and display info.
     */
    auto showDisplayInfo() -> void;
    auto showState() -> void;
    auto callStateToString(CallState state) -> std::string;

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;

    /// Call display info
    std::string m_callDisplayInfo;

    /// Call state
    CallState m_callState;
};

}  // namespace communication
}  // namespace sampleApp

#endif  // SAMPLEAPP_COMMUNICATIONS_COMMUNICATIONS_HANDLER_H

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
#ifndef AASB_ALEXA_ALERTSHANDLER_H
#define AASB_ALEXA_ALERTSHANDLER_H

#include <memory>

#include <AACE/Alexa/Alerts.h>
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

/**
 * AASB Implementation for @c aace::alexa::Alerts
 */
class AlertsHandler : public aace::alexa::Alerts {
public:
    /**
     * Creates a new instance of @c AlertsHandler.
     *
     * @param logger An instance of logger.
     * @param responseDispatcher An object through which the directives for alerts will be sent
     *
     */
    static std::shared_ptr<AlertsHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // aace::alexa::Alerts interface

    auto alertStateChanged(const std::string& alertToken, Alerts::AlertState state, const std::string& reason)
        -> void override;
    auto alertCreated(const std::string& alertToken, const std::string& detailedInfo) -> void override;
    auto alertDeleted(const std::string& alertToken) -> void override;

private:
    AlertsHandler(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // To send directive to service
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_ALERTSHANDLER_H
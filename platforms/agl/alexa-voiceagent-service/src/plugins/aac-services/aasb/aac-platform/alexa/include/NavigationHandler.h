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
#ifndef AASB_NAVIGATION_NAVIGATIONHANDLER_H
#define AASB_NAVIGATION_NAVIGATIONHANDLER_H

#include <memory>

#include <AACE/Navigation/Navigation.h>

#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace navigation {

/**
 * Platform implementation for @c aace::navigation::Navigation.
 *
 * AASB clients will be notified, through @c IAlexaCapabilityDirectiveListener, about the
 * directives to set new destination for navigation and for cancelling navigation.
 */
class NavigationHandler : public aace::navigation::Navigation
{
public:
    /**
     * Creates an instance of @c NavigationHandler.
     *
     * @param logger An instance of logger.
     * @param directiveDispatcher An object through which the directives for navigation
     *      received from alexa cloud will be dispatched to AASB clients.
     */
    static std::shared_ptr<NavigationHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::navigation::Navigation
    /// @{
    bool setDestination( const std::string& payload ) override;
    bool cancelNavigation() override;
    /// @}

private:
    /**
     * Constructor for @c NavigationHandler.
     */
    NavigationHandler(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // To send directive to service
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;
};

} // navigation
} // aasb

#endif // AASB_NAVIGATION_NAVIGATIONHANDLER_H
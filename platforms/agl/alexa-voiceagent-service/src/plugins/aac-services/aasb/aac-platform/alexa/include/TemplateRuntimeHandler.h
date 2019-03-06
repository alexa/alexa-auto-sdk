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
#ifndef AASB_ALEXA_TEMPLATERUNTIMEHANDLER_H
#define AASB_ALEXA_TEMPLATERUNTIMEHANDLER_H

#include <memory>

#include <AACE/Alexa/TemplateRuntime.h>
#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

class TemplateRuntimeHandler : public aace::alexa::TemplateRuntime {
public:
    static std::shared_ptr<TemplateRuntimeHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::alexa::TemplateRuntime Functions
    /// @{
    void renderTemplate(const std::string& payload) override;
    void clearTemplate() override;
    void renderPlayerInfo(const std::string& payload) override;
    void clearPlayerInfo() override;
    /// @}

private:
    TemplateRuntimeHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // DirectiveDispatcher to send status info
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_TEMPLATERUNTIMEHANDLER_H
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

#ifndef SAMPLEAPP_VPA_DIRECTIVEHANDLER_H
#define SAMPLEAPP_VPA_DIRECTIVEHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/VPA/DirectiveHandler.h>

namespace sampleApp {
namespace vpa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  VPADirectiveHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class VPADirectiveHandler : public aace::vpa::VpaDirective /* isa PlatformInterface */ {
  private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

  protected:
    VPADirectiveHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

  public:
    template <typename... Args> static auto create(Args &&... args) -> std::shared_ptr<VPADirectiveHandler> {
        return std::shared_ptr<VPADirectiveHandler>(new VPADirectiveHandler(args...));
    }

    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::vpa::VpaDirective interface

    auto sendDirective(const std::string &payload) -> bool override;

  private:
    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;
};

} // namespace vpa
} // namespace sampleApp

#endif // SAMPLEAPP_VPA_DIRECTIVEHANDLER_H
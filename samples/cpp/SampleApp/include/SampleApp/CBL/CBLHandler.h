/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_CBL_CBLHANDLER_H
#define SAMPLEAPP_CBL_CBLHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/CBL/CBL.h>

namespace sampleApp {

class ApplicationContext; // forward declare

namespace cbl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CBLHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBLHandler : public aace::cbl::CBL /* isa PlatformInterface */ {
  private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

  protected:
    CBLHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

  public:
    template <typename... Args> static auto create(Args &&... args) -> std::shared_ptr<CBLHandler> {
        return std::shared_ptr<CBLHandler>(new CBLHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::cbl::CBL interface

    auto cblStateChanged(CBLState state, CBLStateChangedReason reason, const std::string &url, const std::string &code) -> void override;
    auto clearRefreshToken() -> void override;

  private:
    auto getRefreshToken() -> std::string override;
    auto setRefreshToken(const std::string &refreshToken) -> void override;

  private:
    std::shared_ptr<ApplicationContext> m_applicationContext{};
    std::weak_ptr<View> m_console{};
    bool m_busy{};

    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;
    auto setupUI() -> void;
};

} // namespace cbl
} // namespace sampleApp

#endif // SAMPLEAPP_CBL_CBLHANDLER_H

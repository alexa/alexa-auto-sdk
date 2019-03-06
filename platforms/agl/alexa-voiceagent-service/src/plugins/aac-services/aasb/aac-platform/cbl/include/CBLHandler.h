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
#ifndef AASB_CBL_CBLHANDLER_H
#define AASB_CBL_CBLHANDLER_H

#include <memory>
#include <mutex>

#include <AACE/CBL/CBL.h>
#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace cbl {

class CBLHandler : public aace::cbl::CBL {
public:
    static std::shared_ptr<CBLHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::cbl::CBL Functions
    /// @{
    void cblStateChanged(CBLState state, CBLStateChangedReason reason, const std::string& url, const std::string& code) override;
    void clearRefreshToken() override;
    void setRefreshToken(const std::string& refreshToken) override;
    std::string getRefreshToken() override;
    /// @}

    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    CBLHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    std::string convertCBLStateToString(CBLState state);
    std::string convertCBLStateChangedReasonToString(CBLStateChangedReason reason);

    void setRefreshTokenInternal(const std::string& refreshToken);
    std::string getRefreshTokenInternal();
    void clearRefreshTokenInternal();

    // Mutex that protects updates to in memory refresh token
    std::mutex m_mutex;

    // In memory refresh token
    std::string m_RefreshToken;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // To send directive to service
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;
};

}  // namespace cbl
}  // namespace aasb

#endif  // AASB_CBL_CBLHANDLER_H
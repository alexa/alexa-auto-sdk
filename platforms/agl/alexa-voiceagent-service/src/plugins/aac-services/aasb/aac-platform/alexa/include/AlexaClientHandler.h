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
#ifndef AASB_ALEXA_ALEXACLIENTHANDLER_H
#define AASB_ALEXA_ALEXACLIENTHANDLER_H

#include <memory>

#include <AACE/Alexa/AlexaClient.h>
#include "DirectiveDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

class AlexaClientHandler : public aace::alexa::AlexaClient {
public:
    static std::shared_ptr<AlexaClientHandler> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    /// @name aace::alexa::AlexaClient Functions
    /// @{
    void dialogStateChanged(DialogState state) override;
    void connectionStatusChanged(ConnectionStatus status, ConnectionChangedReason reason) override;
    void authStateChanged(AuthState state, AuthError error) override;
    /// @}

    std::string getAuthState() const;
    std::string getDialogState() const;
    std::string getConnectionStatus() const;

private:
    AlexaClientHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher);

    static std::string convertDialogStateToString(DialogState state);
    static std::string convertConnectionStatusToString(ConnectionStatus status);
    static std::string convertConnectionChangedReasonToString(ConnectionChangedReason reason);
    static std::string convertAuthStateToString(AuthState state);
    static std::string convertAuthErrorToString(AuthError error);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // To send directive to service
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;

    AuthState m_authState;
    ConnectionStatus m_connectionStatus;
    DialogState m_dialogState;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_ALEXACLIENTHANDLER_H
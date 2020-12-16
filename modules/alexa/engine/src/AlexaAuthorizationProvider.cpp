/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// JSON for Modern C++
#include <nlohmann/json.hpp>

#include "AACE/Engine/Alexa/AlexaAuthorizationProvider.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::authorization;
using json = nlohmann::json;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AlexaAuthorizationProvider");

std::shared_ptr<AlexaAuthorizationProvider> AlexaAuthorizationProvider::create(
    const std::string& service,
    std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface) {
    try {
        ThrowIf(service.empty(), "invalidService");
        ThrowIfNull(authorizationManagerInterface, "invalidAuthorizationManagerInterface");

        auto alexaAuthorizationProvider =
            std::shared_ptr<AlexaAuthorizationProvider>(new AlexaAuthorizationProvider(service));
        ThrowIfNull(alexaAuthorizationProvider, "createFailed");

        ThrowIfNot(alexaAuthorizationProvider->initialize(authorizationManagerInterface), "initializeFailed");

        return alexaAuthorizationProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

AlexaAuthorizationProvider::AlexaAuthorizationProvider(const std::string& service) :
        m_service(service), alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

bool AlexaAuthorizationProvider::initialize(
    std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface) {
    try {
        m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
        m_authorizationManager = authorizationManagerInterface;

        auto m_authorizationManager_lock = m_authorizationManager.lock();
        ThrowIfNull(m_authorizationManager_lock, "invalidAuthorizationManagerReference");
        m_authorizationManager_lock->registerAuthorizationAdapter(m_service, shared_from_this());
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AlexaAuthorizationProvider::doShutdown() {
    m_executor.shutdown();
    m_authorizationManager.reset();
}

bool AlexaAuthorizationProvider::startAuthorization(const std::string& data) {
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock{m_mutex};
        if (m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED ||
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            AACE_ERROR(LX(TAG).d("m_currentAuthState", m_currentAuthState));
            Throw("previousStartAuthorizationyInProgressOrAuthorized");
        }

        auto m_authorizationManager_lock = m_authorizationManager.lock();
        ThrowIfNull(m_authorizationManager_lock, "invalidAuthorizationManagerReference");
        auto result = m_authorizationManager_lock->startAuthorization(m_service);
        if (result == AuthorizationManagerInterface::StartAuthorizationResult::FAILED) {
            m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;

            auto listener = getAuthorizationProviderListener();
            ThrowIfNull(listener, "invalidListenerReference");
            listener->onAuthorizationError(m_service, "START_AUTHORIZATION_FAILED");

            // Although start of authorization resulted in failure, we consider this
            // as successful call from the perspective of AuthorizationProvider API
            return true;
        }

        m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING;

        m_executor.submit([this]() {
            AACE_DEBUG(LX("startAuthorizationInsideExecutor"));
            auto listener = getAuthorizationProviderListener();
            ThrowIfNull(listener, "invalidListenerReference");
            listener->onAuthorizationStateChanged(
                m_service, AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING);
            // clang-format off
            json requestPayload = {
                {"type","requestAuthorization"}
            };
            // clang-format on
            listener->onEventReceived(m_service, requestPayload.dump());
        });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
}

bool AlexaAuthorizationProvider::cancelAuthorization() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "notSupportedWhenUnauthorized");

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            // Reset to UNAUTHORIZED if current state is AUTHORIZING, this allows to start  authorization again.
            m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
            m_executor.submit([this]() {
                auto listener = getAuthorizationProviderListener();
                ThrowIfNull(listener, "invalidListenerReference");
                listener->onAuthorizationStateChanged(
                    m_service, AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED);
            });
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
    return true;
}

bool AlexaAuthorizationProvider::logout() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING,
            "notAllowedDuringAuthorizing");
        m_executor.submit([this]() {
            AACE_DEBUG(LX("logoutInsideExecutor"));
            auto m_authorizationManager_lock = m_authorizationManager.lock();
            ThrowIfNull(m_authorizationManager_lock, "invalidAuthorizationManagerReference");
            auto result = m_authorizationManager_lock->logout(m_service);
            if (!result) {
                AACE_ERROR(LX(TAG).d("reason", "logoutFailed"));
                auto listener = getAuthorizationProviderListener();
                ThrowIfNull(listener, "invalidListenerReference");
                listener->onAuthorizationError(m_service, "LOGOUT_FAILED");
            }
        });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
}

bool AlexaAuthorizationProvider::sendEvent(const std::string& data) {
    AACE_DEBUG(LX(TAG));
    try {
        /* Expected JSON format
            {
                "type":"authStateChangeEvent",
                "payload": {
                    "state":{STRING}
                }
            }
        */
        std::lock_guard<std::mutex> lock{m_mutex};
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "ignoringAsInUnauthorizedState");
        auto m_authorizationManager_lock = m_authorizationManager.lock();
        ThrowIfNull(m_authorizationManager_lock, "invalidAuthorizationManagerReference");
        auto eventPayload = json::parse(data);
        if (eventPayload.find("type") != eventPayload.end() && eventPayload["type"].is_string()) {
            auto type = eventPayload["type"];
            if (type == "authStateChangeEvent") {
                if (eventPayload.find("payload") != eventPayload.end() && eventPayload["payload"].is_object()) {
                    auto payloadObject = eventPayload["payload"];
                    if (payloadObject.find("state") != payloadObject.end() && payloadObject["state"].is_string()) {
                        auto state = payloadObject["state"];
                        if (state == "AUTHORIZED") {
                            if (m_currentAuthState ==
                                AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
                                m_currentAuthState =
                                    AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED;
                                m_executor.submit([this]() {
                                    auto listener = getAuthorizationProviderListener();
                                    ThrowIfNull(listener, "invalidListenerReference");
                                    listener->onAuthorizationStateChanged(
                                        m_service,
                                        AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED);
                                });
                            }
                            m_authorizationManager_lock->authStateChanged(
                                m_service,
                                AuthorizationManagerInterface::State::REFRESHED,
                                AuthorizationManagerInterface::Error::SUCCESS);
                        } else if (state == "UNAUTHORIZED") {
                            m_authorizationManager_lock->authStateChanged(
                                m_service,
                                AuthorizationManagerInterface::State::UNINITIALIZED,
                                AuthorizationManagerInterface::Error::SUCCESS);
                        }
                    } else {
                        AACE_ERROR(LX(TAG).d("unknownState", data));
                        return false;
                    }
                } else {
                    AACE_ERROR(LX(TAG).d("invalidPayload", data));
                    return false;
                }
            }
        } else {
            AACE_ERROR(LX(TAG).d("missingType", data));
            return false;
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AlexaAuthorizationProvider::deregister() {
    AACE_DEBUG(LX(TAG));
    auto listener = getAuthorizationProviderListener();
    ThrowIfNull(listener, "invalidListenerReference");

    // clang-format off
    json requestPayload = {
        {"type","logout"}
    };
    // clang-format on
    listener->onEventReceived(m_service, requestPayload.dump());

    listener->onAuthorizationStateChanged(
        m_service, AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
}

std::string AlexaAuthorizationProvider::getAuthToken() {
    // To prevent flooding of debug logs, no debug log added here.
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "inUnauthorizedState");
        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");

        auto result = listener->onGetAuthorizationData(m_service, "accessToken");
        if (result.empty()) {
            return "";
        }
        auto accessTokenJson = json::parse(result);
        return accessTokenJson.at("accessToken");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("m_currentAuthState", m_currentAuthState));
        return "";
    }
}

void AlexaAuthorizationProvider::onAuthFailure(const std::string& token) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "inUnauthorizedState");
        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");
        listener->onAuthorizationError(m_service, "AUTH_FAILURE", token);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("m_currentAuthState", m_currentAuthState));
        return;
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace

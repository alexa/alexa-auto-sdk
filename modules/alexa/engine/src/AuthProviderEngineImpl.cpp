/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <sstream>
#include <nlohmann/json.hpp>

#include "AACE/Engine/Alexa/AuthProviderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;
using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AuthProviderEngineImpl");

/// Service name used to register with @c AuthorizationManager
static const std::string SERVICE_NAME = "alexa:authProviderEngineImpl";

/// Timeout to wait for the callbacks from authorization provider. Timoout is set
/// to sufficiently large value to allow any AVS cloud sync that maybe in progress
/// when authorization or logout is triggered.
static const std::chrono::seconds AUTHORIZATION_PROVIDER_DEFAULT_TIMEOUT(15);

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AuthProviderEngineImpl";

/// Counter metrics for AuthProvider Platform APIs
static const std::string METRIC_AUTH_PROVIDER_GET_AUTH_TOKEN = "GetAuthToken";
static const std::string METRIC_AUTH_PROVIDER_GET_AUTH_STATE = "GetAuthState";
static const std::string METRIC_AUTH_PROVIDER_AUTH_FAILURE = "AuthFailure";
static const std::string METRIC_AUTH_PROVIDER_AUTH_STATE_CHANGED = "AuthStateChanged";

std::shared_ptr<AuthProviderEngineImpl> AuthProviderEngineImpl::create(
    std::shared_ptr<aace::alexa::AuthProvider> authProvider,
    std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface) {
    std::shared_ptr<AuthProviderEngineImpl> authProviderEngineImpl = nullptr;
    try {
        ThrowIfNull(authProvider, "invalidAuthProviderPlatformInterface");
        ThrowIfNull(authorizationManagerInterface, "invalidAuthorizationManagerInterface");

        authProviderEngineImpl = std::shared_ptr<AuthProviderEngineImpl>(new AuthProviderEngineImpl(authProvider));

        ThrowIfNot(
            authProviderEngineImpl->initialize(authorizationManagerInterface),
            "initializeAuthProviderEngineImplFailed");

        // set the platform engine interface reference
        authProvider->setEngineInterface(authProviderEngineImpl);

        return authProviderEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (authProviderEngineImpl != nullptr) {
            authProviderEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool AuthProviderEngineImpl::initialize(
    std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface) {
    try {
        m_alexaAuthorizationProvider = AlexaAuthorizationProvider::create(SERVICE_NAME, authorizationManagerInterface);
        ThrowIfNull(m_alexaAuthorizationProvider, "createAlexaAuthorizationProviderFailed");
        m_alexaAuthorizationProvider->setListener(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

AuthProviderEngineImpl::AuthProviderEngineImpl(std::shared_ptr<aace::alexa::AuthProvider> authProvider) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_state(AuthProviderEngineState::INITIALIZED),
        m_authProviderPlatformInterface(authProvider),
        m_resetToAuthorizingState(false) {
}

void AuthProviderEngineImpl::doShutdown() {
    if (m_alexaAuthorizationProvider) {
        m_alexaAuthorizationProvider->shutdown();
        m_alexaAuthorizationProvider.reset();
    }
    if (m_authProviderPlatformInterface != nullptr) {
        m_authProviderPlatformInterface->setEngineInterface(nullptr);
    }
}

void AuthProviderEngineImpl::startAuthorization() {
    AACE_DEBUG(LX(TAG));
    try {
        std::unique_lock<std::mutex> lock(m_stateMutex);
        ThrowIf(m_state == AuthProviderEngineState::AUTHORIZING, "authorizationInProgress");
        ThrowIf(m_state == AuthProviderEngineState::AUTHORIZED, "authorizationAlreadyActive");

        if (!m_alexaAuthorizationProvider->startAuthorization("")) {
            Throw("startAuthorizationFailed");
        }
        // Wait for confirmation callback from authorization provider before considering this synchronous event complete
        std::unique_lock<std::mutex> cv_lock(m_cvMutex);
        if (!m_cv.wait_for(cv_lock, AUTHORIZATION_PROVIDER_DEFAULT_TIMEOUT, [this]() {
                return (m_state == AuthProviderEngineState::AUTHORIZING || m_state == AuthProviderEngineState::ERROR);
            })) {
            AACE_ERROR(LX("startAuthorizationFailed").d("reason", "timeout"));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthProviderEngineImpl::stopAuthorization() {
    AACE_DEBUG(LX(TAG));
    try {
        std::unique_lock<std::mutex> lock(m_stateMutex);
        ThrowIf(m_state == AuthProviderEngineState::UNAUTHORIZED, "authorizationNotActive");

        // Between stop and start of the Engine, the customer Id  might have changed.
        // The Engine cannot detect a changed customer Id, so trigger the logout process.
        if (m_state == AuthProviderEngineState::AUTHORIZED) {
            AACE_DEBUG(LX(TAG).m("performLogout"));
            if (!m_alexaAuthorizationProvider->logout()) {
                Throw("logoutFailed");
            }
        } else if (m_state == AuthProviderEngineState::AUTHORIZING) {
            AACE_DEBUG(LX(TAG).m("performCancel"));
            if (!m_alexaAuthorizationProvider->cancelAuthorization()) {
                Throw("cancelAuthorizationFailed");
            }
        }
        // Wait for confirmation callback from authorization provider before considering this synchronous event complete
        std::unique_lock<std::mutex> cv_lock(m_cvMutex);
        if (!m_cv.wait_for(cv_lock, AUTHORIZATION_PROVIDER_DEFAULT_TIMEOUT, [this]() {
                return (m_state == AuthProviderEngineState::UNAUTHORIZED || m_state == AuthProviderEngineState::ERROR);
            })) {
            AACE_ERROR(LX("logoutOrCancelAuthorizationFailed").d("reason", "timeout"));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthProviderEngineImpl::onAuthStateChanged(AuthState authState, AuthError authError) {
    std::stringstream state;
    std::stringstream error;
    state << authState;
    error << authError;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onAuthStateChanged",
        {METRIC_AUTH_PROVIDER_AUTH_STATE_CHANGED, state.str(), error.str()});
    AACE_DEBUG(LX(TAG).d("authState", authState).d("authError", authError));
    try {
        std::unique_lock<std::mutex> lock(m_stateMutex);
        ThrowIf(m_state == AuthProviderEngineState::UNAUTHORIZED, "authorizationNotActive");

        json payload;
        if (authState == AuthState::REFRESHED) {
            payload["state"] = "AUTHORIZED";
        } else {
            payload["state"] = "UNAUTHORIZED";
        }
        // clang-format off
        json eventJson = {
                {"type", "authStateChangeEvent"}, 
                {"payload", payload}
        };
        // clang-format on
        m_alexaAuthorizationProvider->sendEvent(eventJson.dump());

        // Between UNINITIALIZED and REFRESHED auth states, the customer Id might have changed.
        // The Engine cannot detect a changed customer Id, therefore trigger logout on AuthState::UNINITIALIZED.
        if (authState == AuthState::UNINITIALIZED) {
            if (!m_alexaAuthorizationProvider->logout()) {
                Throw("logoutFailed");
            }
            // Wait for confirmation callback from authorization provider before considering this synchronous event complete
            if (!m_cv.wait_for(lock, AUTHORIZATION_PROVIDER_DEFAULT_TIMEOUT, [this]() {
                    return (
                        m_state == AuthProviderEngineState::UNAUTHORIZED || m_state == AuthProviderEngineState::ERROR);
                })) {
                AACE_ERROR(LX("logoutFailed").d("reason", "timeout"));
            }
            // Logging out sets the AuthProviderEngineState to UNAUTHORIZED which then prevents application from
            // doing subsequent re-login until Engine is restarted. As SDK now supports re-login without requiring
            // Engine to be restarted, we must reset AuthProviderEngineState to AUTHORIZING.
            m_resetToAuthorizingState = true;
            if (!m_alexaAuthorizationProvider->startAuthorization("")) {
                Throw("startAuthorizationFailed");
            }

            // Wait for confirmation callback from authorization provider before considering this synchronous event complete
            if (!m_cv.wait_for(lock, AUTHORIZATION_PROVIDER_DEFAULT_TIMEOUT, [this]() {
                    return (
                        m_state == AuthProviderEngineState::AUTHORIZING || m_state == AuthProviderEngineState::ERROR);
                })) {
                m_resetToAuthorizingState = false;
                AACE_ERROR(LX("startAuthorizationFailed").d("reason", "timeout"));
            }
        }
    } catch (std::exception& ex) {
        m_resetToAuthorizingState = false;
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
};

void AuthProviderEngineImpl::onAuthorizationStateChanged(
    const std::string& service,
    AuthorizationProviderListenerInterface::AuthorizationState state) {
    AACE_DEBUG(LX(TAG).d("service", service).d("state", state));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        std::unique_lock<std::mutex> lock(m_cvMutex);
        if (state == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            m_state = AuthProviderEngineState::AUTHORIZING;
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED) {
            m_state = AuthProviderEngineState::AUTHORIZED;
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED) {
            m_state = AuthProviderEngineState::UNAUTHORIZED;
        }
        lock.unlock();

        m_cv.notify_one();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthProviderEngineImpl::onAuthorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& message) {
    AACE_DEBUG(LX(TAG).d("service", service).d("error", error));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");

        if (error == "AUTH_FAILURE") {
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onAuthorizationError", {METRIC_AUTH_PROVIDER_AUTH_FAILURE});
            m_authProviderPlatformInterface->authFailure(message);
        } else if (error == "START_AUTHORIZATION_FAILED" || error == "LOGOUT_FAILED") {
            AACE_ERROR(LX(TAG).d("service", service).d("error", error));
            m_resetToAuthorizingState = false;
            std::unique_lock<std::mutex> lock(m_cvMutex);
            m_state = AuthProviderEngineState::ERROR;
            lock.unlock();

            m_cv.notify_one();
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AuthProviderEngineImpl::onEventReceived(const std::string& service, const std::string& request) {
    AACE_DEBUG(LX(TAG).d("service", service));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");

        auto requestPayload = json::parse(request);
        if (requestPayload.find("type") != requestPayload.end() && requestPayload["type"].is_string()) {
            auto type = requestPayload["type"];
            if (type == "requestAuthorization") {
                if (!m_resetToAuthorizingState) {  // Prevent calling platform API when we are silently falling back to AUTHORIZING
                    emitCounterMetrics(
                        METRIC_PROGRAM_NAME_SUFFIX, "onEventReceived", {METRIC_AUTH_PROVIDER_GET_AUTH_STATE});
                    auto state = m_authProviderPlatformInterface->getAuthState();
                    json payload;
                    if (state == AuthState::REFRESHED) {
                        payload["state"] = "AUTHORIZED";
                    } else {
                        payload["state"] = "UNAUTHORIZED";
                    }
                    // clang-format off
                    json eventJson = {
                        {"type", "authStateChangeEvent"}, 
                        {"payload", payload}
                    };
                    // clang-format on
                    m_alexaAuthorizationProvider->sendEvent(eventJson.dump());
                } else {
                    AACE_INFO(LX(TAG)
                                  .m("ignoreRequestAuthorizationEvent")
                                  .d("m_resetToAuthorizingState", m_resetToAuthorizingState));
                    m_resetToAuthorizingState = false;
                }
            } else if (type == "logout") {
                //no-op
            } else {
                AACE_WARN(LX(TAG).d("service", service).d("request", request));
            }
        } else {
            Throw("typeNotFound");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service).d("request", request));
    }
}

std::string AuthProviderEngineImpl::onGetAuthorizationData(const std::string& service, const std::string& key) {
    AACE_DEBUG(LX(TAG).d("service", service));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");

        if (key == "accessToken") {
            emitCounterMetrics(
                METRIC_PROGRAM_NAME_SUFFIX, "onGetAuthorizationData", {METRIC_AUTH_PROVIDER_GET_AUTH_TOKEN});
            json accessTokenJson;
            accessTokenJson["accessToken"] = m_authProviderPlatformInterface->getAuthToken();
            return accessTokenJson.dump();
        }
        Throw("keyNotSupported");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("key", key));
    }
    return "";
}

void AuthProviderEngineImpl::onSetAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    AACE_DEBUG(LX(TAG));
    // no-op
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace

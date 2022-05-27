/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <nlohmann/json.hpp>

#include "AACE/Engine/CBL/CBLEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace cbl {

using namespace aace::engine::utils::metrics;
using namespace aace::cbl;
using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLEngineImpl");

/// Service name used to register with @c AuthorizationManager
static const std::string SERVICE_NAME = "alexa:cblEngineImpl";

/// Timeout to wait for the callbacks from authorization provider.
static const std::chrono::seconds TIMEOUT(2);

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "CBLEngineImpl";

/// Counter metrics for CBL Platform APIs
static const std::string METRIC_CBL_CBL_STATE_CHANGED = "CblStateChanged";
static const std::string METRIC_CBL_CLEAR_REFRESH_TOKEN = "ClearRefreshToken";
static const std::string METRIC_CBL_SET_REFRESH_TOKEN = "SetRefreshToken";
static const std::string METRIC_CBL_GET_REFRESH_TOKEN = "GetRefreshToken";
static const std::string METRIC_CBL_SET_USER_PROFILE = "SetUserProfile";
static const std::string METRIC_CBL_START = "Start";
static const std::string METRIC_CBL_CANCEL = "Cancel";
static const std::string METRIC_CBL_RESET = "Reset";

CBLEngineImpl::CBLEngineImpl(std::shared_ptr<CBL> cblPlatformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_cblPlatformInterface(cblPlatformInterface),
        m_state(CBLEngineState::INITIALIZED) {
}

std::shared_ptr<CBLEngineImpl> CBLEngineImpl::create(
    std::shared_ptr<CBL> cblPlatformInterface,
    std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::chrono::seconds codePairRequestTimeout,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
    std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
    bool enableUserProfile) {
    std::shared_ptr<CBLEngineImpl> cblEngineImpl = nullptr;

    try {
        ThrowIfNull(cblPlatformInterface, "invalidCBLPlatformInterface");

        cblEngineImpl = std::shared_ptr<CBLEngineImpl>(new CBLEngineImpl(cblPlatformInterface));

        ThrowIfNot(
            cblEngineImpl->initialize(
                authorizationManagerInterface,
                deviceInfo,
                codePairRequestTimeout,
                alexaEndpoints,
                localeAssetManager,
                networkObserver,
                propertyManager,
                enableUserProfile),
            "initializeCBLEngineImplFailed");

        // set the cbb engine interface
        cblPlatformInterface->setEngineInterface(cblEngineImpl);

        return cblEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (cblEngineImpl != nullptr) {
            cblEngineImpl->shutdown();
        }
        return nullptr;
    }
}

bool CBLEngineImpl::initialize(
    std::shared_ptr<aace::engine::alexa::AuthorizationManagerInterface> authorizationManagerInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::chrono::seconds codePairRequestTimeout,
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> alexaEndpoints,
    std::weak_ptr<aace::engine::alexa::LocaleAssetsManager> localeAssetManager,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
    bool enableUserProfile) {
    try {
        ThrowIfNull(authorizationManagerInterface, "invalidAuthorizationManagerInterface");
        ThrowIfNull(deviceInfo, "invalidDeviceInfo");

        auto configuration =
            CBLConfiguration::create(deviceInfo, codePairRequestTimeout, alexaEndpoints, localeAssetManager);
        ThrowIfNull(configuration, "nullCBLAuthDelegateConfiguration");

        m_cblAuthorizationProvider = CBLAuthorizationProvider::create(
            SERVICE_NAME,
            authorizationManagerInterface,
            configuration,
            networkObserver,
            propertyManager,
            enableUserProfile,
            shared_from_this());
        ThrowIfNull(m_cblAuthorizationProvider, "createCBLAuthorizationProviderFailed");
        m_cblAuthorizationProvider->setListener(shared_from_this());

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

void CBLEngineImpl::doShutdown() {
    if (m_cblAuthorizationProvider) {
        m_cblAuthorizationProvider->shutdown();
        m_cblAuthorizationProvider.reset();
    }
    if (m_cblPlatformInterface) {
        m_cblPlatformInterface->setEngineInterface(nullptr);
        m_cblPlatformInterface.reset();
    }
}

void CBLEngineImpl::enable() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "enable", {METRIC_CBL_GET_REFRESH_TOKEN});
    auto refreshToken = m_cblPlatformInterface->getRefreshToken();
    if (!refreshToken.empty()) {
        json refreshTokenJson;
        refreshTokenJson["refreshToken"] = refreshToken;
        m_cblAuthorizationProvider->startAuthorizationLegacy(refreshTokenJson.dump(), false);
    }
}

void CBLEngineImpl::disable() {
    AACE_DEBUG(LX(TAG));
    onCancel();
}

void CBLEngineImpl::onStart() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onStart", {METRIC_CBL_START});
    // Explicit start, pass empty data.
    m_cblAuthorizationProvider->startAuthorizationLegacy("", true);
}

void CBLEngineImpl::onCancel() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onCancel", {METRIC_CBL_CANCEL});
    m_cblAuthorizationProvider->cancelAuthorization();
}

void CBLEngineImpl::onReset() {
    AACE_DEBUG(LX(TAG));
    try {
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onReset", {METRIC_CBL_RESET});
        // Keeping the backward compatibility by synchronizing the logout process.
        if (!m_cblAuthorizationProvider->logout()) {
            Throw("logoutFailed");
        }
        // Wait for confirmation callback from authorization provider before
        // considering this synchronous event complete
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_cv.wait_for(lock, TIMEOUT, [this]() {
                return (m_state == CBLEngineState::UNAUTHORIZED || m_state == CBLEngineState::ERROR);
            })) {
            AACE_ERROR(LX("logoutFailed").d("reason", "timeout"));
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void CBLEngineImpl::onAuthorizationStateChanged(
    const std::string& service,
    AuthorizationProviderListenerInterface::AuthorizationState state) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        std::lock_guard<std::mutex> lock(m_mutex);
        if (state == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            m_state = CBLEngineState::AUTHORIZING;
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED) {
            m_state = CBLEngineState::AUTHORIZED;
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED) {
            m_state = CBLEngineState::UNAUTHORIZED;
        } else {
            AACE_WARN(LX(TAG).m("unexpectedState").d("service", service).d("state", state));
        }
        m_cv.notify_one();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service));
    }
}

void CBLEngineImpl::onAuthorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& message) {
    AACE_DEBUG(LX(TAG));
    std::stringstream cblState;
    std::stringstream reason;
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        if (error == "LOGOUT_FAILED") {
            AACE_ERROR(LX(TAG).d("service", service).d("error", error));
            std::lock_guard<std::mutex> lock(m_mutex);
            m_state = CBLEngineState::ERROR;

            m_cv.notify_one();
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void CBLEngineImpl::onEventReceived(const std::string& service, const std::string& request) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        auto requestPayload = json::parse(request);
        std::stringstream cblState;
        std::stringstream reason;
        if (requestPayload.find("type") != requestPayload.end() && requestPayload["type"].is_string()) {
            auto type = requestPayload["type"];
            if (type == "cbl-code") {
                if (requestPayload.find("payload") != requestPayload.end() && requestPayload["payload"].is_object()) {
                    auto payload = requestPayload["payload"];
                    if (payload.find("code") != payload.end() && payload["code"].is_string() &&
                        payload.find("url") != payload.end() && payload["url"].is_string()) {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        auto eventListeners = m_eventListeners;
                        lock.unlock();
                        for (const auto& eventListener : eventListeners) {
                            if (auto listener = eventListener.lock()) {
                                listener->onCBLCode(payload["url"], payload["code"]);
                            }
                        }
                    } else {
                        Throw("invalidCodeOrUrl");
                    }
                } else {
                    Throw("missingPayload");
                }
            } else if (type == "user-profile") {
                if (requestPayload.find("payload") != requestPayload.end() && requestPayload["payload"].is_object()) {
                    auto payload = requestPayload["payload"];
                    if (payload.find("name") != payload.end() && payload["name"].is_string() &&
                        payload.find("email") != payload.end() && payload["email"].is_string()) {
                        emitCounterMetrics(
                            METRIC_PROGRAM_NAME_SUFFIX, "onEventReceived", {METRIC_CBL_SET_USER_PROFILE});
                        m_cblPlatformInterface->setUserProfile(payload["name"], payload["email"]);
                    } else {
                        Throw("invalidNameOrEmail");
                    }
                } else {
                    Throw("missingPayload");
                }
            } else {
                Throw("typeNotSupported");
            }
        } else {
            Throw("typeNotFound");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("service", service));
    }
}

std::string CBLEngineImpl::onGetAuthorizationData(const std::string& service, const std::string& key) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        if (key == "refreshToken") {
            json refreshTokenJson;
            emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetAuthorizationData", {METRIC_CBL_GET_REFRESH_TOKEN});
            refreshTokenJson["refreshToken"] = m_cblPlatformInterface->getRefreshToken();
            return refreshTokenJson.dump();
        }
        AACE_DEBUG(LX(TAG).d("key", key));
        Throw("keyNotSupported");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("key", key));
        return "";
    }
}

void CBLEngineImpl::onSetAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");

        if (key == "refreshToken") {
            if (!data.empty()) {
                auto refreshTokenJson = json::parse(data);
                if (refreshTokenJson.contains("refreshToken") && refreshTokenJson["refreshToken"].is_string()) {
                    emitCounterMetrics(
                        METRIC_PROGRAM_NAME_SUFFIX, "onSetAuthorizationData", {METRIC_CBL_SET_REFRESH_TOKEN});
                    m_cblPlatformInterface->setRefreshToken(refreshTokenJson["refreshToken"]);
                } else {
                    Throw("invalidrefreshToken");
                }
            } else {
                emitCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX, "onSetAuthorizationData", {METRIC_CBL_CLEAR_REFRESH_TOKEN});
                m_cblPlatformInterface->clearRefreshToken();
            }
            return;
        }
        Throw("keyNotSupported");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("key", key));
    }
}

void CBLEngineImpl::addEventListener(std::shared_ptr<CBLEventListenerInterface> eventListener) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_eventListeners.push_back(eventListener);
}

void CBLEngineImpl::removeEventListener(std::shared_ptr<CBLEventListenerInterface> eventListener) {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_eventListeners.begin(); it != m_eventListeners.end(); ++it) {
        if (it->lock() == eventListener) {
            m_eventListeners.erase(it);
            return;
        }
    }
}

void CBLEngineImpl::cblStateChanged(
    aace::cbl::CBL::CBLState state,
    aace::cbl::CBL::CBLStateChangedReason reason,
    const std::string& url,
    const std::string& code) {
    if (m_cblPlatformInterface) {
        std::stringstream ssCblState;
        ssCblState << state;
        if (state == CBL::CBLState::STOPPING) {
            std::stringstream ssReason;
            ssReason << reason;
            emitCounterMetrics(
                METRIC_PROGRAM_NAME_SUFFIX,
                "cblStateChanged",
                {METRIC_CBL_CBL_STATE_CHANGED, ssCblState.str(), ssReason.str()});
        } else {
            emitCounterMetrics(
                METRIC_PROGRAM_NAME_SUFFIX, "cblStateChanged", {METRIC_CBL_CBL_STATE_CHANGED, ssCblState.str()});
        }
        m_cblPlatformInterface->cblStateChanged(state, reason, url, code);
    }
}

}  // namespace cbl
}  // namespace engine
}  // namespace aace

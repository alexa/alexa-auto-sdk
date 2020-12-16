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

#include <nlohmann/json.hpp>

#include "AACE/Engine/CBL/CBLEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace cbl {

using namespace aace::cbl;
using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLEngineImpl");

/// Service name used to register with @c AuthorizationManager
static const std::string SERVICE_NAME = "alexa:cblEngineImpl";

/// Timeout to wait for the callbacks from authorization provider.
static const std::chrono::seconds TIMEOUT(2);

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
    std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
    bool enableUserProfile) {
    try {
        ThrowIfNull(authorizationManagerInterface, "invalidAuthorizationManagerInterface");
        ThrowIfNull(deviceInfo, "invalidDeviceInfo");

        auto configuration =
            CBLConfiguration::create(deviceInfo, codePairRequestTimeout, alexaEndpoints, localeAssetManager);
        ThrowIfNull(configuration, "nullCBLAuthDelegateConfiguration");

        m_cblAuthorizationProvider = CBLAuthorizationProvider::create(
            SERVICE_NAME, authorizationManagerInterface, configuration, propertyManager, enableUserProfile);
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
    // Explict start, pass empty data.
    m_cblAuthorizationProvider->startAuthorizationLegacy("", true);
}

void CBLEngineImpl::onCancel() {
    AACE_DEBUG(LX(TAG));
    m_cblAuthorizationProvider->cancelAuthorization();
}

void CBLEngineImpl::onReset() {
    AACE_DEBUG(LX(TAG));
    try {
        // Keeping the backward compatiblity by synchronizing the logout process.
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
            m_cblPlatformInterface->cblStateChanged(CBL::CBLState::STARTING, CBL::CBLStateChangedReason::NONE, "", "");
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED) {
            m_state = CBLEngineState::AUTHORIZED;
            m_cblPlatformInterface->cblStateChanged(
                CBL::CBLState::REFRESHING_TOKEN, CBL::CBLStateChangedReason::NONE, "", "");
        } else if (state == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED) {
            m_state = CBLEngineState::UNAUTHORIZED;
            m_cblPlatformInterface->cblStateChanged(
                CBL::CBLState::STOPPING, CBL::CBLStateChangedReason::SUCCESS, "", "");
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
    try {
        ThrowIf(service != SERVICE_NAME, "unexpectedService");
        if (error == "UNKNOWN_ERROR") {
            m_cblPlatformInterface->cblStateChanged(CBL::CBLState::STOPPING, CBL::CBLStateChangedReason::ERROR, "", "");
        } else if (error == "TIMEOUT") {
            m_cblPlatformInterface->cblStateChanged(
                CBL::CBLState::STOPPING, CBL::CBLStateChangedReason::TIMEOUT, "", "");
        } else if (error == "CODE_PAIR_EXPIRED") {
            m_cblPlatformInterface->cblStateChanged(
                CBL::CBLState::STOPPING, CBL::CBLStateChangedReason::CODE_PAIR_EXPIRED, "", "");
        } else if (error == "AUTHORIZATION_EXPIRED") {
            m_cblPlatformInterface->cblStateChanged(
                CBL::CBLState::STOPPING, CBL::CBLStateChangedReason::AUTHORIZATION_EXPIRED, "", "");
        } else if (error == "LOGOUT_FAILED") {
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
        if (requestPayload.find("type") != requestPayload.end() && requestPayload["type"].is_string()) {
            auto type = requestPayload["type"];
            if (type == "cbl-code") {
                if (requestPayload.find("payload") != requestPayload.end() && requestPayload["payload"].is_object()) {
                    auto payload = requestPayload["payload"];
                    if (payload.find("code") != payload.end() && payload["code"].is_string() &&
                        payload.find("url") != payload.end() && payload["url"].is_string()) {
                        m_cblPlatformInterface->cblStateChanged(
                            CBL::CBLState::REQUESTING_CODE_PAIR, CBL::CBLStateChangedReason::SUCCESS, "", "");
                        m_cblPlatformInterface->cblStateChanged(
                            CBL::CBLState::CODE_PAIR_RECEIVED,
                            CBL::CBLStateChangedReason::SUCCESS,
                            payload["url"],
                            payload["code"]);
                        m_cblPlatformInterface->cblStateChanged(
                            CBL::CBLState::REQUESTING_TOKEN, CBL::CBLStateChangedReason::NONE, "", "");
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
                    m_cblPlatformInterface->setRefreshToken(refreshTokenJson["refreshToken"]);
                } else {
                    Throw("invalidrefreshToken");
                }
            } else {
                m_cblPlatformInterface->clearRefreshToken();
            }
            return;
        }
        Throw("keyNotSupported");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("key", key));
    }
}

}  // namespace cbl
}  // namespace engine
}  // namespace aace

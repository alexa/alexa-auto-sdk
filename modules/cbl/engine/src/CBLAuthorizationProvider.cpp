/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

// JSON for Modern C++
#include <nlohmann/json.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/Utils/LibcurlUtils/HttpResponseCodes.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/RetryTimer.h>

#include <AACE/Alexa/AlexaProperties.h>
#include "AACE/Engine/CBL/CBLAuthorizationProvider.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace cbl {

using namespace aace::engine::authorization;
using namespace aace::engine::alexa;
using namespace aace::engine::utils::metrics;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::libcurlUtils;
using namespace rapidjson;
using json = nlohmann::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLAuthorizationProvider");

/// Key for user_code values in JSON returned by @c LWA
static const char JSON_KEY_USER_CODE[] = "user_code";

/// Key for device_code values in JSON returned by @c LWA
static const char JSON_KEY_DEVICE_CODE[] = "device_code";

/// Key for verification_uri values in JSON returned by @c LWA
static const char JSON_KEY_VERIFICATION_URI[] = "verification_uri";

/// Key for expires_in values in JSON returned by @c LWA
static const char JSON_KEY_EXPIRES_IN[] = "expires_in";

/// Key for interval values in JSON returned by @c LWA
static const char JSON_KEY_INTERVAL[] = "interval";

/// Key for interval values in JSON returned by @c LWA
static const char JSON_KEY_TOKEN_TYPE[] = "token_type";

/// Key for access_token values in JSON returned by @c LWA
static const char JSON_KEY_ACCESS_TOKEN[] = "access_token";

/// Key for refresh_token values in JSON returned by @c LWA
static const char JSON_KEY_REFRESH_TOKEN[] = "refresh_token";

/// Key for error values in JSON returned by @c LWA
static const char JSON_KEY_ERROR[] = "error";

/// Expected token_type value returned from token requests to @c LWA.
static const std::string JSON_VALUE_BEARER = "bearer";

/// response_type key in POST requests to @c LWA.
static const std::string POST_KEY_RESPONSE_TYPE = "response_type";

/// client_id key in POST requests to @c LWA.
static const std::string POST_KEY_CLIENT_ID = "client_id";

/// scope key in POST requests to @c LWA.
static const std::string POST_KEY_SCOPE = "scope";

/// scope_data key in POST requests to @c LWA.
static const std::string POST_KEY_SCOPE_DATA = "scope_data";

/// grant_type key in POST requests to @c LWA.
static const std::string POST_KEY_GRANT_TYPE = "grant_type";

/// device_code key in POST requests to @c LWA.
static const std::string POST_KEY_DEVICE_CODE = "device_code";

/// user_code key in POST requests to @c LWA.
static const std::string POST_KEY_USER_CODE = "user_code";

/// refresh_token key in POST requests to @c LWA.
static const std::string POST_KEY_REFRESH_TOKEN = "refresh_token";

/// refresh_token value in POST requests to @c LWA.
static const std::string POST_VALUE_REFRESH_TOKEN = "refresh_token";

/// device_code value in POST requests to @c LWA.
static const std::string POST_VALUE_DEVICE_CODE = "device_code";

/// alexa:all value in POST requests to @c LWA.
static const std::string POST_VALUE_ALEXA_ALL = "alexa:all";

/// HTTP Header line specifying URL encoded data
static const std::string HEADER_LINE_URLENCODED = "Content-Type: application/x-www-form-urlencoded";

/// Prefix of HTTP header line specifying language.
static const std::string HEADER_LINE_LANGUAGE_PREFIX = "Accept-Language: ";

/// The Accept-Language header value to use if this class was not initialized with a locale.
static const std::string DEFAULT_ACCEPT_LANGUAGE_VALUE = "en-US";

/// Min time to wait between attempt to poll for a token while authentication is pending.
static const std::chrono::seconds MIN_TOKEN_REQUEST_INTERVAL = std::chrono::seconds(5);

/// Max time to wait between attempt to poll for a token while authentication is pending.
static const std::chrono::seconds MAX_TOKEN_REQUEST_INTERVAL = std::chrono::seconds(60);

/// Scale factor to apply to interval between token poll requests when a 'slow_down' response is received.
static const int TOKEN_REQUEST_SLOW_DOWN_FACTOR = 2;

/// Endpoint to request user profile
static const std::string USER_PROFILE_DEFAULT_ENDPOINT = "https://api.amazon.com/user/profile";

/// Host header information
static const std::string HOST_HEADER_DATA_USER_PROFILE = "Host: api.amazon.com";

/// Bearer header information
static const std::string BEARER_HEADER_DATA_USER_PROFILE = "Authorization: bearer ";

/// Success Error Reason
static const std::string AUTHORIZATION_ERROR_REASON_SUCCESS = "SUCCESS";

/// Timeout Error Reason
static const std::string AUTHORIZATION_ERROR_REASON_TIMEOUT = "TIMEOUT";

/// Unknown Error Reason
static const std::string AUTHORIZATION_ERROR_REASON_UNKNOWN_ERROR = "UNKNOWN_ERROR";

/// Code Pair Expired Error Reason
static const std::string AUTHORIZATION_ERROR_REASON_CODE_PAIR_EXPIRED = "CODE_PAIR_EXPIRED";

/// Authorization Expired Error Reason
static const std::string AUTHORIZATION_ERROR_REASON_AUTHORIZATION_EXPIRED = "AUTHORIZATION_EXPIRED";

/// JSON key for the refresh token
static const std::string AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY = "refreshToken";

/// Key for the refresh token used in set/get authorization data
static const std::string AUTHORIZATION_DATA_REFRESH_TOKEN_KEY = "refreshToken";

/// Authorization request type user profile
static const std::string AUTHORIZATION_REQUEST_TYPE_USER_PROFILE = "user-profile";

/// Authorization request type cbl code
static const std::string AUTHORIZATION_REQUEST_TYPE_CBL_CODE = "cbl-code";

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "CBLAuthorizationProvider";

/// Metric for CBL timeout
static const std::string METRIC_CODEPAIRREQUEST_TIMEOUT = "CodePairRequestTimeOut";

/// Metric for failed code pair request
static const std::string METRIC_CODEPAIRREQUEST_FAILED = "CodePairRequestFailed";

/// Metric for successful code pair request
static const std::string METRIC_CODEPAIRREQUEST_SUCCESS = "CodePairRequestSuccess";

/// Metric for expired code pair
static const std::string METRIC_CODEPAIR_EXPIRED = "CodePairExpired";

/// Metric for failed request for token
static const std::string METRIC_REQUESTTOKEN_FAILED = "RequestTokenFailed";

/// Metric for successful request for token
static const std::string METRIC_REQUESTTOKEN_SUCCESS = "RequestTokenSuccess";

/// Metric for failed refresh of token
static const std::string METRIC_REFRESHTOKEN_FAILED = "RefreshTokenFailed";

/// Metric for successful refresh of token
static const std::string METRIC_REFRESHTOKEN_SUCCESS = "RefreshTokenSuccess";

/// Map error names from @c LWA to @c AuthObserverInterface::Error values.
static const std::unordered_map<std::string, AuthObserverInterface::Error> g_nameToErrorMap = {
    {"authorization_pending", AuthObserverInterface::Error::AUTHORIZATION_PENDING},
    {"invalid_client", AuthObserverInterface::Error::INVALID_VALUE},
    {"invalid_code_pair", AuthObserverInterface::Error::INVALID_CODE_PAIR},
    {"invalid_grant", AuthObserverInterface::Error::AUTHORIZATION_EXPIRED},
    {"invalid_request", AuthObserverInterface::Error::INVALID_REQUEST},
    {"InvalidValue", AuthObserverInterface::Error::INVALID_VALUE},
    {"servererror", AuthObserverInterface::Error::SERVER_ERROR},
    {"slow_down", AuthObserverInterface::Error::SLOW_DOWN},
    {"unauthorized_client", AuthObserverInterface::Error::UNAUTHORIZED_CLIENT},
    {"unsupported_grant_type", AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE}};

/**
 * Helper function to convert from @c LWA error names to @c AuthObserverInterface::Error values.
 *
 * @param error The string in the @c error field returned by @c LWA
 * @return the Error enum code corresponding to @c error. If error is "", returns SUCCESS. If it is an unknown error,
 * returns UNKNOWN_ERROR.
 */
static AuthObserverInterface::Error getErrorCode(const std::string& error) {
    try {
        if (error.empty()) {
            return AuthObserverInterface::Error::SUCCESS;
        } else {
            auto it = g_nameToErrorMap.find(error);
            if (it != g_nameToErrorMap.end()) {
                return it->second;
            }
            Throw("unknownError");
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("error", error));
        return AuthObserverInterface::Error::UNKNOWN_ERROR;
    }
}

/**
 * Function to convert the number of times we have already retried to the time to perform the next retry.
 *
 * @param retryCount The number of times we have retried
 * @return The time that the next retry should be attempted
 */
static std::chrono::steady_clock::time_point calculateTimeToRetry(int retryCount) {
    /**
     * Table of retry backoff values based upon page 77 of
     * @see https://images-na.ssl-images-amazon.com/images/G/01/mwsportal/
     * doc/en_US/offamazonpayments/LoginAndPayWithAmazonIntegrationGuide.pdf
     */
    const static std::vector<int> retryBackoffTimes = {
        0,      // Retry 1:  0.00s range with 50% randomization: [ 0.0s.  0.0s]
        1000,   // Retry 2:  1.00s range with 50% randomization: [ 0.5s,  1.5s]
        2000,   // Retry 3:  2.00s range with 50% randomization: [ 1.0s,  3.0s]
        4000,   // Retry 4:  4.00s range with 50% randomization: [ 2.0s,  6.0s]
        10000,  // Retry 5: 10.00s range with 50% randomization: [ 5.0s, 15.0s]
        30000,  // Retry 6: 30.00s range with 50% randomization: [15.0s, 45.0s]
        60000   // Retry 7: 60.00s range with 50% randomization: [30.0s, 90.0s]
    };

    // Retry Timer Object.
    alexaClientSDK::avsCommon::utils::RetryTimer RETRY_TIMER(retryBackoffTimes);

    return std::chrono::steady_clock::now() + RETRY_TIMER.calculateTimeToRetry(retryCount);
}

/**
 * Map an HTTP status code to an @c AuthObserverInterface::Error value.
 *
 * @param code The code to map from.
 * @return The value the code was mapped to.
 */
static AuthObserverInterface::Error mapHTTPCodeToError(long code) {
    AuthObserverInterface::Error error = AuthObserverInterface::Error::INTERNAL_ERROR;
    switch (code) {
        case HTTPResponseCode::SUCCESS_OK:
            error = AuthObserverInterface::Error::SUCCESS;
            break;

        case HTTPResponseCode::BAD_REQUEST:
            error = AuthObserverInterface::Error::INVALID_REQUEST;
            break;

        case HTTPResponseCode::SERVER_INTERNAL_ERROR:
            error = AuthObserverInterface::Error::SERVER_ERROR;
            break;

        case HTTPResponseCode::HTTP_RESPONSE_CODE_UNDEFINED:
        case HTTPResponseCode::SUCCESS_NO_CONTENT:
        default:
            error = AuthObserverInterface::Error::UNKNOWN_ERROR;
            break;
    }
    AACE_DEBUG(LX(TAG).d("code", code).d("error", error));
    return error;
}

/**
 * Perform common parsing of an @c LWA response.
 *
 * @param response The response to parse.
 * @param document The document to populate from the body of the response.
 * @return The status from the initial parsing of the response.
 */
static AuthObserverInterface::Error parseLWAResponse(
    const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
    rapidjson::Document* document) {
    try {
        ThrowIfNull(document, "nullDocument");

        auto result = mapHTTPCodeToError(response.code);

        if (document->Parse(response.body.c_str()).HasParseError()) {
            AACE_ERROR(LX(TAG)
                           .d("reason", "parseJsonFailed")
                           .d("position", document->GetErrorOffset())
                           .d("error", GetParseError_En(document->GetParseError()))
                           .sensitive("body", response.body));
            if (AuthObserverInterface::Error::SUCCESS == result) {
                result = AuthObserverInterface::Error::UNKNOWN_ERROR;
            }
            return result;
        }

        if (result != AuthObserverInterface::Error::SUCCESS) {
            std::string error;
            auto it = document->FindMember(JSON_KEY_ERROR);
            if (it != document->MemberEnd() && it->value.IsString()) {
                error = it->value.GetString();
                if (!error.empty()) {
                    result = getErrorCode(error);
                    AACE_DEBUG(LX(TAG).d("error", error).d("errorCode", result));
                }
            }
        }
        return result;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return AuthObserverInterface::Error::INTERNAL_ERROR;
    }
}

std::shared_ptr<CBLAuthorizationProvider> CBLAuthorizationProvider::create(
    const std::string& service,
    std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface,
    std::shared_ptr<CBLConfigurationInterface> configuration,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver,
    std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
    bool enableUserProfile,
    std::shared_ptr<CBLLegacyEventNotificationInterface> legacyEventNotifier) {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(service.empty(), "invalidService");
        ThrowIfNull(authorizationManagerInterface, "invalidAuthorizationManagerInterface");
        ThrowIfNull(configuration, "nullCBLAuthDelegateConfiguration");
        ThrowIfNull(propertyManager, "nullPropertyManagerServiceInterface");

        auto cblAuthorizationProvider = std::shared_ptr<CBLAuthorizationProvider>(new CBLAuthorizationProvider(
            service, authorizationManagerInterface, configuration, enableUserProfile, legacyEventNotifier));
        ThrowIfNull(cblAuthorizationProvider, "createFailed");

        ThrowIfNot(cblAuthorizationProvider->initialize(propertyManager, networkObserver), "initializeFailed");

        return cblAuthorizationProvider;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

CBLAuthorizationProvider::CBLAuthorizationProvider(
    const std::string& service,
    std::shared_ptr<AuthorizationManagerInterface> authorizationManagerInterface,
    std::shared_ptr<CBLConfigurationInterface> configuration,
    bool enableUserProfile,
    std::shared_ptr<CBLLegacyEventNotificationInterface> legacyEventNotifier) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_configuration{configuration},
        m_isStopping{false},
        m_authFailureReported{false},
        m_networkWakeup{true},
        m_explicitAuthorizationRequest{false},
        m_legacyCBLExplicitStart{false},
        m_authState{AuthObserverInterface::State::UNINITIALIZED},
        m_authError{AuthObserverInterface::Error::SUCCESS},
        m_tokenExpirationTime{std::chrono::time_point<std::chrono::steady_clock>::max()},
        m_retryCount{0},
        m_newRefreshToken{false},
        m_threadActive{false},
        m_stateChangeReason{AUTHORIZATION_ERROR_REASON_SUCCESS},
        m_enableUserProfile{enableUserProfile},
        m_service(service),
        m_currentAuthState(AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED),
        m_authorizationManager(authorizationManagerInterface),
        m_legacyEventNotifier(legacyEventNotifier) {
}

bool CBLAuthorizationProvider::initialize(
    std::shared_ptr<aace::engine::propertyManager::PropertyManagerServiceInterface> propertyManager,
    std::shared_ptr<aace::engine::network::NetworkObservableInterface> networkObserver) {
    try {
        auto authorizationManager_lock = m_authorizationManager.lock();
        ThrowIfNull(authorizationManager_lock, "invalidAuthorizationManagerReference");
        authorizationManager_lock->registerAuthorizationAdapter(m_service, shared_from_this());

        if (m_enableUserProfile) {
            m_scope = POST_VALUE_ALEXA_ALL + " profile";
        } else {
            m_scope = POST_VALUE_ALEXA_ALL;
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        m_locale = propertyManager->getProperty(aace::alexa::property::LOCALE);
        propertyManager->addListener(aace::alexa::property::LOCALE, shared_from_this());
        lock.unlock();

        m_networkObserver = networkObserver;

        if (m_networkObserver != nullptr) {  // This could be null when NetworkInfoProvider interface is not registered.
            m_networkObserver->addObserver(shared_from_this());
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void CBLAuthorizationProvider::doShutdown() {
    m_executor.waitForSubmittedTasks();
    m_executor.shutdown();

    // Stop and wait for the thread to finish
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isStopping = true;
        m_wake.notify_one();
    }
    if (m_authorizationFlowThread.joinable()) {
        m_authorizationFlowThread.join();
    }
    if (m_configuration) {
        m_configuration.reset();
    }

    if (m_networkObserver != nullptr) {
        m_networkObserver->removeObserver(shared_from_this());
        m_networkObserver.reset();
    }
}

void CBLAuthorizationProvider::startAuthorizationLegacy(const std::string& data, bool explicitStart) {
    AACE_DEBUG(LX(TAG));
    m_legacyCBLExplicitStart = explicitStart;
    startAuthorization(data);
}

bool CBLAuthorizationProvider::startAuthorization(const std::string& data) {
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED ||
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            AACE_ERROR(LX(TAG).d("m_currentAuthState", m_currentAuthState));
            Throw("previousAuthorizationInProgressOrAuthorized");
        }

        // The m_explicitAuthorizationRequest is to set to true to indicate when a fresh authorization started by either
        // passing an empty data, empty json or empty refresh token inside the json. This is used later to fall back
        // to the code pair state if application by chance provides an invalid refresh token.
        m_explicitAuthorizationRequest = false;
        if (!data.empty()) {
            auto dataPayload = json::parse(data);
            if (dataPayload.contains(AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY)) {
                if (dataPayload[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY].is_string()) {
                    m_refreshToken = dataPayload[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY];
                    if (m_refreshToken.empty()) {
                        m_explicitAuthorizationRequest = true;
                    }
                } else {
                    Throw("refreshTokenIsNotString");
                }
            } else {
                m_explicitAuthorizationRequest = true;
            }
        } else {
            m_explicitAuthorizationRequest = true;
        }

        m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING;

        auto authorizationManager_lock = m_authorizationManager.lock();
        ThrowIfNull(authorizationManager_lock, "invalidAuthorizationManagerReference");
        auto result = authorizationManager_lock->startAuthorization(m_service);
        if (result == AuthorizationManagerInterface::StartAuthorizationResult::FAILED) {
            m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
            m_executor.submit([this]() {
                try {
                    auto listener = getAuthorizationProviderListener();
                    ThrowIfNull(listener, "invalidListenerReference");
                    listener->onAuthorizationError(m_service, "START_AUTHORIZATION_FAILED");
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "startAuthorizationInsideExecutor").d("reason", ex.what()));
                }
            });
            // Although start of authorization failed, we consider this
            // as successful call from the perspective of AuthorizationProvider API.
            return true;
        }

        if (m_threadActive == false) {
            m_isStopping = false;
            if (m_authorizationFlowThread.joinable()) {
                m_authorizationFlowThread.join();
            }
            m_threadActive = true;
            m_authorizationFlowThread = std::thread(&CBLAuthorizationProvider::handleAuthorizationFlow, this);
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
}

bool CBLAuthorizationProvider::cancelAuthorization() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "notSupportedWhenUnauthorized");

        stopAuthFlowThread(false);

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            // Reset to UNAUTHORIZED if current state is AUTHORIZING, this allows to start  authorization again.
            m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
            m_executor.submit([this]() {
                try {
                    auto listener = getAuthorizationProviderListener();
                    ThrowIfNull(listener, "invalidListenerReference");
                    listener->onAuthorizationStateChanged(
                        m_service, AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "cancelAuthorizationInsideExecutor").d("reason", ex.what()));
                }
            });
        }
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
}

bool CBLAuthorizationProvider::logout() {
    AACE_DEBUG(LX(TAG));
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING,
            "notAllowedDuringAuthorizing");
        stopAuthFlowThread(false);
        m_executor.submit([this]() {
            try {
                AACE_DEBUG(LX(TAG, "logoutInsideExecutor"));
                auto authorizationManager_lock = m_authorizationManager.lock();
                ThrowIfNull(authorizationManager_lock, "invalidAuthorizationManagerReference");
                auto result = authorizationManager_lock->logout(m_service);
                if (!result) {
                    auto listener = getAuthorizationProviderListener();
                    ThrowIfNull(listener, "invalidListenerReference");
                    listener->onAuthorizationError(m_service, "LOGOUT_FAILED");
                    Throw("logoutFailed");
                }
            } catch (std::exception& ex) {
                AACE_ERROR(LX(TAG, "logoutInsideExecutor").d("reason", ex.what()));
            }
        });
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("currentAuthState", m_currentAuthState).d("reason", ex.what()));
        return false;
    }
}

void CBLAuthorizationProvider::handleAuthorizationFlow() {
    AACE_DEBUG(LX(TAG));
    m_flowState = FlowState::STARTING;
    while (!isStopping()) {
        auto nextFlowState = FlowState::STOPPING;
        switch (m_flowState) {
            case FlowState::STARTING:
                nextFlowState = handleStarting();
                break;
            case FlowState::REQUESTING_CODE_PAIR:
                nextFlowState = handleRequestingCodePair();
                break;
            case FlowState::REQUESTING_TOKEN:
                nextFlowState = handleRequestingToken();
                break;
            case FlowState::REFRESHING_TOKEN:
                nextFlowState = handleRefreshingToken();
                break;
            case FlowState::STOPPING:
                nextFlowState = handleStopping();
                break;
        }
        m_flowState = nextFlowState;
    }
    m_threadActive = false;
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthorizationProvider::doPost(
    const std::string& url,
    const std::vector<std::string> headerLines,
    const std::vector<std::pair<std::string, std::string>>& data,
    std::chrono::seconds timeout) {
    try {
        // Creating the HttpPost on every doPost is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpPost = HttpPost::create();
        ThrowIfNull(httpPost, "nullHttpPost");

        return httpPost->doPost(url, headerLines, data, timeout);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse();
    }
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthorizationProvider::doGet(
    const std::string& url,
    const std::vector<std::string>& headers) {
    try {
        // Creating the HttpGet on every doGet is by design to ensure that curl in libcurlUtils uses the
        // latest provided curl options.
        auto httpGet = alexaClientSDK::avsCommon::utils::libcurlUtils::HttpGet::create();
        ThrowIfNull(httpGet, "nullHttpGet");

        return httpGet->doGet(url, headers, m_configuration->getRequestTimeout());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse();
    }
}

CBLAuthorizationProvider::FlowState CBLAuthorizationProvider::handleStarting() {
    try {
        AACE_DEBUG(LX(TAG));
        if (m_legacyEventNotifier) {
            m_legacyEventNotifier->cblStateChanged(
                CBLLegacyEventNotificationInterface::CBLState::STARTING,
                CBLLegacyEventNotificationInterface::CBLStateChangedReason::NONE);
        }

        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");
        listener->onAuthorizationStateChanged(
            m_service, AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING);

        if (m_legacyCBLExplicitStart) {
            m_legacyCBLExplicitStart = false;
            // The legacy CBL:start() requires to get the refresh token to decide on next flow state
            auto data = listener->onGetAuthorizationData(m_service, AUTHORIZATION_DATA_REFRESH_TOKEN_KEY);
            if (!data.empty()) {
                auto refreshTokenJson = json::parse(data);
                if (refreshTokenJson.contains(AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY) &&
                    refreshTokenJson[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY].is_string()) {
                    std::string refreshToken = refreshTokenJson[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY];
                    if (!refreshToken.empty()) {
                        return FlowState::REFRESHING_TOKEN;
                    }
                }
            }
            return FlowState::REQUESTING_CODE_PAIR;
        }

        if (m_explicitAuthorizationRequest) {
            return FlowState::REQUESTING_CODE_PAIR;
        }

        return FlowState::REFRESHING_TOKEN;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}  // namespace cbl

CBLAuthorizationProvider::FlowState CBLAuthorizationProvider::handleRequestingCodePair() {
    try {
        AACE_DEBUG(LX(TAG));
        if (m_legacyEventNotifier) {
            m_legacyEventNotifier->cblStateChanged(
                CBLLegacyEventNotificationInterface::CBLState::REQUESTING_CODE_PAIR,
                CBLLegacyEventNotificationInterface::CBLStateChangedReason::SUCCESS);
        }

        m_retryCount = 0;
        std::chrono::steady_clock::time_point codePairRequestTimeout =
            std::chrono::steady_clock::now() + m_configuration->getCodePairRequestTimeout();
        while (!isStopping()) {
            if (std::chrono::steady_clock::now() >= codePairRequestTimeout) {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX, "handleRequestingCodePair", METRIC_CODEPAIRREQUEST_TIMEOUT, 1);
                m_stateChangeReason = AUTHORIZATION_ERROR_REASON_TIMEOUT;
                return FlowState::STOPPING;
            }

            auto result = receiveCodePairResponse(requestCodePair());
            std::stringstream codePairResult;
            codePairResult << result;
            if (result == AuthObserverInterface::Error::SUCCESS) {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX, "handleRequestingCodePair", METRIC_CODEPAIRREQUEST_SUCCESS, 1);
            } else {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX,
                    "handleRequestingCodePair",
                    {METRIC_CODEPAIRREQUEST_FAILED, codePairResult.str()});
            }
            switch (result) {
                case AuthObserverInterface::Error::SUCCESS:
                    return FlowState::REQUESTING_TOKEN;
                case AuthObserverInterface::Error::UNKNOWN_ERROR:
                case AuthObserverInterface::Error::AUTHORIZATION_FAILED:
                case AuthObserverInterface::Error::SERVER_ERROR:
                case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED:
                case AuthObserverInterface::Error::INVALID_CODE_PAIR:
                case AuthObserverInterface::Error::AUTHORIZATION_PENDING:
                case AuthObserverInterface::Error::SLOW_DOWN:
                    break;
                case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT:
                case AuthObserverInterface::Error::INVALID_REQUEST:
                case AuthObserverInterface::Error::INVALID_VALUE:
                case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE:
                case AuthObserverInterface::Error::INTERNAL_ERROR:
                case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID: {
                    setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                    m_stateChangeReason = AUTHORIZATION_ERROR_REASON_UNKNOWN_ERROR;
                    return FlowState::STOPPING;
                }
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            m_wake.wait_until(lock, calculateTimeToRetry(m_retryCount++), [this] { return m_isStopping; });
        }

        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_SUCCESS;
        return FlowState::STOPPING;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthorizationProvider::FlowState CBLAuthorizationProvider::handleRequestingToken() {
    try {
        AACE_DEBUG(LX(TAG));
        if (m_legacyEventNotifier) {
            m_legacyEventNotifier->cblStateChanged(
                CBLLegacyEventNotificationInterface::CBLState::REQUESTING_TOKEN,
                CBLLegacyEventNotificationInterface::CBLStateChangedReason::NONE);
        }

        auto interval = MIN_TOKEN_REQUEST_INTERVAL;
        while (!isStopping()) {
            // If the code pair expired, stop
            if (std::chrono::steady_clock::now() >= m_codePairExpirationTime) {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX, "handleRequestingToken", METRIC_CODEPAIR_EXPIRED, 1);
                m_stateChangeReason = AUTHORIZATION_ERROR_REASON_CODE_PAIR_EXPIRED;
                return FlowState::STOPPING;
            }

            auto result = receiveTokenResponse(requestToken(), true);
            std::stringstream requestTokenResult;
            requestTokenResult << result;
            if (result == AuthObserverInterface::Error::SUCCESS) {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX, "handleRequestingToken", METRIC_REQUESTTOKEN_SUCCESS, 1);
            } else {
                emitUniqueCounterMetrics(
                    METRIC_PROGRAM_NAME_SUFFIX,
                    "handleRequestingToken",
                    {METRIC_REQUESTTOKEN_FAILED, requestTokenResult.str()});
            }
            switch (result) {
                case AuthObserverInterface::Error::SUCCESS:
                    if (m_enableUserProfile) {
                        handleRequestingUserProfile();
                    }
                    m_newRefreshToken = true;
                    m_explicitAuthorizationRequest = false;  // Reset since we got successful in getting token
                    return FlowState::REFRESHING_TOKEN;
                case AuthObserverInterface::Error::UNKNOWN_ERROR:
                case AuthObserverInterface::Error::AUTHORIZATION_FAILED:
                case AuthObserverInterface::Error::SERVER_ERROR:
                case AuthObserverInterface::Error::AUTHORIZATION_PENDING:
                    break;
                case AuthObserverInterface::Error::SLOW_DOWN:
                    interval = std::min(interval * TOKEN_REQUEST_SLOW_DOWN_FACTOR, MAX_TOKEN_REQUEST_INTERVAL);
                    break;
                case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED:
                    if (m_explicitAuthorizationRequest) {
                        // Fall back to requesting code pair state if the application provides an invalid refresh token during
                        // explicit start of CBL authorization.
                        m_explicitAuthorizationRequest =
                            false;  // Don't loop continuously in code pair requesting state
                        return FlowState::REQUESTING_CODE_PAIR;
                    } else {
                        setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_AUTHORIZATION_EXPIRED;
                        return FlowState::STOPPING;
                    }
                case AuthObserverInterface::Error::INVALID_CODE_PAIR:
                    return FlowState::REQUESTING_CODE_PAIR;
                case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT:
                case AuthObserverInterface::Error::INVALID_REQUEST:
                case AuthObserverInterface::Error::INVALID_VALUE:
                case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE:
                case AuthObserverInterface::Error::INTERNAL_ERROR:
                case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID: {
                    setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                    m_stateChangeReason = AUTHORIZATION_ERROR_REASON_UNKNOWN_ERROR;
                    return FlowState::STOPPING;
                }
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            m_wake.wait_for(lock, interval, [this] { return m_isStopping; });
        }

        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_SUCCESS;
        return FlowState::STOPPING;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthorizationProvider::FlowState CBLAuthorizationProvider::handleRefreshingToken() {
    try {
        AACE_DEBUG(LX(TAG));
        if (m_legacyEventNotifier) {
            m_legacyEventNotifier->cblStateChanged(
                CBLLegacyEventNotificationInterface::CBLState::REFRESHING_TOKEN,
                CBLLegacyEventNotificationInterface::CBLStateChangedReason::NONE);
        }

        m_retryCount = 0;
        while (!isStopping()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            bool isAboutToExpire =
                (AuthObserverInterface::State::REFRESHED == m_authState && m_tokenExpirationTime < m_timeToRefresh);

            auto nextActionTime = (isAboutToExpire ? m_tokenExpirationTime : m_timeToRefresh);

            m_networkWakeup = false;
            m_wake.wait_until(
                lock, nextActionTime, [this] { return m_authFailureReported || m_isStopping || m_networkWakeup; });

            if (m_networkWakeup) {
                m_networkWakeup = false;
            }

            if (m_isStopping) {
                break;
            }

            auto nextState = m_authState;
            if (isAboutToExpire && !m_authFailureReported) {
                m_accessToken.clear();
                lock.unlock();
                nextState = AuthObserverInterface::State::EXPIRED;
            } else {
                m_authFailureReported = false;
                isAboutToExpire = false;
                bool newRefreshToken = m_newRefreshToken;
                m_newRefreshToken = false;
                lock.unlock();

                auto listener = getAuthorizationProviderListener();
                ThrowIfNull(listener, "invalidListenerReference");

                auto data = listener->onGetAuthorizationData(m_service, AUTHORIZATION_DATA_REFRESH_TOKEN_KEY);
                ThrowIf(data.empty(), "invalidAuthorizationData");

                auto refreshTokenJson = json::parse(data);
                if (refreshTokenJson.contains(AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY) &&
                    refreshTokenJson[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY].is_string()) {
                    m_refreshToken = refreshTokenJson[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY];
                } else {
                    Throw("invalidRefreshToken");
                }

                auto result = receiveTokenResponse(requestRefresh(), false);
                m_refreshToken.clear();
                std::stringstream refreshTokenResult;
                refreshTokenResult << result;
                if (result == AuthObserverInterface::Error::SUCCESS) {
                    emitUniqueCounterMetrics(
                        METRIC_PROGRAM_NAME_SUFFIX, "handleRefreshingToken", METRIC_REQUESTTOKEN_SUCCESS, 1);
                } else {
                    emitUniqueCounterMetrics(
                        METRIC_PROGRAM_NAME_SUFFIX,
                        "handleRefreshingToken",
                        {METRIC_REQUESTTOKEN_FAILED, refreshTokenResult.str()});
                }
                switch (result) {
                    case AuthObserverInterface::Error::SUCCESS:
                        m_retryCount = 0;
                        nextState = AuthObserverInterface::State::REFRESHED;
                        break;
                    case AuthObserverInterface::Error::UNKNOWN_ERROR:
                    case AuthObserverInterface::Error::SERVER_ERROR:
                    case AuthObserverInterface::Error::AUTHORIZATION_FAILED:
                    case AuthObserverInterface::Error::AUTHORIZATION_PENDING:
                    case AuthObserverInterface::Error::SLOW_DOWN:
                        m_timeToRefresh = calculateTimeToRetry(m_retryCount++);
                        break;
                    case AuthObserverInterface::Error::INVALID_CODE_PAIR:
                        clearRefreshToken();
                        return FlowState::REQUESTING_CODE_PAIR;
                    case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED:
                        clearRefreshToken();
                        if (m_explicitAuthorizationRequest) {
                            // Fall back to requesting code pair state if the application provides an invalid refresh token during
                            // explicit start of CBL authorization.
                            m_explicitAuthorizationRequest = false;  // Not to loop continuously
                            return FlowState::REQUESTING_CODE_PAIR;
                        } else {
                            setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                            m_stateChangeReason = AUTHORIZATION_ERROR_REASON_AUTHORIZATION_EXPIRED;
                            return FlowState::STOPPING;
                        }
                    case AuthObserverInterface::Error::INVALID_REQUEST:
                        if (newRefreshToken) {
                            setAuthError(AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID);
                        }
                    // Falls through
                    case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT:
                    case AuthObserverInterface::Error::INVALID_VALUE:
                    case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE:
                    case AuthObserverInterface::Error::INTERNAL_ERROR:
                    case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID: {
                        clearRefreshToken();
                        setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_UNKNOWN_ERROR;
                        return FlowState::STOPPING;
                    }
                }
            }
            setAuthState(nextState);
        }

        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_SUCCESS;
        return FlowState::STOPPING;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthorizationProvider::FlowState CBLAuthorizationProvider::handleStopping() {
    AACE_DEBUG(LX(TAG));
    if (m_legacyEventNotifier) {
        auto stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::SUCCESS;
        if (m_stateChangeReason == AUTHORIZATION_ERROR_REASON_UNKNOWN_ERROR) {
            stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::ERROR;
        } else if (m_stateChangeReason == AUTHORIZATION_ERROR_REASON_TIMEOUT) {
            stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::TIMEOUT;
        } else if (m_stateChangeReason == AUTHORIZATION_ERROR_REASON_CODE_PAIR_EXPIRED) {
            stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::CODE_PAIR_EXPIRED;
        } else if (m_stateChangeReason == AUTHORIZATION_ERROR_REASON_AUTHORIZATION_EXPIRED) {
            stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::AUTHORIZATION_EXPIRED;
        } else if (m_stateChangeReason == AUTHORIZATION_ERROR_REASON_SUCCESS) {
            stateChangeReason = CBLLegacyEventNotificationInterface::CBLStateChangedReason::SUCCESS;
        }
        m_legacyEventNotifier->cblStateChanged(
            CBLLegacyEventNotificationInterface::CBLState::STOPPING, stateChangeReason);
    }

    if (m_stateChangeReason != AUTHORIZATION_ERROR_REASON_SUCCESS) {
        auto listener = getAuthorizationProviderListener();
        if (listener != nullptr) {
            listener->onAuthorizationError(m_service, m_stateChangeReason);
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_currentAuthState != AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED) {
                m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
                listener->onAuthorizationStateChanged(
                    m_service, AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED);
            }
        } else {
            AACE_ERROR(LX(TAG).m("invalidListenerReference"));
        }

        // Reset the reason.
        m_stateChangeReason = AUTHORIZATION_ERROR_REASON_SUCCESS;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_isStopping = true;
    return FlowState::STOPPING;
}

void CBLAuthorizationProvider::handleRequestingUserProfile() {
    try {
        AACE_DEBUG(LX(TAG));

        auto response = requestUserProfile();
        ThrowIfNot(response.code == HTTPResponseCode::SUCCESS_OK, "Error making request");

        std::string name;
        std::string email;

        Document document;
        ThrowIf(document.Parse(response.body.c_str()).HasParseError(), "Could not parse response");

        auto root = document.GetObject();

        if (root.HasMember("name") && root["name"].IsString()) {
            name = root["name"].GetString();
        }
        ThrowIf(name.empty(), "Missing name in response payload");

        if (root.HasMember("email") && root["email"].IsString()) {
            email = root["email"].GetString();
        }
        ThrowIf(email.empty(), "Missing email in response payload");

        AACE_DEBUG(LX(TAG).d("name", name).d("email", email));

        // clang-format off
        json requestJson = {
            {"type", AUTHORIZATION_REQUEST_TYPE_USER_PROFILE},
            {"payload",{
                {"name", name},
                {"email", email}
            }}
        };
        // clang-format on
        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");
        listener->onEventReceived(m_service, requestJson.dump());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

HTTPResponse CBLAuthorizationProvider::requestCodePair() {
    AACE_DEBUG(LX(TAG));
    const std::vector<std::pair<std::string, std::string>> postData = {
        {POST_KEY_RESPONSE_TYPE, POST_VALUE_DEVICE_CODE},
        {POST_KEY_CLIENT_ID, m_configuration->getClientId()},
        {POST_KEY_SCOPE, m_scope},
        {POST_KEY_SCOPE_DATA, m_configuration->getScopeData()},
    };
    std::unique_lock<std::mutex> lock(m_localeMutex);
    std::string acceptLanguage = m_locale;
    lock.unlock();

    if (acceptLanguage.empty()) {
        AACE_WARN(LX(TAG).m("No locale setting available. Using default").d("default", DEFAULT_ACCEPT_LANGUAGE_VALUE));
        acceptLanguage = DEFAULT_ACCEPT_LANGUAGE_VALUE;
    } else {
        // When locale is set to dual use the primary one. Dual locale have the `/` as delimiter.
        size_t position = acceptLanguage.find("/");
        if (position != std::string::npos) {
            acceptLanguage = acceptLanguage.substr(0, position);
        }
    }

    const std::vector<std::string> headerLines = {HEADER_LINE_URLENCODED, HEADER_LINE_LANGUAGE_PREFIX + acceptLanguage};

    return doPost(
        m_configuration->getRequestCodePairUrl(), headerLines, postData, m_configuration->getRequestTimeout());
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthorizationProvider::requestToken() {
    AACE_DEBUG(LX(TAG));
    const std::vector<std::pair<std::string, std::string>> postData = {{POST_KEY_GRANT_TYPE, POST_VALUE_DEVICE_CODE},
                                                                       {POST_KEY_DEVICE_CODE, m_deviceCode},
                                                                       {POST_KEY_USER_CODE, m_userCode}};
    const std::vector<std::string> headerLines = {HEADER_LINE_URLENCODED};

    m_requestTime = std::chrono::steady_clock::now();

    return doPost(m_configuration->getRequestTokenUrl(), headerLines, postData, m_configuration->getRequestTimeout());
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthorizationProvider::requestRefresh() {
    AACE_DEBUG(LX(TAG));
    const std::vector<std::pair<std::string, std::string>> postData = {
        {POST_KEY_GRANT_TYPE, POST_VALUE_REFRESH_TOKEN},
        {POST_KEY_REFRESH_TOKEN, m_refreshToken},
        {POST_KEY_CLIENT_ID, m_configuration->getClientId()}};
    const std::vector<std::string> headerLines = {HEADER_LINE_URLENCODED};

    // Don't wait for this request so long that we would be late to notify our observer if the token expires.
    m_requestTime = std::chrono::steady_clock::now();
    auto timeout = m_configuration->getRequestTimeout();
    if (AuthObserverInterface::State::REFRESHED == m_authState) {
        auto timeUntilExpired = std::chrono::duration_cast<std::chrono::seconds>(m_tokenExpirationTime - m_requestTime);
        if ((timeout > timeUntilExpired) && (timeUntilExpired > std::chrono::seconds::zero())) {
            timeout = timeUntilExpired;
        }
    }

    return doPost(m_configuration->getRequestTokenUrl(), headerLines, postData, timeout);
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthorizationProvider::requestUserProfile() {
    AACE_DEBUG(LX(TAG));
    const std::vector<std::string> getHeaderData = {HOST_HEADER_DATA_USER_PROFILE,
                                                    BEARER_HEADER_DATA_USER_PROFILE + m_accessToken};

    return doGet(USER_PROFILE_DEFAULT_ENDPOINT, getHeaderData);
}

AuthObserverInterface::Error CBLAuthorizationProvider::receiveCodePairResponse(const HTTPResponse& response) {
    try {
        AACE_DEBUG(LX(TAG).d("code", response.code).sensitive("body", response.body));

        Document document;
        auto result = parseLWAResponse(response, &document);
        setAuthError(result);

        if (result != AuthObserverInterface::Error::SUCCESS) {
            AACE_DEBUG(LX(TAG).d("result", result));
            return result;
        }

        auto it = document.FindMember(JSON_KEY_USER_CODE);
        if (it != document.MemberEnd() && it->value.IsString()) {
            m_userCode = it->value.GetString();
        }

        it = document.FindMember(JSON_KEY_DEVICE_CODE);
        if (it != document.MemberEnd() && it->value.IsString()) {
            m_deviceCode = it->value.GetString();
        }

        std::string verificationUri;
        it = document.FindMember(JSON_KEY_VERIFICATION_URI);
        if (it != document.MemberEnd() && it->value.IsString()) {
            verificationUri = it->value.GetString();
        }

        int64_t expiresInSeconds = 0;
        it = document.FindMember(JSON_KEY_EXPIRES_IN);
        if (it != document.MemberEnd() && it->value.IsUint64()) {
            expiresInSeconds = it->value.GetUint64();
        }

        int64_t intervalSeconds = 0;
        it = document.FindMember(JSON_KEY_INTERVAL);
        if (it != document.MemberEnd() && it->value.IsUint64()) {
            intervalSeconds = it->value.GetUint64();
        }

        if (m_userCode.empty() || m_deviceCode.empty() || verificationUri.empty() || 0 == expiresInSeconds) {
            AACE_ERROR(LX(TAG)
                           .d("reason", "missing or InvalidResponseProperty")
                           .d("user_code", m_userCode)
                           .sensitive("device_code", m_deviceCode)
                           .d("verification_uri", verificationUri)
                           .d("expiresIn", expiresInSeconds)
                           .d("interval", intervalSeconds));
            return AuthObserverInterface::Error::UNKNOWN_ERROR;
        }

        m_codePairExpirationTime = std::chrono::steady_clock::now() + std::chrono::seconds(expiresInSeconds);

        if (m_legacyEventNotifier) {
            m_legacyEventNotifier->cblStateChanged(
                CBLLegacyEventNotificationInterface::CBLState::CODE_PAIR_RECEIVED,
                CBLLegacyEventNotificationInterface::CBLStateChangedReason::SUCCESS,
                verificationUri,
                m_userCode);
        }

        // clang-format off
        json requestJson = {
            {"type", AUTHORIZATION_REQUEST_TYPE_CBL_CODE},
            {"payload", {
                {"code", m_userCode},
                {"url", verificationUri}
            }}
        };
        // clang-format on

        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");
        listener->onEventReceived(m_service, requestJson.dump());

        return result;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return AuthObserverInterface::Error::UNKNOWN_ERROR;
    }
}

AuthObserverInterface::Error CBLAuthorizationProvider::receiveTokenResponse(
    const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
    bool expiresImmediately) {
    try {
        AACE_DEBUG(LX(TAG).d("code", response.code).sensitive("body", response.body));

        Document document;
        auto result = parseLWAResponse(response, &document);
        setAuthError(result);

        if (result != AuthObserverInterface::Error::SUCCESS) {
            AACE_DEBUG(LX(TAG).d("result", result));
            return result;
        }

        std::string accessToken;
        auto it = document.FindMember(JSON_KEY_ACCESS_TOKEN);
        if (it != document.MemberEnd() && it->value.IsString()) {
            accessToken = it->value.GetString();
        }

        std::string refreshToken;
        it = document.FindMember(JSON_KEY_REFRESH_TOKEN);
        if (it != document.MemberEnd() && it->value.IsString()) {
            refreshToken = it->value.GetString();
        }

        std::string tokenType;
        it = document.FindMember(JSON_KEY_TOKEN_TYPE);
        if (it != document.MemberEnd() && it->value.IsString()) {
            tokenType = it->value.GetString();
        }

        int64_t expiresInSeconds = 0;
        it = document.FindMember(JSON_KEY_EXPIRES_IN);
        if (it != document.MemberEnd() && it->value.IsUint64()) {
            expiresInSeconds = it->value.GetUint64();
        }

        if (accessToken.empty() || refreshToken.empty() || tokenType != JSON_VALUE_BEARER || 0 == expiresInSeconds) {
            AACE_ERROR(LX(TAG)
                           .d("reason", "missing or InvalidResponseProperty")
                           .sensitive("access_token", accessToken)
                           .d("refresh_token", refreshToken)
                           .d("token_type", tokenType)
                           .d("expiresIn", expiresInSeconds));
            return AuthObserverInterface::Error::UNKNOWN_ERROR;
        }

        if (expiresImmediately) {
            expiresInSeconds = 0;
        }

        setRefreshToken(refreshToken);
        m_tokenExpirationTime = m_requestTime + std::chrono::seconds(expiresInSeconds);
        m_timeToRefresh = m_tokenExpirationTime - m_configuration->getAccessTokenRefreshHeadStart();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_accessToken = accessToken;
        }

        return AuthObserverInterface::Error::SUCCESS;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return AuthObserverInterface::Error::UNKNOWN_ERROR;
    }
}

void CBLAuthorizationProvider::stopAuthFlowThread(bool resetData, bool notifyAuthStateChange) {
    try {
        AACE_DEBUG(LX(TAG).d("resetData", resetData).d("notifyAuthStateChange", notifyAuthStateChange));

        handleStopping();

        m_wake.notify_one();
        if (m_authorizationFlowThread.joinable()) {
            m_authorizationFlowThread.join();
        }

        m_threadActive = false;
        if (resetData) {
            m_accessToken.clear();
            clearRefreshToken();
            setAuthState(AuthObserverInterface::State::UNINITIALIZED, notifyAuthStateChange);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void CBLAuthorizationProvider::setAuthState(AuthObserverInterface::State newAuthState, bool notifyAuthStateChange) {
    try {
        AACE_DEBUG(LX(TAG).d("newAuthState", newAuthState));

        std::lock_guard<std::mutex> lock(m_mutex);
        if (newAuthState == m_authState) {
            return;
        }
        m_authState = newAuthState;
        AACE_DEBUG(LX(TAG).d("state", m_authState).d("error", m_authError));

        if (m_authState == AuthObserverInterface::State::REFRESHED &&
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZING) {
            m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED;
            auto listener = getAuthorizationProviderListener();
            ThrowIfNull(listener, "invalidListenerReference");
            listener->onAuthorizationStateChanged(
                m_service, AuthorizationProviderListenerInterface::AuthorizationState::AUTHORIZED);
        }

        if (notifyAuthStateChange) {
            auto authorizationManager_lock = m_authorizationManager.lock();
            ThrowIfNull(authorizationManager_lock, "invalidAuthorizationManagerReference");
            authorizationManager_lock->authStateChanged(m_service, m_authState, m_authError);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void CBLAuthorizationProvider::setAuthError(AuthObserverInterface::Error authError) {
    try {
        AACE_DEBUG(LX(TAG).d("authError", authError));

        std::lock_guard<std::mutex> lock(m_mutex);
        m_authError = authError;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void CBLAuthorizationProvider::setRefreshToken(const std::string& refreshToken) {
    AACE_DEBUG(LX(TAG));
    if (refreshToken != m_refreshToken) {
        auto listener = getAuthorizationProviderListener();
        ThrowIfNull(listener, "invalidListenerReference");
        json refreshTokenJson;
        refreshTokenJson[AUTHORIZATION_JSON_DATA_REFRESH_TOKEN_KEY] = refreshToken;
        listener->onSetAuthorizationData(m_service, AUTHORIZATION_DATA_REFRESH_TOKEN_KEY, refreshTokenJson.dump());
    }
}

void CBLAuthorizationProvider::clearRefreshToken() {
    AACE_DEBUG(LX(TAG));
    m_refreshToken.clear();
    auto listener = getAuthorizationProviderListener();
    ThrowIfNull(listener, "invalidListenerReference");
    listener->onSetAuthorizationData(m_service, AUTHORIZATION_DATA_REFRESH_TOKEN_KEY, "");
}

bool CBLAuthorizationProvider::isStopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isStopping;
}

bool CBLAuthorizationProvider::sendEvent(const std::string& payload) {
    AACE_DEBUG(LX(TAG));
    // no-op
    return true;
}

void CBLAuthorizationProvider::deregister() {
    AACE_DEBUG(LX(TAG));

    // notifyAuthStateChange = false because we don't need to notify auth state
    // change back to AuthorizationManager.
    stopAuthFlowThread(true, false);

    auto listener = getAuthorizationProviderListener();
    ThrowIfNull(listener, "invalidListenerReference");
    listener->onAuthorizationStateChanged(
        m_service, AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentAuthState = AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED;
}

std::string CBLAuthorizationProvider::getAuthToken() {
    // To prevent flooding of debug logs, no debug log added here.
    try {
        ThrowIf(
            m_currentAuthState == AuthorizationProviderListenerInterface::AuthorizationState::UNAUTHORIZED,
            "inUnauthorizedState");
        return m_accessToken;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void CBLAuthorizationProvider::onAuthFailure(const std::string& token) {
    AACE_DEBUG(LX(TAG));
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (token.empty() || token == m_accessToken) {
            m_authFailureReported = true;
            m_wake.notify_one();
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return;
    }
}

void CBLAuthorizationProvider::onNetworkInfoChanged(NetworkInfoObserver::NetworkStatus status, int wifiSignalStrength) {
    AACE_DEBUG(LX(TAG, "onNetworkInfoChanged").d("m_networkStatus", status));
    if (status == NetworkInfoObserver::NetworkStatus::CONNECTED) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_networkWakeup = true;
        m_wake.notify_all();
    }
}

void CBLAuthorizationProvider::onNetworkInterfaceChangeStatusChanged(
    const std::string& networkInterface,
    NetworkInterfaceChangeStatus status) {
    // No action required as we don't create and persist network connection using AVS LibCurlUtils.
}

void CBLAuthorizationProvider::propertyChanged(const std::string& name, const std::string& newValue) {
    AACE_DEBUG(LX(TAG).d("name", name).d("newValue", newValue));
    if (name == aace::alexa::property::LOCALE) {
        std::lock_guard<std::mutex> lock(m_localeMutex);
        m_locale = newValue;
    }
}

}  // namespace cbl
}  // namespace engine
}  // namespace aace

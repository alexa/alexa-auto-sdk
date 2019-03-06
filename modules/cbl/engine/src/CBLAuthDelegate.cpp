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

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpPost.h>
#include <AVSCommon/Utils/LibcurlUtils/HttpResponseCodes.h>
#include <AVSCommon/Utils/Logger/Logger.h>

#include "AACE/Engine/CBL/CBLAuthDelegate.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace cbl {

using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::libcurlUtils;
using namespace alexaClientSDK::registrationManager;
using namespace rapidjson;

/// String to identify log entries originating from this file.
static const std::string TAG("CBLAuthDelegate");

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

/// Min time to wait between attempt to poll for a token while authentication is pending.
static const std::chrono::seconds MIN_TOKEN_REQUEST_INTERVAL = std::chrono::seconds(5);

/// Max time to wait between attempt to poll for a token while authentication is pending.
static const std::chrono::seconds MAX_TOKEN_REQUEST_INTERVAL = std::chrono::seconds(60);

/// Scale factor to apply to interval between token poll requests when a 'slow_down' response is received.
static const int TOKEN_REQUEST_SLOW_DOWN_FACTOR = 2;

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
            Throw( "unknowError") ;
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getErrorCode").d("reason", ex.what()));
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
    AACE_DEBUG(LX(TAG,"mapHTTPStatusToError").d("code", code).d("error", error));
    return error;
}

/**
 * Perform common parsing of an @c LWA response.
 *
 * @param response The response to parse.
 * @param document The document to populate from the body of the response.
 * @return The status from the initial parsing of the response.
 */
AuthObserverInterface::Error parseLWAResponse(
    const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
    rapidjson::Document* document) {

    try {
        ThrowIfNull( document, "nullDocument" );

        auto result = mapHTTPCodeToError(response.code);

        if (document->Parse(response.body.c_str()).HasParseError()) {
            AACE_ERROR(LX(TAG,"parseLWAResponseFailed")
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
                    AACE_DEBUG(LX(TAG,"errorInLwaResponseBody").d("error", error).d("errorCode", result));
                }
            }
        }

        return result;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"parseLWAResponse").d("reason", ex.what()));
        return AuthObserverInterface::Error::INTERNAL_ERROR;
    }
}

std::shared_ptr<CBLAuthDelegate> CBLAuthDelegate::create(
    std::shared_ptr<CustomerDataManager> customerDataManager,
    std::shared_ptr<CBLAuthDelegateConfiguration> configuration,
    std::shared_ptr<CBLAuthRequesterInterface> cblAuthRequester) {

    try {
        AACE_DEBUG(LX(TAG,"create"));

        ThrowIfNull( customerDataManager, "nullDataManager" );
        ThrowIfNull( configuration, "nullCBLAuthDelegateConfiguration");
        ThrowIfNull( cblAuthRequester, "nullCBLAuthRequester");

        std::shared_ptr<HttpPost> httpPost = HttpPost::create();
        ThrowIfNull( httpPost, "nullHttpPost" );
       
        std::shared_ptr<CBLAuthDelegate> cblAuthDelegate = std::shared_ptr<CBLAuthDelegate>( new CBLAuthDelegate( customerDataManager, configuration, cblAuthRequester, httpPost ) );
        return cblAuthDelegate;
    } 
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

CBLAuthDelegate::~CBLAuthDelegate() {
    stop();
    m_cblAuthRequester.reset();
}

void CBLAuthDelegate::addAuthObserver(std::shared_ptr<AuthObserverInterface> observer) {
    try {
        AACE_DEBUG(LX(TAG,"addAuthObserver").d("observer", observer.get()));

        std::lock_guard<std::mutex> lock(m_mutex);
        ThrowIfNull( observer, "nullObserver" );

        if( m_observers.insert(observer).second == false ) {
            Throw( "observerAlreadyAdded")
        }
        observer->onAuthStateChange(m_authState, m_authError);
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"addAuthObserver").d("reason", ex.what()));
    }
}

void CBLAuthDelegate::removeAuthObserver(std::shared_ptr<AuthObserverInterface> observer) {
    try {
        AACE_DEBUG(LX(TAG,"removeAuthObserver").d("observer", observer.get()));

        std::lock_guard<std::mutex> lock(m_mutex);
        ThrowIfNull( observer, "nullObserver");

        if( m_observers.erase(observer) == 0 ) {
            Throw( "observerNotAdded");
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"removeAuthObserver").d("reason", ex.what()));
    }
}

std::string CBLAuthDelegate::getAuthToken() {
    return m_accessToken;
}

void CBLAuthDelegate::onAuthFailure(const std::string& token) {
}

void CBLAuthDelegate::clearData() {
    AACE_DEBUG(LX(TAG,"clearData"));
    stop();
    m_cblAuthRequester->clearRefreshToken();
}

CBLAuthDelegate::CBLAuthDelegate(
    std::shared_ptr<CustomerDataManager> customerDataManager,
    std::shared_ptr<CBLAuthDelegateConfiguration> configuration,
    std::shared_ptr<CBLAuthRequesterInterface> cblAuthRequester,
    std::shared_ptr<HttpPost> httpPost) :
        CustomerDataHandler{customerDataManager},
        m_httpPost{httpPost},
        m_cblAuthRequester{cblAuthRequester},
        m_configuration{configuration},
        m_isStopping{false},
        m_authState{AuthObserverInterface::State::UNINITIALIZED},
        m_authError{AuthObserverInterface::Error::SUCCESS},
        m_tokenExpirationTime{std::chrono::time_point<std::chrono::steady_clock>::max()},
        m_retryCount{0},
        m_newRefreshToken{false},
        m_threadActive{false} {
    AACE_DEBUG(LX(TAG,"CBLAuthDelegate"));
}

void CBLAuthDelegate::stop() {
    try {
        AACE_DEBUG(LX(TAG,"stop"));
        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::STOPPING, CBLAuthRequesterInterface::CBLStateChangedReason::NONE);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_isStopping = true;
        }
        m_wake.notify_one();
        if (m_authorizationFlowThread.joinable()) {
            m_authorizationFlowThread.join();
        }
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_accessToken.clear();
            m_threadActive = false;
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()));
    }
}

void CBLAuthDelegate::handleAuthorizationFlow() {
    AACE_DEBUG(LX(TAG,"handleAuthorizationFlow"));

    auto flowState = FlowState::STARTING;
    while (!isStopping()) {
        auto nextFlowState = FlowState::STOPPING;
        switch (flowState) {
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
        flowState = nextFlowState;
    }
    m_threadActive = false;
}

CBLAuthDelegate::FlowState CBLAuthDelegate::handleStarting() {
    try {
        AACE_DEBUG(LX(TAG,"handleStarting"));
        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::STARTING, CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS);

        if( m_cblAuthRequester->getRefreshToken() != std::string() ) {
            return FlowState::REFRESHING_TOKEN;
        }

        AACE_DEBUG(LX(TAG,"getRefreshTokenFailed"));
        return FlowState::REQUESTING_CODE_PAIR;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleStarting").d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthDelegate::FlowState CBLAuthDelegate::handleRequestingCodePair() {
    try {
        AACE_DEBUG(LX(TAG,"handleRequestingCodePair"));
        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::REQUESTING_CODE_PAIR, CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS);

        m_retryCount = 0;
        std::chrono::steady_clock::time_point codePairRequestTimeout = std::chrono::steady_clock::now() + m_configuration->getCodePairRequestTimeout();
        while (!isStopping()) {

            if (std::chrono::steady_clock::now() >= codePairRequestTimeout) {
                m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::TIMEOUT;
                return FlowState::STOPPING;
            }

            auto result = receiveCodePairResponse(requestCodePair());
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
                    m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::ERROR;
                    return FlowState::STOPPING;
                }
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            m_wake.wait_until(lock, calculateTimeToRetry(m_retryCount++), [this] { return m_isStopping; });
        }

        m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS;
        return FlowState::STOPPING;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleRequestingCodePair").d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthDelegate::FlowState CBLAuthDelegate::handleRequestingToken() {
    try {
        AACE_DEBUG(LX(TAG,"handleRequestingToken"));
        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::REQUESTING_TOKEN, CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS);

        auto interval = MIN_TOKEN_REQUEST_INTERVAL;

        while (!isStopping()) {
            // If the code pair expired, stop
            if (std::chrono::steady_clock::now() >= m_codePairExpirationTime) {
                m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::CODE_PAIR_EXPIRED;
                return FlowState::STOPPING;
            }

            auto result = receiveTokenResponse(requestToken(), true);
            switch (result) {
                case AuthObserverInterface::Error::SUCCESS:
                    m_newRefreshToken = true;
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
                case AuthObserverInterface::Error::INVALID_CODE_PAIR:
                    return FlowState::REQUESTING_CODE_PAIR;
                case AuthObserverInterface::Error::UNAUTHORIZED_CLIENT:
                case AuthObserverInterface::Error::INVALID_REQUEST:
                case AuthObserverInterface::Error::INVALID_VALUE:
                case AuthObserverInterface::Error::UNSUPPORTED_GRANT_TYPE:
                case AuthObserverInterface::Error::INTERNAL_ERROR:
                case AuthObserverInterface::Error::INVALID_CBL_CLIENT_ID: {
                    setAuthState(AuthObserverInterface::State::UNRECOVERABLE_ERROR);
                    m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::ERROR;
                    return FlowState::STOPPING;
                }
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            m_wake.wait_for(lock, interval, [this] { return m_isStopping; });
        }

        m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS;
        return FlowState::STOPPING;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleRequestingToken").d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthDelegate::FlowState CBLAuthDelegate::handleRefreshingToken() {
    try {
        AACE_DEBUG(LX(TAG,"handleRefreshingToken"));
        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::REFRESHING_TOKEN, CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS);

        m_retryCount = 0;

        while (!isStopping()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            bool isAboutToExpire =
                (AuthObserverInterface::State::REFRESHED == m_authState && m_tokenExpirationTime < m_timeToRefresh);

            auto nextActionTime = (isAboutToExpire ? m_tokenExpirationTime : m_timeToRefresh);

            if ( !m_accessToken.empty() && m_wake.wait_until(lock, nextActionTime, [this] { return m_isStopping; })) { 
                break;
            }

            auto nextState = m_authState;
            if (isAboutToExpire) {
                m_accessToken.clear();
                lock.unlock();
                nextState = AuthObserverInterface::State::EXPIRED;
            } else {
                bool newRefreshToken = m_newRefreshToken;
                m_newRefreshToken = false;
                lock.unlock();
                m_refreshToken = m_cblAuthRequester->getRefreshToken();
                auto result = receiveTokenResponse(requestRefresh(), false);
                m_refreshToken.clear();
                switch (result) {
                    case AuthObserverInterface::Error::SUCCESS:
                        m_retryCount = 0;
                        nextState = AuthObserverInterface::State::REFRESHED;
                        break;
                    case AuthObserverInterface::Error::UNKNOWN_ERROR:
                    case AuthObserverInterface::Error::AUTHORIZATION_FAILED:
                    case AuthObserverInterface::Error::SERVER_ERROR:
                    case AuthObserverInterface::Error::AUTHORIZATION_PENDING:
                    case AuthObserverInterface::Error::SLOW_DOWN:
                        m_timeToRefresh = calculateTimeToRetry(m_retryCount++);
                        break;
                    case AuthObserverInterface::Error::AUTHORIZATION_EXPIRED:
                    case AuthObserverInterface::Error::INVALID_CODE_PAIR:
                        clearRefreshToken();
                        return FlowState::REQUESTING_CODE_PAIR;
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
                        m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::ERROR;
                        return FlowState::STOPPING;
                    }
                }
            }
            setAuthState(nextState);
        }

        m_stateChangeReason = CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS;
        return FlowState::STOPPING;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleRefreshingToken").d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

CBLAuthDelegate::FlowState CBLAuthDelegate::handleStopping() {
    try {
        AACE_DEBUG(LX(TAG,"handleStopping"));

        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::STOPPING, m_stateChangeReason);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isStopping = true;
        return FlowState::STOPPING;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"handleStopping").d("reason", ex.what()));
        return FlowState::STOPPING;
    }
}

HTTPResponse CBLAuthDelegate::requestCodePair() {
    AACE_DEBUG(LX(TAG,"requestCodePair"));

    const std::vector<std::pair<std::string, std::string>> postData = {
        {POST_KEY_RESPONSE_TYPE, POST_VALUE_DEVICE_CODE},
        {POST_KEY_CLIENT_ID, m_configuration->getClientId()},
        {POST_KEY_SCOPE, POST_VALUE_ALEXA_ALL},
        {POST_KEY_SCOPE_DATA, m_configuration->getScopeData()},
    };
    const std::vector<std::string> headerLines = {HEADER_LINE_URLENCODED};

    return m_httpPost->doPost(
        m_configuration->getRequestCodePairUrl(), headerLines, postData, m_configuration->getRequestTimeout());
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthDelegate::requestToken() {
    AACE_DEBUG(LX(TAG,"requestToken"));

    const std::vector<std::pair<std::string, std::string>> postData = {{POST_KEY_GRANT_TYPE, POST_VALUE_DEVICE_CODE},
                                                                       {POST_KEY_DEVICE_CODE, m_deviceCode},
                                                                       {POST_KEY_USER_CODE, m_userCode}};
    const std::vector<std::string> headerLines = {HEADER_LINE_URLENCODED};

    m_requestTime = std::chrono::steady_clock::now();

    return m_httpPost->doPost(
        m_configuration->getRequestTokenUrl(), headerLines, postData, m_configuration->getRequestTimeout());
}

alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse CBLAuthDelegate::requestRefresh() {
    AACE_DEBUG(LX(TAG,"requestRefresh"));

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

    return m_httpPost->doPost(m_configuration->getRequestTokenUrl(), headerLines, postData, timeout);
}

AuthObserverInterface::Error CBLAuthDelegate::receiveCodePairResponse(const HTTPResponse& response) {
    try {
        AACE_DEBUG(LX(TAG,"receiveCodePairResponse").d("code", response.code).sensitive("body", response.body));

        Document document;
        auto result = parseLWAResponse(response, &document);
        setAuthError(result);

        if (result != AuthObserverInterface::Error::SUCCESS) {
            AACE_DEBUG(LX(TAG,"receiveCodePairResponseFailed").d("result", result));
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
            AACE_ERROR(LX(TAG,"receiveCodePairResponseFailed")
                            .d("reason", "missing or InvalidResponseProperty")
                            .d("user_code", m_userCode)
                            .sensitive("device_code", m_deviceCode)
                            .d("verification_uri", verificationUri)
                            .d("expiresIn", expiresInSeconds)
                            .d("interval", intervalSeconds));
            return AuthObserverInterface::Error::UNKNOWN_ERROR;
        }

        m_codePairExpirationTime = std::chrono::steady_clock::now() + std::chrono::seconds(expiresInSeconds);

        m_cblAuthRequester->cblStateChanged(CBLAuthRequesterInterface::CBLState::CODE_PAIR_RECEIVED, CBLAuthRequesterInterface::CBLStateChangedReason::SUCCESS, verificationUri, m_userCode);

        return result;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"receiveCodePairResponse").d("reason", ex.what()));
        return AuthObserverInterface::Error::UNKNOWN_ERROR;
    }
}

AuthObserverInterface::Error CBLAuthDelegate::receiveTokenResponse(
    const alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse& response,
    bool expiresImmediately) {

    try {
        AACE_DEBUG(LX(TAG,"receiveTokenResponse").d("code", response.code).sensitive("body", response.body));

        Document document;
        auto result = parseLWAResponse(response, &document);
        setAuthError(result);

        if (result != AuthObserverInterface::Error::SUCCESS) {
            AACE_DEBUG(LX(TAG,"receiveTokenResponseFailed").d("result", result));
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
            AACE_ERROR(LX(TAG,"receiveTokenResponseFailed")
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
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"receiveTokenResponse").d("reason", ex.what()));
        return AuthObserverInterface::Error::UNKNOWN_ERROR;
    }
}

void CBLAuthDelegate::setAuthState(AuthObserverInterface::State newAuthState) {
    try {
        AACE_DEBUG(LX(TAG,"setAuthState").d("newAuthState", newAuthState));

        std::lock_guard<std::mutex> lock(m_mutex);
        if (newAuthState == m_authState) {
            return;
        }
        m_authState = newAuthState;

        if (!m_observers.empty()) {
            AACE_DEBUG(LX(TAG,"callingOnAuthStateChange").d("state", m_authState).d("error", m_authError));
            for (auto observer : m_observers) {
                observer->onAuthStateChange(m_authState, m_authError);
            }
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setAuthState").d("reason", ex.what()));
    }
}

void CBLAuthDelegate::setAuthError(AuthObserverInterface::Error authError) {
    try {
        AACE_DEBUG(LX(TAG,"setAuthError").d("authError", authError));

        std::lock_guard<std::mutex> lock(m_mutex);
        m_authError = authError;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setAuthError").d("reason", ex.what()));
    }
}

void CBLAuthDelegate::setRefreshToken(const std::string& refreshToken) {
    m_cblAuthRequester->setRefreshToken( refreshToken );
}

void CBLAuthDelegate::clearRefreshToken() {
    AACE_DEBUG(LX(TAG,"clearRefreshToken"));

    m_refreshToken.clear();
    m_cblAuthRequester->clearRefreshToken();
}

bool CBLAuthDelegate::isStopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isStopping;
}

void CBLAuthDelegate::start( bool onStart ) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        if ( onStart && m_cblAuthRequester->getRefreshToken() == std::string() ) {
            return;
        }
        m_isStopping = false;
        if ( m_threadActive == false ) {
            if ( m_authorizationFlowThread.joinable() ) {
                m_authorizationFlowThread.join();
            }
            m_threadActive = true;
            m_authorizationFlowThread = std::thread(&CBLAuthDelegate::handleAuthorizationFlow, this);
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"start").d("reason", ex.what()));
    }
}

void CBLAuthDelegate::cancel() {
    stop();
}

} // aace::engine::cbl
} // aace::engine
} // aace

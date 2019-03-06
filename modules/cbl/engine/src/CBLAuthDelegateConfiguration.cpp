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

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/Logger/Logger.h>

#include "AACE/Engine/CBL/CBLAuthDelegateConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace cbl {

using namespace rapidjson;

/// String to identify log entries originating from this file.
static const std::string TAG("CBLAuthDelegateConfiguration");

/// Key for alexa:all values in JSON sent to @c LWA
static const char JSON_KEY_ALEXA_ALL[] = "alexa:all";

/// Key for productID values in JSON sent to @c LWA
static const char JSON_KEY_PRODUCT_ID[] = "productID";

/// Key for productInstanceAttributes values in JSON sent to @c LWA
static const char JSON_KEY_PRODUCT_INSTANCE_ATTRIBUTES[] = "productInstanceAttributes";

/// Key for deviceSerialNumber values in JSON sent to @c LWA
static const char JSON_KEY_DEVICE_SERIAL_NUMBER[] = "deviceSerialNumber";

/// Default value for configured requestTimeout.
static const std::chrono::minutes DEFAULT_REQUEST_TIMEOUT = std::chrono::minutes(1);

/// Default value for configured accessTokenRefreshHeadStart.
static const std::chrono::minutes DEFAULT_ACCESS_TOKEN_REFRESH_HEAD_START = std::chrono::minutes(10);

/// Path suffix for URL used in code pair requests to @C LWA.
static const std::string REQUEST_CODE_PAIR_PATH = "create/codepair";

/// Path suffix for URL used in code pair token requests to @c LWA.
static const std::string REQUEST_TOKEN_PATH = "token";

/// Path suffix for URl used in token refresh requests to @c LWA.
static const std::string REFRESH_TOKEN_PATH = "token";

std::shared_ptr<CBLAuthDelegateConfiguration> CBLAuthDelegateConfiguration::create(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
    std::chrono::seconds codePairRequestTimeout,
    const std::string& lwaUrl ) {
    
    try {
        AACE_DEBUG(LX(TAG,"create"));
        std::shared_ptr<CBLAuthDelegateConfiguration> configuration = std::shared_ptr<CBLAuthDelegateConfiguration>( new CBLAuthDelegateConfiguration() );
        ThrowIfNull( configuration, "CBLAuthDelegateConfigurationInvalid" )

        configuration->initialize( deviceInfo, codePairRequestTimeout, lwaUrl );
        return configuration;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

bool CBLAuthDelegateConfiguration::initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        const std::string& lwaUrl ) {

    try {
        AACE_DEBUG(LX(TAG,"init"));

        m_deviceInfo = deviceInfo;
        m_requestTimeout = DEFAULT_REQUEST_TIMEOUT;
        m_codePairRequestTimeout = codePairRequestTimeout;
        m_accessTokenRefreshHeadStart = DEFAULT_ACCESS_TOKEN_REFRESH_HEAD_START;

        m_requestCodePairUrl = lwaUrl + REQUEST_CODE_PAIR_PATH;
        m_requestTokenUrl = lwaUrl + REQUEST_TOKEN_PATH;
        m_refreshTokenUrl = lwaUrl + REFRESH_TOKEN_PATH;

        
        if (initScopeData() == false ) {
            Throw( "initScopeDataFailed" )
        }
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
    
}

std::string CBLAuthDelegateConfiguration::getClientId() const {
    return m_deviceInfo->getClientId();
}

std::string CBLAuthDelegateConfiguration::getProductId() const {
    return m_deviceInfo->getProductId();
}

std::string CBLAuthDelegateConfiguration::getDeviceSerialNumber() const {
    return m_deviceInfo->getDeviceSerialNumber();
}

std::chrono::seconds CBLAuthDelegateConfiguration::getRequestTimeout() const {
    return m_requestTimeout;
}

std::chrono::seconds CBLAuthDelegateConfiguration::getCodePairRequestTimeout() const {
    return m_codePairRequestTimeout;
}

std::chrono::seconds CBLAuthDelegateConfiguration::getAccessTokenRefreshHeadStart() const {
    return m_accessTokenRefreshHeadStart;
}

std::string CBLAuthDelegateConfiguration::getRequestCodePairUrl() const {
    return m_requestCodePairUrl;
}

std::string CBLAuthDelegateConfiguration::getRequestTokenUrl() const {
    return m_requestTokenUrl;
}

std::string CBLAuthDelegateConfiguration::getRefreshTokenUrl() const {
    return m_refreshTokenUrl;
}

std::string CBLAuthDelegateConfiguration::getScopeData() const {
    return m_scopeData;
}

bool CBLAuthDelegateConfiguration::initScopeData() {
    try {
        AACE_DEBUG(LX(TAG,"initScopeData"));

        Document scopeData;
        scopeData.SetObject();
        auto& allocator = scopeData.GetAllocator();
        Value productInstanceAttributes(kObjectType);
        productInstanceAttributes.AddMember(JSON_KEY_DEVICE_SERIAL_NUMBER, Value( getDeviceSerialNumber().c_str(), allocator ), allocator);
        Value alexaColonAll(kObjectType);
        alexaColonAll.AddMember(JSON_KEY_PRODUCT_ID, Value( getProductId().c_str(), allocator ), allocator);
        alexaColonAll.AddMember(JSON_KEY_PRODUCT_INSTANCE_ATTRIBUTES, productInstanceAttributes, allocator);
        scopeData.AddMember(JSON_KEY_ALEXA_ALL, alexaColonAll, allocator);

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);

        ThrowIfNot( scopeData.Accept(writer), "acceptFailed" )

        m_scopeData = buffer.GetString();
        AACE_DEBUG(LX(TAG,"initScopeDataSucceeded").sensitive("scopeData", m_scopeData));

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initScopeData").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::cbl
} // aace::engine
} // aace

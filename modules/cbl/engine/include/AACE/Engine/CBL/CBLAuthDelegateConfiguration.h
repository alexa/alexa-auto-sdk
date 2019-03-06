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

#ifndef AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_CONFIGURATION_H
#define AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_CONFIGURATION_H

#include <chrono>
#include <memory>
#include <string>

namespace aace {
namespace engine {
namespace cbl {

class CBLAuthDelegateConfiguration {
public:

    static std::shared_ptr<CBLAuthDelegateConfiguration> create(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        const std::string& lwaUrl );

    std::string getClientId() const;
    std::string getProductId() const;
    std::string getDeviceSerialNumber() const;
    std::chrono::seconds getRequestTimeout() const;
    std::chrono::seconds getCodePairRequestTimeout() const;
    std::chrono::seconds getAccessTokenRefreshHeadStart() const;
    std::string getRequestCodePairUrl() const;
    std::string getRequestTokenUrl() const;
    std::string getRefreshTokenUrl() const;
    std::string getScopeData() const;

private:

    bool initialize( std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::chrono::seconds codePairRequestTimeout,
        const std::string& lwaUrl );
    bool initScopeData();

    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
    std::chrono::seconds m_requestTimeout;
    std::chrono::seconds m_codePairRequestTimeout;
    std::chrono::seconds m_accessTokenRefreshHeadStart;
    std::string m_requestCodePairUrl;
    std::string m_requestTokenUrl;
    std::string m_refreshTokenUrl;
    std::string m_scopeData;
};

} // aace::engine::cbl
} // aace::engine
} // aace

#endif
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

#ifndef AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_CONFIGURATION_INTERFACE_H
#define AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_CONFIGURATION_INTERFACE_H

#include <chrono>
#include <memory>
#include <string>

namespace aace {
namespace engine {
namespace cbl {

class CBLConfigurationInterface {
public:
    /// Virtual destructor
    virtual ~CBLConfigurationInterface() = default;

public:
    virtual std::string getClientId() const = 0;
    virtual std::string getProductId() const = 0;
    virtual std::string getDeviceSerialNumber() const = 0;
    virtual std::chrono::seconds getRequestTimeout() const = 0;
    virtual std::chrono::seconds getCodePairRequestTimeout() const = 0;
    virtual std::chrono::seconds getAccessTokenRefreshHeadStart() const = 0;
    virtual std::string getRequestCodePairUrl() const = 0;
    virtual std::string getRequestTokenUrl() const = 0;
    virtual std::string getRefreshTokenUrl() const = 0;
    virtual std::string getScopeData() const = 0;

    /**
     * Function to get the default locale to be used in HTTP Accept-Language header while
     * requesting the code pair.
     */
    virtual std::string getDefaultLocale() const = 0;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CBL_CBL_AUTH_DELEGATE_CONFIGURATION_INTERFACE_H
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

#ifndef AACE_JNI_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H
#define AACE_JNI_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H

#include <AACE/Network/NetworkInfoProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace network {

//
// NetworkInfoProviderHandler
//

class NetworkInfoProviderHandler : public aace::network::NetworkInfoProvider {
public:
    NetworkInfoProviderHandler(jobject obj);

    // aace::network::NetworkInfoProvider
    NetworkStatus getNetworkStatus() override;
    int getWifiSignalStrength() override;

private:
    JObject m_obj;
};

//
// NetworkInfoProviderBinder
//

class NetworkInfoProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    NetworkInfoProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_networkInfoProviderHandler;
    }

    std::shared_ptr<NetworkInfoProviderHandler> getNetworkInfoProviderHandler() {
        return m_networkInfoProviderHandler;
    }

private:
    std::shared_ptr<NetworkInfoProviderHandler> m_networkInfoProviderHandler;
};

//
// JNetworkStatus
//

class JNetworkStatusConfig : public EnumConfiguration<NetworkInfoProviderHandler::NetworkStatus> {
public:
    using T = NetworkInfoProviderHandler::NetworkStatus;

    const char* getClassName() override {
        return "com/amazon/aace/network/NetworkInfoProvider$NetworkStatus";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"},
                {T::DISCONNECTED, "DISCONNECTED"},
                {T::DISCONNECTING, "DISCONNECTING"},
                {T::CONNECTED, "CONNECTED"},
                {T::CONNECTING, "CONNECTING"}};
    }
};

using JNetworkStatus = JEnum<NetworkInfoProviderHandler::NetworkStatus, JNetworkStatusConfig>;

}  // namespace network
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H

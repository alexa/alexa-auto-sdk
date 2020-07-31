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

#include "AACE/Network/NetworkInfoProvider.h"

namespace aace {
namespace network {

NetworkInfoProvider::~NetworkInfoProvider() = default;  // key function

void NetworkInfoProvider::networkStatusChanged(NetworkStatus status, int wifiSignalStrength) {
    if (m_networkInfoProviderEngineInterface != nullptr) {
        m_networkInfoProviderEngineInterface->networkInfoChanged(
            static_cast<NetworkStatus>(status), wifiSignalStrength);
    }
}

void NetworkInfoProvider::setEngineInterface(
    std::shared_ptr<NetworkInfoProviderEngineInterface> networkInfoProviderEngineInterface) {
    m_networkInfoProviderEngineInterface = networkInfoProviderEngineInterface;
}

}  // namespace network
}  // namespace aace

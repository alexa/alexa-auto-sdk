/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Network/NetworkInfoProviderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace network {

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.NetworkInfoProviderEngineImpl");

std::shared_ptr<NetworkInfoProviderEngineImpl> NetworkInfoProviderEngineImpl::create() {
    return std::shared_ptr<NetworkInfoProviderEngineImpl>( new NetworkInfoProviderEngineImpl() );
}

NetworkInfoProviderEngineImpl::NetworkInfoProviderEngineImpl():
    m_networkInterface("") {
}

void NetworkInfoProviderEngineImpl::addObserver( std::shared_ptr<NetworkInfoObserver> observer ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    m_observers.insert( observer );
}

void NetworkInfoProviderEngineImpl::removeObserver( std::shared_ptr<NetworkInfoObserver> observer ) {
    std::lock_guard<std::mutex> lock( m_mutex );
    m_observers.erase( observer );
}

void NetworkInfoProviderEngineImpl::networkInfoChanged( NetworkStatus status, int wifiSignalStrength )
{
    std::lock_guard<std::mutex> lock( m_mutex );

    for( const auto& next : m_observers ) {
        next->onNetworkInfoChanged( status, wifiSignalStrength );
    }
}

bool NetworkInfoProviderEngineImpl::setNetworkInterface( const std::string& networkInterface ) {
    AACE_INFO(LX(TAG,"setNetworkInterface").d("networkInterface", networkInterface));

    m_networkInterface = networkInterface;

    std::lock_guard<std::mutex> lock( m_mutex );

    //Notify the begin
    for( const auto& next : m_observers ) {
        next->onNetworkInterfaceChangeStatusChanged( m_networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::BEGIN );
    }

    //Notify to Change network interface
    for( const auto& next : m_observers ) {
        next->onNetworkInterfaceChangeStatusChanged( m_networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::CHANGE );
    }

    // Notify Completed
    for( const auto& next : m_observers ) {
        next->onNetworkInterfaceChangeStatusChanged( m_networkInterface, NetworkInfoObserver::NetworkInterfaceChangeStatus::COMPLETED );
    }

    return true;
}

std::string NetworkInfoProviderEngineImpl::getNetworkInterface() {
    return m_networkInterface;
}

} // aace::engine::network
} // aace::engine
} // aace

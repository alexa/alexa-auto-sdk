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

// aace/network/NetworkInfoProviderBinder.h
// This is an automatically generated file.

#ifndef AACE_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H
#define AACE_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Network/NetworkInfoProvider.h"
#include "aace/core/PlatformInterfaceBinder.h"

class NetworkInfoProviderBinder : public PlatformInterfaceBinder, public aace::network::NetworkInfoProvider {
public:
    NetworkInfoProviderBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    aace::network::NetworkInfoProvider::NetworkStatus getNetworkStatus() override;
    int getWifiSignalStrength() override;

public:
    aace::network::NetworkInfoProviderEngineInterface::NetworkStatus convertNetworkStatus( JNIEnv* env, jobject obj );

private:
    jobject convert( aace::network::NetworkInfoProviderEngineInterface::NetworkStatus status );

private:
    jmethodID m_javaMethod_getNetworkStatus = nullptr;
    jmethodID m_javaMethod_getWifiSignalStrength = nullptr;

    // NetworkStatus
    ObjectRef m_enum_NetworkStatus_UNKNOWN;
    ObjectRef m_enum_NetworkStatus_DISCONNECTED;
    ObjectRef m_enum_NetworkStatus_DISCONNECTING;
    ObjectRef m_enum_NetworkStatus_CONNECTED;
    ObjectRef m_enum_NetworkStatus_CONNECTING;
};

#endif //AACE_NETWORK_NETWORK_INFO_PROVIDER_BINDER_H

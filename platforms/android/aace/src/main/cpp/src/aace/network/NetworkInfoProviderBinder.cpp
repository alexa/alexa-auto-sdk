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

// aace/network/NetworkInfoProviderBinder.cpp
// This is an automatically generated file.

#include "aace/network/NetworkInfoProviderBinder.h"

void NetworkInfoProviderBinder::initialize( JNIEnv* env )
{
    m_javaMethod_getNetworkStatus = env->GetMethodID( getJavaClass(), "getNetworkStatus", "()Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
    m_javaMethod_getWifiSignalStrength = env->GetMethodID( getJavaClass(), "getWifiSignalStrength", "()I" );

    // NetworkStatus
    jclass networkStatusEnumClass = env->FindClass( "com/amazon/aace/network/NetworkInfoProvider$NetworkStatus" );
    m_enum_NetworkStatus_UNKNOWN = NativeLib::FindEnum( env, networkStatusEnumClass, "UNKNOWN", "Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
    m_enum_NetworkStatus_DISCONNECTED = NativeLib::FindEnum( env, networkStatusEnumClass, "DISCONNECTED", "Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
    m_enum_NetworkStatus_DISCONNECTING = NativeLib::FindEnum( env, networkStatusEnumClass, "DISCONNECTING", "Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
    m_enum_NetworkStatus_CONNECTED = NativeLib::FindEnum( env, networkStatusEnumClass, "CONNECTED", "Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
    m_enum_NetworkStatus_CONNECTING = NativeLib::FindEnum( env, networkStatusEnumClass, "CONNECTING", "Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;" );
}

aace::network::NetworkInfoProvider::NetworkStatus NetworkInfoProviderBinder::getNetworkStatus()
{
    aace::network::NetworkInfoProvider::NetworkStatus result = aace::network::NetworkInfoProvider::NetworkStatus();

    if( getJavaObject() != nullptr && m_javaMethod_getNetworkStatus != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = convertNetworkStatus( context.getEnv(), context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getNetworkStatus ));
        }
    }

    return result;
}

int NetworkInfoProviderBinder::getWifiSignalStrength()
{
    int result = 0;

    if( getJavaObject() != nullptr && m_javaMethod_getWifiSignalStrength != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallIntMethod( getJavaObject(), m_javaMethod_getWifiSignalStrength );
        }
    }
    return result;
}

jobject NetworkInfoProviderBinder::convert( aace::network::NetworkInfoProviderEngineInterface::NetworkStatus status )
{

    switch( status )
    {
        case aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::UNKNOWN:
            return m_enum_NetworkStatus_UNKNOWN.get();
        case aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTED:
            return m_enum_NetworkStatus_DISCONNECTED.get();
        case aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTING:
            return m_enum_NetworkStatus_DISCONNECTING.get();
        case aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTED:
            return m_enum_NetworkStatus_CONNECTED.get();
        case aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTING:
            return m_enum_NetworkStatus_CONNECTING.get();
    }
}

aace::network::NetworkInfoProviderEngineInterface::NetworkStatus NetworkInfoProviderBinder::convertNetworkStatus( JNIEnv* env, jobject obj )
{
    if( m_enum_NetworkStatus_DISCONNECTED.isSameObject( env, obj ) ) {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTED;
    }
    else if( m_enum_NetworkStatus_DISCONNECTING.isSameObject( env, obj ) ) {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::DISCONNECTING;
    }
    else if( m_enum_NetworkStatus_CONNECTED.isSameObject( env, obj ) ) {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTED;
    }
    else if( m_enum_NetworkStatus_CONNECTING.isSameObject( env, obj ) ) {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTING;
    }
    else if( m_enum_NetworkStatus_UNKNOWN.isSameObject( env, obj ) ) {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::UNKNOWN;
    }
    else {
        return aace::network::NetworkInfoProviderEngineInterface::NetworkStatus::CONNECTING;
    }
}

// JNI
#define NETWORKINFOPROVIDER(cptr) ((NetworkInfoProviderBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_network_NetworkInfoProvider_networkStatusChanged( JNIEnv * env , jobject /* this */, jlong cptr, jobject status, jint wifiSignalStrength ) {
    NETWORKINFOPROVIDER(cptr)->networkStatusChanged( NETWORKINFOPROVIDER(cptr)->convertNetworkStatus( env, status ), wifiSignalStrength );
}

}

// END OF FILE

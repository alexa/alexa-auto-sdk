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

#include <AACE/JNI/Network/NetworkInfoProviderBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.network.NetworkInfoProviderBinder";

// type aliases
using NetworkStatus = aace::jni::network::NetworkInfoProviderHandler::NetworkStatus;

namespace aace {
namespace jni {
namespace network {

//
// NetworkInfoProviderBinder
//

NetworkInfoProviderBinder::NetworkInfoProviderBinder(jobject obj) {
    m_networkInfoProviderHandler = std::make_shared<NetworkInfoProviderHandler>(obj);
}

//
// NetworkInfoProviderHandler
//

NetworkInfoProviderHandler::NetworkInfoProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/network/NetworkInfoProvider") {
}

NetworkStatus NetworkInfoProviderHandler::getNetworkStatus() {
    try_with_context {
        jobject statusObj;
        ThrowIfNot(
            m_obj.invoke(
                "getNetworkStatus", "()Lcom/amazon/aace/network/NetworkInfoProvider$NetworkStatus;", &statusObj),
            "invokeFailed");

        return JNetworkStatus::from(statusObj, NetworkStatus::UNKNOWN);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getNetworkStatus", ex.what());
        return NetworkStatus::UNKNOWN;
    }
}

int NetworkInfoProviderHandler::getWifiSignalStrength() {
    try_with_context {
        jint wifiSignalStrength;
        ThrowIfNot(m_obj.invoke("getWifiSignalStrength", "()I", &wifiSignalStrength), "invokeFailed");

        return static_cast<int>(wifiSignalStrength);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getWifiSignalStrength", ex.what());
        return 0;
    }
}

}  // namespace network
}  // namespace jni
}  // namespace aace

#define NETWORK_INFO_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::network::NetworkInfoProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_network_NetworkInfoProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::network::NetworkInfoProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_network_NetworkInfoProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto networkInfoProviderBinder = NETWORK_INFO_PROVIDER_BINDER(ref);
        ThrowIfNull(networkInfoProviderBinder, "invalidNetworkInfoProviderBinder");
        delete networkInfoProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_network_NetworkInfoProvider_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_network_NetworkInfoProvider_networkStatusChanged(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject status,
    jint wifiSignalStrength) {
    try {
        auto networkInfoProviderBinder = NETWORK_INFO_PROVIDER_BINDER(ref);
        ThrowIfNull(networkInfoProviderBinder, "invalidNetworkInfoProviderBinder");

        NetworkStatus statusType;
        ThrowIfNot(aace::jni::network::JNetworkStatus::checkType(status, &statusType), "invalidNetworkStatusType");

        networkInfoProviderBinder->getNetworkInfoProviderHandler()->networkStatusChanged(
            statusType, static_cast<int>(wifiSignalStrength));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_network_NetworkInfoProvider_networkStatusChanged", ex.what());
    }
}
}

/*
 * Copyright 2017-2018, 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Bluetooth/BluetoothProviderBinder.h>
#include <AACE/JNI/Bluetooth/GATTServerBinder.h>
#include <AACE/JNI/Bluetooth/BluetoothServerSocketBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.bluetooth.BluetoothProviderBinder";

namespace aace {
namespace jni {
namespace bluetooth {

//
// BluetoothProviderBinder
//

BluetoothProviderBinder::BluetoothProviderBinder(jobject obj) {
    m_bluetoothProviderHandler = std::make_shared<BluetoothProviderHandler>(obj);
}

//
// BluetoothProviderHandler
//

BluetoothProviderHandler::BluetoothProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/bluetooth/BluetoothProvider") {
}

// aace::bluetooth::BluetoothProvider
std::shared_ptr<aace::bluetooth::GATTServer> BluetoothProviderHandler::createGATTServer() {
    try_with_context {
        jobject result = nullptr;
        ThrowIfNot(
            m_obj.invoke("createGATTServer", "()Lcom/amazon/aace/bluetooth/GATTServer;", &result),
            "invokeMethodFailed");
        ThrowIfNull(result, "invalidGATTServer");

        // create a GATT server JObject
        JObject gattServerObj(result, "com/amazon/aace/bluetooth/GATTServer");
        ThrowIfJavaEx(env, "invalidGATTServerObj");

        // get the GATT server binder native ref
        jlong nativeRef;
        ThrowIfNot(gattServerObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an GATT server ptr
        auto gattServerBinder = reinterpret_cast<GATTServerBinder*>(nativeRef);
        ThrowIfNull(gattServerBinder, "invalidGATTServerBinder");

        // get the GATT server handler from the binder
        return gattServerBinder->getGATTServerHandler();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "createGATTServer", ex.what());
        return nullptr;
    }
}

std::shared_ptr<aace::bluetooth::BluetoothServerSocket> BluetoothProviderHandler::listenUsingRfcomm(
    const std::string& name,
    const std::string& uuid) {
    try_with_context {
        jobject result = nullptr;
        ThrowIfNot(
            m_obj.invoke(
                "listenUsingRfcomm",
                "(Ljava/lang/String;Ljava/lang/String;)Lcom/amazon/aace/bluetooth/BluetoothServerSocket;",
                &result,
                JString(name).get(),
                JString(uuid).get()),
            "invokeMethodFailed");
        ThrowIfNull(result, "invalidBluetoothServerSocket");

        // create a server socket JObject
        JObject sockectObj(result, "com/amazon/aace/bluetooth/BluetoothServerSocket");
        ThrowIfJavaEx(env, "invalidBluetoothServerSocketObj");

        // get the server socket binder native ref
        jlong nativeRef;
        ThrowIfNot(sockectObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an server socket shared_ptr
        auto socketBinder = reinterpret_cast<std::shared_ptr<BluetoothServerSocketBinder>*>(nativeRef);
        ThrowIfNull(socketBinder, "invalidBluetoothServerSocketBinder");

        return *socketBinder;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "listenUsingRfcomm", ex.what());
        return nullptr;
    }
}

std::shared_ptr<aace::bluetooth::BluetoothServerSocket> BluetoothProviderHandler::listenUsingiAP2(
    const std::string& protocol) {
    try_with_context {
        jobject result = nullptr;
        ThrowIfNot(
            m_obj.invoke(
                "listenUsingiAP2",
                "(Ljava/lang/String;)Lcom/amazon/aace/bluetooth/BluetoothServerSocket;",
                &result,
                JString(protocol).get()),
            "invokeMethodFailed");
        ThrowIfNull(result, "invalidBluetoothServerSocket");

        // create a server socket JObject
        JObject sockectObj(result, "com/amazon/aace/bluetooth/BluetoothServerSocket");
        ThrowIfJavaEx(env, "invalidBluetoothServerSocketObj");

        // get the server socket binder native ref
        jlong nativeRef;
        ThrowIfNot(sockectObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an server socket shared_ptr
        auto socketBinder = reinterpret_cast<std::shared_ptr<BluetoothServerSocketBinder>*>(nativeRef);
        ThrowIfNull(socketBinder, "invalidBluetoothServerSocketBinder");

        return *socketBinder;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "listenUsingRfcomm", ex.what());
        return nullptr;
    }
}

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

#define BLUETOOTH_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::bluetooth::BluetoothProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_bluetooth_BluetoothProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::bluetooth::BluetoothProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_bluetooth_BluetoothProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto bluetoothProviderBinder = BLUETOOTH_PROVIDER_BINDER(ref);
        ThrowIfNull(bluetoothProviderBinder, "invalidBluetoothProviderBinder");
        delete bluetoothProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_bluetooth_BluetoothProvider_disposeBinder", ex.what());
    }
}
}

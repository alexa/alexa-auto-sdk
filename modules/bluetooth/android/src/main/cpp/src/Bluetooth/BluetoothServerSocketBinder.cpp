/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Bluetooth/BluetoothServerSocketBinder.h>
#include <AACE/JNI/Bluetooth/BluetoothSocketBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.bluetooth.BluetoothServerSocketBinder";

namespace aace {
namespace jni {
namespace bluetooth {

//
// BluetoothServerSocketBinder
//

BluetoothServerSocketBinder::BluetoothServerSocketBinder(jobject obj) :
        m_obj(obj, "com/amazon/aace/bluetooth/BluetoothServerSocket") {
}

std::shared_ptr<aace::bluetooth::BluetoothSocket> BluetoothServerSocketBinder::accept() {
    try_with_context {
        jobject result = nullptr;
        ThrowIfNot(m_obj.invoke("accept", "()Lcom/amazon/aace/bluetooth/BluetoothSocket;", &result), "invokeFailed");

        // create a socket JObject
        JObject sockectObj(result, "com/amazon/aace/bluetooth/BluetoothSocket");
        ThrowIfJavaEx(env, "invalidBluetoothServerSocketObj");

        // get the server socket binder native ref
        jlong nativeRef;
        ThrowIfNot(sockectObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an server server ptr
        auto socketBinder = reinterpret_cast<std::shared_ptr<BluetoothSocketBinder>*>(nativeRef);
        ThrowIfNull(socketBinder, "invalidBluetoothSocketBinder");

        return *socketBinder;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return nullptr;
    }
}

void BluetoothServerSocketBinder::close() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("close", "()V", nullptr), "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_bluetooth_BluetoothServerSocket_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(
        new std::shared_ptr<BluetoothServerSocketBinder>(new BluetoothServerSocketBinder(obj)));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_bluetooth_BluetoothServerSocket_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto binder = reinterpret_cast<std::shared_ptr<BluetoothServerSocketBinder>*>(ref);
        ThrowIfNull(binder, "invalidBluetoothServerSocketBinder");
        delete binder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace
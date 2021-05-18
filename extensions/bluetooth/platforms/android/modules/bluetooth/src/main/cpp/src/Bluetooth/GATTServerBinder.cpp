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

#include <AACE/JNI/Bluetooth/GATTServerBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

#include <vector>
#include <istream>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.bluetooth.GATTServerBinder";

// type aliases
using ConnectionState = aace::jni::bluetooth::GATTServerHandler::ConnectionState;

namespace aace {
namespace jni {
namespace bluetooth {

//
// GATTServerBinder
//

GATTServerBinder::GATTServerBinder(jobject obj) {
    m_gattServerHandler = std::make_shared<GATTServerHandler>(obj);
}

//
// GATTServerHandler
//

GATTServerHandler::GATTServerHandler(jobject obj) : m_obj(obj, "com/amazon/aace/bluetooth/GATTServer") {
}

// aace::bluetooth::GATTServer
bool GATTServerHandler::start(const std::string& configuration) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("start", "(Ljava/lang/String;)Z", &result, JString(configuration).get()), "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "start", ex.what());
        return false;
    }
}

bool GATTServerHandler::stop() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("stop", "()Z", &result), "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "stop", ex.what());
        return false;
    }
}

bool GATTServerHandler::setCharacteristicValue(
    const std::string& serviceId,
    const std::string& characteristicId,
    aace::bluetooth::ByteArrayPtr data) {
    try_with_context {
        // copy the data stream into a byte array
        JByteArray arr(data->size());
        ThrowIfNot(arr.copyTo(0, data->size(), (jbyte*)data->data()), "copyToArrayFailed");

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "setCharacteristicValue",
                "(Ljava/lang/String;Ljava/lang/String;[B)Z",
                &result,
                JString(serviceId).get(),
                JString(characteristicId).get(),
                arr.get()),
            "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "start", ex.what());
        return false;
    }
}

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

#define GATT_SERVER_BINDER(ref) reinterpret_cast<aace::jni::bluetooth::GATTServerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_bluetooth_GATTServer_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::bluetooth::GATTServerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_bluetooth_GATTServer_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto gattServerBinder = GATT_SERVER_BINDER(ref);
        ThrowIfNull(gattServerBinder, "invalidGATTServerBinder");
        delete gattServerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_bluetooth_GATTServer_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_bluetooth_GATTServer_connectionStateChanged(
    JNIEnv* env,
    jobject,
    jlong ref,
    jstring device,
    jobject state) {
    try {
        auto gattServerBinder = GATT_SERVER_BINDER(ref);
        ThrowIfNull(gattServerBinder, "invalidGATTServerBinder");

        ConnectionState stateType;
        ThrowIfNot(aace::jni::bluetooth::JConnectionState::checkType(state, &stateType), "invalidConnectionStateType");

        gattServerBinder->getGATTServerHandler()->connectionStateChanged(JString(device).toStdStr(), stateType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_bluetooth_GATTServer_connectionStateChanged", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_bluetooth_GATTServer_requestCharacteristic(
    JNIEnv* env,
    jobject,
    jlong ref,
    jstring device,
    jint requestId,
    jstring serviceId,
    jstring characteristicId,
    jbyteArray data) {
    try {
        auto gattServerBinder = GATT_SERVER_BINDER(ref);
        ThrowIfNull(gattServerBinder, "invalidGATTServerBinder");

        JByteArray arr(data);

        auto bap = aace::bluetooth::createByteArray(arr.size(), false);

        bap->insert(bap->end(), arr.ptr(), arr.ptr() + arr.size());

        return gattServerBinder->getGATTServerHandler()->requestCharacteristic(
            JString(device).toStdStr(),
            requestId,
            JString(serviceId).toStdStr(),
            JString(characteristicId).toStdStr(),
            std::move(bap));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_bluetooth_GATTServer_requestCharacteristic", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_bluetooth_GATTServer_requestDescriptor(
    JNIEnv* env,
    jobject,
    jlong ref,
    jstring device,
    jint requestId,
    jstring serviceId,
    jstring characteristicId,
    jstring descriptorId,
    jbyteArray data) {
    try {
        auto gattServerBinder = GATT_SERVER_BINDER(ref);
        ThrowIfNull(gattServerBinder, "invalidGATTServerBinder");

        JByteArray arr(data);

        auto bap = aace::bluetooth::createByteArray(arr.size());

        bap->insert(bap->end(), arr.ptr(), arr.ptr() + arr.size());

        jboolean result = gattServerBinder->getGATTServerHandler()->requestDescriptor(
            JString(device).toStdStr(),
            requestId,
            JString(serviceId).toStdStr(),
            JString(characteristicId).toStdStr(),
            JString(descriptorId).toStdStr(),
            std::move(bap));

        return result;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_bluetooth_GATTServer_requestDescriptor", ex.what());
        return false;
    }
}
}

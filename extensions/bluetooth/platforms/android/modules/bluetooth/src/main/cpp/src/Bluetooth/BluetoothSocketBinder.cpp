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

#include <AACE/JNI/Bluetooth/BluetoothSocketBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.bluetooth.BluetoothSocketBinder";

namespace aace {
namespace jni {
namespace bluetooth {

//
// BluetoothSocketBinder
//

BluetoothSocketBinder::BluetoothSocketBinder(jobject obj) : m_obj(obj, "com/amazon/aace/bluetooth/BluetoothSocket") {
}

int BluetoothSocketBinder::read(uint8_t* data, size_t off, size_t len) {
    try_with_context {
        jint result = 0;
        JByteArray arr(len);
        ThrowIfNot(m_obj.invoke("read", "([BII)I", &result, arr.get(), 0, len), "invokeFailed");
        if (result > 0) {
            std::memcpy(data + off, arr.ptr(), result);
        }
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return 0;
    }
    return 0;
}

void BluetoothSocketBinder::write(uint8_t* data, size_t off, size_t len) {
    try_with_context {
        JByteArray arr(len);
        ThrowIfNot(arr.copyTo(0, len, reinterpret_cast<jbyte*>(data + off)), "copyToArrayFailed");
        ThrowIfNot(m_obj.invoke<void>("write", "([BII)V", nullptr, arr.get(), 0, len), "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

void BluetoothSocketBinder::close() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("close", "()V", nullptr), "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_bluetooth_BluetoothSocket_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new std::shared_ptr<BluetoothSocketBinder>(new BluetoothSocketBinder(obj)));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_bluetooth_BluetoothSocket_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto binder = reinterpret_cast<std::shared_ptr<BluetoothSocketBinder>*>(ref);
        ThrowIfNull(binder, "invalidBluetoothSocketBinder");
        delete binder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}
}

}  // namespace bluetooth
}  // namespace jni
}  // namespace aace

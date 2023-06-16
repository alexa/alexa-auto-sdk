#include <climits>
#include <memory>
/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/JNI/Core/NativeLib.h"
#include "AACE/JNI/MobileBridge/MobileBridgeBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.mobileBridge.MobileBridgeBinder";

namespace aace {
namespace jni {
namespace mobileBridge {

//
// MobileBridgeBinder
//

MobileBridgeBinder::MobileBridgeBinder(jobject obj) {
    m_mobileBridgeHandler = std::make_shared<MobileBridgeHandler>(obj);
}

//
// MobileBridgeHandler
//

MobileBridgeHandler::MobileBridgeHandler(jobject obj) : m_obj(obj, "com/amazon/aace/mobileBridge/MobileBridge") {
}

MobileBridgeHandler::~MobileBridgeHandler() {
}

std::vector<std::shared_ptr<aace::mobileBridge::Transport>> MobileBridgeHandler::getTransports() {
    // TODO: not implemented
    return {};
}

std::shared_ptr<aace::mobileBridge::MobileBridge::Connection> MobileBridgeHandler::connect(
    const std::string& transportId) {
    // TODO: not implemented
    return nullptr;
}

void MobileBridgeHandler::disconnect(const std::string& transportId) {
    // TODO: not implemented
}

void MobileBridgeHandler::onActiveTransportChange(const std::string& transportId, const std::string& transportState) {
    // TODO: not implemented
}

void MobileBridgeHandler::onDeviceHandshaked(
    const std::string& transportId,
    const std::string& deviceToken,
    const std::string& friendlyName) {
    // TODO: not implemented
}

void MobileBridgeHandler::onInfo(const std::string& deviceToken, uint32_t messageId, const std::string& message) {
    // TODO: not implemented
}

bool MobileBridgeHandler::protectSocket(int socket) {
    // TODO: not implemented
    return false;
}

}  // namespace mobileBridge
}  // namespace jni
}  // namespace aace

#define MOBILE_BRIDGE_BINDER(ref) reinterpret_cast<aace::jni::mobileBridge::MobileBridgeBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_mobileBridge_MobileBridge_createBinder(__unused JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::mobileBridge::MobileBridgeBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_mobileBridge_MobileBridge_disposeBinder(__unused JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto mobileBridgeBinder = MOBILE_BRIDGE_BINDER(ref);
        ThrowIfNull(mobileBridgeBinder, "invalidMobileBridgeBinder");
        delete mobileBridgeBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_mobileBridge_MobileBridge_start(__unused JNIEnv* env, jobject /* this */, jlong ref, jint tunFd) {
    try {
        auto mobileBridgeBinder = MOBILE_BRIDGE_BINDER(ref);
        ThrowIfNull(mobileBridgeBinder, "invalidMobileBridgeBinder");
        mobileBridgeBinder->getMobileBridge()->start(tunFd);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_mobileBridge_MobileBridge_stop(__unused JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto mobileBridgeBinder = MOBILE_BRIDGE_BINDER(ref);
        ThrowIfNull(mobileBridgeBinder, "invalidMobileBridgeBinder");
        mobileBridgeBinder->getMobileBridge()->stop();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
    }
}

}  // extern "C"

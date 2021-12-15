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

#include <AACE/JNI/PhoneControl/PhoneCallControllerBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.phonecontrol.PhoneCallControllerBinder";

// type aliases
using ConnectionState = aace::jni::phonecontrol::PhoneCallControllerHandler::ConnectionState;
using CallState = aace::jni::phonecontrol::PhoneCallControllerHandler::CallState;
using CallError = aace::jni::phonecontrol::PhoneCallControllerHandler::CallError;
using DTMFError = aace::jni::phonecontrol::PhoneCallControllerHandler::DTMFError;
using CallingDeviceConfigurationProperty =
    aace::jni::phonecontrol::PhoneCallControllerHandler::CallingDeviceConfigurationProperty;

namespace aace {
namespace jni {
namespace phonecontrol {

//
// PhoneCallControllerBinder
//

PhoneCallControllerBinder::PhoneCallControllerBinder(jobject obj) {
    m_phoneCallControllerHandler = std::shared_ptr<PhoneCallControllerHandler>(new PhoneCallControllerHandler(obj));
}

//
// PhoneCallControllerHandler
//

PhoneCallControllerHandler::PhoneCallControllerHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/phonecontrol/PhoneCallController") {
}

bool PhoneCallControllerHandler::dial(const std::string& payload) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("dial", "(Ljava/lang/String;)Z", &result, JString(payload).get()), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "dial", ex.what());
        return false;
    }
}

bool PhoneCallControllerHandler::redial(const std::string& payload) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("redial", "(Ljava/lang/String;)Z", &result, JString(payload).get()), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "redial", ex.what());
        return false;
    }
}

void PhoneCallControllerHandler::answer(const std::string& payload) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("answer", "(Ljava/lang/String;)V", nullptr, JString(payload).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "answer", ex.what());
    }
}

void PhoneCallControllerHandler::stop(const std::string& payload) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("stop", "(Ljava/lang/String;)V", nullptr, JString(payload).get()), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "stop", ex.what());
    }
}

void PhoneCallControllerHandler::sendDTMF(const std::string& payload) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("sendDTMF", "(Ljava/lang/String;)V", nullptr, JString(payload).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "sendDTMF", ex.what());
    }
}

}  // namespace phonecontrol
}  // namespace jni
}  // namespace aace

#define PHONECALLCONTROLLER_BINDER(ref) reinterpret_cast<aace::jni::phonecontrol::PhoneCallControllerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::phonecontrol::PhoneCallControllerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");
        delete phoneCallControllerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_connectionStateChanged(
    JNIEnv* env,
    jobject,
    jlong ref,
    jobject state) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        ConnectionState stateType;
        ThrowIfNot(
            aace::jni::phonecontrol::JConnectionState::checkType(state, &stateType), "invalidConnectionStateType");

        phoneCallControllerBinder->getPhoneCallController()->connectionStateChanged(stateType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_connectionStateChanged", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_callStateChanged(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject state,
    jstring callId,
    jstring callerId) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        CallState stateType;
        ThrowIfNot(aace::jni::phonecontrol::JCallState::checkType(state, &stateType), "invalidCallStateType");

        phoneCallControllerBinder->getPhoneCallController()->callStateChanged(
            stateType, JString(callId).toStdStr(), JString(callerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_callStateChanged", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring callId,
    jobject code,
    jstring message) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        CallError codeType;
        ThrowIfNot(aace::jni::phonecontrol::JCallError::checkType(code, &codeType), "invalidCallError");

        phoneCallControllerBinder->getPhoneCallController()->callFailed(
            JString(callId).toStdStr(), codeType, JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_callerIdReceived(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring callId,
    jstring callerId) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        phoneCallControllerBinder->getPhoneCallController()->callerIdReceived(
            JString(callId).toStdStr(), JString(callerId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_callerIdReceived", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFSucceeded(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring callId) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        phoneCallControllerBinder->getPhoneCallController()->sendDTMFSucceeded(JString(callId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFSucceeded", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFFailed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring callId,
    jobject code,
    jstring message) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        DTMFError codeType;
        ThrowIfNot(aace::jni::phonecontrol::JDTMFError::checkType(code, &codeType), "invalidDTMFError");

        phoneCallControllerBinder->getPhoneCallController()->sendDTMFFailed(
            JString(callId).toStdStr(), codeType, JString(message).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_sendDTMFFailed", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_phonecontrol_PhoneCallController_deviceConfigurationUpdated(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobjectArray configurations,
    jbooleanArray configurationValues) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        std::unordered_map<CallingDeviceConfigurationProperty, bool> deviceConfigurationMap;
        jboolean* boolArray = env->GetBooleanArrayElements(configurationValues, NULL);
        JObjectArray configObjArr(configurations);

        jobject configObj;
        CallingDeviceConfigurationProperty propertyType;

        for (int i = 0; i < configObjArr.size(); i++) {
            ThrowIfNot(configObjArr.getAt(i, &configObj), "getArrayValueFailed");
            ThrowIfNot(
                aace::jni::phonecontrol::JCallingDeviceConfigurationProperty::checkType(configObj, &propertyType),
                "invalidCalingDeviceConfigurationProperty");

            deviceConfigurationMap[propertyType] = (boolArray[i] == JNI_TRUE) ? true : false;
        }

        phoneCallControllerBinder->getPhoneCallController()->deviceConfigurationUpdated(deviceConfigurationMap);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(
            TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_deviceConfigurationUpdated", ex.what());
    }
}

JNIEXPORT jstring JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_createCallId(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto phoneCallControllerBinder = PHONECALLCONTROLLER_BINDER(ref);
        ThrowIfNull(phoneCallControllerBinder, "invalidPhoneCallControllerBinder");

        return JString(phoneCallControllerBinder->getPhoneCallController()->createCallId()).get();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_phonecontrol_PhoneCallController_createCallId", ex.what());
        return JString().get();
    }
}
}
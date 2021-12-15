/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/CarControl/CarControlBinder.h>

// String to identify log entries originating from this file
static const char TAG[] = "aace.jni.carControl.CarControl";

namespace aace {
namespace jni {
namespace carControl {

//
// CarControlBinder
//

CarControlBinder::CarControlBinder(jobject obj) {
    m_carControlHandler = std::shared_ptr<CarControlHandler>(new CarControlHandler(obj));
}

//
// CarControlHandler
//

CarControlHandler::CarControlHandler(jobject obj) : m_obj(obj, "com/amazon/aace/carControl/CarControl") {
}

bool CarControlHandler::turnPowerControllerOn(const std::string& controlId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("turnPowerControllerOn", "(Ljava/lang/String;)V", nullptr, JString(controlId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "turnPowerControllerOn", ex.what());
        return false;
    }
}

bool CarControlHandler::turnPowerControllerOff(const std::string& controlId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("turnPowerControllerOff", "(Ljava/lang/String;)V", nullptr, JString(controlId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "turnPowerControllerOff", ex.what());
        return false;
    }
}

bool CarControlHandler::isPowerControllerOn(const std::string& controlId, bool& isOn) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("isPowerControllerOn", "(Ljava/lang/String;)Z", &result, JString(controlId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        isOn = result == JNI_TRUE ? true : false;
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "isPowerControllerOn", ex.what());
        return false;
    }
}

bool CarControlHandler::turnToggleControllerOn(const std::string& controlId, const std::string& controllerId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "turnToggleControllerOn",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "turnToggleControllerOn", ex.what());
        return false;
    }
}

bool CarControlHandler::turnToggleControllerOff(const std::string& controlId, const std::string& controllerId) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "turnToggleControllerOff",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "turnToggleControllerOff", ex.what());
        return false;
    }
}

bool CarControlHandler::isToggleControllerOn(
    const std::string& controlId,
    const std::string& controllerId,
    bool& isOn) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "isToggleControllerOn",
                "(Ljava/lang/String;Ljava/lang/String;)Z",
                &result,
                JString(controlId).get(),
                JString(controllerId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        isOn = result == JNI_TRUE ? true : false;
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "isToggleControllerOn", ex.what());
        return false;
    }
}

bool CarControlHandler::setRangeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    double value) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "setRangeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;D)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get(),
                value),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setRangeControllerValue", ex.what());
        return false;
    }
}

bool CarControlHandler::adjustRangeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    double delta) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "adjustRangeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;D)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get(),
                delta),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "adjustRangeControllerValue", ex.what());
        return false;
    }
}

bool CarControlHandler::getRangeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    double& value) {
    try_with_context {
        jdouble result;
        ThrowIfNot(
            m_obj.invoke(
                "getRangeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;)D",
                &result,
                JString(controlId).get(),
                JString(controllerId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        value = result;
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getRangeControllerValue", ex.what());
        return false;
    }
}

bool CarControlHandler::setModeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    const std::string& value) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "setModeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get(),
                JString(value).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setModeControllerValue", ex.what());
        return false;
    }
}

bool CarControlHandler::adjustModeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    int delta) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "adjustModeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;I)V",
                nullptr,
                JString(controlId).get(),
                JString(controllerId).get(),
                delta),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "adjustModeControllerValue", ex.what());
        return false;
    }
}

bool CarControlHandler::getModeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    std::string& value) {
    try_with_context {
        jstring result;
        ThrowIfNot(
            m_obj.invoke(
                "getModeControllerValue",
                "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                &result,
                JString(controlId).get(),
                JString(controllerId).get()),
            "invokeMethodFailed");
        if (env->ExceptionCheck() == JNI_TRUE) {
            env->ExceptionClear();
            return false;
        }
        value = JString(result).toStdStr();
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getModeControllerValue", ex.what());
        return false;
    }
}

}  // namespace carControl
}  // namespace jni
}  // namespace aace

#define CAR_CONTROL_BINDER(ref) reinterpret_cast<aace::jni::carControl::CarControlBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_carControl_CarControl_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::carControl::CarControlBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControl_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_carControl_CarControl_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto carControlBinder = CAR_CONTROL_BINDER(ref);
        ThrowIfNull(carControlBinder, "invalidCarControlBinder");
        delete carControlBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_carControl_CarControl_disposeBinder", ex.what());
    }
}
}
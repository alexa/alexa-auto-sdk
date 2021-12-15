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

#include <AACE/JNI/Navigation/NavigationBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.navigation.NavigationBinder";

namespace aace {
namespace jni {
namespace navigation {

//
// NavigationBinder
//

NavigationBinder::NavigationBinder(jobject obj) {
    m_navigationHandler = std::shared_ptr<NavigationHandler>(new NavigationHandler(obj));
}

//
// NavigationHandler
//

NavigationHandler::NavigationHandler(jobject obj) : m_obj(obj, "com/amazon/aace/navigation/Navigation") {
}

bool NavigationHandler::cancelNavigation() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("cancelNavigation", "()Z", &result), "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "cancelNavigation", ex.what());
        return false;
    }
}

std::string NavigationHandler::getNavigationState() {
    try_with_context {
        jstring result;
        ThrowIfNot(m_obj.invoke("getNavigationState", "()Ljava/lang/String;", &result), "invokeFailed");
        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getNavigationState", ex.what());
        return std::string("");
    }
}

void NavigationHandler::startNavigation(const std::string& payload) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("startNavigation", "(Ljava/lang/String;)V", nullptr, JString(payload).get()),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "startNavigation", ex.what());
    }
}

void NavigationHandler::showPreviousWaypoints() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("showPreviousWaypoints", "()V", nullptr), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "showPreviousWaypoints", ex.what());
    }
}

void NavigationHandler::navigateToPreviousWaypoint() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("navigateToPreviousWaypoint", "()V", nullptr), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "navigateToPreviousWaypoint", ex.what());
    }
}

void NavigationHandler::showAlternativeRoutes(AlternateRouteType routeType) {
    try_with_context {
        jobject routeTypeObj;
        ThrowIfNot(JAlternateRouteType::checkType(routeType, &routeTypeObj), "invalidAlternateRouteType");

        ThrowIfNot(
            m_obj.invoke<void>(
                "showAlternativeRoutes",
                "(Lcom/amazon/aace/navigation/Navigation$AlternateRouteType;)V",
                nullptr,
                routeTypeObj),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "showAlternativeRoutes", ex.what());
    }
}

void NavigationHandler::controlDisplay(ControlDisplay controlDisplay) {
    try_with_context {
        jobject controlDisplayObj;
        ThrowIfNot(JControlDisplay::checkType(controlDisplay, &controlDisplayObj), "invalidControlDisplay");

        ThrowIfNot(
            m_obj.invoke<void>(
                "controlDisplay",
                "(Lcom/amazon/aace/navigation/Navigation$ControlDisplay;)V",
                nullptr,
                controlDisplayObj),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "controlDisplay", ex.what());
    }
}

void NavigationHandler::announceManeuver(const std::string& payload) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>("announceManeuver", "(Ljava/lang/String;)V", nullptr, JString(payload).get()),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "announceManeuver", ex.what());
    }
}

void NavigationHandler::announceRoadRegulation(RoadRegulation roadRegulation) {
    try_with_context {
        jobject roadRegulationObj;
        ThrowIfNot(JRoadRegulation::checkType(roadRegulation, &roadRegulationObj), "invalidRoadRegulation");

        ThrowIfNot(
            m_obj.invoke<void>(
                "announceRoadRegulation",
                "(Lcom/amazon/aace/navigation/Navigation$RoadRegulation;)V",
                nullptr,
                roadRegulationObj),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "announceRoadRegulation", ex.what());
    }
}

}  // namespace navigation
}  // namespace jni
}  // namespace aace

#define NAVIGATION_BINDER(ref) reinterpret_cast<aace::jni::navigation::NavigationBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_navigation_Navigation_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::navigation::NavigationBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_navigation_Navigation_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto navigationBinder = NAVIGATION_BINDER(ref);
        ThrowIfNull(navigationBinder, "invalidNavigationBinder");
        delete navigationBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_navigation_Navigation_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_navigation_Navigation_navigationEvent(JNIEnv* env, jobject, jlong ref, jobject event) {
    try {
        auto navigationBinder = NAVIGATION_BINDER(ref);
        ThrowIfNull(navigationBinder, "invalidNavigationBinder");

        aace::navigation::NavigationEngineInterface::EventName eventName;
        ThrowIfNot(aace::jni::navigation::JEventName::checkType(event, &eventName), "invalidEventName");

        navigationBinder->getNavigation()->navigationEvent(eventName);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_navigation_Navigation_navigationEvent", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_navigation_Navigation_navigationError(
    JNIEnv* env,
    jobject,
    jlong ref,
    jobject type,
    jobject code,
    jstring description) {
    try {
        auto navigationBinder = NAVIGATION_BINDER(ref);
        ThrowIfNull(navigationBinder, "invalidNavigationBinder");

        aace::navigation::NavigationEngineInterface::ErrorType errorType;
        ThrowIfNot(aace::jni::navigation::JErrorType::checkType(type, &errorType), "invalidErrorType");

        aace::navigation::NavigationEngineInterface::ErrorCode errorCode;
        ThrowIfNot(aace::jni::navigation::JErrorCode::checkType(code, &errorCode), "invalidErrorCode");

        navigationBinder->getNavigation()->navigationError(errorType, errorCode, JString(description).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_navigation_Navigation_navigationError", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_navigation_Navigation_showAlternativeRoutesSucceeded(
    JNIEnv* env,
    jobject,
    jlong ref,
    jstring payload) {
    try {
        auto navigationBinder = NAVIGATION_BINDER(ref);
        ThrowIfNull(navigationBinder, "invalidNavigationBinder");

        navigationBinder->getNavigation()->showAlternativeRoutesSucceeded(JString(payload).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_navigation_Navigation_showAlternativeRoutesSucceeded", ex.what());
    }
}
}

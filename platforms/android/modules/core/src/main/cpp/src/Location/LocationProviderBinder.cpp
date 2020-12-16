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

#include <AACE/JNI/Location/LocationProviderBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.LocationProviderBinder";

namespace aace {
namespace jni {
namespace location {

//
// LocationProviderBinder
//

LocationProviderBinder::LocationProviderBinder(jobject obj) {
    m_locationProviderHandler = std::make_shared<LocationProviderHandler>(obj);
}

//
// LocationProviderHandler
//

LocationProviderHandler::LocationProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/location/LocationProvider") {
}

aace::location::Location LocationProviderHandler::getLocation() {
    try_with_context {
        jobject result = nullptr;

        ThrowIfNot(m_obj.invoke("getLocation", "()Lcom/amazon/aace/location/Location;", &result), "invokeMethodFailed");
        ThrowIfNull(result, "invalidLocation");

        return JLocation(result).getLocation();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getLocation", ex.what());
        return aace::location::Location(aace::location::Location::UNDEFINED, aace::location::Location::UNDEFINED);
    }
}

std::string LocationProviderHandler::getCountry() {
    try_with_context {
        jstring result;

        ThrowIfNot(m_obj.invoke("getCountry", "()Ljava/lang/String;", &result), "invokeMethodFailed");

        return JString(result).toStdStr();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getCountry", ex.what());
        return std::string();
    }
}

//
// JLocation
//
aace::location::Location JLocation::getLocation() {
    try_with_context {
        jdouble jundefined;
        ThrowIfNot(getStatic("UNDEFINED", &jundefined), "getStaticFailed");

        jdouble latitude;
        ThrowIfNot(invoke("getLatitude", "()D", &latitude), "invokeMethodFailed");
        if (latitude == jundefined) {
            latitude = aace::location::Location::UNDEFINED;
        }

        jdouble longitude;
        ThrowIfNot(invoke("getLongitude", "()D", &longitude), "invokeMethodFailed");
        if (longitude == jundefined) {
            longitude = aace::location::Location::UNDEFINED;
        }

        jdouble altitude;
        ThrowIfNot(invoke("getAltitude", "()D", &altitude), "invokeMethodFailed");
        if (altitude == jundefined) {
            altitude = aace::location::Location::UNDEFINED;
        }

        jdouble accuracy;
        ThrowIfNot(invoke("getAccuracy", "()D", &accuracy), "invokeMethodFailed");
        if (accuracy == jundefined) {
            accuracy = aace::location::Location::UNDEFINED;
        }

        return aace::location::Location(latitude, longitude, altitude, accuracy);
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.alexa.LocationProviderBinder:JLocation", "getLocation", ex.what());
        return aace::location::Location(aace::location::Location::UNDEFINED, aace::location::Location::UNDEFINED);
    }
}

}  // namespace location
}  // namespace jni
}  // namespace aace

#define LOCATION_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::location::LocationProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_location_LocationProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::location::LocationProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_location_LocationProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto locationProviderBinder = LOCATION_PROVIDER_BINDER(ref);
        ThrowIfNull(locationProviderBinder, "invalidLocationProviderBinder");
        delete locationProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_location_LocationProvider_disposeBinder", ex.what());
    }
}
}

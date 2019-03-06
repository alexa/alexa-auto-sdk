/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/location/LocationBinder.cpp
// This is an automatically generated file.

#include "aace/location/LocationBinder.h"
#include "NativeLib.h"
#include "android_debug.h"

LocationBinder::LocationBinder( double latitude, double longitude, double altitude, double accuracy, std::chrono::system_clock::time_point time ) : aace::location::Location( latitude, longitude, altitude, accuracy, time )
{
}

LocationBinder::LocationBinder( double latitude, double longitude, double accuracy, std::chrono::system_clock::time_point time ) : aace::location::Location( latitude, longitude, accuracy, time )
{
}

LocationBinder::LocationBinder( double latitude, double longitude, std::chrono::system_clock::time_point time ) : aace::location::Location( latitude, longitude, time )
{
}

LocationBinder::LocationBinder( const aace::location::Location & location ) : aace::location::Location( location )
{
}

long LocationBinder::bind( JNIEnv* env, jobject javaObj )
{
    jclass javaObjectClass = env->GetObjectClass( javaObj );
    m_javaObj = env->NewGlobalRef( javaObj );

    return (long) this;
}

// JNI
#define LOCATION(cptr) ((LocationBinder *) cptr)

extern "C" {

JNIEXPORT jdouble JNICALL
Java_com_amazon_aace_location_Location_getLatitude( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return LOCATION(cptr)->getLatitude();
}

JNIEXPORT jdouble JNICALL
Java_com_amazon_aace_location_Location_getLongitude( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return LOCATION(cptr)->getLongitude();
}

JNIEXPORT jdouble JNICALL
Java_com_amazon_aace_location_Location_getAltitude( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return LOCATION(cptr)->getAltitude();
}

JNIEXPORT jdouble JNICALL
Java_com_amazon_aace_location_Location_getAccuracy( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return LOCATION(cptr)->getAccuracy();
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_location_Location_getTime( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return (jlong) static_cast<jlong>( std::chrono::duration_cast<std::chrono::nanoseconds>(LOCATION(cptr)->getTime().time_since_epoch()).count() ); //@ LOCATION(cptr)->getTime()
}

JNIEXPORT jstring JNICALL
Java_com_amazon_aace_location_Location_getTimeAsString( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return env->NewStringUTF( LOCATION(cptr)->getTimeAsString().c_str() ); //@ TODO LOCATION(cptr)->getTimeAsString();
}

}

// END OF FILE

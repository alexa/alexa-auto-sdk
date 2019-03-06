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

// aace/location/LocationProviderBinder.cpp
// This is an automatically generated file.

#include "aace/location/LocationProviderBinder.h"

void LocationProviderBinder::initialize( JNIEnv* env )
{
    m_javaMethod_getLocation = env->GetMethodID( getJavaClass(), "getLocation", "()Lcom/amazon/aace/location/Location;" );
    m_javaMethod_getCountry = env->GetMethodID( getJavaClass(), "getCountry", "()Ljava/lang/String;" );

    // Location class
    m_javaClass_Location = NativeLib::FindClass( env, "com/amazon/aace/location/Location" );
    m_javaMethod_getLatitude = env->GetMethodID( m_javaClass_Location.get(), "getLatitude", "()D" );
    m_javaMethod_getLongitude = env->GetMethodID( m_javaClass_Location.get(), "getLongitude", "()D" );
    m_javaMethod_getAltitude = env->GetMethodID( m_javaClass_Location.get(), "getAltitude", "()D" );
    m_javaMethod_getAccuracy = env->GetMethodID( m_javaClass_Location.get(), "getAccuracy", "()D" );
}

aace::location::Location LocationProviderBinder::getLocation()
{
    if( getJavaObject() != nullptr && m_javaMethod_getLocation != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jobject javaObj_location = context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getLocation );

            if( javaObj_location != nullptr && context.getEnv()->IsInstanceOf( javaObj_location, m_javaClass_Location.get() ) )
            {
                jdouble latitude = context.getEnv()->CallDoubleMethod( javaObj_location, m_javaMethod_getLatitude );
                jdouble longitude = context.getEnv()->CallDoubleMethod( javaObj_location, m_javaMethod_getLongitude );
                jdouble altitude = context.getEnv()->CallDoubleMethod( javaObj_location, m_javaMethod_getAltitude );
                jdouble accuracy = context.getEnv()->CallDoubleMethod( javaObj_location, m_javaMethod_getAccuracy );

                return aace::location::Location( latitude, longitude, altitude, accuracy );
            }
        }
    }

    return aace::location::Location( aace::location::Location::UNDEFINED, aace::location::Location::UNDEFINED );
}

std::string LocationProviderBinder::getCountry()
{
    if( getJavaObject() != nullptr && m_javaMethod_getCountry != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            return NativeLib::convert( context.getEnv(), (jstring) context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getCountry ) );
        }
    }

    return "";
}

// JNI
#define LOCATIONPROVIDER(cptr) ((LocationProviderBinder *) cptr)

extern "C" {

}

// END OF FILE

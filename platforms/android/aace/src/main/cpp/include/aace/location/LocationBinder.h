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

// aace/location/LocationBinder.h
// This is an automatically generated file.

#ifndef AACE_LOCATION_LOCATION_BINDER_H
#define AACE_LOCATION_LOCATION_BINDER_H

#include <jni.h>

#include "AACE/Location/LocationProvider.h"

class LocationBinder : public aace::location::Location {
private:
    LocationBinder( double latitude, double longitude, double altitude, double accuracy, std::chrono::system_clock::time_point time );
    LocationBinder( double latitude, double longitude, double accuracy, std::chrono::system_clock::time_point time );
    LocationBinder( double latitude, double longitude, std::chrono::system_clock::time_point time );
    LocationBinder( const aace::location::Location & location );

public:
    long bind( JNIEnv* env, jobject javaObj );

private:
    jobject m_javaObj = nullptr;

};

#endif //AACE_LOCATION_LOCATION_BINDER_H

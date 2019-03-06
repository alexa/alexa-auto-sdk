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

// aace/location/LocationProviderBinder.h
// This is an automatically generated file.

#ifndef AACE_LOCATION_LOCATION_PROVIDER_BINDER_H
#define AACE_LOCATION_LOCATION_PROVIDER_BINDER_H

#include "AACE/Location/LocationProvider.h"
#include "aace/core/PlatformInterfaceBinder.h"

class LocationProviderBinder : public PlatformInterfaceBinder, public aace::location::LocationProvider {
public:
    LocationProviderBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    aace::location::Location getLocation() override;
    std::string getCountry() override;

private:
    ClassRef m_javaClass_Location;

    jmethodID m_javaMethod_getLocation = nullptr;
    jmethodID m_javaMethod_getCountry = nullptr;

    jmethodID m_javaMethod_getLatitude = nullptr;
    jmethodID m_javaMethod_getLongitude = nullptr;
    jmethodID m_javaMethod_getAltitude = nullptr;
    jmethodID m_javaMethod_getAccuracy = nullptr;
};

#endif //AACE_LOCATION_LOCATION_PROVIDER_BINDER_H

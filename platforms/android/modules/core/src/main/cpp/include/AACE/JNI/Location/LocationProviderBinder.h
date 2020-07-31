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

#ifndef AACE_JNI_LOCATION_LOCATION_PROVIDER_BINDER_H
#define AACE_JNI_LOCATION_LOCATION_PROVIDER_BINDER_H

#include <AACE/Location/LocationProvider.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace location {

//
// LocationProviderHandler
//

class LocationProviderHandler : public aace::location::LocationProvider {
public:
    LocationProviderHandler(jobject obj);

public:
    aace::location::Location getLocation() override;
    std::string getCountry() override;

private:
    JObject m_obj;
};

//
// LocationProviderBinder
//

class LocationProviderBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    LocationProviderBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_locationProviderHandler;
    }

private:
    std::shared_ptr<LocationProviderHandler> m_locationProviderHandler;
};

//
// JLocation
//

class JLocation : public JObject {
public:
    JLocation(jobject obj) : JObject(obj, "com/amazon/aace/location/Location") {
    }
    aace::location::Location getLocation();
};

}  // namespace location
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_LOCATION_LOCATION_PROVIDER_BINDER_H

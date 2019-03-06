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

// aace/navigation/NavigationBinder.h
// This is an automatically generated file.

#ifndef AACE_NAVIGATION_NAVIGATION_BINDER_H
#define AACE_NAVIGATION_NAVIGATION_BINDER_H

#include "AACE/Navigation/Navigation.h"
#include "aace/core/PlatformInterfaceBinder.h"

class NavigationBinder : public PlatformInterfaceBinder, public aace::navigation::Navigation {
public:
    NavigationBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool setDestination( const std::string & payload ) override;
    bool cancelNavigation() override;

private:
    jmethodID m_javaMethod_setDestination_payload = nullptr;
    jmethodID m_javaMethod_cancelNavigation = nullptr;

};

#endif //AACE_NAVIGATION_NAVIGATION_BINDER_H

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

#ifndef AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H
#define AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H

#include <AACE/Navigation/Navigation.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace navigation {

    //
    // NavigationHandler
    //

    class NavigationHandler : public aace::navigation::Navigation {
    public:
        NavigationHandler( jobject obj );

        // aace::navigation::Navigation
        bool setDestination( const std::string & payload ) override;
        bool cancelNavigation() override;
        std::string getNavigationState() override;

    private:
        JObject m_obj;
    };

    //
    // NavigationBinder
    //

    class NavigationBinder : public aace::jni::core::PlatformInterfaceBinder {
    public:
        NavigationBinder( jobject obj );

        std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
            return m_navigationHandler;
        }

    private:
        std::shared_ptr<NavigationHandler> m_navigationHandler;
    };

} // aace::jni::navigation
} // aace::jni
} // aace

#endif // AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H

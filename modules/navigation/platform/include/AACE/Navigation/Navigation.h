/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_NAVIGATION_NAVIGATION_H
#define AACE_NAVIGATION_NAVIGATION_H

#include <string>

#include "AACE/Core/PlatformInterface.h"

/** @file */

namespace aace {
namespace navigation {

/**
 * Navigation should be extended to handle navigation directives from the Engine.
 */
class Navigation : public aace::core::PlatformInterface {
protected:
    Navigation() = default;

public:
    virtual ~Navigation();

    /**
     * Notifies the platform implementation to set the navigation destination
     *
     * @param [in] payload JSON data containing the destination information
     * @code{.json})
     * "destination": {
     *    "coordinate": {
     *        "latitudeInDegrees": {{DOUBLE}},
     *        "longitudeInDegrees": {{DOUBLE}}
     *    },
     *    "name": "{{STRING}}",
     *    "singleLineDisplayAddress": "{{STRING}}"
     *    "multipleLineDisplayAddress": "{{STRING}}",
     * }
     * @endcode
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool setDestination( const std::string& payload ) = 0;

    /**
     * Notifies the platform implementation to cancel navigation
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool cancelNavigation() = 0;
};

} // aace::navigation
} // aace

#endif // AACE_NAVIGATION_NAVIGATION_H

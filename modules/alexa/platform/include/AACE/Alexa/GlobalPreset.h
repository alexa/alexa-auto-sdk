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

#ifndef AACE_ALEXA_GLOBAL_PRESET_H
#define AACE_ALEXA_GLOBAL_PRESET_H

#include <chrono>
#include <string>

#include "AACE/Core/PlatformInterface.h"

/** @file */

namespace aace {
namespace alexa {

class GlobalPreset : public aace::core::PlatformInterface {
public:
protected:
    GlobalPreset() = default;

public:
    virtual ~GlobalPreset();

    /**
     * Called after receiving a global preset play directive
     *
     * @param [in] The preset integer from the playbackContextToken
     */
    virtual void setGlobalPreset(int preset) = 0;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_GLOBAL_PRESET_H

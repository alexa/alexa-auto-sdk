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

#include <AACE/Engine/Core/EngineMacros.h>

#include "Audio.h"

namespace agl {
namespace audio {

// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

using namespace agl::common::interfaces;

static std::string TAG = "agl::audio::Audio";

std::shared_ptr<Audio> Audio::create(
    std::shared_ptr<agl::common::interfaces::ILogger> logger,
    shared_ptr<agl::common::interfaces::IAFBApi> api) {
    return std::shared_ptr<Audio>(new Audio(logger, api));
}

Audio::Audio(std::shared_ptr<ILogger> logger, std::shared_ptr<IAFBApi> api) : m_logger(logger), m_api(api) {
}

std::string Audio::openChannel(const std::string& role) {
    // For now, return the given role as the device string, to match
    // the expectation of the PipeWire sink configuration in the
    // gstreamer output code.
    return role;
}

bool Audio::setChannelVolume(const std::string& role, int volume) {
    return true;
}

}  // namespace audio
}  // namespace agl
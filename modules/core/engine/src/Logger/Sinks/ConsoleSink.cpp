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

#include <iostream>

#include "AACE/Engine/Logger/Sinks/ConsoleSink.h"
#include "AACE/Engine/Logger/LogFormatter.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.sink.ConsoleSink");

ConsoleSink::ConsoleSink(const std::string& id) : Sink(id) {
}

std::shared_ptr<ConsoleSink> ConsoleSink::create(const std::string& id) {
    return std::shared_ptr<ConsoleSink>(new ConsoleSink(id));
}

void ConsoleSink::log(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    std::cout << aace::engine::logger::LogFormatter::format(level, time, threadMoniker, text) << std::endl;
}

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace

/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <stdio.h>
#include <unistd.h>

#include "AACE/Engine/Logger/Sinks/ConsoleSink.h"
#include "AACE/Engine/Logger/LogFormatter.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

ConsoleSink::ConsoleSink(const std::string& id) : Sink(id) {
#ifdef AAC_EMIT_COLOR_LOGS
    if (isatty(fileno(stdout))) {
        m_formatter = aace::engine::logger::LogFormatter::createColor();
    } else {
        m_formatter = aace::engine::logger::LogFormatter::createPlainText();
    }
#else
    m_formatter = aace::engine::logger::LogFormatter::createPlainText();
#endif
}

std::shared_ptr<ConsoleSink> ConsoleSink::create(const std::string& id) {
    return std::shared_ptr<ConsoleSink>(new ConsoleSink(id));
}

void ConsoleSink::log(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* source,
    const char* threadMoniker,
    const char* text) {
    std::cout << m_formatter->format(level, time, source, threadMoniker, text) << std::endl;
}

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace

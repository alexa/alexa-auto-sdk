/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef NO_SYSLOG
#include <syslog.h>
#endif
#include <iostream>

#include <AACE/Engine/Logger/Sinks/SyslogSink.h>
#include <AACE/Engine/Logger/LogFormatter.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace logger {
namespace sink {

SyslogSink::SyslogSink(const std::string& id) : Sink(id) {
#ifndef NO_SYSLOG
    openlog(nullptr, 0, LOG_USER);
    setlogmask(LOG_UPTO(LOG_DEBUG));
    m_formatter = aace::engine::logger::LogFormatter::createPlainText();
#else
    AACE_WARN(LX(TAG).m("Syslog support is not enabled!"));
#endif
}

SyslogSink::~SyslogSink() {
#ifndef NO_SYSLOG
    closelog();
#endif
}

std::shared_ptr<SyslogSink> SyslogSink::create(const std::string& id) {
    return std::shared_ptr<SyslogSink>(new SyslogSink(id));
}

void SyslogSink::log(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* source,
    const char* threadMoniker,
    const char* text) {
#ifndef NO_SYSLOG
    int syslogLevel = LOG_ERR;

    switch (level) {
        case Level::VERBOSE:
            syslogLevel = LOG_DEBUG;
            break;

        case Level::INFO:
            syslogLevel = LOG_INFO;
            break;

        case Level::WARN:
            syslogLevel = LOG_WARNING;
            break;

        case Level::ERROR:
            syslogLevel = LOG_ERR;
            break;

        case Level::CRITICAL:
            syslogLevel = LOG_CRIT;
            break;

        case Level::METRIC:
            syslogLevel = LOG_INFO;
            break;

        default:
            syslogLevel = LOG_ERR;
            AACE_NOT_REACHED;
    }

    syslog(
        syslogLevel,
        "%s",
        m_formatter->format(level, std::chrono::system_clock::time_point(), source, threadMoniker, text).c_str());
#endif
}

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace

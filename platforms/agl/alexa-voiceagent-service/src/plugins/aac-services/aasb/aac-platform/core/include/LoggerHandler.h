/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_CORE_LOGGER_LOGGERHANDLER_H
#define AASB_CORE_LOGGER_LOGGERHANDLER_H

#include <AACE/Logger/Logger.h>

namespace aasb {
namespace core {
namespace logger {

class LoggerHandler : public aace::logger::Logger {
public:
    static std::shared_ptr<LoggerHandler> create();

    /// @name aace::logger::Logger Functions
    /// @{
    bool logEvent(
        Level level,
        std::chrono::system_clock::time_point time,
        const std::string& source,
        const std::string& message) override;
    /// @}

    void log(Level level, const std::string& tag, const std::string& message);

private:
    LoggerHandler() = default;
};

}  // namespace logger
}  // namespace core
}  // namespace aasb

#endif  // AASB_CORE_LOGGER_LOGGERHANDLER_H
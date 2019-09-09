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
#include "utilities/logging/Logger.h"

namespace agl {
namespace utilities {
namespace logging {

// Constructor
Logger::Logger(afb_api_t api) {
    mApi = api;
}

unique_ptr<Logger> Logger::create(afb_api_t api) {
    auto logger = std::unique_ptr<Logger>(new Logger(api));
    return logger;
}

void Logger::log(Level level, const std::string& tag, const std::string& message) {
    string format_msg = "Tag: " + tag + ", message: " + message;
    switch (level) {
        case Level::NOTICE:
            AFB_API_NOTICE(mApi, format_msg.c_str());
            break;
        case Level::WARNING:
            AFB_API_WARNING(mApi, format_msg.c_str());
            break;
        case Level::DEBUG:
            AFB_API_DEBUG(mApi, format_msg.c_str());
            break;
        case Level::ERROR:
            AFB_API_ERROR(mApi, format_msg.c_str());
            break;
        case Level::INFO:
            AFB_API_INFO(mApi, format_msg.c_str());
            break;
        default:
            break;
    }
}

}  // namespace logging
}  // namespace utilities
}  // namespace agl

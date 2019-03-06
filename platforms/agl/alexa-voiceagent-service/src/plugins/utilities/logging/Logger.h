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
#ifndef AGL_UTILITIES_LOGGING_LOGGER_H_
#define AGL_UTILITIES_LOGGING_LOGGER_H_

#include <memory>

extern "C" {
#define AFB_BINDING_VERSION 3
#include "afb-definitions.h"
#include "ctl-plugin.h"
};

#include "interfaces/utilities/logging/ILogger.h"

using namespace std;

namespace agl {
namespace utilities {
namespace logging {

class Logger : public agl::common::interfaces::ILogger {
public:
    static std::unique_ptr<Logger> create(AFB_ApiT api);

    // ILogger interface
    void log(Level level, const std::string& tag, const std::string& message) override;

private:
    Logger(AFB_ApiT api);

    // Binding API reference
    AFB_ApiT mApi;
};

}  // namespace logging
}  // namespace utilities
}  // namespace agl

#endif  // AGL_UTILITIES_LOGGING_LOGGER_H_

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

#ifndef AACE_ENGINE_LOGGER_LOGGER_SERVICE_INTERFACE_H
#define AACE_ENGINE_LOGGER_LOGGER_SERVICE_INTERFACE_H

#include <memory>

#include "Sinks/Sink.h"

namespace aace {
namespace engine {
namespace logger {

class LoggerServiceInterface {
public:
    virtual ~LoggerServiceInterface();

public:
    virtual bool addSink(std::shared_ptr<aace::engine::logger::sink::Sink> sink) = 0;
    virtual bool removeSink(const std::string& id) = 0;
};

}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_LOGGER_SERVICE_INTERFACE_H

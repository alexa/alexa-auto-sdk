/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H
#define AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H

#include "AACE/Engine/Core/EngineServiceManager.h"
#include "AACE/Engine/Logger/EngineLogger.h"

// exceptions
#define Throw(reason) throw std::runtime_error(reason);
#define ThrowIf(arg,reason) if(arg){throw std::runtime_error(reason);}
#define ThrowIfNot(arg,reason) if(!(arg)){throw std::runtime_error(reason);}
#define ThrowIfNull(arg,reason) if((arg)==nullptr){throw std::runtime_error(reason);}
#define ThrowIfNotNull(arg,reason) if((arg)!=nullptr){throw std::runtime_error(reason);}
#define ReturnIf(arg,result) if(arg){return(result);}
#define ReturnIfNot(arg, result) if(!(arg)){return(result);}

// logging
#define AACE_LOGGER (aace::engine::logger::EngineLogger::getInstance())
#define AACE_LOG_LEVEL aace::engine::logger::EngineLogger::Level
#define AACE_LOG(level, entry)                                   \
    do {                                                         \
        AACE_LOGGER->log(level, entry);                          \
    } while (false)

#ifdef AACE_DEBUG_LOG_ENABLED
#define AACE_DEBUG(entry) AACE_LOG(AACE_LOG_LEVEL::VERBOSE, entry)
#define AACE_VERBOSE(entry) AACE_LOG(AACE_LOG_LEVEL::VERBOSE, entry)
#else // AACE_DEBUG_LOG_ENABLED
#define AACE_DEBUG(entry)
#define AACE_VERBOSE(entry)
#endif // AACE_DEBUG_LOG_ENABLED

#ifdef AAC_LATENCY_LOGS_ENABLED
#define AACE_METRIC(entry) AACE_LOG(AACE_LOG_LEVEL::METRIC, entry)
#else // AAC_LATENCY_LOGS_ENABLED
#define AACE_METRIC(entry)
#endif // AAC_LATENCY_LOGS_ENABLED

#define AACE_INFO(entry) AACE_LOG(AACE_LOG_LEVEL::INFO, entry)
#define AACE_WARN(entry) AACE_LOG(AACE_LOG_LEVEL::WARN, entry)
#define AACE_ERROR(entry) AACE_LOG(AACE_LOG_LEVEL::ERROR, entry)
#define AACE_CRITICAL(entry) AACE_LOG(AACE_LOG_LEVEL::CRITICAL, entry)
// creates a log event for the aace logger
#define LX(tag, event) aace::engine::logger::LogEntry(tag, event)

#endif // AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H

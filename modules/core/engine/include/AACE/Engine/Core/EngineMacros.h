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

#ifndef AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H
#define AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H

#include "AACE/Engine/Core/EngineServiceManager.h"
#include "AACE/Engine/Logger/EngineLogger.h"

#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)
#define VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N
#define macro_dispatcher(func, ...) macro_dispatcher_(func, VA_NUM_ARGS(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) func##nargs

// exceptions
#define Throw(reason) throw std::runtime_error(reason)
#define ThrowIf(arg, reason)                  \
    do {                                      \
        if (arg) {                            \
            throw std::runtime_error(reason); \
        }                                     \
    } while (false)
#define ThrowIfNot(arg, reason)               \
    do {                                      \
        if (!(arg)) {                         \
            throw std::runtime_error(reason); \
        }                                     \
    } while (false)
#define ThrowIfNull(arg, reason)              \
    do {                                      \
        if ((arg) == nullptr) {               \
            throw std::runtime_error(reason); \
        }                                     \
    } while (false)
#define ThrowIfNotNull(arg, reason)           \
    do {                                      \
        if ((arg) != nullptr) {               \
            throw std::runtime_error(reason); \
        }                                     \
    } while (false)

#define ReturnIf(...) macro_dispatcher(ReturnIf, __VA_ARGS__)(__VA_ARGS__)
#define ReturnIf1(arg) \
    do {               \
        if (arg) {     \
            return;    \
        }              \
    } while (false)
#define ReturnIf2(arg, result) \
    do {                       \
        if (arg) {             \
            return (result);   \
        }                      \
    } while (false)

#define ReturnIfNot(...) macro_dispatcher(ReturnIfNot, __VA_ARGS__)(__VA_ARGS__)
#define ReturnIfNot1(arg) \
    do {                  \
        if (!(arg)) {     \
            return;       \
        }                 \
    } while (false)
#define ReturnIfNot2(arg, result) \
    do {                          \
        if (!(arg)) {             \
            return (result);      \
        }                         \
    } while (false)

// logging
#define AACE_LOGGER (aace::engine::logger::EngineLogger::getInstance())
#define AACE_LOG_LEVEL aace::engine::logger::EngineLogger::Level
#define AACE_LOG(level, entry)          \
    do {                                \
        AACE_LOGGER->log(level, entry); \
    } while (false)

#ifdef AACE_DEBUG_LOG_ENABLED
#define AACE_DEBUG(entry) AACE_LOG(AACE_LOG_LEVEL::VERBOSE, entry)
#define AACE_VERBOSE(entry) AACE_LOG(AACE_LOG_LEVEL::VERBOSE, entry)
#else  // AACE_DEBUG_LOG_ENABLED
#define AACE_DEBUG(entry)
#define AACE_VERBOSE(entry)
#endif  // AACE_DEBUG_LOG_ENABLED

#ifdef AAC_LATENCY_LOGS_ENABLED
#define AACE_METRIC(entry) AACE_LOG(AACE_LOG_LEVEL::METRIC, entry)
#else  // AAC_LATENCY_LOGS_ENABLED
#define AACE_METRIC(entry)
#endif  // AAC_LATENCY_LOGS_ENABLED

#define AACE_INFO(entry) AACE_LOG(AACE_LOG_LEVEL::INFO, entry)
#define AACE_WARN(entry) AACE_LOG(AACE_LOG_LEVEL::WARN, entry)
#define AACE_ERROR(entry) AACE_LOG(AACE_LOG_LEVEL::ERROR, entry)
#define AACE_CRITICAL(entry) AACE_LOG(AACE_LOG_LEVEL::CRITICAL, entry)

// creates a log event for the aace logger
#define LX(...) macro_dispatcher(LX, __VA_ARGS__)(__VA_ARGS__)
#define LX1(tag) aace::engine::logger::LogEntry(tag, __func__)
#define LX2(tag, event) aace::engine::logger::LogEntry(tag, event)

#define DX aace::engine::logger::LogEntry(__FILE__, __func__)

#endif  // AACE_ENGINE_CORE_ENGINE_EXCEPTIONS_H

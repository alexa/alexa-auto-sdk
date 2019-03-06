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

// aace/logger/LoggerBinder.h
// This is an automatically generated file.

#ifndef AACE_LOGGER_LOGGER_BINDER_H
#define AACE_LOGGER_LOGGER_BINDER_H

#include "AACE/Logger/Logger.h"
#include "aace/core/PlatformInterfaceBinder.h"

class LoggerBinder : public PlatformInterfaceBinder, public aace::logger::Logger {
public:
    LoggerBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool logEvent( aace::logger::Logger::Level level, std::chrono::system_clock::time_point time, const std::string &source, const std::string &message ) override;

public:
    aace::logger::LoggerEngineInterface::Level convertLevel( JNIEnv* env, jobject obj );

private:
    jobject convert( aace::logger::LoggerEngineInterface::Level level );

private:
    jmethodID m_javaMethod_logEvent_level_time_module_message = nullptr;

    // Level
    ObjectRef m_enum_Level_VERBOSE;
    ObjectRef m_enum_Level_INFO;
    ObjectRef m_enum_Level_METRIC;
    ObjectRef m_enum_Level_WARN;
    ObjectRef m_enum_Level_ERROR;
    ObjectRef m_enum_Level_CRITICAL;
};

#endif //AACE_LOGGER_LOGGER_BINDER_H

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

// aace/logger/LoggerBinder.cpp
// This is an automatically generated file.

#include "aace/logger/LoggerBinder.h"

void LoggerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_logEvent_level_time_module_message = env->GetMethodID( getJavaClass(), "logEvent", "(Lcom/amazon/aace/logger/Logger$Level;JLjava/lang/String;[B)Z" );

    // Level
    jclass levelEnumClass = env->FindClass( "com/amazon/aace/logger/Logger$Level" );
    m_enum_Level_VERBOSE = NativeLib::FindEnum( env, levelEnumClass, "VERBOSE", "Lcom/amazon/aace/logger/Logger$Level;" );
    m_enum_Level_INFO = NativeLib::FindEnum( env, levelEnumClass, "INFO", "Lcom/amazon/aace/logger/Logger$Level;" );
    m_enum_Level_METRIC =  NativeLib::FindEnum( env, levelEnumClass, "METRIC", "Lcom/amazon/aace/logger/Logger$Level;" );
    m_enum_Level_WARN = NativeLib::FindEnum( env, levelEnumClass, "WARN", "Lcom/amazon/aace/logger/Logger$Level;" );
    m_enum_Level_ERROR = NativeLib::FindEnum( env, levelEnumClass, "ERROR", "Lcom/amazon/aace/logger/Logger$Level;" );
    m_enum_Level_CRITICAL = NativeLib::FindEnum( env, levelEnumClass, "CRITICAL", "Lcom/amazon/aace/logger/Logger$Level;" );
}

bool LoggerBinder::logEvent( aace::logger::Logger::Level level, std::chrono::system_clock::time_point time, const std::string& source, const std::string& message )
{
    bool result = false;

    if( getJavaObject() != nullptr && m_javaMethod_logEvent_level_time_module_message != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring sourceStr = context.getEnv()->NewStringUTF( source.c_str() );
            jbyteArray messageArray = context.getEnv()->NewByteArray(message.size());
            context.getEnv()->SetByteArrayRegion(messageArray, 0, message.size(), (const jbyte*)message.c_str());
            
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_logEvent_level_time_module_message, convert( level ), std::chrono::duration_cast<std::chrono::milliseconds>( time.time_since_epoch() ).count(), sourceStr, messageArray );

            context.getEnv()->DeleteLocalRef( sourceStr );
            context.getEnv()->DeleteLocalRef( messageArray );
        }
    }
    return result;
}

jobject LoggerBinder::convert( aace::logger::LoggerEngineInterface::Level level )
{
    switch( level )
    {
        case aace::logger::LoggerEngineInterface::Level::VERBOSE:
            return m_enum_Level_VERBOSE.get();
        case aace::logger::LoggerEngineInterface::Level::INFO:
            return m_enum_Level_INFO.get();
        case aace::logger::LoggerEngineInterface::Level::METRIC:
            return m_enum_Level_METRIC.get();
        case aace::logger::LoggerEngineInterface::Level::WARN:
            return m_enum_Level_WARN.get();
        case aace::logger::LoggerEngineInterface::Level::ERROR:
            return m_enum_Level_ERROR.get();
        case aace::logger::LoggerEngineInterface::Level::CRITICAL:
            return m_enum_Level_CRITICAL.get();
    }
}

aace::logger::LoggerEngineInterface::Level LoggerBinder::convertLevel( JNIEnv* env, jobject obj )
{
    if( m_enum_Level_VERBOSE.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::VERBOSE;
    }
    else if( m_enum_Level_INFO.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::INFO;
    }
    else if( m_enum_Level_METRIC.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::METRIC;
    }
    else if( m_enum_Level_WARN.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::WARN;
    }
    else if( m_enum_Level_ERROR.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::ERROR;
    }
    else if( m_enum_Level_CRITICAL.isSameObject( env, obj ) ) {
        return aace::logger::LoggerEngineInterface::Level::CRITICAL;
    }
    else {
        return aace::logger::LoggerEngineInterface::Level::CRITICAL;
    }
}

// JNI
#define LOGGER(cptr) ((LoggerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_logger_Logger_log( JNIEnv * env , jobject /* this */, jlong cptr, jobject level, jstring tag, jstring message ) {
    LOGGER(cptr)->log( LOGGER(cptr)->convertLevel( env, level ), NativeLib::convert( env, tag ), NativeLib::convert( env, message ) );
}

}

// END OF FILE

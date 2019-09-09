/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Core/EngineConfigurationBinder.h>
#include <AACE/JNI/Alexa/AlexaConfigurationBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.config.AlexaConfigurationBinder";

// type aliases
using TemplateRuntimeTimeout = aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeout;
using TemplateRuntimeTimeoutType = aace::alexa::config::AlexaConfiguration::TemplateRuntimeTimeoutType;
using EqualizerBand = aace::jni::alexa::EqualizerControllerHandler::EqualizerBand;

namespace aace {
namespace jni {
namespace alexa {

    //
    // JTemplateRuntimeTimeout
    //

    TemplateRuntimeTimeout JTemplateRuntimeTimeout::getTemplateRuntimeTimeout()
    {
        try_with_context
        {
            jobject timeoutTypeObj;
            ThrowIfNot( invoke( "getType", "()Lcom/amazon/aace/alexa/AlexaConfiguration$TemplateRuntimeTimeoutType;", &timeoutTypeObj ), "invokeMethodFailed" );

            TemplateRuntimeTimeoutType checkedTimeoutTypeObj;
            ThrowIfNot( JTemplateRuntimeTimeoutType::checkType( timeoutTypeObj, &checkedTimeoutTypeObj ), "invalidTimeoutType" );

            jint checkedInt;
            ThrowIfNot( invoke( "getValue", "()I", &checkedInt ), "invokeMethodFailed" );

            return { checkedTimeoutTypeObj, std::chrono::milliseconds(checkedInt) };
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"getTemplateRuntimeTimeout",ex.what());
            return {};
        }
    }

    std::vector<TemplateRuntimeTimeout> JTemplateRuntimeTimeout::convert( jobjectArray timeoutArrObj )
    {
        try_with_context
        {
            std::vector<TemplateRuntimeTimeout> runtimeTimeouts;
            JObjectArray arr( timeoutArrObj );
            jobject next;

            for( int j = 0; j < arr.size(); j++ ) {
                ThrowIfNot( arr.getAt( j, &next ), "getArrayValueFailed" );
                runtimeTimeouts.push_back( aace::jni::alexa::JTemplateRuntimeTimeout( next ).getTemplateRuntimeTimeout() );
            }

            return runtimeTimeouts;
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"convert",ex.what());
            return {};
        }
    }
} // aace::jni::alexa
} // aace::jni
} // aace

// JNI
extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createDeviceInfoConfigBinder( JNIEnv* env, jobject obj, jstring deviceSerialNumber, jstring clientId, jstring productId )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createDeviceInfoConfig( JString(deviceSerialNumber).toStdStr(), JString(clientId).toStdStr(), JString(productId).toStdStr() );
            ThrowIfNull( config, "createDeviceInfoConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfigurationBinder_createDeviceInfoConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAlertsConfigBinder( JNIEnv* env, jobject obj, jstring databaseFilePath )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createAlertsConfig( JString(databaseFilePath).toStdStr() );
            ThrowIfNull( config, "createAlertsConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createAlertsConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createNotificationsConfigBinder( JNIEnv* env, jobject obj, jstring databaseFilePath )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createNotificationsConfig( JString(databaseFilePath).toStdStr() );
            ThrowIfNull( config, "createNotificationsConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createNotificationsConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCertifiedSenderConfigBinder( JNIEnv* env, jobject obj, jstring databaseFilePath )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createCertifiedSenderConfig( JString(databaseFilePath).toStdStr() );
            ThrowIfNull( config, "createCertifiedSenderConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCertifiedSenderConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCurlConfigBinder( JNIEnv* env, jobject obj, jstring certsPath, jstring iface )
    {
        try
        {
            auto config = iface != nullptr ?
                aace::alexa::config::AlexaConfiguration::createCurlConfig( JString( certsPath ).toStdStr(), JString( iface ).toStdStr() ) :
                aace::alexa::config::AlexaConfiguration::createCurlConfig( JString( certsPath ).toStdStr() );

            ThrowIfNull( config, "createCurlConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createCurlConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSettingsConfigBinder( JNIEnv* env, jobject obj, jstring databaseFilePath, jstring locale )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createSettingsConfig( JString(databaseFilePath).toStdStr(), JString(locale).toStdStr() );
            ThrowIfNull( config, "createSettingsConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSettingsConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createMiscStorageConfigBinder( JNIEnv* env, jobject obj, jstring databaseFilePath )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createMiscStorageConfig( JString(databaseFilePath).toStdStr() );
            ThrowIfNull( config, "createMiscStorageConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createMiscStorageConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSystemConfigBinder( JNIEnv* env, jobject obj, jint firmwareVersion )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createSystemConfig( firmwareVersion );
            ThrowIfNull( config, "createSystemConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSystemConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeechRecognizerConfigBinder( JNIEnv* env, jobject obj, jstring encoderName )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createSpeechRecognizerConfig( JString(encoderName).toStdStr() );
            ThrowIfNull( config, "createSpeechRecognizerConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createSpeechRecognizerConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createTemplateRuntimeTimeoutConfigBinder( JNIEnv* env, jobject obj, jobjectArray timeoutList )
    {
        try
        {
            auto config = aace::alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig( aace::jni::alexa::JTemplateRuntimeTimeout::convert( timeoutList ) );
            ThrowIfNull( config, "createTemplateRuntimeConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createTemplateRuntimeTimeoutConfigBinder",ex.what());
            return 0;
        }
    }

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_config_AlexaConfiguration_createEqualizerControllerConfigBinder(
            JNIEnv* env, jobject obj, jobjectArray supportedBands, jint minLevel, jint maxLevel, jobjectArray defaultBandLevels )
    {
        try
        {
            std::vector<EqualizerBand> equalizerBands;
            JObjectArray bandsObjArr( supportedBands );

            jobject bandObj;
            EqualizerBand band;

            for( int j = 0; j < bandsObjArr.size(); j++ )
            {
                ThrowIfNot( bandsObjArr.getAt( j, &bandObj ), "getArrayValueFailed" );
                ThrowIfNot( aace::jni::alexa::JEqualizerBand::checkType( bandObj, &band ), "invalidEqualizerBandType" );

                equalizerBands.push_back( band );
            }
            auto config = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
                equalizerBands, minLevel, maxLevel, aace::jni::alexa::JEqualizeBandLevel::convert( defaultBandLevels ) );
            ThrowIfNull( config, "createEqualizerControllerConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_alexa_config_AlexaConfiguration_createEqualizerControllerConfigBinder",ex.what());
            return 0;
        }
    }
}

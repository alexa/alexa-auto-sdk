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
#include <AACE/AmazonLite/AmazonLiteConfiguration.h>
#include "AACE/JNI/AmazonLite/AmazonLiteConfigurationBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.amazonlite.config.AmazonLiteConfigurationBinder";

// type aliases
using ModelConfig = aace::amazonLite::config::AmazonLiteConfiguration::ModelConfig;

namespace aace {
namespace jni {
namespace amazonLite {

    //
    // JModelConfig
    //

    ModelConfig JModelConfig::getModelConfig()
    {
        try_with_context
        {
            jstring modelConfigLocaleObj;
            ThrowIfNot( invoke( "getLocale", "()Ljava/lang/String;", &modelConfigLocaleObj ), "invokeGetLocaleMethodFailed" );

            jstring modelConfigPathObj;
            ThrowIfNot( invoke( "getPath", "()Ljava/lang/String;", &modelConfigPathObj ), "invokeGetPathMethodFailed" );

            return { JString(modelConfigLocaleObj).toStdStr(), JString(modelConfigPathObj).toStdStr() };
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"getModelConfig",ex.what());
            return {};
        }
    }

    std::vector<ModelConfig> JModelConfig::convert( jobjectArray modelConfigArrObj )
    {
        try_with_context
        {
            // convert java model config
            std::vector<ModelConfig> modelConfig;
            JObjectArray arr( modelConfigArrObj );
            jobject next;

            for( int j = 0; j < arr.size(); j++ ) {
                ThrowIfNot( arr.getAt( j, &next ), "getArrayValueFailed" );
                modelConfig.push_back( aace::jni::amazonLite::JModelConfig( next ).getModelConfig() );
            }

            return modelConfig;
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"convert",ex.what());
            return {};
        }
    }

} // aace::jni::amazonLite
} // aace:jni
} // aace

// JNI
extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_amazonlite_config_AmazonLiteConfiguration_createAmazonLiteConfigBinder( JNIEnv * env, jobject obj, jstring rootPath, jobjectArray arr )
    {
        try
        {
            auto config = aace::amazonLite::config::AmazonLiteConfiguration::createAmazonLiteConfig( JString(rootPath).toStdStr(), aace::jni::amazonLite::JModelConfig::convert( arr ) );
            ThrowIfNull( config, "createAmazonLiteConfigFailed" );

            return reinterpret_cast<long>( new aace::jni::core::config::EngineConfigurationBinder( config ) );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_amazonlite_config_AmazonLiteConfiguration_createAmazonLiteConfigBinder",ex.what());
            return 0;
        }
    }
}

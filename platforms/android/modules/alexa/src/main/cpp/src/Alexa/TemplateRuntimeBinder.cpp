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

#include <AACE/JNI/Alexa/TemplateRuntimeBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.TemplateRuntimeBinder";

namespace aace {
namespace jni {
namespace alexa {

    //
    // TemplateRuntimeBinder
    //

    TemplateRuntimeBinder::TemplateRuntimeBinder( jobject obj ) {
        m_templateRuntimeHandler = std::make_shared<TemplateRuntimeHandler>( obj );
    }

    //
    // TemplateRuntimeHandler
    //

    TemplateRuntimeHandler::TemplateRuntimeHandler( jobject obj ) : m_obj( obj, "com/amazon/aace/alexa/TemplateRuntime" ) {
    }

    void TemplateRuntimeHandler::renderTemplate( const std::string& payload )
    {
        try_with_context
        {
            ThrowIfNot( m_obj.invoke<void>( "renderTemplate", "(Ljava/lang/String;)V", nullptr, JString(payload).get() ), "invokeMethodFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"renderTemplate",ex.what());
        }
    }

    void TemplateRuntimeHandler::renderPlayerInfo( const std::string& payload )
    {
        try_with_context
        {
            ThrowIfNot( m_obj.invoke<void>( "renderPlayerInfo", "(Ljava/lang/String;)V", nullptr, JString(payload).get() ), "invokeMethodFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"renderPlayerInfo",ex.what());
        }
    }

    void TemplateRuntimeHandler::clearTemplate()
    {
        try_with_context
        {
            ThrowIfNot( m_obj.invoke<void>( "clearTemplate", "()V", nullptr ), "invokeMethodFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"clearTemplate",ex.what());
        }
    }

    void TemplateRuntimeHandler::clearPlayerInfo()
    {
        try_with_context
        {
            ThrowIfNot( m_obj.invoke<void>( "clearPlayerInfo", "()V", nullptr ), "invokeMethodFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"clearPlayerInfo",ex.what());
        }
    }

} // aace::alexa
} // aace::jni
} // aace

#define TEMPLATE_RUNTIME_BINDER(ref) reinterpret_cast<aace::jni::alexa::TemplateRuntimeBinder *>( ref )

extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_alexa_TemplateRuntime_createBinder( JNIEnv* env, jobject obj )  {
        return reinterpret_cast<long>( new aace::jni::alexa::TemplateRuntimeBinder( obj ) );
    }
    
    JNIEXPORT void JNICALL
    Java_com_amazon_aace_alexa_TemplateRuntime_disposeBinder( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto templateRuntimeBinder = TEMPLATE_RUNTIME_BINDER(ref);
            ThrowIfNull( templateRuntimeBinder, "invalidTemplateRuntimeBinder" );
            delete templateRuntimeBinder;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aaced_alexa_TemplateRuntime_disposeBinder",ex.what());
        }
    }
}



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

#include <AACE/JNI/Navigation/NavigationBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.navigation.NavigationBinder";

namespace aace {
namespace jni {
namespace navigation {

    //
    // NavigationBinder
    //

    NavigationBinder::NavigationBinder( jobject obj ) {
        m_navigationHandler = std::shared_ptr<NavigationHandler>( new NavigationHandler( obj ) );
    }

    //
    // NavigationHandler
    //

    NavigationHandler::NavigationHandler( jobject obj ) : m_obj( obj, "com/amazon/aace/navigation/Navigation" ) {
    }

    bool NavigationHandler::setDestination( const std::string& payload )
    {
        try_with_context
        {
            jboolean result;
            ThrowIfNot( m_obj.invoke( "setDestination", "(Ljava/lang/String;)Z", &result, JString(payload).get() ), "invokeFailed" );
            return result;
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"setDestination",ex.what());
            return false;
        }
    }

    bool NavigationHandler::cancelNavigation()
    {
        try_with_context
        {
            jboolean result;
            ThrowIfNot( m_obj.invoke( "cancelNavigation", "()Z", &result ), "invokeFailed" );
            return result;
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"cancelNavigation",ex.what());
            return false;
        }
    }

    std::string NavigationHandler::getNavigationState()
    {
        try_with_context
        {
            jstring result;
            ThrowIfNot( m_obj.invoke( "getNavigationState", "()Ljava/lang/String;", &result ), "invokeFailed" );
            return JString( result ).toStdStr();
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"getNavigationState",ex.what());
            return std::string("");
        }
    }

} // aace::jni::navigation
} // aace::jni
} // aace

#define NAVIGATION_BINDER(ref) reinterpret_cast<aace::jni::navigation::NavigationBinder *>( ref )

extern "C"
{

    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_navigation_Navigation_createBinder( JNIEnv* env, jobject obj )  {
        return reinterpret_cast<long>( new aace::jni::navigation::NavigationBinder( obj ) );
    }

    JNIEXPORT void JNICALL
    Java_com_amazon_aace_navigation_Navigation_disposeBinder( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto navigationBinder = NAVIGATION_BINDER(ref);
            ThrowIfNull( navigationBinder, "invalidNavigationBinder" );
            delete navigationBinder;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_navigation_Navigation_disposeBinder",ex.what());
        }
    }
}

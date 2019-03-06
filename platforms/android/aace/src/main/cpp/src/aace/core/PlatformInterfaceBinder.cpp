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

#include "aace/core/PlatformInterfaceBinder.h"

PlatformInterfaceBinder::~PlatformInterfaceBinder()
{
    ThreadContext context;

    if( context.getEnv() != nullptr ) {
        dispose( context.getEnv() );
    }
}

void PlatformInterfaceBinder::bind( JNIEnv* env, jobject javaObj )
{
    m_javaObj = ObjectRef( env, javaObj );
    m_javaClass = ClassRef( env, env->GetObjectClass( javaObj ) );

    // let the subclass initialize
    initialize( env );
}

void PlatformInterfaceBinder::initialize( JNIEnv* env ) {
}

void PlatformInterfaceBinder::dispose( JNIEnv* env ) {
}

jobject PlatformInterfaceBinder::getJavaObject() {
    return m_javaObj.get();
}

jclass PlatformInterfaceBinder::getJavaClass() {
    return (jclass) m_javaClass.get();
}

// END OF FILE

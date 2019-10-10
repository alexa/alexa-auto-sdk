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

// aace/alexa/GlobalPresetBinder.cpp

#include "aace/alexa/GlobalPresetBinder.h"

void GlobalPresetBinder::initialize( JNIEnv* env )
{
    m_javaMethod_setGlobalPreset_preset = env->GetMethodID( getJavaClass(), "setGlobalPreset", "(I)V" );
}

void GlobalPresetBinder::setGlobalPreset( int preset )
{
    if( getJavaObject() != nullptr && m_javaMethod_setGlobalPreset_preset != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_setGlobalPreset_preset, preset );
        }
    }
}
// JNI
#define GLOBALPRESET(cptr) ((GlobalPresetBinder *) cptr)
extern "C" {
}
// END OF FILE

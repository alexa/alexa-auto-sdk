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

#ifndef AACE_CORE_PLATFORM_INTERFACE_BINDER_H
#define AACE_CORE_PLATFORM_INTERFACE_BINDER_H

#include "NativeLib.h"

class PlatformInterfaceBinder {
public:
    virtual ~PlatformInterfaceBinder();

public:
    void bind( JNIEnv* env, jobject javaObj );

protected:
    virtual void initialize( JNIEnv* env );
    virtual void dispose( JNIEnv* env );

    jobject getJavaObject();
    jclass getJavaClass();

private:
    ObjectRef m_javaObj;
    ClassRef m_javaClass;
};

#endif // AACE_CORE_PLATFORM_INTERFACE_BINDER_H

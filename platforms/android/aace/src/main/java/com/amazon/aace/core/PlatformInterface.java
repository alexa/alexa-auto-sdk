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

package com.amazon.aace.core;

/**
 * PlatformInterface is the base class for all platform interfaces.
 * It can be extended to create custom platform interfaces.
 */
public class PlatformInterface
{
    public PlatformInterface() {
        super();
    }

    // ptr to the native object
    private long m_nativeObject;

    /**
     * @internal
     * Set the native object associated with this instance.
     */
    final public void setNativeObject( long nativeObject ) {
        m_nativeObject = nativeObject;
    }

    /**
     * @internal
     * Get the native object associated with this instance.
     */
    final public long getNativeObject() {
        return m_nativeObject;
    }
}

// END OF FILE

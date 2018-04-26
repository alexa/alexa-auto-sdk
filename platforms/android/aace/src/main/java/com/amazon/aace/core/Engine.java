/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/core/Engine.java
// This is an automatically generated file.

package com.amazon.aace.core;

import com.amazon.aace.core.config.EngineConfiguration;

/**
 * The @c Engine must be instantiated by the platform implementation. Along with the platform interfaces, it is responsible for managing the interactions between the platform and AVS.
 * The platform implementation should *not* extend Engine. It is currently extended in the SDK.
 */
public class Engine extends PlatformInterface
{
    static {
        // Used to load the 'aace-lib' library on initialization
        System.loadLibrary( "aace-lib" );
    }

    /**
     * Creates a new instance of an Engine object.
     */
    static public Engine create()
    {
        Engine engine = new Engine();

        return create( engine ) ? engine : null;
    }

    private Engine() {
    }

    /**
     * Sets the Engine configuration to a set of configuration objects.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configurationList Collection of @c aace::config::EngineConfiguration objects passed in as @c std::vector<aace::config::EngineConfiguration*>.
     * @return @c true if successful, else @c false.
     */
    public boolean configure( EngineConfiguration[] configurationList ) {
        return configure( getNativeObject(), configurationList );
    }

    /**
     * Starts the Engine and attempts to establish a connection to AVS.
     *
     * @return @c true if successful, else @c false.
     * @sa stop()
     */
    public boolean start() {
        return start( getNativeObject() );
    }

    /**
     * Stops the Engine and shuts down the connection to AVS.
     *
     * @return @c true if successful, else @c false.
     * @sa start()
     */
    public boolean stop() {
        return stop( getNativeObject() );
    }

    public boolean dispose() {
        return dispose( getNativeObject() );
    }

    public boolean registerPlatformInterface( PlatformInterface platformInterface ) {
        return registerPlatformInterface( getNativeObject(), platformInterface );
    }

    public boolean setNativeEnv( String name, String value ) {
        return setNativeEnv( getNativeObject(), name, value );
    }

    // creates the native Engine instance
    static private native boolean create( Engine engine );

    // Native Engine JNI methods
    private native boolean configure( long nativeObject, EngineConfiguration[] configurationList );
    private native boolean start( long nativeObject );
    private native boolean stop( long nativeObject );
    private native boolean dispose( long nativeObject );
    private native boolean registerPlatformInterface( long nativeObject, PlatformInterface platformInterface );
    private native boolean setNativeEnv( long nativeObj, String name, String value );
}

// END OF FILE

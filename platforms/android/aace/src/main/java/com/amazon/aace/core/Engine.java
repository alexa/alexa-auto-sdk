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

import com.amazon.aace.core.config.EngineConfiguration;

/**
 * The Engine must be instantiated by the platform implementation. Along with the platform interfaces,
 * it is responsible for managing the interactions between the platform and AVS.
 * The platform implementation should *not* extend Engine; it is extended in the SDK.
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
     * Sets the Engine configuration to a set of configuration objects
     *
     * @param  configurationList A collection of @c com.amazon.aace.core.config.EngineConfiguration objects as an
     * @c EngineConfiguration[]
     * @return @c true if the Engine configuration was successful, else @c false
     */
    public boolean configure( EngineConfiguration[] configurationList ) {
        return configure( getNativeObject(), configurationList );
    }

    /**
     * Starts the Engine and attempts to establish a connection to AVS
     *
     * @return @c true if the Engine was started, else @c false
     *
     * @sa stop()
     */
    public boolean start() {
        return start( getNativeObject() );
    }

    /**
     * Stops the Engine and shuts down the connection to AVS
     *
     * @return @c true if the Engine was stopped, else @c false
     *
     * @sa start()
     */
    public boolean stop() {
        return stop( getNativeObject() );
    }

    /**
     * Shuts down the engine and releases all of its resources
     *
     * @return @c true if the Engine was shut down, else @c false
     */
    public boolean shutdown() {
        return shutdown( getNativeObject() );
    }

    public boolean dispose() {
        return dispose( getNativeObject() );
    }

    /**
     * Registers a @c PlatformInterface instance with the Engine
     * The platform implementation must register each interface required by the application.
     *
     * @param  platformInterface The @c PlatformInterface instance to register
     *
     * @return @c true if the @c PlatformInterface instance was registered, else @c false
     *
     * @sa com.amazon.aace.core.PlatformInterface
     */
    public boolean registerPlatformInterface( PlatformInterface platformInterface ) {
        return registerPlatformInterface( getNativeObject(), platformInterface );
    }

    /**
     * Sets a property value in the Engine
     *
     * @param  key The key used by the Engine to identify the property
     *
     * @param  value The property value to set in the Engine
     *
     * @return @c true if property value was set, else @c false
     */
    public boolean setProperty( String key, String value ) {
        return setProperty( getNativeObject(), key, value );
    }

    /**
     * Returns a property value from the Engine
     *
     * @param  key The key used by the Engine to identify the property
     *
     * @returns The property value as a string
     */
    public String getProperty( String key ) {
        return getProperty( getNativeObject(), key );
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
    private native boolean shutdown( long nativeObject );
    private native boolean dispose( long nativeObject );
    private native boolean registerPlatformInterface( long nativeObject, PlatformInterface platformInterface );
    private native boolean setProperty( long nativeObject, String key, String value );
    private native String getProperty( long nativeObject, String key );
    private native boolean setNativeEnv( long nativeObj, String name, String value );
}

// END OF FILE

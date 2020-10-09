/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import android.content.Context;
import android.util.Log;

import com.amazon.aace.core.config.EngineConfiguration;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.InputStream;

/**
 * The Engine must be instantiated by the platform implementation. Along with the platform interfaces,
 * it is responsible for managing the interactions between the platform and AVS.
 * The platform implementation should *not* extend Engine; it is extended in the SDK.
 */
final public class Engine extends NativeRef {
    private static final String TAG = "CoreEngine";

    private Engine() {}

    /**
     * Creates a new instance of an Engine object.
     */
    static public Engine create(Context context) {
        loadModuleLibraries(context);
        return new Engine();
    }

    /**
     * Sets the Engine configuration to a set of configuration objects
     *
     * @param  configurationList A collection of @c com.amazon.aace.core.EngineConfiguration objects as an
     * @c EngineConfiguration[]
     * @return @c true if the Engine configuration was successful, else @c false
     */
    public boolean configure(EngineConfiguration[] configurationList) {
        long[] configurationRefList = new long[configurationList.length];

        for (int j = 0; j < configurationList.length; j++) {
            configurationRefList[j] = configurationList[j].getNativeRef();
        }

        return configure(getNativeRef(), configurationRefList);
    }

    /**
     * Starts the Engine and attempts to establish a connection to AVS
     *
     * @return @c true if the Engine was started, else @c false
     *
     * @sa stop()
     */
    public boolean start() {
        return start(getNativeRef());
    }

    /**
     * Stops the Engine and shuts down the connection to AVS
     *
     * @return @c true if the Engine was stopped, else @c false
     *
     * @sa start()
     */
    public boolean stop() {
        return stop(getNativeRef());
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
    public boolean registerPlatformInterface(PlatformInterface platformInterface) {
        return registerPlatformInterface(getNativeRef(), platformInterface.getNativeRef());
    }

    // Retrieve library names of built modules
    private static void loadModuleLibraries(Context context) {
        try {
            String folderName = "meta-aac";
            String libraryKey = "libraries";
            String category = "name";
            String[] fileList = context.getAssets().list(folderName);
            for (String s : fileList) {
                InputStream is = context.getAssets().open(folderName + "/" + s);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    JSONArray jsonArray = obj.getJSONArray(libraryKey);
                    for (int i = 0; i < jsonArray.length(); i++) {
                        String libraryName = jsonArray.getJSONObject(i).getString(category);
                        System.loadLibrary(libraryName);
                    }
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    public boolean setNativeEnv(String name, String value) {
        return setNativeEnv(getNativeRef(), name, value);
    }

    protected long createNativeRef() {
        return createBinder();
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean configure(long nativeRef, long[] configurationRefList);
    private native boolean start(long nativeRef);
    private native boolean stop(long nativeRef);
    private native boolean registerPlatformInterface(long nativeRef, long platformInterfaceRef);
    private native boolean setNativeEnv(long nativeRef, String name, String value);
}

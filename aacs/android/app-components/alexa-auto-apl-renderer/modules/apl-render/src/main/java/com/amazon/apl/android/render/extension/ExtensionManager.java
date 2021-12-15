/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.extension;

import android.util.Log;

import com.amazon.apl.android.Event;
import com.amazon.apl.android.ExtensionCommandDefinition;
import com.amazon.apl.android.ExtensionEventHandler;
import com.amazon.apl.android.ExtensionFilterDefinition;
import com.amazon.apl.android.RootConfig;
import com.amazon.apl.android.dependencies.IExtensionEventCallback;
import com.amazon.apl.android.providers.IExtension;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Extension Manager responsible for containing available extensions and registering requested extension
 * with the root config.
 */
public class ExtensionManager implements IExtensionEventCallback {
    private static final String TAG = ExtensionManager.class.getSimpleName();

    private Map<String, IExtension> mExtensions;
    private RootConfig mRootConfig;
    private DiscoveryCallback mDiscoveryCallback;

    //-------------------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------------------

    public ExtensionManager(RootConfig rootConfig) {
        mRootConfig = rootConfig;
        mExtensions = new HashMap<>();
    }

    private IExtension getBuiltInExtension(String uri) {
        return mExtensions.get(uri);
    }

    //-------------------------------------------------------------------------
    // Public methods
    //-------------------------------------------------------------------------

    /**
     * Interface for notifying when the extensions are loaded and registered.
     */
    public interface DiscoveryCallback { void onComplete(); }

    /**
     * Adds a built in extension so that it is registered with the APL runtime.
     *
     * @param extension The extension to add.
     */
    public void addBuiltInExtension(IExtension extension) {
        mExtensions.put(extension.getUri(), extension);
    }

    /**
     * This method will register the built in extensions that are requested by the
     * APL document only if they are available in the extension manager.
     *
     * @param requestedExtensionUris Extensions requested through the APL document.
     * @param content The APL runtime content.
     * @param context The Android context.
     * @param discoveryCallback The callback to execute once extensions are registered.
     */
    public void registerRequestedExtensions(Set<String> requestedExtensionUris, DiscoveryCallback discoveryCallback) {
        mDiscoveryCallback = discoveryCallback;

        for (String requestedExtensionUri : requestedExtensionUris) {
            IExtension extension = getBuiltInExtension(requestedExtensionUri);
            if (extension != null) {
                Log.v(TAG, "Registering extension: " + requestedExtensionUri);
                mRootConfig.registerExtensionEnvironment(extension.getUri(), extension.getEnvironment());
                for (ExtensionCommandDefinition command : extension.getCommandDefinitions()) {
                    mRootConfig.registerExtensionCommand(command);
                }
                for (ExtensionFilterDefinition filter : extension.getFilterDefinitions()) {
                    mRootConfig.registerExtensionFilter(filter);
                }
                for (ExtensionEventHandler handler : extension.getEventHandlers()) {
                    mRootConfig.registerExtensionEventHandler(handler);
                }
            } else {
                Log.v(TAG, "Extension not supported by runtime: " + requestedExtensionUri);
            }
        }

        Log.v(TAG, "Finished registering extensions");
        mDiscoveryCallback.onComplete();
    }

    //-------------------------------------------------------------------------
    // IExtensionEventCallback
    //-------------------------------------------------------------------------

    @Override
    public void onExtensionEvent(String name, String uri, Event event, Map<String, Object> source,
            Map<String, Object> custom, IExtensionEventCallbackResult resultCallback) {
        handleOnExtensionEvent(name, uri, event, source, custom, resultCallback);
    }

    @Override
    public void onExtensionEvent(String name, String uri, Map<String, Object> source, Map<String, Object> custom,
            IExtensionEventCallbackResult resultCallback) {
        handleOnExtensionEvent(name, uri, null, source, custom, resultCallback);
    }

    private void handleOnExtensionEvent(String name, String uri, Event event, Map<String, Object> source,
            Map<String, Object> custom, IExtensionEventCallbackResult resultCallback) {
        Log.v(TAG,
                "handleOnExtensionEvent uri: " + uri + " event: " + event + " source: " + source
                        + " custom: " + custom);
        IExtension extension = getBuiltInExtension(uri);
        if (extension != null) {
            IExtensionEventCallback callback = extension.getCallback();
            if (callback != null) {
                callback.onExtensionEvent(name, uri, event, source, custom, resultCallback);
            } else {
                Log.w(TAG, "Callback not handled for uri:" + uri);
            }
        } else {
            Log.w(TAG, "Extension not found uri:" + uri);
        }
    }
}

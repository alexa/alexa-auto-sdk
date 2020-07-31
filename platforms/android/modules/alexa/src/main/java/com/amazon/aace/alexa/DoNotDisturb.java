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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * DoNotDisturb should be extended to handle receving state changes from the engine,
 * and for sending state change events for the Do Not Disturb Setting.
 * For more information about Do Not Disturb see the interface overview:
 * https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html
 */
public class DoNotDisturb extends PlatformInterface {
    /**
     * Handle setting of DND directive.
     *
     * @param doNotDisturb setting state
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public void setDoNotDisturb(boolean doNotDisturb) {}

    /**
     * Notifies the Engine of a platform request to set the DND State
     * @return @c true if the engine implementation successfully sent the event,
     * else @c false
     */
    final public boolean doNotDisturbChanged(boolean doNotDisturb) {
        return doNotDisturbChanged(getNativeRef(), doNotDisturb);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean doNotDisturbChanged(long nativeObject, boolean doNotDisturb);
}

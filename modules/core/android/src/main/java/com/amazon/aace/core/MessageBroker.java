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

import com.amazon.aace.core.NativeRef;

import java.util.function.Function;

final public class MessageBroker extends NativeRef {
    public interface MessageHandler { public void messageReceived(String message); }

    public final void subscribe(MessageHandler handler, String topic, String action) {
        subscribe(getNativeRef(), handler, topic, action);
    }

    public final void publish(String message) {
        publish(getNativeRef(), message);
    }

    public final MessageStream openStream(String streamId, MessageStream.Mode mode) {
        return openStream(getNativeRef(), streamId, mode);
    }

    // NativeRef implementation
    protected long createNativeRef() {
        return 0;
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native void disposeBinder(long nativeRef);
    private native void subscribe(long nativeRef, MessageHandler handler, String topic, String action);
    private native void publish(long nativeRef, String message);
    private native MessageStream openStream(long nativeRef, String streamId, MessageStream.Mode mode);
}

// END OF FILE

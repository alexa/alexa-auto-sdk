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

package com.amazon.aace.core.config;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

final public class StreamConfiguration extends EngineConfiguration {
    private ByteArrayOutputStream mByteArray = new ByteArrayOutputStream();

    private StreamConfiguration(InputStream is) {
        try {
            byte[] buffer = new byte[1024];
            int size;

            while ((size = is.read(buffer)) > 0) {
                mByteArray.write(buffer, 0, size);
            }
        } catch (Throwable ex) {
            ex.printStackTrace();
        }
    }

    public static StreamConfiguration create(InputStream is) {
        return new StreamConfiguration(is);
    }

    protected long createNativeRef() {
        return createBinder(mByteArray.toByteArray());
    }

    // Native Engine JNI methods
    private native long createBinder(byte[] data);
}

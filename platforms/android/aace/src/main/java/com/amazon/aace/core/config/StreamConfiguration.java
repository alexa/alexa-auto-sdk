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

package com.amazon.aace.core.config;

import java.io.InputStream;

/**
 * Configuration class used to provide configuration data from a stream. The contents
 * of the stream should be JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 */
public class StreamConfiguration extends EngineConfiguration {
    private InputStream mStream;

    /**
     * Creates a new instance of @c StreamConfiguration by providing the input stream
     * containing JSON data
     *
     * @param  stream The input stream
     */
    public static StreamConfiguration create( InputStream is ) {
        return new StreamConfiguration( is );
    }

    /**
     * private constructor
     */
    private StreamConfiguration( InputStream is ) {
        mStream = is;
    }

    @Override
    /**
     *
     * @return A pointer to a @c InputStream object containing the JSON configuration data
     */
    public InputStream getStream() {
        return mStream;
    }
}

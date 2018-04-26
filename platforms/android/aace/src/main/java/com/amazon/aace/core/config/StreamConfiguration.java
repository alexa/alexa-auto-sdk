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

package com.amazon.aace.core.config;

import java.io.InputStream;

/**
 * Configuration class used to provide configuration data from a stream. The contents
 * of the stream should be @c JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 *
 * @sa [Configuring the Engine](index.html#configuring-the-engine)
 */
public class StreamConfiguration extends EngineConfiguration
{
    private InputStream m_stream;

    /**
     * Creates a new instance of @c StreamConfiguration by providing the input stream
     * containing @c JSON data.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] stream Input stream.
     */
    static public StreamConfiguration create( InputStream is ) {
        return new StreamConfiguration( is );
    }

    /**
     * private constructor
     */
    private StreamConfiguration( InputStream is ) {
        m_stream = is;
    }

    @Override
    public InputStream getStream() {
        return m_stream;
    }
}

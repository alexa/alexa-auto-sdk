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

// aace/core/config/EngineConfiguration.java
// This is an automatically generated file.

package com.amazon.aace.core.config;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

/**
 * Abstract base class for providing @c JSON configuration data to the Engine.
 */
abstract public class EngineConfiguration
{
    /**
     * @return Pointer to a @c std::istream object containing the @c JSON configuration data.
     */
    abstract public InputStream getStream();

    /**
     * @return @c ByteArray of the @c std::istream object.
     */
    final public byte[] getBytes()
    {
        try
        {
            InputStream is = getStream();
            ByteArrayOutputStream os = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int len = 0;

            while( (len = is.read(buffer)) > 0 ) {
                os.write( buffer, 0, len );
            }

            return os.toByteArray();
        }
        catch( Throwable ex ) {
            ex.printStackTrace();
        }

        return new byte[0];
    }
}

// END OF FILE

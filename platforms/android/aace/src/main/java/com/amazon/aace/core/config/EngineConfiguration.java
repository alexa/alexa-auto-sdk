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

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Base class for providing JSON configuration data to the Engine.
 */
abstract public class EngineConfiguration {

    private static final String sTag = EngineConfiguration.class.getSimpleName();

    /**
     * @return Pointer to an @c InputStream object containing the @c JSON configuration data.
     */
    abstract public InputStream getStream();

    /**
     * @return @c ByteArray of the @c InputStream object.
     */
    final public byte[] getBytes() {
        try ( InputStream is = getStream();
              ByteArrayOutputStream os = new ByteArrayOutputStream()
        ) {
            byte[] buffer = new byte[1024];
            int len;
            while( ( len = is.read( buffer ) ) > 0 ) os.write( buffer, 0, len );
            return os.toByteArray();
        } catch ( IOException e ) { Log.e( sTag, e.getMessage() ); }

        return new byte[0];
    }
}

// END OF FILE

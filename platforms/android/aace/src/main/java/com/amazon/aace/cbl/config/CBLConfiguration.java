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

package com.amazon.aace.cbl.config;

import android.util.Log;

import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;

import org.json.JSONObject;
import org.json.JSONException;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

/**
 * A factory interface for creating CBL configuration objects
 */
public class CBLConfiguration {

    private static final String sTag = CBLConfiguration.class.getSimpleName();

    static public EngineConfiguration createCBLConfig( int seconds ) {
        EngineConfiguration cblConfig = null;

        JSONObject config = new JSONObject();
        try {
            JSONObject aaceCBLElement = new JSONObject();
            aaceCBLElement.put( "requestTimeout", seconds );
            config.put( "aace.cbl", aaceCBLElement );
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }

        String configStr = config.toString();
        try ( InputStream is = new ByteArrayInputStream(
                configStr.getBytes( StandardCharsets.UTF_8.name() ) )
        ) {
            cblConfig = StreamConfiguration.create( is );
        } catch ( IOException e ) { Log.e( sTag, e.getMessage() ); }

        return cblConfig;
    }
}

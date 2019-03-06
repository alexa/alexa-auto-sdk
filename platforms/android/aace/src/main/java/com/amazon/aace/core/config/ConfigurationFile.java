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

import java.io.FileInputStream;
import java.io.FileNotFoundException;

/**
 * Configuration class used to provide configuration data from a file. The contents
 * of the file should be JSON data with values corresponding to the content provided
 * by the other configuration objects.
 */
public class ConfigurationFile {

    private static final String sTag = ConfigurationFile.class.getSimpleName();

    /**
     * Creates a new instance of @c ConfigurationFile by providing the path to a configuration file
     * containing JSON data.
     *
     * @param  configFilePath The path to the configuration file
     */
    public static EngineConfiguration create( String configFilePath ) {
        try {
            return StreamConfiguration.create( new FileInputStream( configFilePath ) );
        } catch ( FileNotFoundException e ) {
            Log.e( sTag, e.getMessage() );
            return null;
        }
    }
}

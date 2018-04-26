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

import java.io.FileInputStream;

/**
 * Configuration class used to provide configuration data from a file. The contents
 * of the file should be @c JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 *
 * @sa [Configuring the Engine](index.html#configuring-the-engine)
 */
public class ConfigurationFile
{
    /**
     * Creates a new instance of @c EngineConfiguration by providing the path to a configuration file
     * containing @c JSON data.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configFilePath Path to the configuration file.
     */
    static public EngineConfiguration create( String configFilePath )
    {
        try
        {
            return StreamConfiguration.create( new FileInputStream( configFilePath ) );
        }
        catch( Throwable ex ) {
            return null;
        }
    }
}

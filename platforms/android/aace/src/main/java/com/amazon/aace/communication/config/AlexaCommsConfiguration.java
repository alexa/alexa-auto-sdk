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

package com.amazon.aace.communication.config;

import android.util.Log;

import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aace.core.config.StreamConfiguration;

import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

/**
 * A factory interface for creating Alexa Comms configuration objects
 */
public class AlexaCommsConfiguration {

    private static final String sTag = AlexaCommsConfiguration.class.getSimpleName();

    static public EngineConfiguration createCommsConfig(String certsPath) {
        EngineConfiguration commsConfig = null;

        try {
            JSONObject config = new JSONObject();

            JSONObject commsElement = new JSONObject();
            config.put("communications", commsElement);

            File sipCertificateFile = new File(certsPath, "09789157.0");
            commsElement.put("sipCertificateFilePath", sipCertificateFile.getAbsolutePath());

            JSONObject inputDeviceSettingsElement = new JSONObject();
            inputDeviceSettingsElement.put("driverName", "default");
            inputDeviceSettingsElement.put("deviceName", "default");
            commsElement.put("inputDevice", inputDeviceSettingsElement);

            JSONObject outputDeviceSettingsElement = new JSONObject();
            outputDeviceSettingsElement.put("driverName", "default");
            outputDeviceSettingsElement.put("deviceName", "default");
            commsElement.put("outputDevice", outputDeviceSettingsElement);

            String configStr = config.toString();
            InputStream is = new ByteArrayInputStream(configStr.getBytes(StandardCharsets.UTF_8.name()));

            commsConfig = StreamConfiguration.create( is );
        } catch( Throwable ex ) {
            Log.e( sTag, ex.getMessage() );
        }
        return commsConfig;
    }
}

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


    /**
     * Creates configuration for A2A calling.
     *
     * @param certsPath Path where certificate(s) required for connecting to AVS cloud can be found.
     * @return Configuration object.
     */
    static public EngineConfiguration createCommsConfig(String certsPath) {
        // By default do not connect to AWS IOT metrics services. The space in a string is intentional
        // to express the intent that we do not wish to connect to metrics services.
        return createCommsConfig(certsPath, " ", " ", "");
    }

    /**
     * Creates configuration for A2A calling.
     *
     * @param certsPath Path where certificate(s) required for connecting to AVS cloud can be found.
     * @param metricsDeviceTypeId AWS IOT device type id for connecting to AWS metrics services.
     * @param metricsIOTHostAddress AWS IOT metrics services host address.
     * @param metricsIOTCertDirPath Directory where AWS IOT certificate for connecting to metric service can be found.
     * @return Configuration object.
     */
     static public EngineConfiguration createCommsConfig(
             String certsPath, String metricsDeviceTypeId, String metricsIOTHostAddress, String metricsIOTCertDirPath) {
        EngineConfiguration commsConfig = null;

        try {
            JSONObject config = new JSONObject();

            JSONObject commsElement = new JSONObject();
            config.put("communications", commsElement);

            File sipCertificateFile = new File(certsPath, "09789157.0");
            commsElement.put("sipCertificateFilePath", sipCertificateFile.getAbsolutePath());
            commsElement.put("iotCertificateDirPath", metricsIOTCertDirPath);
            commsElement.put("deviceTypeId", metricsDeviceTypeId);
            commsElement.put("iotHostAddress", metricsIOTHostAddress);

            JSONObject inputDeviceSettingsElement = new JSONObject();
            inputDeviceSettingsElement.put("driverName", "audioproxy");
            inputDeviceSettingsElement.put("deviceName", "audioproxy");
            commsElement.put("inputDevice", inputDeviceSettingsElement);

            JSONObject outputDeviceSettingsElement = new JSONObject();
            outputDeviceSettingsElement.put("driverName", "audioproxy");
            outputDeviceSettingsElement.put("deviceName", "audioproxy");
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

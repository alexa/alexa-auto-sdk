/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aacsconstants;

public class AACSConstants {
    public static final String AACS_PACKAGE_NAME = "com.amazon.alexaautoclientservice";
    public static final String AACS_CLASS_NAME = "com.amazon.alexaautoclientservice.AlexaAutoClientService";
    public static String AACS = "AACS";

    public static final String NEW_CONFIG = "newConfig";
    public static final String CONFIG_MODULE = "configModule";
    public static final String PAYLOAD = "payload";
    public static final String AACS_GENERAL_CONFIG = "aacs.general";
    public static final String AACS_AASB_CONFIG = "aacs.aasb";

    public static final String AACS_PROPERTY_URI = "com.amazon.alexaautoclientservice.AACSPropertyContentProvider";
    public static final String[] ALEXA_PROPERTIES = {AACSPropertyConstants.WAKEWORD_SUPPORTED,
            AACSPropertyConstants.FIRMWARE_VERSION, AACSPropertyConstants.LOCALE,
            AACSPropertyConstants.COUNTRY_SUPPORTED, AACSPropertyConstants.TIMEZONE,
            AACSPropertyConstants.WAKEWORD_ENABLED, AACSPropertyConstants.OPERATING_COUNTRY,
            AACSPropertyConstants.VERSION, AACSPropertyConstants.NETWORK_INTERFACE};

    // AACS Permission
    public static final String AACS_PERMISSION = "com.amazon.alexaautoclientservice";
    public static final String AACS_PING_PERMISSION = "com.amazon.alexaautoclientservice.ping";
}

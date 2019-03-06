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

package com.amazon.aace.alexa;

public class AlexaProperties {

    /**
     * This property is used with Engine.setProperty() to change the AVS endpoint. The value must be
     * a valid AVS endpoint URL. See https://developer.amazon.com/docs/alexa-voice-service/api-overview.html#endpoints
     * @hideinitializer
     */
    public static final String AVS_ENDPOINT = "aace.alexa.endpoint";

    /**
     * This read-only property is used with Engine.getProperty() to check if the Engine has wake word support.
     * If wake word is not supported in the Engine, attempts to enable wake word detection by
     * the @c SpeechRecognizer will fail.
     * @hideinitializer
     */
    public static final String WAKEWORD_SUPPORTED = "aace.alexa.wakewordSupported";

    /**
     * This property is used with Engine.setProperty() to change the firmware version that is reported
     * to AVS. The value must be a positive, signed 32-bit integer represented
     * as a string.
     * @hideinitializer
     */
    public static final String FIRMWARE_VERSION = "aace.alexa.system.firmwareVersion";

    /**
     * This property is used with Engine.setProperty() to change the current locale setting for AVS.
     * The value should be a valid locale accepted by AVS. Calling Engine.getProperty() with the
     * SUPPORTED_LOCALES property provides the list of supported locales.
     * @hideinitializer
     */
    public static final String LOCALE = "aace.alexa.setting.locale";

    /**
     * This read-only property is used with Engine.getProperty() to see all AVS supported locales. The return
     * value is a comma-separated list, e.g. "de-DE,en-AU,..."
     * @hideinitializer
     */
    public static final String SUPPORTED_LOCALES = "aace.alexa.supportedLocales";

    /**
     *
     * This read-only property is used with Engine.getProperty() to check if the vehicle's country is supported.
     * @hideinitializer
     */
    public static final String COUNTRY_SUPPORTED = "aace.alexa.countrySupported";
}

/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
     * This read-only property is used with
     * com.amazon.aace.propertyManager.PropertyManager.getProperty() to check
     * if the Engine has wake word support. If wake word is not supported in
     * the Engine, attempts to enable wake word detection by the
     * @c SpeechRecognizer will fail.
     * @hideinitializer
     */
    public static final String WAKEWORD_SUPPORTED = "aace.alexa.wakewordSupported";

    /**
     * This property is used with
     * com.amazon.aace.propertyManager.PropertyManager.setProperty() to change
     * the firmware version that is reported to AVS. The value must be a
     * positive, 32-bit signed integer represented as a string.
     * @hideinitializer
     */
    public static final String FIRMWARE_VERSION = "aace.alexa.system.firmwareVersion";

    /**
     * This property is used with
     * com.amazon.aace.propertyManager.PropertyManager.setProperty() to change
     * the current locale setting for Alexa. The value must be one of the
     * following:
     *      @li A valid locale accepted by AVS as a string. E.g. "en-US"
     *      @li A valid locale pair. The format is a string containing two valid
     *          locales separated with a forward slash. E.g. "en-US/es-US"
     * @hideinitializer
     */
    public static final String LOCALE = "aace.alexa.setting.locale";

    /**
     * This read-only property is used with
     * com.amazon.aace.propertyManager.PropertyManager.getProperty() to check
     * if the vehicle's country is supported.
     * @hideinitializer
     */
    public static final String COUNTRY_SUPPORTED = "aace.alexa.countrySupported";

    /**
     * This property is used with
     * com.amazon.aace.propertyManager.PropertyManager.setProperty() to change the
     * current timezone setting of the device. The value must be a valid timezone
     * accepted by AVS.
     * Calling com.amazon.aace.propertyManager.PropertyManager.getProperty()
     * with the TIMEZONE property provides the current timezone of the device.
     * @hideinitializer
     */
    public static final String TIMEZONE = "aace.alexa.timezone";

    /**
     * This property is used with
     * com.amazon.aace.propertyManager.PropertyManager.setProperty() to change
     * the current wake word enabled setting. The value must be a boolean
     * represented as a string, i.e. "true" or "false". Call
     * com.amazon.aace.propertyManager.PropertyManager.getProperty() with the
     * WAKEWORD_ENABLED property to check whether wake word is enabled.
     */
    public static final String WAKEWORD_ENABLED = "aace.alexa.wakewordEnabled";
}

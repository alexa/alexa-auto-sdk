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

package com.amazon.aace.alexa;

public class AlexaProperties {

    /**
     * Property used to change the AVS endpoint with Engine::setProperty(). The value should be
     * a valid AVS endpoint URL.
     */
    public static final String AVS_ENDPOINT = "aace.alexa.endpoint";

    /**
     * Property used to return if the engine has wakeword support with Engine::getProperty().
     * If wakeword is not supported in the engine, then attempts to enable wakeword detection in
     * the @c SpeechRecognizer will fail.
     */
    public static final String WAKEWORD_SUPPORTED = "aace.alexa.wakewordSupported";

    /**
     * Property used to change the firmware version that is reported to AVS when sending the
     * SoftwareInfo event.
     */
    public static final String FIRMWARE_VERSION = "aace.alexa.system.firmwareVersion";

    /**
     * Property used to change the current locale setting for AVS. The value should be a valid
     * locale accepted by AVS.
     */
    public static final String LOCALE = "aace.alexa.setting.locale";
}

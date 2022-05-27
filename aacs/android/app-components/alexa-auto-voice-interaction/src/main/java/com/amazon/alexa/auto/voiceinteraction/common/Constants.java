/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.voiceinteraction.common;

public class Constants {
    // Voice Interaction Topic
    public static final String TOPIC_VOICE_ANIMATION = "VOICE_ANIMATION";
    public static final String TOPIC_ALEXA_CONNECTION = "ALEXA_CONNECTION";

    // Voice Interaction Action
    public static final String ACTION_ALEXA_NOT_CONNECTED = "ALEXA_NOT_CONNECTED";
    public static final String ACTION_CUSTOM_PAYLOAD = "ACTION_CUSTOM_PAYLOAD";

    // AACS components
    public static final String AACS_PACKAGE = "com.amazon.alexaautoclientservice";
    public static final String AACS_CLASS = "com.amazon.alexaautoclientservice.AlexaAutoClientService";

    // AACS intents
    public static final String DIALOG_STATE_CHANGE = "com.amazon.aacs.aasb.DialogStateChanged";
    public static final String CONNECTION_STATUS_CONNECTED = "CONNECTED";
    public static final String CONNECTION_STATUS_DISCONNECTED = "DISCONNECTED";
}

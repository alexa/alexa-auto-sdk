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
package com.amazon.aacsconstants;

public class AssistantConstants {
    // Assistant ID for Alexa
    public static final int ASSISTANT_ID_ALEXA = 2;

    // No Assistant is specified
    public static final int ASSISTANT_ID_NONE = 0;

    // For all Assistants
    public static final int ASSISTANT_ID_ALL = 1;

    // Custom Assistant policy that means the 
    // custom assistant is local-only while Alexa is online-only.
    public static final String ON_DEVICE_POLICY = "ON_DEVICE";

    // Custom Assistant policy that means the 
    // custom assistant shares connection with Alexa.
    public static final String POWERED_BY_ALEXA_POLICY = "POWERED_BY_ALEXA";
}

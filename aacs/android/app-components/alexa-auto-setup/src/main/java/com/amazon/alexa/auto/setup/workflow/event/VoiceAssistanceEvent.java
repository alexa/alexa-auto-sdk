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
package com.amazon.alexa.auto.setup.workflow.event;

/**
 * Events that are Voice Assistance related.
 */
public class VoiceAssistanceEvent {
    public static final String VOICE_ASSISTANCE_INTRODUCTION_FINISHED = "Voice_Assistance_Introduction_Finished";
    public static final String WORK_TOGETHER_FINISHED = "Work_Together_Finished";
    public static final String ALEXA_ONLY = "Alexa_Only";
    public static final String NON_ALEXA_ONLY = "NonAlexa_Only";
    public static final String ALEXA_CBL_AUTH_FINISHED = "Alexa_CBL_Auth_Finished";
    public static final String NON_ALEXA_ENABLEMENT_FINISHED = "NonAlexa_Enablement_Finished";
    public static final String SEPARATE_ADDRESSBOOK_CONSENT_SETUP_NOT_FINISHED =
            "Separate_AddressBook_Consent_Setup_Not_Finished";
    public static final String SEPARATE_ADDRESSBOOK_CONSENT_SETUP_FINISHED =
            "Separate_AddressBook_Consent_Setup_Finished";
    public static final String ALEXA_ADDRESSBOOK_CONSENT_SETUP_FINISHED = "Alexa_AddressBook_Consent_Setup_Finished";
}
